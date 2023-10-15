/*   Abstract syntax tree code


 Header file   
 Shaun Cooper Spring 2023

 This header file outlines the code for ast.c and defines enums and functions
 used in building and printing the abstract syntax tree. The tree uses ASTnodes
 with an ASTtype and the tree only has a s1 (left), s2 (right), and next connection.

 Jacob Yoder 5/5/2023
 - added A_RETURN and A_CONTINUE
*/

#include <stdio.h>
#include <malloc.h>

#include "symtable.h"

#ifndef AST_H
#define AST_H
int mydebug;

/* define the enumerated types for the AST.  THis is used to tell us what 
sort of production rule we came across */

enum ASTtype {
   A_FUNCTIONDEC,
   A_VARDEC,
   A_COMPOUND,
   A_WRITE,
   A_READ,
   A_NUM,
   A_EXPR,
   A_EXPRSTMT,
   A_VAR,
   A_ASSIGN,
   A_WHILE,
   A_IF,
   A_IFELSE,
   A_RETURN,
   A_CONTINUE,
   A_BREAK,
   A_PARAM,
   A_ARG,
   A_CALL
};

// Math Operators

enum AST_OPERATORS {
   A_PLUS,
   A_MINUS,
   A_UMINUS,
   A_MULT,
   A_DIV,
   A_MOD,
   A_LT,
   A_LE,
   A_GT,
   A_GE,
   A_EQ,
   A_NE
};

enum AST_MY_DATA_TYPE {
   A_BOOLTYPE,
   A_VOIDTYPE,
   A_INTTYPE
};

/* define a type AST node which will hold pointers to other AST nodes that 
 * will allow us to represent the parsed code 
*/

typedef struct ASTnodetype
{
     enum ASTtype type;
     enum AST_OPERATORS operator;
     char * name;
     char* label;
     int value;
     enum AST_MY_DATA_TYPE my_data_type;
     struct SymbTab *symbol;
     struct ASTnodetype *s1,*s2, *next ; /* used to connect all nodes in the tree */
} ASTnode;


/* uses malloc to create an ASTnode and passes back the heap address of the newly created node */
ASTnode *ASTCreateNode(enum ASTtype mytype);

/* prints spaces for tabbing on the ASTprint() */
void PT(int howmany);


/*  Print out the abstract syntax tree */
void ASTprint(int level,ASTnode *p);

//int check_params(ASTnode* actual, SymbTab* formal);
#endif // of AST_H
