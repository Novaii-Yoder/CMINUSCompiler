// emit.c
// 4/27/2023
// Jacob Yoder
//
// Program that emits MIPS code from AST tree
// Grade at level "f" this code impliments Read/Write, Assignments, If/While, 
// 			Arrays, Calls, Break/Continue

#include "ast.h"
#include "emit.h"
#include <string.h>
#include <stdlib.h>
int GLABEL = 0; // Global Label counter variable
char GFUNCNAME[100]; // Global string for which function we are currently in
int GMAXARGS = 8;
struct StackNode* breakStack = NULL;
struct StackNode* continueStack = NULL;
// PRE: PTR to AST tree, PTR to ouput File
// POST: prints out MIPS code into file, based on AST node tree
void EMIT(ASTnode* p, FILE* fp){
	//printf("EMIT CALLED");
	if(p == NULL){return;}
	if(fp == NULL){return;}
	
	fprintf(fp,"# MIPS CODE GENERATE by CS370\n\n");
	fprintf(fp,".data\n\n");
	
	EMIT_STRINGS(p, fp);
	fprintf(fp, ".align 2\n");

	//printf("EMITTING GLOBALS");
	EMIT_GLOBALS(p, fp);
	fprintf(fp, ".text\n\n");
	fprintf(fp, ".globl main\n\n"); 
	EMIT_AST(p, fp);
}

// PRE: Ptr to ASTnode or NULL
// POST: prints out MIPS code into the file from the tree
void EMIT_AST(ASTnode* p, FILE* fp){
	if(p == NULL){return;}
	switch(p->type){
		case A_VARDEC: //no real action
			EMIT_AST(p->next, fp);
			break;
		case A_FUNCTIONDEC:
			emit_function(p,fp);
			EMIT_AST(p->next, fp);
			break;
		case A_COMPOUND: // no action for s1 (vardec) already in stack size
			EMIT_AST(p->s2, fp);
			EMIT_AST(p->next, fp);
			break;
		case A_WRITE: // deal with it using our helper function
			emit_write(p, fp);
			EMIT_AST(p->next,fp);
			break;
		case A_READ:
			emit_read(p, fp);
			EMIT_AST(p->next, fp);
			break;
		case A_EXPR:
			emit_expr(p, fp);
			printf("THIS IS A_EXPR IN EMIT_AST()\n\n\n");
			//EMIT_AST(p->next, fp);
			break;
		case A_EXPRSTMT:
			if (p->s1 != NULL) {
				emit_expr(p->s1, fp);
			} // if to not call expression with only";"
			EMIT_AST(p->next, fp);
			break;
		case A_IF:
			emit_if(p, fp);
			EMIT_AST(p->next, fp);
			break;
		case A_IFELSE:
			emit_ifelse(p, fp);
			EMIT_AST(p->next, fp);
			break;
		case A_RETURN:
			emit_return(p, fp);
			EMIT_AST(p->next, fp);
			break;
		case A_BREAK:
			emit_break(p, fp);
			EMIT_AST(p->next, fp);	
			break;
		case A_CONTINUE:
			emit_continue(p, fp);
			EMIT_AST(p->next, fp);	
			break;
		case A_ASSIGN:
			emit_assign(p, fp);
			EMIT_AST(p->next, fp);	
			break;
		case A_WHILE:
			emit_while(p, fp);
			EMIT_AST(p->next, fp);
			break; 

		default:
			printf("EMIT_AST case %d not implemented\n", p->type);
			exit(1);
	} // end of switch
} // end of EMIT_AST()

// PRE: PTR to AST tree, PTR to ouput File
// POST: prints out MIPS code for global strings to file
void EMIT_STRINGS(ASTnode* p, FILE* fp){
	if(p == NULL){return;}
	if(p->type == A_WRITE && p->name != NULL){
		p->label = create_label();
		fprintf(fp, "%s: .asciiz\t%s\n", p->label, p->name);
	}
	EMIT_STRINGS(p->s1, fp);
	EMIT_STRINGS(p->s2, fp);
	EMIT_STRINGS(p->next, fp);
}

// PRE: PTR to AST tree, PTR to ouput File
// POST: prints out MIPS code for global variables to file
void EMIT_GLOBALS(ASTnode* p, FILE* fp){
   	int i;
   	if(p == NULL){return;}
	if(p->type != A_VARDEC){return;}
	if(p->symbol->level != 0){return;}
  	fprintf(fp,"%s: .space %d\t# global variable\n",p->symbol->name, p->symbol->mysize * WSIZE);
	EMIT_GLOBALS(p->s1, fp);
	EMIT_GLOBALS(p->next, fp);
}// end of EMIT_GLOBALS

