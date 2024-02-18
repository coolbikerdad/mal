
CC = gcc
CFLAGS = -g
LIBS = -lgc -lreadline -lpcre2-8

S0_SRC = step0_repl.c
S1_SRC = step1_read_print.c reader.c types.c printer.c
S2_SRC = step2_eval.c reader.c types.c printer.c functions.c

S0_HEADERS = 
S1_HEADERS = reader.h types.h printer.h
S2_HEADERS = reader.h types.h printer.h functions.h

S0 = step0_repl
S1 = step1_read_print
S2 = step2_eval

all: $(S0) $(S1) $(S2)

$(S0): $(S0_SRC) $(S0_HEADERS)
	$(CC) $(CFLAGS) $(S0_SRC) $(LIBS) -o $(S0)

$(S1): $(S1_SRC) $(S1_HEADERS)
	$(CC) $(CFLAGS) $(S1_SRC) $(LIBS) -o $(S1)

$(S2): $(S2_SRC) $(S2_HEADERS)
	$(CC) $(CFLAGS) $(S2_SRC) $(LIBS) -o $(S2)


