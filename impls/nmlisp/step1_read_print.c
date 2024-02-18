#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "types.h"
#include "reader.h"
#include "printer.h"

node *READ(char *s)
{
	return read_str(s);
}

node *EVAL(node *s)
{
	return s;
}

char *PRINT(node *t)
{
	return pr_str(t,1);
}

char *rep(char *s)
{
	node *t;
	char *o;
	t = READ(s);
	t = EVAL(t);
	o = PRINT(t);
	return o;
}

int main()
{
	static char *line_read = (char *)NULL;
	char *result;

	while(1) {
	  	/* If the buffer has already been allocated,
     			return the memory to the free pool. */
  		if(line_read) {
			free(line_read);
      			line_read = (char *)NULL;
    		}
		line_read = readline("user> ");
		/* If the line has any text in it,
     			save it on the history. */
  		if(line_read && *line_read)
    			add_history(line_read);
		if(line_read) {
			result = rep(line_read);
			printf("%s\n",result);
		}	
		else {
			printf("\n");
			break;
		}
	}
}