// PRE: possible label, command, comment
// POST: formatted output to the file
void emit(FILE *fp, char* label, char* command, char* comment){
	if(strcmp("", comment) == 0){
		if(strcmp("", label) == 0){
			fprintf(fp, "\t%s\t\t\n", command);
		}else{
			fprintf(fp, "%s:\t%s\t\t\n", label, command);
		}
	}else{
		if(strcmp("", label) == 0){
			fprintf(fp, "\t%s\t\t# %s\n", command, comment);
		}else{
			fprintf(fp, "%s:\t%s\t\t# %s\n", label, command, comment);
		}
	} // prints out any combination of formatted emit
} // end of emit()


// PRE: Assume one up global variable GLABEL
// POST: Returns string with the format _L%d and increments the global variable GLABEL
char* create_label(){
	char hold[100];
	char* s;
	sprintf(hold, "_L%d", GLABEL++);
	s = strdup(hold);
	return s;
} // end create_label


// PRE: PTR to ASTnode A_FUNCTIONDEC
// POST: print MIPS code to file
void emit_function(ASTnode *p, FILE* fp){
	char s[100];
	emit(fp,p->name,"","Function definition");
	strcpy(GFUNCNAME, p->name);
	// carve out the Stack for activation record
	emit(fp, "", "move $a1, $sp","Activation Record carve out copy SP");
	sprintf(s, "subi $a1, $a1, %d", p->symbol->offset * WSIZE);
	emit(fp, "", s, "Activation Record carve out size of function");
	emit(fp, "", "sw $ra, ($a1)", "Store Return Address");
	sprintf(s, "sw $sp %d($a1)",WSIZE);
	emit(fp, "", s, "Store the old Stack pointer");
	emit(fp, "", "move $sp, $a1","Make SP the current activaiton record");
	fprintf(fp, "\n\n");
	
	// copy the parameters to the formal from registers $t0 et
	if(p->s1 != NULL){
		emit_params(p->s1, fp);
	}
	// generate the compound statment
	EMIT_AST(p->s2, fp);

	emit_return(p, fp);
	fprintf(fp, "\n\t\t\t # END OF FUNCTION\n\n");
}// end of emit_function

// PRE: PTR to A_WRITE
// POST: prints MIPS code to genereate WRITE string or WRITE number to file
//		depending on out arguments
void emit_write(ASTnode* p, FILE* fp){
	char s[100];
	// if a string
	if(p->name != NULL){
		sprintf(s, "la $a0, %s", p->label);
		emit(fp, "", s, "The string address");
		emit(fp, "", "li $v0, 4", "Setup for print string");
		emit(fp, "", "syscall\t","Call write string");
		fprintf(fp,"\n");
	}else{
		emit_expr(p->s1, fp); //now $a0 has the expression value
		emit(fp, "", "li $v0, 1", "Setup for print number");
		emit(fp, "", "syscall\t","Call write number");
		fprintf(fp,"\n");
	}// end of if else

} // end of emit_write()

// PRE: PTR to A_READ
// POST: print MIPS code to read in a value and place it in the VAR of READ

void emit_read(ASTnode* p, FILE* fp){
	emit_var(p->s1, fp); // $a0 will be the location of the variable
	emit(fp, "", "li $v0, 5", "Setup for read in value");
	emit(fp, "", "syscall\t","Call read in value ($v0 now has the read in value)");
	emit(fp, "", "sw $v0, ($a0)", "Store the read in value into memory");
	fprintf(fp,"\n");
} // end of emit_read()

// PRE: PTR to expression family
// POST: print MIPS code that sets $a0 to the value of the expression

