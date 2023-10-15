/*   Abstract syntax tree code

     This code is used to define an AST node, 
    routine for printing out the AST
    defining an enumerated type so we can figure out what we need to
    do with this.  The ENUM is basically going to be every non-terminal
    and terminal in our language.

    Shaun Cooper Spring 2023

    Jacob Yoder 5/5/2023
    - Added case handling for A_BREAK and A_CONTINUE
*/

#include <stdio.h>
#include <malloc.h>
#include "ast.h" 
#include "symtable.h"

/* uses malloc to create an ASTnode and passes back the heap address of the newley created node */
//  PRE:  Ast Node Type
//  POST:   PTR To heap memory and ASTnode set and all other pointers set to NULL
ASTnode *ASTCreateNode(enum ASTtype mytype)
{
    ASTnode *p;
    if (mydebug) fprintf(stderr,"Creating AST Node \n");
    p=(ASTnode *)malloc(sizeof(ASTnode));
    p->type=mytype;
    p->s1=NULL;
    p->s2=NULL;
    p->next=NULL;
    p->value=0;
    p->symbol=NULL;
    p->label=NULL;
    return(p);
}

/*  Helper function to print tabbing */
//PRE:  Number of spaces desired
//POST:  Number of spaces printed on standard output

void PT(int howmany)
{
	for (int i = 0; i < howmany; i++){
		printf(" ");
	} // Prints a number of spaces equal to the value of input
}

//  PRE:  A declaration type
//  POST:  A character string that is the name of the type
//          Typically used in formatted printing
char * ASTtypeToString(enum ASTtype mytype)
{
    switch(mytype) {
	case A_INTTYPE:
		return "INT";
		break;
	case A_VOIDTYPE:
		return "VOID";
		break;
	default:
		printf("ASTtypeToString UNKOWN type\n");
		return "UNKNOWN Type";
    } // end switch

}



/*  Print out the abstract syntax tree */
// PRE:   PRT to an ASTtree
// POST:  indented output using AST order printing with indentation

