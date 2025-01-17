#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gc.h>
#include "gc/leak_detector.h"
#include "reader.h"
#include "printer.h"
#include "exceptions.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

/* The token regular expression we want is:
    [\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*)
*/
#define TOKEN_RE "[\\s,]*(~@|[\\[\\]{}()'`~^@]|\"(?:\\\\.|[^\\\\\"])*\"?|;.*|\
[^\\s\\[\\]{}('\"`,;)]*)"
#define MAX_TOKEN_SIZE 1024
#define TOKEN_INCREMENT 1

/* Is a token a special form? */
int findspecial(char *s)
{
    char **t = special_forms;
    int n = 0;
    while(*t) {
        if(strcmp(s, *t) == 0)
            return n + NODE_SPECIAL_START;
        t++;
        n++;
    }
    return 0;
}

/* Break a string into tokens according to regular expression above */
char **tokenise(char *str)
{
    static pcre2_code *re = NULL;
    int errornumber;
    PCRE2_SIZE erroroffset;

    pcre2_match_data *match_data = NULL;
    int rc;
    PCRE2_SPTR subject = (PCRE2_SPTR)str;
    PCRE2_SIZE subject_length = (PCRE2_SIZE)strlen(str);
    PCRE2_SIZE subject_offset = 0;
    PCRE2_SIZE *ovector;
    int i;

    char token_buffer[MAX_TOKEN_SIZE+1];
    char **tokens = (char **)GC_MALLOC(TOKEN_INCREMENT*sizeof(char *));
    int token_count = 0;
    int token_capacity = TOKEN_INCREMENT;

    if(!re) {
        /* printf("compiling token regular expression\n"); */
        re = pcre2_compile(
            (PCRE2_SPTR)TOKEN_RE,  /* the pattern */
            PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
            0,                     /* default options */
            &errornumber,          /* for error number */
            &erroroffset,          /* for error offset */
            NULL);                 /* use default compile context */
    }

    if(re == NULL) {
        /* Compilation failed, should not happen */
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        printf("PCRE2 compilation failed at offset %d: %s\n", 
               (int)erroroffset, buffer);
        raise(SIGSEGV);
    }

    /* Allocate a block for match data */
    match_data = pcre2_match_data_create_from_pattern(re, NULL);

    /* Get matches */
    tokens[0] = NULL;
    for(;;) {
        rc = pcre2_match(
            re,                   /* the compiled pattern */
            subject,              /* the subject string */
            subject_length,       /* the length of the subject */
            subject_offset,       /* start at offset 0 in the subject */
            0,                    /* default options */
            match_data,           /* block for storing the result */
            NULL);                /* use default match context */

        if(rc < 0) {
            /* No match, should not happen - RE will match an empty string */
            printf("no match");
            pcre2_match_data_free(match_data);
            return NULL;
        }

        /* Found a match */
        ovector = pcre2_get_ovector_pointer(match_data);

        /* We will only get a single match due to the format of TOKEN_RE */
        /* Extract the token found */
        {
            int start = (int)ovector[2];
            int end = (int)ovector[3];
            if(start == end) {
                /* printf("null token. done.\n"); */
                break;
            }
            if(end - start > MAX_TOKEN_SIZE) {
                throw_exception("overlong token found", NULL, 1);
            }
            if(str[start] != ';') {
                /* Ignore comments*/
                strncpy(token_buffer,&str[start],end-start);
                token_buffer[end-start] = '\0';

                /* Store token in return list */
                if(token_count == token_capacity - 1) {
                    token_capacity += TOKEN_INCREMENT;
                    tokens = (char **)GC_REALLOC(tokens,
                                        token_capacity*sizeof(char *));
                }
                tokens[token_count] = 
                    (char *)GC_MALLOC(strlen(token_buffer));
                strcpy(tokens[token_count],token_buffer);
                token_count++;
            }
        }
        subject_offset = ovector[1];
    }
    pcre2_match_data_free(match_data);
    return tokens;
}

void free_tokens(char **tokens)
{
    char **t = tokens;
    if(!t)
        return;
    while(*t) {
        GC_FREE(*t);
        t++;
    }
    GC_FREE(tokens);    
}

void print_tokens(char **tokens)
{
    char **t = tokens;
    if(!t)
        return;
    while(*t) {
        printf("{%s}",*t);
        t++;
    }
    printf("\n");
    free_tokens(tokens);
}

char *inplace_dequote_string(char *s)
{
    int l = strlen(s);
    int r = 0, w = 0;
    char c;

    for(r = 0; r < l; r++, w++) {
        if(s[r] == '\\' && r == l-1) {
            throw_exception("unbalanced backslash", NULL, 1);
            return NULL;
        }
        if(s[r] == '\\' && r < l-1) {
            r++;
            c = s[r];
            switch(c) {
                case 'n':
                    s[w] = '\n';
                    break;
                case '"':
                    s[w] = '"';
                    break;
                case '\\':
                    s[w] = '\\';
                    break;
                default:
                    s[w] = c;
                    break;
            }
        }
        else
            s[w] = s[r];
    }
    s[w] = '\0';
    return s;
}

node *read_atom(Reader *r)
{
    char *t = reader_next(r);
    int i = 0;
    char c = t[0], c2 = t[1];
    node *n = NULL;
    int sf = 0;

    if((c == '-' && c2 >= '0' && c2 <= '9' )|| (c >= '0' && c <= '9')) {
        i = atoi(t);
        n = newnode(NODE_INT, NULL, NULL);
        n -> value.int_value = i;
        return n;
    }
    if(c == '"') {
        /* Make sure string is balanced */
        int l = strlen(t);
        if(l <= 1 || t[l-1] != '"') {
            throw_exception("unbalanced quote in string", NULL, 1);
            return NULL;
        }
        n = newnode(NODE_STRING, NULL, NULL);
        t[l-1] = '\0';
        t = inplace_dequote_string(t);
        if(!t)
            return NULL;
        n -> value.string_value = &t[1];
        return n;
    }
    if(c == ':') {
        int l = strlen(t);
        if(l == 1) {
            throw_exception("missing key", NULL, 1);
            return NULL;
        }
        n = newnode(NODE_KEY, NULL, NULL);
        n -> value.string_value = &t[1];
        return n;
    }

    /* Special nodes */
    if(strcmp(t,"nil") == 0)
        return newnode(NODE_NIL, NULL, NULL);
    if(strcmp(t,"true") == 0)
        return newnode(NODE_TRUE, NULL, NULL);
    if(strcmp(t,"false") == 0)
        return newnode(NODE_FALSE, NULL, NULL);

   /* Detect special forms */
    if((sf = findspecial(t))) {
        n = newnode(sf,NULL,NULL);
        return n;
    }
    else {
        n = newnode(NODE_SYMBOL, NULL, NULL);
        n -> value.string_value = t;
        return n;
    }
    return n;
}

node *read_list(Reader *r)
{
    node *list = NULL;
    node *n = NULL;
    node *tail = list;

    /* Next symbol should be a '(' */
    char *t = reader_next(r);
    if(t[0] != '(') {
        throw_exception("no open paren to startlist", NULL, 1);
        return NULL;
    }

    for(;;)
    {
        /* Look for a closing paren */
        char *p = reader_peek(r);
        if(!p) {
            throw_exception("end of input", NULL, 1);
            return NULL;
        }
        if(p[0] == ')') {
            /* End of list, we are done */
            (void) reader_next(r);
            if(!list)
                list = newnode(NODE_LIST,NULL,NULL);
            return list;
        }

        /* Get next list item */
        node *n = read_form(r);
        node *nn;

        if(!n)
            return NULL;

        if(!list) {
            list = newnode(NODE_LIST,n,NULL);
            tail = list;
        }
        else {
            nn = newnode(NODE_LIST,n,NULL);
            tail -> right = nn;
            tail = nn;
        }
    }
}

node *read_vector(Reader *r)
{
    node *list = NULL;
    node *n = NULL;
    node *tail = list;

    /* Next symbol should be a '(' */
    char *t = reader_next(r);
    if(t[0] != '[') {
        throw_exception("no open paren to start vector", NULL, 1);
        return NULL;
    }

    for(;;)
    {
        /* Look for a closing paren */
        char *p = reader_peek(r);
        if(!p) {
            throw_exception("end of input", NULL, 1);
            return NULL;
        }
        if(p[0] == ']') {
            /* End of list, we are done */
            (void) reader_next(r);
            if(!list)
                list = newnode(NODE_VEC,NULL,NULL);
            return list;
        }

        /* Get next list item */
        node *n = read_form(r);
        node *nn;

        if(!n)
            return NULL;

        if(!list) {
            list = newnode(NODE_VEC,n,NULL);
            tail = list;
        }
        else {
            nn = newnode(NODE_VEC,n,NULL);
            tail -> right = nn;
            tail = nn;
        }
    }
}

node *read_hashmap(Reader *r)
{
    node *list = NULL;
    node *n = NULL;
    node *tail = list;

    /* Next symbol should be a '{' */
    char *t = reader_next(r);
    if(t[0] != '{') {
        throw_exception("not an open paren to start hashmap", NULL, 1);
        return NULL;
    }

    for(;;)
    {
        /* Look for a closing paren */
        char *p = reader_peek(r);
        if(!p) {
            throw_exception("end of input", NULL, 1);
            return NULL;
        }
        if(p[0] == '}') {
            /* End of list, we are done */
            (void) reader_next(r);
            if(!list)
                list = newnode(NODE_HASH,NULL,NULL);
            return list;
        }

        /* Get next list item */
        node *n = read_form(r);
        node *nn;

        if(!n)
            return NULL;

        if(!list) {
            list = newnode(NODE_HASH,n,NULL);
            tail = list;
        }
        else {
            nn = newnode(NODE_HASH,n,NULL);
            tail -> right = nn;
            tail = nn;
        }
    }
}

node *read_form(Reader *r)
{
    char *t = reader_peek(r);
    node *n = NULL, *n2 = NULL;

    if(!t)
        return NULL;
    
    if(t[0] == '(')
        return read_list(r);
    if(t[0] == '[')
        return read_vector(r);    
    if(t[0] == '{')
        return read_hashmap(r);
    if(t[0] == '\'') {
        (void) reader_next(r);
        n = read_form(r);
        return newnode(NODE_LIST,
                       newnode(NODE_QUOTE,NULL,NULL),
                       newnode(NODE_LIST,n,NULL));
    }
    if(t[0] == '`') {
        (void) reader_next(r);
        n = read_form(r);
        return newnode(NODE_LIST,
                       newnode(NODE_QQUOTE,NULL,NULL),
                       newnode(NODE_LIST,n,NULL));
    }
    if(t[0] == '~' && t[1] == '@') {
        (void) reader_next(r);
        n = read_form(r);
        return newnode(NODE_LIST,
                       newnode(NODE_SUQUOTE,NULL,NULL),
                       newnode(NODE_LIST,n,NULL));
    }
    if(t[0] == '~') {
        (void) reader_next(r);
        n = read_form(r);
        return newnode(NODE_LIST,
                       newnode(NODE_UQUOTE,NULL,NULL),
                       newnode(NODE_LIST,n,NULL));
    }   
    if(t[0] == '@') {
        (void) reader_next(r);
        n = read_form(r);
        return newnode(NODE_LIST,newsymbol("deref"),
                       newnode(NODE_LIST,n,NULL));
    }   
    if(t[0] == '^') {
        (void) reader_next(r);
        n = read_form(r);
        n2 = read_form(r);
        return newnode(NODE_LIST,
                       newnode(NODE_META,NULL,NULL),
                       newnode(NODE_LIST,n2,newnode(NODE_LIST,n,NULL)));
    }   
    return read_atom(r);
}

node *read_str(char *str)
{
    Reader *r =(Reader *)GC_MALLOC(sizeof(Reader));
    r -> position = 0;
    r -> tokens = tokenise(str);
    return read_form(r);
}

node *read_dash_string(node *t)
{
    if(!t || t -> type != NODE_LIST || !t -> left || 
        t -> left -> type != NODE_STRING) {
        throw_exception("reading non-string", t, 1);
        return NULL;
    }
    return read_str(t -> left -> value.string_value);
}

node *slurp(node *t)
{
    /* Read a file */
    char *file, *buffer;
    FILE *fd;
    struct stat buf;
    int sz;
    node *a;

    if(!t || t -> type != NODE_LIST || !t -> left || 
        t -> left -> type != NODE_STRING) {
        throw_exception("slurping non-string file name", t, 1);
        return NULL;
    }
    file = t -> left -> value.string_value;
    if((fd = fopen(file, "r")) == NULL) {
        throw_exception("file not found", t -> left, 1);
        return NULL;
    }
    fstat(fileno(fd), &buf);
    sz = buf.st_size;

    buffer = GC_MALLOC(sz+1);
    fread(buffer, 1, sz, fd);
    buffer[sz] = '\0';

    a = newnode(NODE_STRING, NULL, NULL);
    a -> value.string_value = buffer;

    return a;
}

char *reader_peek(Reader *r)
{
    return r -> tokens[r -> position];
}

char *reader_next(Reader *r)
{
    char *t = r -> tokens[r -> position];
    r -> position++;
    return t;
}

void print_node(node *n) {
    if(!n)
        return;
    printf("Node %s:\n", node_types[n -> type]);
    switch(n -> type) {
        case NODE_INT:
            printf("%d", n -> value.int_value);
            break;
        case NODE_SYMBOL:
            printf("%s", n -> value.string_value);
            break;
        case NODE_LIST:
            printf("list:\n");
            print_node(n -> left);
            print_node(n -> right);
            break;
        default:
            printf("???");
    }
    printf("\n");
}

int reader_main()
{
    node *n;
    /* GC_set_find_leak(1); */
    GC_INIT();
    /*
    printf("TOKEN_RE is %s\n", TOKEN_RE);
    print_tokens(tokenise("    123   "));
    print_tokens(tokenise(" ( + \"fred\" (*  333 ;4)  )"));
    print_tokens(tokenise(""));
    */
    n = read_str(" ( + \"fred\" (*  333 4)  )");
    printf("%s\n",pr_str(n,0));

    GC_gcollect();
    return 0;
}