void emit_expr(ASTnode* p, FILE* fp){
	char s[100];
	// base cases
	switch(p->type) {
		case A_NUM:
			sprintf(s, "li $a0, %d", p->value);
			emit(fp, "", s, "Expression is a constant");
			return;
			break;
		case A_EXPR: // handled after switch
			break;
		case A_VAR:
			emit_var(p, fp); // $a0 is the memory location
			emit(fp, "", "lw $a0, ($a0)", "Expression is a VAR");
			return;
			break;
		case A_CALL:
			emit_call(p, fp);
			return;
			break;
		default:
			printf("emit_expr case %d not implemented\n", p->type);
			exit(1);
	} // end of switch
	
	if(p->operator == A_UMINUS){ // Handle unary minus
		emit_expr(p->s1, fp);
		emit(fp, "", "neg $a0, $a0", "EXPRESSION UMINUS");
		fprintf(fp, "\n");
		return;
	} // end of if

	emit_expr(p->s1, fp);
	sprintf(s, "sw $a0, %d($sp)", p->symbol->offset * WSIZE);
	emit(fp, "", s, "Expression store LHS temporarily");
	emit_expr(p->s2, fp);
	emit(fp, "", "move $a1, $a0", "RHS needs to be moved to $a1");
	sprintf(s, "lw $a0, %d($sp)", p->symbol->offset * WSIZE);
	emit(fp, "", s, "Expression restore LHS from memory");
	switch(p->operator){
		case A_PLUS: 
			emit(fp, "", "add $a0, $a0, $a1", "EXPRESSION ADD");
			break;
	        case A_MINUS: 
			emit(fp, "", "sub $a0, $a0, $a1", "EXPRESSION SUB");
			break;
		case A_MULT: 
			emit(fp, "", "mult $a0, $a1", "EXPRESSION MULT");
			emit(fp, "", "mflo $a0", "EXPRESSION MULT");
			break;
		case A_MOD: 
			emit(fp, "", "div $a0, $a1", "EXPRESSION MOD");
			emit(fp, "", "mfhi $a0", "EXPRESSION MOD");
			break;
		case A_DIV: 
			emit(fp, "", "div $a0, $a1", "EXPRESSION DIV");
			emit(fp, "", "mflo $a0", "EXPRESSION DIV");
			break;
		case A_GT: 
			emit(fp, "", "slt $a0, $a1, $a0", "EXPRESSION GREATERTHAN");
			break;
		case A_GE: 
			emit(fp, "", "add $a0, $a0, 1", "EXPRESSION ADD GREATERTHAN/EQUAL");
			emit(fp, "", "slt $a0, $a1, $a0", "EXPRESSION GREATERTHAN/EQUAL");
			break;
		case A_LT: 
			emit(fp, "", "slt $a0, $a0, $a1", "EXPRESSION LESSTHAN");
			break;
		case A_LE: 
			emit(fp, "", "add $a1, $a1, 1", "EXPRESSION ADD LESSTHAN/EQUAL");
			emit(fp, "", "slt $a0, $a0, $a1", "EXPRESSION LESSTHAN/EQUAL");
			break;
		case A_EQ: 
			emit(fp, "", "slt $t2, $a0, $a1", "EXPRESSION EQUAL");
			emit(fp, "", "slt $t3, $a1, $a0", "EXPRESSION EQUAL");
			emit(fp, "", "nor $a0, $t2, $t3", "EXPRESSION EQUAL");
			emit(fp, "", "andi $a0, 1      ", "EXPRESSION EQUAL");
			break;
		case A_NE: 
			emit(fp, "", "slt $t2, $a0, $a1", "EXPRESSION NOT EQUAL");
			emit(fp, "", "slt $t3, $a1, $a0", "EXPRESSION NOT EQUAL");
			emit(fp, "", "or $a0, $t2, $t3", "EXPRESSION NOT EQUAL");
			break;
		default:
			printf("Unknown Operator in emit_expr()\n");
	} // end of switch for operator
	fprintf(fp, "\n");
} // end of emit_expr()

// PRE: PTR to VAR
// POST: $a0 has the memory location (L-value) of VAR
void emit_var(ASTnode* p, FILE* fp){
	char s[100];
	if(p->type == A_VARDEC && p->symbol->level == 0){ // global variable declaration
		sprintf(s, "la $a0, %s", p->name);
		emit(fp,"",s,"EMIT Var global variable");
	}else{ // var call
	       	if (p->symbol->SubType != SYM_ARRAY){ // not array
			if(p->symbol->level == 0){ // is global var
				sprintf(s, "la $a0, %s", p->name);
				emit(fp,"",s,"EMIT Var global variable");
			}else{ // is local var
				emit(fp, "", "move $a0, $sp", "EMIT Var local make a copy of stackpointer");
				sprintf(s, "addi $a0, $a0, %d", p->symbol->offset * WSIZE);
				emit(fp,"",s,"EMIT Var local stack pointer plus ofset");
			} // end of else if
		}else{ // is array
			emit_expr(p->s1, fp);
			emit(fp, "", "move $a1, $a0", "VAR copy index array in a1");
			sprintf(s, "sll $a1, $a1, %d", LOG_WSIZE);
			emit(fp, "", s, "Multiply the index by wordsize via SLL");
			if(p->symbol->level == 0){ // is global var
				sprintf(s, "la $a0, %s", p->name);
				emit(fp,"",s,"EMIT Var global variable");
			}else{ // is local var
				emit(fp, "", "move $a0, $sp", "EMIT Var local make a copy of stackpointer");
				sprintf(s, "addi $a0, $a0, %d", p->symbol->offset * WSIZE);
				emit(fp,"",s,"EMIT Var local stack pointer plus ofset");
			} // end of else if

			emit(fp, "", "add $a0 $a0 $a1", "VAR array add internal offset");
		} // end of if else
	}// end of if else
	// add on array index if needed
	

} // emit_var()


