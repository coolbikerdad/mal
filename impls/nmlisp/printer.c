#include "types.h"
#include <stdio.h>
#include <string.h>
#include <gc.h>
#include "printer.h"

/* Print a node tree */
void printnode(node *tree)
{
    node *t = tree;

    if(!tree)
        return;

    switch(tree -> type) {
        case NODE_INT:
            printf("%d", tree -> value.int_value);
            break;
        case NODE_SYMBOL:
            printf("%s", tree -> value.string_value);
            break;
        case NODE_NIL:
            printf("nil");
            break;
        case NODE_LAMBDA:
        case NODE_FNSTAR:
            printf("fn*");
            break;
        case NODE_STRING:
            printf("\"%s\"", tree -> value.string_value);
            break;
        case NODE_LIST:
            printf("(");
            while(t && t -> type == NODE_LIST) {
                printnode(t -> left);
                t = t-> right;
                printf(" ");
            }
            printf("<-)");
            break;
        case NODE_VEC:
            printf("[");
            while(t && t -> type == NODE_VEC) {
                printnode(t -> left);
                t = t-> right;
                printf(" ");
            }
            printf("<-]");
            break; 
        case NODE_QUOTE:
        case NODE_QQUOTE:
        case NODE_SUQUOTE:
            printf("%s", node_types[t -> type - NODE_SPECIAL_START]);
            break;
    }
}

/* String reprsentation of a node */
#define WRITER_INCREMENT 20

/* Ensure Writer has capacity */
void writer_capacity(Writer *w, int n)
{
    if(n + w -> position >= w -> capacity) {
        /* printf("increasing Writer capacity\n"); */

        w -> capacity += WRITER_INCREMENT;
        w -> buffer = GC_REALLOC(w -> buffer, w -> capacity + 1);
    }
}

/* Backup a Writer - e.g. after last item in a list */
void writer_backspace(Writer *w)
{
    if(w -> capacity) {
        w -> position--;
        w -> buffer[w -> position] = '\0';
    }
}

/* Add a character to a Writer */
void writer_putc(Writer *w, char c)
{
    writer_capacity(w, 1);
    w -> buffer[w -> position] = c;
    w -> position++;
    w -> buffer[w -> position] = '\0';
}

/* Add a string to a Writer */
void writer_puts(Writer *w, char * s)
{
    int n = strlen(s);
    for(int i = 0; i < n; i++)
        writer_putc(w, s[i]);
}

/* Add an integer to a Writer */
void writer_puti(Writer *w, int i) 
{
    char buffer[20]; /* Big enough for an integer representation */
    sprintf(buffer,"%d",i);
    writer_puts(w,buffer);
}

void enquote_string(char *d, char *s)
{
    int l = strlen(s);
    int r = 0, w = 0;

    for(r = 0; r < l; r++,w++) {
        switch(s[r]) {
            case '\n':
                d[w] = '\\';
                d[++w] = 'n';
                break;
            case '\\':
                d[w] = '\\';
                d[++w] = '\\';
                break;
            case '"':
                d[w] = '\\';
                d[++w] = '"';  
                break;
            default:
                d[w] = s[r];
                break;   
        }
    }
    d[w] = '\0';
}

