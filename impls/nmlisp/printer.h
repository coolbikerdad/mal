#include "types.h"

char *pr_str(node *, int);
node *prn(node *);
node *println(node *);
node *pr_dash_str(node *);
node *str(node *);
char *print_to_string(node *, char *, int);

void printnode(node *);

typedef struct Writer {
    char *buffer;
    int position;
    int capacity;
} Writer;