// PRE: PTR to a A_IF
// POST: print MIPS code to file for just if 
void emit_if(ASTnode* p, FILE* fp){
	char s[100];
	char* L1 = create_label();
	emit_expr(p->s1, fp);
	sprintf(s, "beq $a0 , $0 %s", L1);
	emit(fp, "", s, "If condition");
	fprintf(fp, "\n\t\t\t# The positive portion of IF\n");
	EMIT_AST(p->s2, fp); // emit if body
	sprintf(s , "%s", L1);
	emit(fp, s, "", "End of if");
	
} // end of emit_if()


// PRE: PTR to a A_IFELSE
// POST: print MIPS code to file for just if else
void emit_ifelse(ASTnode* p, FILE* fp){
	char s[100];
	char* L1 = create_label();
	char* L2 = create_label();
	emit_expr(p->s1->s1, fp);
	sprintf(s, "beq $a0 , $0 %s", L1);
	emit(fp, "", s, "If condition");
	fprintf(fp, "\n\t\t\t# The positive portion of IF\n");
	EMIT_AST(p->s1->s2, fp); // emit if body
	sprintf(s , "j %s", L2);
	emit(fp, "", s, "Jump to after else");
	sprintf(s , "%s", L1);
	emit(fp, s, "", "Start of else");
	fprintf(fp, "\n\t\t\t# The negative portion of IF (ELSE)\n");
	EMIT_AST(p->s2, fp); // emit else body
	sprintf(s , "%s", L2);
	emit(fp, s, "", "End of else");

}// end of emit_ifelse()


// PRE: PTR to A_RETURN
// POST: print MIPS code to restore old environment and return
void emit_return(ASTnode* p, FILE* fp){
	char s[100];
	// implicit return if we are in main or not
	if(p->type == A_RETURN && p->s1 != NULL){ // checks to see if return has an explicit value
		emit_expr(p->s1, fp);
	}else{
		emit(fp, "", "li $a0, 0", "RETURN has no specifiec value, set to zero");
	} // end of else if
	// restore RA and SP before we return
	emit(fp, "", "lw $ra ($sp)", "Restore old environment RA");
	sprintf(s, "lw $sp %d($sp)", WSIZE);
	emit(fp, "", s, "Return from function store SP");
	fprintf(fp, "\n");
	
	if(strcmp(GFUNCNAME, "main") == 0){ // exit the system if in main
		emit(fp, "", "li $v0, 10", "Exit from Main we are done");
		emit(fp, "", "syscall\t", "EXIT everything");
	}else{ // jump back to the called
		emit(fp, "", "jr $ra\t", "Return to the caller");
	} // end of if else
} // end of emit_return()



// PRE: PTR to A_ASSIGN
// POST: print MIPS code to assign variables
void emit_assign(ASTnode* p, FILE* fp){
	char s[100];
	emit_expr(p->s2, fp);
	sprintf(s, "sw $a0, %d($sp)", p->symbol->offset * WSIZE);
	emit(fp, "", s, "Assign store RHS temporarily");
	emit_var(p->s1, fp);
	sprintf(s, "lw $a1, %d($sp)", p->symbol->offset * WSIZE);
	emit(fp, "", s, "Assign get RHS temporarily");
	emit(fp, "", "sw $a1, ($a0)", "Assign place RHS into memory");
	
} // end of emit_assign()