void sprintnode(Writer *w, node *tree, int readably)
{
    node *t = tree;

    if(!tree)
        return; 

    switch(tree -> type) {
        case NODE_INT:
            writer_puti(w, tree -> value.int_value);
            break;
        case NODE_SYMBOL:
            writer_puts(w, tree -> value.string_value);
            break;
         case NODE_KEY:
            writer_putc(w, ':');
            writer_puts(w, tree -> value.string_value);
            break;
        case NODE_NIL:
            writer_puts(w,"nil");
            break;
        case NODE_FNSTAR:
            writer_puts(w,"fn*");
            break;
        case NODE_TRUE:
            writer_puts(w,"true");
            break;
        case NODE_FALSE:
            writer_puts(w,"false");
            break;
        case NODE_STRING:
            if(readably) {
                char *s = GC_MALLOC(2*strlen(tree -> value.string_value));
                writer_putc(w, '"');
                enquote_string(s, tree -> value.string_value);
                writer_puts(w,s);
                writer_putc(w, '"');
                GC_FREE(s);
            }
            else
                writer_puts(w, tree -> value.string_value);
            break;
        case NODE_LIST:
            writer_putc(w,'(');
            while(t && t -> type == NODE_LIST) {
                sprintnode(w, t -> left, readably);
                t = t -> right;
                writer_putc(w, ' ');
            }
            writer_backspace(w);
            writer_putc(w,')');
            break;
        case NODE_VEC:
            writer_putc(w,'[');
            while(t && t -> type == NODE_VEC) {
                sprintnode(w, t -> left, readably);
                t = t -> right;
                writer_putc(w, ' ');
            }
            writer_backspace(w);
            writer_putc(w,']');
            break;
         case NODE_HASH:
            writer_putc(w,'{');
            while(t && t -> type == NODE_HASH) {
                sprintnode(w, t -> left, readably);
                t = t -> right;
                writer_putc(w, ' ');
            }
            writer_backspace(w);
            writer_putc(w,'}');
            break;
        case NODE_QUOTE:
        case NODE_QQUOTE:
        case NODE_UQUOTE:
        case NODE_SUQUOTE:
        case NODE_META:
        case NODE_DEFBANG:
        case NODE_DEFMACRO:
        case NODE_MACROEXPAND:
        case NODE_LETSTAR:
        case NODE_TRY:
        case NODE_CATCH:
            writer_puts(w,special_forms[tree -> type - NODE_SPECIAL_START]);
            break;
        case NODE_FUNC:
            writer_puts(w,"#builtin");
            break;
        case NODE_IF:
            writer_puts(w,"if");
            break;
        case NODE_LAMBDA:
            writer_puts(w,"(lambda ");
            sprintnode(w, t -> left, readably);
            sprintnode(w, t -> right, readably);
            writer_puts(w,")"); 
            break;
        case NODE_MACRO:
            writer_puts(w,"(macro ");
            sprintnode(w, t -> left, readably);
            sprintnode(w, t -> right, readably);
            writer_puts(w,")");
            break;
        case NODE_ATOM:
            writer_puts(w,"(atom ");
            sprintnode(w, t -> left, readably);
            writer_puts(w,")");
            break;
    }
}

char *pr_str(node *tree, int readably)
{
    Writer *w = GC_MALLOC(sizeof(Writer));
    char *output = NULL;
    
    writer_capacity(w,1);
    sprintnode(w, tree, readably);
    output = w -> buffer;
    w -> buffer = NULL;
    GC_FREE(w);
    return output;
}

char *print_to_string(node *tree, char *sep, int readably)
{
    Writer *w = GC_MALLOC(sizeof(Writer));
    char *output = NULL;
    node *t = tree;
    int b = strlen(sep);
    int i = 0;

    writer_capacity(w,1);
    while(t) {
        sprintnode(w, t -> left, readably);
        t = t -> right;
        writer_puts(w,sep);
        i++;
    }
    /* Backspace last seperator */
    if(i && b) {
        for(i = 0; i < b; i++)
            writer_backspace(w);
    }
    output = w -> buffer;
    w -> buffer = NULL;
    GC_FREE(w);
    return output;
}

node *prn(node *tree)
{
    char *s = print_to_string(tree, " ", 1);
    printf("%s\n", s);
    return newnode(NODE_NIL, NULL, NULL);
}

node *pr_dash_str(node *tree) 
{
    char *s = print_to_string(tree, " ", 1);
    node *r = newnode(NODE_STRING, NULL, NULL);
    r -> value.string_value = s;
    return r;
}

node *str(node *tree) 
{
    char *s = print_to_string(tree, "", 0);
    node *r = newnode(NODE_STRING, NULL, NULL);
    r -> value.string_value = s;
    return r;
}

node *println(node *tree)
{
    char *s = print_to_string(tree, " ", 0);
    printf("%s\n", s);
    return newnode(NODE_NIL, NULL, NULL);    
}

