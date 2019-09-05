#define EOI		0	/* End of input			*/
#define SEMI		1	/* ; 				*/
#define PLUS 		2	/* + 				*/
#define TIMES		3	/* * 				*/
#define MINUS		4	/* - 				*/
#define DIV		5	/* /			*/
#define LP		6	/* (				*/
#define RP		7	/* )				*/
#define NUM_OR_ID	8	/* Decimal Number or Identifier */
#define IF  9   /* if */
#define THEN  10   /* THEN */
#define WHILE  11   /* WHILE */
#define DO  12   /* DO */
#define BEGIN  13   /* BEGIN */
#define END  14   /* END */
#define GT  15   /* GREATER THAN */
#define LT  16   /* LESS THAN */
#define EQ  17   /* EQUAL */
#define COL  18   /* COLON */
#define ID  19   /* ID */


extern char *yytext;		/* in lex.c			*/
extern int yyleng;
extern int yylineno;
