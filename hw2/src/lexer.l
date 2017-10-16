%option noyywrap 
%{
#include <stdio.h>
#include "header.h" 
int linenumber;
symtab * lookup();
symtab * ptr;
void     insertID();
void 	 printSymTab();
/* You need to define for all tokens in C--, here are some examples */

/* Reserved words */
#define RETURN		1
#define TYPEDEF		2
#define IF		3
#define ELSE		4
#define INT		5
#define FLOAT		6
#define FOR		7
#define VOID		8
#define WHILE		9

/* Literals */
#define INTLIT		10
#define FLOATLIT 	11
#define STRLIT		12

/* Identifiers */
#define ID		13

/* operators */
#define OP_ASSIGN 	14
#define OP_OR 		15
#define OP_AND		16
#define OP_NOT		17
#define OP_LE		18
#define OP_GE		19
#define OP_LT		20
#define OP_GT		21
#define OP_NE		22
#define OP_EQ		23
#define OP_MINUS	24
#define OP_PLUS		25
#define OP_MUL		26
#define OP_DIV		27

/* seperators */
#define MK_LPAREN 	28
#define MK_RPAREN 	29
#define MK_LBRACE	30
#define MK_RBRACE	31
#define MK_LBRACK	32
#define MK_RBRACK	33
#define MK_SEMICOL	34
#define MK_COMMA	35
#define MK_DOT		36
#define ERROR 		-1


%}

letter   [A-Za-z]
digit    [0-9]
ID	 {letter}({letter}|{digit}|"_")*
WS	 [ \t]+

RWReturn    "return"
RWTypedef   "typedef"
RWIf	    "if"
RWElse	    "else"
RWInt	    "int"
RWFloat	    "float"
RWFor	    "for"
RWVoid	    "void"
RWWhile	    "while"

Int_constant {digit}+
Float_constant ({digit}+\.|({digit}*\.{digit}+([Ee][+-]?{digit}+)?))
String_constant \"([^\n"])*\"
comment \/\*([^*]|\*+[^*/])*\*+\/

/* You need to define the following RE's 
Float_constant
String_constant
comment
*/

OP_assign        "="
OP_or   "||"
OP_and   "&&"
OP_not   "!"
OP_le   "<="
OP_ge   ">="
OP_lt   "<"
OP_gt   ">"
OP_ne   "!="
OP_eq   "=="
OP_plus   "+"
OP_minus   "-"
OP_mul   "*"
OP_div   "/"

/* Other operators appear here */ 


newline  "\n"

DL_lparen "("
DL_rparen ")"
DL_lbrace "{"
DL_rbrace "}"
DL_lbrack "["
DL_rbrack "]"
DL_comma  ","
DL_semicol ";"
DL_dot  "."

/* Other separators appear here */

error    .

%%

{WS}            {}
{RWReturn}      { return RETURN; }
{RWTypedef}     { return TYPEDEF; }
{RWIf}	        { return IF; }
{RWElse}	    { return ELSE; }
{RWInt}         { return INT;  }
{RWFloat}       { return FLOAT; }
{RWFor}	        { return FOR; }
{RWVoid}	    { return VOID; }
{RWWhile}	    { return WHILE; }
{Int_constant}  { /*printf("scanned int %s\n", yytext);*/ return INTLIT; }
{Float_constant}  { /*printf("scanned float %s\n", yytext);*/ return FLOATLIT; }
{String_constant}  { /*printf("scanned string %s\n", yytext);*/ return STRLIT; }
{comment}	{ printf("%s\n", yytext); }
{ID}            {
			ptr = lookup(yytext);
			if (ptr == NULL)
			     insertID(yytext);
			else
			     ptr->counter++;
		}

{OP_assign}     { return OP_ASSIGN; }
{OP_or}         { return OP_OR; }
{OP_and}        { return OP_AND; }
{OP_not}        { return OP_NOT; }
{OP_le}         { return OP_LE; }
{OP_ge}         { return OP_GE; }
{OP_lt}         { return OP_LT; }
{OP_gt}         { return OP_GT; }
{OP_ne}         { return OP_NE; }
{OP_eq}         { return OP_EQ; }
{OP_plus}       { return OP_PLUS; }
{OP_minus}      { return OP_MINUS; }
{OP_mul}        { return OP_MUL; }
{OP_div}        { return OP_DIV; }
{DL_lparen}     { return MK_LPAREN; }
{DL_rparen}     { return MK_RPAREN; }
{DL_lbrace}     { return MK_LBRACE; }
{DL_rbrace}     { return MK_RBRACE; }
{DL_lbrack}     { return MK_LBRACK; }
{DL_rbrack}     { return MK_RBRACK; }
{DL_comma}      { return MK_COMMA; }
{DL_semicol}    { return MK_SEMICOL;}
{DL_dot}	{ return MK_DOT;}
{newline}       linenumber += 1;
{error}         { printf("ERR %s\n", yytext); return ERROR; }


%%

main(int argc, char **argv)
{
 argc--; ++argv;
   if (argc > 0)
      yyin = fopen(argv[0], "r");
   else
      yyin = stdin;
   while(yylex() != ERROR);
   printSymTab();
}