void ASTprint(int level,ASTnode *p)
{
   int i;
   if (p == NULL ) return;
   else
     { 
       switch (p->type) {
        case A_VARDEC : 
                      PT(level); 
		      if(p->value != 0){
		      	printf("Variable %s %s[%d] level %d offset %d\n",
				ASTtypeToString(p->my_data_type),
				p->name,
				p->value,
				p->symbol->level,
				p->symbol->offset
		      	);
		      }else{
			printf("Variable %s %s level %d offset %d\n",
				ASTtypeToString(p->my_data_type),
				p->name,
				p->symbol->level,
				p->symbol->offset
			);
		      }
			ASTprint(level, p->s1); // print additional A_VARDEC of same type
			ASTprint(level, p->next); // print next
                     break;

        case A_FUNCTIONDEC :  
                      PT(level);      
		      printf("Function %s %s level %d offset %d\n", 
			ASTtypeToString(p->my_data_type),
			p->name,
			p->symbol->level,
			p->symbol->offset
		      );
		      ASTprint(level + 1, p->s1); // print parameters
		      ASTprint(level + 1, p->s2); // print compound
		      ASTprint(level, p->next); // print next
                     break;

	case A_COMPOUND:
                      PT(level);
		      printf("Compound Statement\n");
		      ASTprint(level + 1, p->s1); // print local declarations
		      ASTprint(level + 1, p->s2); // print statement list
		      ASTprint(level, p->next); // print next
		     break;

	case A_WRITE:
		      PT(level);
		      if (p->name != NULL) {
		      	printf("Write String %s\n", p->name); // print string
		      } else { // it is an expression
			printf("Write Expression\n");
			ASTprint(level + 1, p->s1); // print expression
		      } 
		      ASTprint(level, p->next); // print next
		     break;

	case A_READ:
		      PT(level);
		      printf("READ STATEMENT\n");
		      ASTprint(level + 1, p->s1); // print variable
		      ASTprint(level, p->next); // print next
		     break;

	case A_NUM:
		      PT(level);
		      printf("NUMBER value %d\n", p->value);
		     break;

	case A_EXPR:
		      PT(level);
		      printf("EXPRESSION operator ");
		      switch(p->operator) {
			case A_PLUS: printf("PLUS\n");break;
			case A_MINUS: printf("MINUS\n");break;
			case A_UMINUS: printf("UNARY-MINUS\n");break;
			case A_MULT: printf("TIMES\n");break;
			case A_MOD: printf("%\n");break;
			case A_DIV: printf("/\n");break;
			case A_GT: printf(">\n");break;
			case A_GE: printf(">=\n");break;
			case A_LT: printf("<\n");break;
			case A_LE: printf("<=\n");break;
			case A_EQ: printf("==\n");break;
			case A_NE: printf("!=\n");break;
			default:
				printf("unknown operator in A_EXPR in ASTprint\n");
		      } // end of switch
		      ASTprint(level + 1, p->s1); // print left
		      ASTprint(level + 1, p->s2); // print right
		     break;

	case A_EXPRSTMT:
		     PT(level);
		     printf("EXPRESSION Statement\n");
		     ASTprint(level + 1, p->s1); // print expression
		     ASTprint(level, p->next); // print next
		     break;

	case A_VAR:
		     PT(level);
		     printf("VARIABLE %s level %d offset %d\n", 
				     p->name,
				     p->symbol->level,
				     p->symbol->offset);
		     if(p->s1 != NULL){
			PT(level + 1);
			printf("[\n");
			ASTprint(level + 2, p->s1); // print expression for arrays
			PT(level + 1);
			printf("]\n");
		     }
		     break;

	case A_ASSIGN:
		     PT(level);
		     printf("ASSIGNMENT STATEMENT\n");
		     ASTprint(level + 1, p->s1); // print variable
		     PT(level);
		     printf("is assigned\n");
		     ASTprint(level + 1, p->s2); // print expression
		     ASTprint(level, p->next); // print next
		     break;

	case A_WHILE:
		     PT(level);
		     printf("WHILE STATEMENT\n");
		     PT(level + 1);
		     printf("WHILE expression\n");
		     ASTprint(level + 2, p->s1); // print expression
		     PT(level + 1);
		     printf("WHILE body\n");
		     ASTprint(level + 2, p->s2); // print body
		     ASTprint(level, p->next); // print next
		     break;

	case A_IF:
		     PT(level);
		     printf("IF STATEMENT\n");
		     PT(level + 1); 
		     printf("IF expression\n");
		     ASTprint(level + 2, p->s1); // print if expression
		     PT(level + 1);
		     printf("IF body\n");
		     ASTprint(level + 2, p->s2); // print if body
		     ASTprint(level, p->next); // print next
		     break;

	case A_IFELSE:
		     ASTprint(level, p->s1); // print if
		     PT(level + 1);
		     printf("ELSE body\n");
		     ASTprint(level + 2, p->s2); // print else statement
		     ASTprint(level, p->next); // print next
		     break;

	case A_RETURN:
		     PT(level);
		     printf("RETURN STATEMENT\n");
		     if(p->s1 != NULL){
			ASTprint(level + 1, p->s1); // print expression
		     } // end if
		     ASTprint(level, p->next); // print next
		     break;

	case A_CONTINUE:
		     PT(level);
		     printf("CONTINUE STATEMENT\n");
		     ASTprint(level, p->next); // print next
		     break;

	case A_BREAK:
		     PT(level);
		     printf("BREAK STATEMENT\n");
		     ASTprint(level, p->next); // print next
		     break;

	case A_PARAM:
		     PT(level);
		     if (p->value == 2){
			break; // if it voids params dont print anything
		     } // end if
		     if(p->value == 1){
			printf("Parameter %s %s[] level %d offset %d\n",
			     ASTtypeToString(p->my_data_type),
			     p->name,
			     p->symbol->level,
			     p->symbol->offset
			);
		     } else {
			printf("Parameter %s %s level %d offset %d\n",
			     ASTtypeToString(p->my_data_type),
			     p->name,
			     p->symbol->level,
			     p->symbol->offset
			);
		     } // end of else if
		     ASTprint(level, p->next); // print next
		     break;

	case A_ARG:
		     PT(level);
		     printf("CALL argument\n");
		     ASTprint(level + 1, p->s1); // print expression
		     ASTprint(level, p->next); // print next
		     break;

	case A_CALL:
		     PT(level);
		     printf("CALL STATEMENT   function %s\n", p->name);
		     PT(level + 1);
		     printf("(\n");
		     ASTprint(level + 1, p->s1); // print call arguments
		     PT(level + 1);
		     printf(")\n");
		     //ASTprint(level, p->next); // print next statment
		     break;


        default: printf("unknown AST Node type %d in ASTprint\n", p->type);


       } // end of switch
     } // end of else

} // end of ASTPrint



/* dummy main program so I can compile for syntax error independently   
main()
{
}
/* */
