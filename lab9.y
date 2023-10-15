%{

/*
	lab9.y (Yacc code)
	Lab9
	Jacob Yoder
	5/5/2023
	
	Sytax analysis of the tokens returned by lex, based on the CMINUS+ language.

	- Changed the A_ASSIGN, to create a temp symbol for use with the MIPS code
	- Changed the A_CALL, to not create a temp symbol, becuase it should be done 
		in only A_ARGS, which is also now does
	- Added continue and break statements
	- Added global var GINWHILE for checking whether continues or breaks are in 
		while statements
	
*/


	/* begin specs */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "symtable.h"
#include "emit.h"

ASTnode *PROGRAM = NULL;

//#include "lex.yy.c"

int yylex(); //added protype

extern int lineno;
extern int mydebug;

int LEVEL = 0; // global variable for keeping track of depth
int OFFSET = 0; // global variable for accumulation of needed runtime space
int GOFFSET = 0; // global variable for accumulation of global variable offset
int MAXOFFSET = 0; // largest offset needed for the current function
int GINWHILE = 0; // global boolean for if we are in a while
#define MAX 3

int regs[MAX];
int addr = 0; // address offset for keeping track of where to insert the symbol into the table
int base, debugsw;

void yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  printf ("Line %d: %s\n",lineno, s);
}


%}
/*  defines the start symbol, what values come back from LEX and how the operators are associated  */
%union {
	int value;
	char* string;
	ASTnode * node;
	enum AST_MY_DATA_TYPE input_type;
	enum AST_OPERATORS operator;
}


%start Program

%token <value> T_NUM
%token <string> T_ID
%token <string> T_STRING
%token T_INT T_VOID T_READ T_WRITE T_RETURN T_WHILE T_IF T_ELSE
%token T_LE T_LT T_GT T_GE T_EQ T_NE T_CONTINUE T_BREAK

%type <node> Var_List Var_Declaration Declaration Declaration_List Fun_Declaration
%type <node> Params Compound_Stmt Local_Declaration Statement_List Statement
%type <node> Write_Stmt Read_Stmt Expression Simple_Expression Additive_Expression
%type <node> Term Factor Var Call Assignment_Stmt Iteration_Stmt Selection_Stmt
%type <node> Return_Stmt Continue_Stmt Break_Stmt Expression_Stmt Param Param_List 
%type <node> Args Arg_List

%type <input_type> Type_Specifier
%type <operator> Addop Multop Relop

%left '|'
%left '&'
%left '+' '-'
%left '*' '/' '%'
%left UMINUS


%%	/* end specs, begin rules */

Program 		:	Declaration_List{ PROGRAM = $1;}
			;

Declaration_List	:	Declaration{ $$ = $1;}
        		| 	Declaration Declaration_List{
					$$ = $1;
					$$->next = $2;
				}
			;

Declaration		:	Var_Declaration{ $$ = $1;}
	     		|	Fun_Declaration{ $$ = $1;}
	     		;

