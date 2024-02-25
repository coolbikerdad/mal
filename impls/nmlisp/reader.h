/*
Reader Functions
*/

#include "types.h"

char **tokenise(char *);
void free_tokens(char **tokens);

typedef struct Reader {
    int position;
    char **tokens;
} Reader;

char *reader_peek(Reader *);
char *reader_next(Reader *);
node *read_form(Reader *);
node *read_atom(Reader *);
node *read_list(Reader *);
node *read_str(char *);
node *read_dash_string(node *);
node *slurp(node *)
;
