// EMIT.H
// 4/27/2023
// Jacob Yoder
//
// interface for other classes to know about available functions
//
// provides connectivity to MIPS generating subroutines
// Grade at level "f" this code impliments Read/Write, Assignments, If/While, 
// 			Arrays, Calls, Break/Continue

#ifndef EMIT_H
#define EMIT_H
#include "ast.h"

#define WSIZE 4
#define LOG_WSIZE 2

void EMIT(ASTnode* p, FILE* fp);

void EMIT_AST(ASTnode* p, FILE* fp);

void EMIT_STRINGS(ASTnode* p, FILE* fp);

void EMIT_GLOBALS(ASTnode* p, FILE* fp);

void emit(FILE* fp, char* label, char* command, char* comment);
void emit_function(ASTnode* p, FILE* fp);
void emit_write(ASTnode* p, FILE* fp);
void emit_read(ASTnode* p, FILE* fp);
void emit_expr(ASTnode* p, FILE* fp);
void emit_var(ASTnode* p, FILE* fp);
void emit_if(ASTnode* p, FILE* fp);
void emit_ifelse(ASTnode* p, FILE* fp);
void emit_return(ASTnode* p, FILE* fp);
void emit_assign(ASTnode* p, FILE* fp);
void emit_call(ASTnode* p, FILE* fp);
void emit_while(ASTnode* p, FILE* fp);
void emit_params(ASTnode* p, FILE* fp);
void emit_break(ASTnode* p, FILE* fp);
void emit_continue(ASTnode* p, FILE* fp);

char* create_label();

typedef struct StackNode  {
	char* label;
	struct StackNode* next;
}StackNode;

int isEmpty(struct StackNode* root);
void push(struct StackNode** root, char* label);
char* pop(struct StackNode** root);
char* peek(struct StackNode* root);



#endif
