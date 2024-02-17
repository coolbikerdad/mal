#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

char *READ(char *s)
{
	return s;
}

char *EVAL(char *s)
{
	return s;
}

char *PRINT(char *s)
{
	return s;
}

char *rep(char *s)
{
	char *t;
	t = READ(s);
	t = EVAL(t);
	t = PRINT(t);
	return t;
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

