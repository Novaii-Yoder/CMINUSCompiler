###

### Lab 9 CS370
### Compiles lab9 on dependent yacc and lex files
### creates a compiler for the CMINUS+ language
### can also run the code with a test file
## Mar 22, 2023
## Jacob Yoder

# all dependent files make lab6
all:	lab9

# lab9 depends on lex, yacc, and ast.c source files
lab9:	lab9.l lab9.y ast.c ast.h symtable.h symtable.c emit.h emit.c
	lex lab9.l
	yacc -d lab9.y
	gcc lex.yy.c y.tab.c ast.c symtable.c emit.c -o lab9

# runs lab9 with the test file
run:	lab9
	./lab9 -o foo < test.c

# removes all files created by make
clean:
	rm -f lab9
	rm lex.yy.c
	rm y.tab.c
	rm y.tab.h
