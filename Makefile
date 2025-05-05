# Autor: xdzuri00 (Adam Dzurilla)

CC=gcc
CFLAGS=-Wall -std=c11 -pedantic -lm
FILES=error.c compiler.c parser.c scanner.c symtable.c expression.c stack.c code_generator.c

FILES_TEST_LEXICAL_ANALYSIS=error.c ./tests/test_lexical_analysis.c scanner.c ./tests/test_utils.c code_generator.c
FILES_TEST_SYNTACTIC_ANALYSIS=error.c symtable.c ./tests/test_syntactic_analysis.c scanner.c stack.c parser.c expression.c ./tests/test_utils.c code_generator.c
FILES_TEST_SYMTABLE=error.c ./tests/test_symtable.c symtable.c
FILES_TEST_CODE_GENERATOR=code_generator.c ./tests/test_code_generator.c

.PHONY: test_lexical
test_lexical:
	$(CC) $(CFLAGS) -o $@  $(FILES_TEST_LEXICAL_ANALYSIS)

.PHONY: test_syntactic
test_syntactic:
	$(CC) $(CFLAGS) -o $@ $(FILES_TEST_SYNTACTIC_ANALYSIS)

.PHONY: test_symtable
test_symtable:
	$(CC) $(CFLAGS) -o $@ $(FILES_TEST_SYMTABLE)

.PHONY: test_code_generator
test_code_generator:
	$(CC) $(CFLAGS) -o $@ $(FILES_TEST_CODE_GENERATOR)


.PHONY: all
all:
	$(CC) $(CFLAGS) -o $@ $(FILES)


.PHONY: clean
clean:
	rm -f all
	rm -f test_lexical
	rm -f test_syntactic
	rm -f test_symtable
	rm -f test_code_generator
