/*  
	Lab 9
	Jacob Yoder
	lab9.l
	5/5/2023

	Simple lexical parser for the CMINUS+ language and YACC   
	- added tokens for break and continue

*/
%{

int mydebug = 0;
int lineno = 1;
#include "ast.h"
#include "y.tab.h"
%}


%%

int		{ 	return(T_INT); }
void		{	return(T_VOID);}
read		{	return(T_READ);}
write		{	return(T_WRITE);}
if		{	return(T_IF);}
else		{	return(T_ELSE);}
while		{	return(T_WHILE);}
return		{	return(T_RETURN);}
continue	{	return(T_CONTINUE);}
break		{	return(T_BREAK);}

"<"		{	return(T_LT);}
"<="		{	return(T_LE);}
"=="		{	return(T_EQ);}
"!="		{	return(T_NE);}
">"		{	return(T_GT);}
">="		{	return(T_GE);}

[a-zA-Z][a-zA-Z0-9_]*		{ if (mydebug) fprintf(stderr,"Id found\n"); 
                       	yylval.string = strdup(yytext);
			//printf("here: %s\n", yylval.string);
			 return(T_ID);
		}
[0-9][0-9]*	{ 	if (mydebug) fprintf(stderr,"Digit found\n"); 
                       	yylval.value = atoi((const char *)yytext); 
			return(T_NUM);
		}
\".*\"		{	yylval.string = strdup(yytext);
			return(T_STRING);
		}
[ \t]		{ 	if (mydebug) fprintf(stderr,"Whitespace found\n");}
\/\/.*\n	{	if (mydebug) fprintf(stderr,"Comment found\n");
			lineno++;}
[=\-+*/%&|();,[\]{}] { if (mydebug) fprintf(stderr,"return a token %c\n",*yytext); 
                      	return (*yytext);
		}
\n 		{ 	if (mydebug) fprintf(stderr,"cariage return %c\n",*yytext); 
		  	lineno++;
                      	// return (*yytext)
		}

%%

int yywrap(void)
{ return 1;}