// PRE: PTR to A_CALL
// POST: print MIPS code to call functions
void emit_call(ASTnode* p, FILE* fp){
	char s[100];
	fprintf(fp, "\n\t\t\t# Setting up Function Call\n");
	fprintf(fp, "\t\t\t# Evaluate Function Parameters\n");
	ASTnode* temp = p->s1;
	while(temp != NULL){
		emit_expr(temp->s1, fp);
		sprintf(s, "sw $a0, %d($sp)", temp->symbol->offset * WSIZE);
		emit(fp, "", s, "Store call Arg temporarily\n");
		temp = temp->next;
	}
	fprintf(fp, "\n\t\t\t# Place Parameters into T registers\n");
	temp = p->s1;
	int count = 0;
	while(temp != NULL){
		if(count >= GMAXARGS){	
			printf("Too many arguments, we can handle only 8\n");
			exit(1);
		}
		sprintf(s, "lw $a0, %d($sp)", temp->symbol->offset * WSIZE);
		emit(fp, "", s, "Pull out stored Arg");
		sprintf(s, "move $t%d, $a0", count);
		emit(fp, "", s, "Move arg in temp");
		count = count + 1;
		temp = temp->next;
	}
	
	fprintf(fp, "\n");
	sprintf(s, "jal %s", p->name);
	emit(fp, "", s, "Call to function $a0 will be return value\n");
} // end of emit_call()


// PRE: PTR to A_WHILE
// POST: print MIPS code for while
void emit_while(ASTnode* p, FILE* fp){
	char s[100];
	char* L1 = create_label();
	char* L2 = create_label();
	
	push(&continueStack, L1);
	push(&breakStack, L2);

	sprintf(s, "%s", L1);
	emit(fp, s, "", "WHILE TOP target");
	emit_expr(p->s1, fp);
	sprintf(s, "beq $a0, $0, %s", L2);
	emit(fp, "", s, "WHILE branch out");
	EMIT_AST(p->s2, fp);
	sprintf(s, "j %s", L1);
	emit(fp, "", s, "WHILE Jump back");
	sprintf(s, "%s", L2);
	emit(fp, s, "", "End of WHILE");

	pop(&continueStack);
	pop(&breakStack);
} // end of emit_while()


// PRE: PTR to A_PARAM list
// POST: print MIPS code for storing parameters
void emit_params(ASTnode* p, FILE* fp){
	char s[100];
	int counter = 0;
	if (p == NULL){ return;} // checks for end of parameters
	if (p->my_data_type == A_VOIDTYPE) {return;} // checks for VOID params
	while(p != NULL){
		if(counter >= GMAXARGS){	
			printf("Too many params, we can handle only 8\n");
			exit(1);
		}
		sprintf(s, "sw $t%d %d($sp)", counter, p->symbol->offset * WSIZE);
		emit(fp, "", s, "Parameter store start of function");
		p = p->next;
		counter++;
	}
} // end of emit_params()


// PRE: PTR to A_BREAK
// POST: print MIPS code for breaking out of while
void emit_break(ASTnode* p, FILE* fp){
	char s[100];
	//fprintf(fp, "\n\t\t\t# BREAK Statement jump inside of while\n");
	sprintf(s, "j %s", peek(breakStack));
	emit(fp, "", s, "BREAK Statement jump inside of while");
} // end of emit_return()


// PRE: PTR to A_CONTINUE
// POST: print MIPS code for continues in whiles
void emit_continue(ASTnode* p, FILE* fp){
	char s[100];
	//fprintf(fp, "\n\t\t\t# CONTINUE Statement jump inside of while\n");
	sprintf(s, "j %s", peek(continueStack));
	emit(fp, "", s, "CONTINUE Statement jump inside of while");
} // end of emit_continue()

// PRE: Data for the new StackNode
// POST: a StackNode with the label, and no next
StackNode* newNode(char* label){
	struct StackNode* stackNode = (StackNode*) malloc(sizeof(struct StackNode));
	stackNode->label = label;
	stackNode-> next = NULL;
	return stackNode;
} // end of newNode()


// PRE: A PTR to a StackNode
// POST: A boolean for weather or not the stack is empty
int isEmpty(struct StackNode* root){
	return !root;
} // end of isEmpty()


// PRE: Address to a StackNode, and data
// POST: Add a new linked list node to the Stack with data
void push(struct StackNode** root, char* label){
	struct StackNode* stackNode = newNode(label);
	stackNode->next = *root;
	*root = stackNode;
} // end of push()

// PRE: Address to a StackNode
// POST: remove the top of the stack and return the label
char* pop(struct StackNode** root){
	if(isEmpty(*root)){
		return NULL;
	}
	struct StackNode* temp = *root;
	*root = (*root)->next;
	char* popped = temp->label;
	free(temp);

	return popped;
} // end of pop()


// PRE: A PTR to a StackNode
// POST: return the label stored in the top node
char* peek(struct StackNode* root){
	if (isEmpty(root)){
		return NULL;
	}
	return root->label;
} // end of peek()