Var_Declaration		:	Type_Specifier Var_List ';'{
		 			// add type to all elements in the list
					ASTnode *p;
					p = $2;
					while (p != NULL) { // loop through linked list
						p->my_data_type = $1;

					// check each variable in the list to see if
					// it has already been defined at our level
						if (Search(p->name, LEVEL,0) != NULL){
							yyerror(p->name);
							yyerror("Symbol already defined");
							exit(1);
						} // end if
					// Symbol not already in table
						if (p->value == 0){ // we have a scalar
							p->symbol = Insert(p->name, p->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
							OFFSET = OFFSET + 1;
						}else { // we have an array
							p->symbol = Insert(p->name, p->my_data_type, SYM_ARRAY, LEVEL, p->value, OFFSET);
							OFFSET = OFFSET + p->value;
						}// end of if else
						
						p = p->s1; 

					} // end while
					$$ = $2;
				}
	     		;

Type_Specifier		:	T_INT{ $$ = A_INTTYPE;}
			|	T_VOID{ $$ = A_VOIDTYPE;}
			;

Var_List		:	T_ID{
					$$ = ASTCreateNode(A_VARDEC);
					$$->name = $1;
				}
			|	T_ID '[' T_NUM ']'{
					$$ = ASTCreateNode(A_VARDEC);
					$$->name = $1;
					$$->value = $3;
				}
	  		|	T_ID ',' Var_List{
					$$ = ASTCreateNode(A_VARDEC);
					$$->name = $1;
					$$->s1 = $3;
				}
			|	T_ID '[' T_NUM ']' ',' Var_List{
					$$ = ASTCreateNode(A_VARDEC);
					$$->name = $1;
					$$->value = $3;
					$$->s1 = $6;
				}
	  		;

Fun_Declaration		:	Type_Specifier T_ID{
					if(Search($2, LEVEL, 0) != NULL){
					//ID has already been defined
						yyerror($2);
						yyerror("Function name already in use");
						exit(1);
					}
				// else not in symbol table, insert it
					Insert($2, $1, SYM_FUNCTION, LEVEL, 1, 0);
					GOFFSET = OFFSET;
					OFFSET = 2;
					MAXOFFSET = OFFSET;
				}
				'(' Params ')'{
					// sets the formal params of the function
					if($5->value == 2){
						Search($2, LEVEL, 0)->fparms = NULL;
					} else{
					Search($2, LEVEL, 0)->fparms = $5;
					} // end of elseif
				}
				Compound_Stmt{
					$$ = ASTCreateNode(A_FUNCTIONDEC);
					$$->my_data_type = $1;
					$$->name = $2;
					$$->s1 = $5;
					$$->s2 = $8;
					
					$$->symbol = Search($$->name, LEVEL, 0);
					$$->symbol->offset = MAXOFFSET;
					OFFSET = GOFFSET;
					// guaranteed to find, because we inserted it above
				}
		 	;

Params			:	T_VOID{
	 				$$ = ASTCreateNode(A_PARAM);
					$$->my_data_type = A_VOIDTYPE;
					$$->value = 2; 
					// value is used by ast.c for differentiating void params
				}
	 		|	Param_List{ $$ = $1;}
			;

Param_List		:	/* empty */{ $$ = NULL;}
	    		|	Param{ $$ = $1;}
	    		|	Param ',' Param_List{
					$$ = $1;
					$$->next = $3;
				}
			;

Param			:	Type_Specifier T_ID{
       					struct SymbTab *p;
					p = Search($2, LEVEL + 1, 0);
					if(p != NULL){
						yyerror($2);
						yyerror("Parameter already defined.");
						exit(1);
					}
					// Parameter has not been defined, so insert
					$$ = ASTCreateNode(A_PARAM);
					$$->my_data_type = $1;
					$$->name = $2;
					$$->symbol = Insert($2, $1, SYM_SCALAR, LEVEL + 1, 1, OFFSET);
					OFFSET = OFFSET + 1;
				}
			|	Type_Specifier T_ID '[' ']'{
					if(Search($2, LEVEL + 1, 0)){
						yyerror($2);
						yyerror("Parameter already defined.");
						exit(1);
					}
					// Parameter has not been defined, so insert

					$$ = ASTCreateNode(A_PARAM);
					$$->my_data_type = $1;
					$$->name = $2;
					$$->value = 1; // used to differentiate arrays in ast.c
					
					$$->symbol = Insert($2, $1, SYM_ARRAY, LEVEL + 1, 1, OFFSET);
					OFFSET = OFFSET + 1;
				}
			;

Compound_Stmt		:	'{' { LEVEL++; }
	       			Local_Declaration Statement_List '}'{
					$$ = ASTCreateNode(A_COMPOUND);
					$$->s1 = $3;
					$$->s2 = $4;
					if(mydebug) {Display();}
					// set MAXOFFSET 
					if(OFFSET > MAXOFFSET){ MAXOFFSET = OFFSET;}
					OFFSET -= Delete(LEVEL); // removes symbols on this level
					LEVEL--; // lowers level
				}
			;

Local_Declaration	:	/* empty */{ $$ = NULL;}
		   	|	Var_Declaration Local_Declaration{
					$$ = $1;
					$$->next = $2;
				}
			;

Statement_List		:	/* empty */{ $$ = NULL;}
			|	Statement Statement_List{
					$$ = $1;
					$$->next = $2;
				}
			;

Statement		:	Expression_Stmt{$$ = $1;}
	   		|	Compound_Stmt{$$ = $1;}
			|	Selection_Stmt{$$ = $1;}
			|	Iteration_Stmt{$$ = $1;}
			|	Assignment_Stmt{$$ = $1;}
			|	Read_Stmt{$$ = $1;}
			|	Write_Stmt{$$ = $1;}
			|	Return_Stmt{$$ = $1;}
			|	Continue_Stmt{$$ = $1;}
			|	Break_Stmt{$$ = $1;}
	   		;

Selection_Stmt		:	T_IF '(' Expression ')' Statement{
					$$ = ASTCreateNode(A_IF);
					$$->s1 = $3;
					$$->s2 = $5;
				}
			|	T_IF '(' Expression ')' Statement T_ELSE Statement{
					$$ = ASTCreateNode(A_IFELSE);
					// creates an 'if' node as the s1
					$$->s1 = ASTCreateNode(A_IF);
					$$->s1->s1 = $3;
					$$->s1->s2 = $5;
					$$->s2 = $7;
				}
			;

Iteration_Stmt		:	T_WHILE '(' Expression ')'
				{GINWHILE++;} 
		
				Statement{
					$$ = ASTCreateNode(A_WHILE);
					$$->s1 = $3;
					$$->s2 = $6;
					GINWHILE--;
				}
			;

Assignment_Stmt		:	Var '=' Simple_Expression ';'{
		 			// type checking between the Var and expression
		 			if($1->my_data_type != $3->my_data_type){
						yyerror("Type mismatch: Assignment Statment");
						exit(1);
					}
					$$ = ASTCreateNode(A_ASSIGN);
					$$->s1 = $1;
					$$->s2 = $3;
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					// sythesized data type
					OFFSET = OFFSET + 1;
				}
		 	;

Read_Stmt		:	T_READ Var ';'{
					$$ = ASTCreateNode(A_READ);
					$$->s1 = $2;
				}
	   		;

Write_Stmt		:	T_WRITE Expression ';'{
					$$ = ASTCreateNode(A_WRITE);
					$$->s1 = $2;
				}
	    		|	T_WRITE T_STRING ';'{
					$$ = ASTCreateNode(A_WRITE);
					$$->name = $2;
				}
	    		;

Return_Stmt		:	T_RETURN ';'{
					$$ = ASTCreateNode(A_RETURN);
				}
	     		|	T_RETURN Expression ';'{
					$$ = ASTCreateNode(A_RETURN);
					$$->s1 = $2;
				}
			;

Continue_Stmt		:	T_CONTINUE ';'{
	       				if(GINWHILE == 0){
						yyerror("Cannot use CONTINUE unless inside a while");
						exit(1);
					}
					$$ = ASTCreateNode(A_CONTINUE);
				}
			;

Break_Stmt		:	T_BREAK ';'{
	       				if(GINWHILE == 0){
						yyerror("Cannot use BREAK unless inside a while");
						exit(1);
					}
					$$ = ASTCreateNode(A_BREAK);
				}
			;

Expression_Stmt		:	Expression ';'{
		 			$$ = ASTCreateNode(A_EXPRSTMT);
					$$->s1 =  $1;
				}
		 	|	';'{
					$$ = ASTCreateNode(A_EXPRSTMT);
				}
			;

Var			:	T_ID{
      					struct SymbTab *p;
					p = Search($1, LEVEL, 1); // finds ID in table
					if(p == NULL) {
						// reference not in symbol table
						yyerror($1);
						yyerror("Symbol used but not defined.");
						exit(1);
					} 
					if(p->SubType != SYM_SCALAR){
						// reference is in symbol table but not scalar
						yyerror($1);
						yyerror("Symbol must be a SCALAR");
						exit(1);
					}
					$$ = ASTCreateNode(A_VAR);
					$$->name = $1;
					$$->symbol = p;
					// passing data type up
					$$->my_data_type = p->Declared_Type;
				}
      			|	T_ID '[' Expression ']'{
      					struct SymbTab *p;
					p = Search($1, LEVEL, 1);
					if(p == NULL) {
						// reference not in symbol table
						yyerror($1);
						yyerror("Symbol used but not defined.");
						exit(1);
					} 
					if(p->SubType != SYM_ARRAY){
						// reference is in symbol table but not array
						yyerror($1);
						yyerror("Symbol must be a ARRAY");
						exit(1);
					}
					$$ = ASTCreateNode(A_VAR);
					$$->name = $1;
					$$->s1 = $3;
					$$->symbol = p;
					// passing data type up
					$$->my_data_type = p->Declared_Type;
				}
			;

Expression		:	Simple_Expression{ $$ = $1;}
	    		;

Simple_Expression	:	Additive_Expression{ $$ = $1;}
		  	|	Additive_Expression Relop Additive_Expression{
					// type checking for Additive Expressions
					if($1->my_data_type != $3->my_data_type){
						yyerror("Type mismatch: Simple_Expression");
						exit(1);
					}
					$$ = ASTCreateNode(A_EXPR);
					$$->s1 = $1;
					$$->s2 = $3;
					$$->operator = $2;
					
					// create temp symbol to allocate memory
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					// sythesized data type
					$$->my_data_type = $1->my_data_type;
					OFFSET = OFFSET + 1;
				}
			;

Relop			:	T_LT{ $$ = A_LT;}
			|	T_LE{ $$ = A_LE;}
			|	T_EQ{ $$ = A_EQ;}
			|	T_NE{ $$ = A_NE;}
			|	T_GT{ $$ = A_GT;}
			|	T_GE{ $$ = A_GE;}
			;

Additive_Expression	:	Term{ $$ = $1;}
		    	|	Additive_Expression Addop Term{
					// type checking between Additive and Term
					if($1->my_data_type != $3->my_data_type){
						yyerror("Type mismatch: Additive_Expression");
						exit(1);
					}
					$$ = ASTCreateNode(A_EXPR);
					$$->s1 = $1;
					$$->s2 = $3;
					$$->operator = $2;
					// create temp symbol to allocate memory
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $1->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					// synthesized data type
					$$->my_data_type = $1->my_data_type;
					OFFSET = OFFSET + 1;
				}
			;

Addop			:	'+'{ $$ = A_PLUS;}
			|	'-'{ $$ = A_MINUS;}
			;

Term			:	Factor{ $$ = $1;}
       			|	Term Multop Factor{
					// type checking between Term and Factor
					if($1->my_data_type != $3->my_data_type){
						yyerror("Type mismatch: Term");
						exit(1);
					}
					$$ = ASTCreateNode(A_EXPR);
					$$->s1 = $1;
					$$->s2 = $3;
					$$->operator = $2;
					// synthesized type
					$$->my_data_type = $1->my_data_type;
					// create temp symbol to allocate memory
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $$->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					OFFSET = OFFSET + 1;
				}
			;

Multop			:	'*'{ $$ = A_MULT;}
	 		|	'/'{ $$ = A_DIV;}
			|	'%'{ $$ = A_MOD;}
			;

Factor			:	'(' Expression ')'{ $$ = $2;}
	 		|	T_NUM{
					$$ = ASTCreateNode(A_NUM);
					$$->value = $1;
					$$->my_data_type = A_INTTYPE;
				}
			|	Var{ $$ = $1;}
			|	Call{ $$ = $1;}
			|	'-' Factor{
					if($2->type != A_NUM){
						if($2->symbol->Declared_Type != A_INTTYPE){
							yyerror("Type mismatch: Unary-Minus");
							exit(1);
						}
					} // end of if to see if Unary-Minus has correct input
					$$ = ASTCreateNode(A_EXPR);
					$$->s1 = $2;
					$$->operator = A_UMINUS;
					$$->my_data_type = A_INTTYPE;
					// create temp symbol to allocate memory
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $$->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					OFFSET = OFFSET + 1;
				}
			;

Call			:	T_ID '(' Args ')'{
       					struct SymbTab *p;
					p = Search($1, 0, 0);
					// check to see if symbol is in the table
					if(p == NULL){
						// function has no been defined
						yyerror($1);
						yyerror("Function name is not defined");
						exit(1);
					}
					// symbol is there but is it NOT a function
					if(p->SubType != SYM_FUNCTION){
						yyerror($1);
						yyerror("Symbol is not defined as a function");
						exit(1);
					}
					// check to see if formal and actual parameters are the same
					// length and type
					if(check_params($3, p->fparms) == 0){
						yyerror("Actual and Formal params do not match");
						exit(1);
					}
       					$$ = ASTCreateNode(A_CALL);
					$$->name = $1;
					$$->s1 = $3;
					$$->symbol = p;
					$$->my_data_type = p->Declared_Type; 
					
				}
			;

Args			:	Arg_List{ $$ = $1;}
       			|	/* empty */{ $$ = NULL;} 
			;

Arg_List		:	Expression{
					$$ = ASTCreateNode(A_ARG);
					$$->s1 = $1;
					$$->my_data_type = $1->my_data_type;
					// create temp symbol to allocate memory
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $$->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					OFFSET = OFFSET + 1;
				}
	  		|	Expression ',' Arg_List{
					$$ = ASTCreateNode(A_ARG);
					$$->s1 = $1;
					$$->next = $3;
					$$->my_data_type = $1->my_data_type;
					// create temp symbol to allocate memory
					$$->name = CreateTemp();
					$$->symbol = Insert($$->name, $$->my_data_type, SYM_SCALAR, LEVEL, 1, OFFSET);
					OFFSET = OFFSET + 1;
				}
			;

%%	/* end of rules, start of program */

void main(int argc, char* argv[])
{ 
	FILE *fp;
        char s[100];
	// read input arguments
	// option -d turn on debug
	// option -o next argument is our outut file name
	for(int i = 0; i < argc; i++){
		if (strcmp(argv[i],"-d") == 0) { mydebug = 1;}
		if (strcmp(argv[i],"-o") == 0) {
			strcpy(s, argv[i + 1]);
			strcat(s, ".asm");
		}
	}

	// now open the file referenced by s
	fp = fopen(s,"w");
	if (fp == NULL){
		printf("Cannor open file %s\n", s);
		exit(1);
	}
	yyparse();
	
	if(mydebug) {
		printf("\nFinished Parsing\n\n\n");
		Display();
		printf("\n\nAST PRINT \n\n");
		ASTprint(0, PROGRAM);
	}

	EMIT(PROGRAM, fp);
}

