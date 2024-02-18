#include "types.h"

char *pr_str(node *, int);
void printnode(node *);

typedef struct Writer {
    char *buffer;
    int position;
    int capacity;
} Writer;

