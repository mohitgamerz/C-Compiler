#include "name.c"
#include "lex.c"
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <ctype.h>

void stmt_list(int);
void statement(int);
int legal_lookahead(int first_arg, ...);
char *factor(int);
char *term(int);
char * expression(int);
char * expression1(int);
FILE *assFile, *interFile, *lexemeFile,*parse;
char REG[7] = {'A','B','C','D','E', 'H', 'L'};
int multiCounter = 0, divCounter = 0;
char *ids[100];
int id_counter=0;
void print_spaces(int spaces)
{
	for(int i=0;i<spaces;i++)
		fprintf(parse, " ");
}

void statements()
{
	/*
		statements -> statement statements | statement
	*/
	parse = fopen("parse.txt","w");
	int spaces = 0;
	fprintf(parse, "statements\n");
	spaces  =2;
	assFile = fopen("Assembly.asm", "w");
	interFile = fopen("Intermediate.txt", "w");

	fprintf(assFile, "%s\n", "ORG 0000h");

	while(!match(EOI))
		statement(spaces);

	fprintf(assFile, "%s\n", "HLT");
	fclose(assFile);
	fclose(interFile);
}

void statement(int spaces)
{
	/*
		statement ->	ID:=expression1;
					  | IF expression1 then statement
					  | while expression1 do statement
					  | expression1;
					  |	begin  stmt_list end
	*/

	char *tempvar;
	print_spaces(spaces);
	fprintf(parse, "statement\n" );
	spaces+=2;
	if(match(ID))
	{

		char assignment[100];
		int loop_var=0;
		for(;loop_var<yyleng;loop_var++)assignment[loop_var]=*(yytext+loop_var);
		assignment[loop_var]='\0';
		if(!( (assignment[0] >= 'a' && assignment[0] <= 'z') || (assignment[0] >= 'A' && assignment[0] <= 'Z') ) ) {
			fprintf( stderr, "%d: Invalid assigment\n", yylineno );
			exit(1);
		}
		ids[id_counter++] = assignment;
	    print_spaces(spaces);
		fprintf(parse, "ID \n");
		advance();
		if( !match( COL) ){
			fprintf( stderr, "%d: Inserting missing colon\n", yylineno );
			goto legal_lookahead_SEMI;
		}else{
			advance();
			if( !match( EQ) ){
				fprintf( stderr, "%d: Inserting missing equal\n", yylineno );
				goto legal_lookahead_SEMI;
			}else{
				print_spaces(spaces);
				fprintf(parse, ":=\n");
				advance();
				tempvar = expression1(spaces);
				fprintf(interFile,"%s <- %s\n",assignment,tempvar);

				if(strcmp(tempvar, "t0") == 0){
					fprintf(assFile, "STA _%s\n", assignment);
				} else {
					fprintf(assFile, "MOV H, A\nPUSH H\nMOV A %c\nSTA _%s\nPOP H\nMOV A, H\n", REG[tempvar[1]-'0'], assignment);
				}
			}
		}
	}
	else if(match(IF)){
		print_spaces(spaces);
		fprintf(parse, "IF\n" );
		advance();
		fprintf(interFile, "if (\n");
		tempvar = expression1(spaces);
		if( !match(THEN) )
		{
			freename(tempvar);
			fprintf( stderr, "%d: Inserting missing then\n", yylineno );
			goto legal_lookahead_SEMI;
		}else{
			print_spaces(spaces);
			fprintf(parse, "THEN\n");
			fprintf(interFile, "%s)\n", tempvar);
			int ifthenLabel = getIfThenLabel();
			fprintf(assFile, "CPI 0\nJZ IFTHEN%d\n", ifthenLabel);
			fprintf(interFile, "%s\n", "then {");
			advance();
			freename(tempvar);
			statement(spaces);
			fprintf(interFile, "%s\n", "}");
			fprintf(assFile, "IFTHEN%d: POP H\nMOV A, H\n", ifthenLabel);
			return;
		}
	}
	else if(match(WHILE))
	{
		print_spaces(spaces);
		fprintf(parse, "WHILE\n");
		advance();
		int loopLabel1 = getLoopLabel();
		fprintf(assFile, "LOOP%d:\n",loopLabel1);
		fprintf(interFile, "while (\n");
		tempvar = expression1(spaces);
		if( !match( DO ) )
		{
			freename(tempvar);
			fprintf( stderr, "%d: Inserting missing do\n", yylineno );
			goto legal_lookahead_SEMI;
		}else{
			print_spaces(spaces);
		    fprintf(parse, "DO\n");
			fprintf(interFile, "%s)\n", tempvar);
			int loopLabel = getLoopLabel();
			fprintf(assFile, "CPI 0\nJZ LOOP%d\n", loopLabel);

			fprintf(interFile, "%s\n", "do {");
			advance();

			freename(tempvar);
			statement(spaces);
			fprintf(interFile, "%s\n", "}");
			fprintf(assFile, "JMP LOOP%d\nLOOP%d: POP H\nMOV A, H\n", loopLabel1, loopLabel);
			return;
		}
	}
	else if(match(BEGIN))
	{
		print_spaces(spaces);
		fprintf(parse, "BEGIN\n");
		fprintf(interFile, "%s\n", "BEGIN{");
		advance();
		stmt_list(spaces);
		if(!match(END))
		{
			fprintf( stderr, "%d: Inserting missing END\n", yylineno );
			goto legal_lookahead_SEMI;
		} else {
			print_spaces(spaces);
		    fprintf(parse, "END\n");
			fprintf(interFile, "} %s\n", "END");
			lexemeFile = fopen("Lexemes.txt", "a+");
			fprintf(lexemeFile, "<END> ");
			fclose(lexemeFile);
			advance();
		}
		return;
	}
	else{
		tempvar = expression1(spaces);
	}

	if(tempvar!=NULL){
		freename( tempvar );
	} else{
		exit(1);
	}

legal_lookahead_SEMI:
		if( match( SEMI ) ){
			advance();
		}else{
			fprintf( stderr, "%d: Inserting missing semicolon\n", yylineno );
		}
}

void stmt_list(int spaces)
{

	/*
		stmt_list -> statement stmt_list | epsilon
	*/
	print_spaces(spaces);
		fprintf(parse, "stmt_list\n");
		spaces +=2;
	while(!match(END)&&!match(EOI))
		statement(spaces);
	if(match(EOI)){
		fprintf( stderr, "%d: End of file reached no END found\n", yylineno );
	}
}

char *expression1(int spaces)
{
	/*
		expression1 ->	  expression
						| expression<expression
						| expression=expression
						| expression>expression
	*/
	print_spaces(spaces);
	fprintf(parse, "expression1\n");
	spaces +=2;
	char *tempvar3;
	char * tempvar=expression(spaces);
	if(match(GT))
	{
		print_spaces(spaces);
		fprintf(parse, "GT\n");
		freename(tempvar);
		tempvar3=newname();
		tempvar=newname();
		fprintf(interFile, "%s <- %s\n", tempvar, tempvar3);
		fprintf(assFile, "MOV %c, %c\n", REG[tempvar[1]-'0'], REG[tempvar3[1]-'0']);

		advance();
		char *tempvar2=expression(spaces);

		fprintf(interFile, "%s <-  %s > %s\n",tempvar3,tempvar,tempvar2);
		int compLabel = getCompareLabel();
		fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nCMP %c\n",REG[tempvar[1]-'0'],REG[tempvar2[1]-'0']);
		fprintf(assFile, "MVI A, 1\nJNZ COMPARE%d\nMVI A, 0\nCOMPARE%d: ", compLabel, compLabel);

		freename(tempvar2);
		freename(tempvar);
		return tempvar3;
	}
	else if(match(LT))
	{
		print_spaces(spaces);
		fprintf(parse, "LT\n");
		freename(tempvar);
		tempvar3=newname();
		tempvar=newname();
		fprintf(interFile, "%s <- %s\n", tempvar, tempvar3);
		fprintf(assFile, "MOV %c, %c\n", REG[tempvar[1]-'0'], REG[tempvar3[1]-'0']);

		advance();
		char *tempvar2=expression(spaces);
		fprintf(interFile, "%s <-  %s < %s\n",tempvar3,tempvar,tempvar2);
		int compLabel = getCompareLabel();
		fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nCMP %c\n",REG[tempvar[1]-'0'],REG[tempvar2[1]-'0']);
		fprintf(assFile, "MVI A, 1\nJC COMPARE%d\nMVI A, 0\nCOMPARE%d: ", compLabel, compLabel);

		freename(tempvar);
		freename(tempvar2);
		return tempvar3;
	}
	else if(match(EQ))
	{
		print_spaces(spaces);
		fprintf(parse, "EQ\n");
		freename(tempvar);
		tempvar3=newname();
		tempvar=newname();
		fprintf(interFile, "%s <- %s\n", tempvar, tempvar3);
		fprintf(assFile, "MOV %c, %c\n", REG[tempvar[1]-'0'], REG[tempvar3[1]-'0']);

		advance();
		char *tempvar2=expression(spaces);
		fprintf(interFile, "%s <-  %s == %s\n",tempvar3,tempvar,tempvar2);
		int compLabel = getCompareLabel();
		fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nCMP %c\n",REG[tempvar[1]-'0'],REG[tempvar2[1]-'0']);
		fprintf(assFile, "MVI A, 1\nJZ COMPARE%d\nMVI A, 0\nCOMPARE%d: ", compLabel, compLabel);

		freename(tempvar);
		freename(tempvar2);
		return tempvar3;
	}
	return tempvar;
}

char *expression(int spaces)
{
	/*
		expression  -> term expression'
		expression' ->    PLUS term expression'
						| MINUS term expression'
						| epsilon
	*/
	print_spaces(spaces);
		fprintf(parse, "expression\n");
	spaces+=2;
	char  *tempvar, *tempvar2;

	tempvar = term(spaces);
	while( match( PLUS ) || match(MINUS) )
	{
		int PlusFlag=(match(PLUS))?1:0;
		if (PlusFlag == 1)
		{
			print_spaces(spaces);
			fprintf(parse, "ADD\n");
		}
		else
		{
			print_spaces(spaces);
		    fprintf(parse, "MINUS\n");
		}
		advance();
		tempvar2 = term(spaces);
		if(PlusFlag){
			fprintf(interFile, "%s += %s\n",tempvar,tempvar2);
			if(strcmp(tempvar, "t0") == 0){
				fprintf(assFile, "ADD %c\n",REG[tempvar2[1]-'0']);
			} else {
				fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nADD %c\nMOV %c, A\nPOP H\nMOV A, H\n",REG[tempvar[1]-'0'], REG[tempvar2[1]-'0'],REG[tempvar[1]-'0']);				// Assembly Code
			}
		}
		else{
			fprintf(interFile, "%s -= %s\n",tempvar,tempvar2);
			if(strcmp(tempvar, "t0") == 0){
				fprintf(assFile, "SUB %c\n",REG[tempvar2[1]-'0']);
			} else {
				fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nSUB %c\nMOV %c, A\nPOP H\nMOV A, H\n",REG[tempvar[1]-'0'], REG[tempvar2[1]-'0'],REG[tempvar[1]-'0']);
			}
		}
		freename( tempvar2 );
	}

	return tempvar;
}

char *term(int spaces)
{
	/*
		term 	->  factor term'
		term' 	->  TIMES factor term'
					| DIV factor term'
					| epsilon
	*/
	print_spaces(spaces);
		fprintf(parse, "term\n");
		spaces +=2;
	char  *tempvar, *tempvar2 ;

	tempvar = factor(spaces);
	while( match( TIMES ) || match(DIV))
	{
		int TimesFlag=(match(TIMES))?1:0;
		if(TimesFlag)
		{
			print_spaces(spaces);
			fprintf(parse, "MUL\n");
		}
		else
		{
			print_spaces(spaces);
		    fprintf(parse, "DIV\n");
		}
		advance();
		tempvar2 = factor(spaces);
		if(TimesFlag){
			
			fprintf(interFile, "%s *= %s\n",tempvar,tempvar2);
			if(strcmp(tempvar, "t0") == 0){
				fprintf(assFile, "MOV L, A\nDCR %c\nMLOOP%d: ADD L\nDCR %c\nJNZ MLOOP%d\n",REG[tempvar2[1]-'0'], multiCounter++,REG[tempvar2[1]-'0'], multiCounter);
			} else {
				fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nMOV L, A\nDCR %c\nMLOOP%d: ADD L\nDCR %c\nJNZ MLOOP%d\nMOV %c, A\nPOP H\nMOV A, H\n",REG[tempvar[1]-'0'], REG[tempvar2[1]-'0'], multiCounter++,REG[tempvar2[1]-'0'], multiCounter,REG[tempvar[1]-'0']);
			}
		}
		else {
			fprintf(interFile, "%s /= %s\n",tempvar,tempvar2);
			if(strcmp(tempvar, "t0") == 0){
				fprintf(assFile, "MVI L, 0\nCMP %c\nJC NOT%d\nDLOOP%d: INR L\nSUB %c\nCMP %c\nJNC DLOOP%d\nNOT%d: MOV A, L\n",REG[tempvar2[1]-'0'],divCounter++,divCounter,REG[tempvar2[1]-'0'],REG[tempvar2[1]-'0'], divCounter,divCounter);
			} else {
				fprintf(assFile, "MOV H, A\nPUSH H\nMOV A, %c\nMVI L, 0\nCMP %c\nJC NOT%d\nDLOOP%d: INR L\nSUB %c\nCMP %c\nJNC DLOOP%d\nNOT%d: MOV A, L\nPOP H\nMOV A, H\n",REG[tempvar[1]-'0'],REG[tempvar2[1]-'0'], divCounter++,divCounter, REG[tempvar2[1]-'0'], REG[tempvar2[1]-'0'], divCounter,divCounter);
			}
		}
		freename( tempvar2 );
	}

	return tempvar;
}

char *factor(int spaces)
{
	/*
		factor ->   NUM_OR_ID
				  | LP expression RP
	*/
	char *tempvar=NULL;
	print_spaces(spaces);
		fprintf(parse, "factor\n");
	spaces +=2;
	if( match(NUM_OR_ID) )
	{
	/* Print the assignment instruction. The %0.*s conversion is a form of
	 * %X.Ys, where X is the field width and Y is the maximum number of
	 * characters that will be printed (even if the string is longer). I'm
	 * using the %0.*s to print the string because it's not \0 terminated.
	 * The field has a default width of 0, but it will grow the size needed
	 * to print the string. The ".*" tells printf() to take the maximum-
	 * number-of-characters count from the next argument (yyleng).
	 */

		print_spaces(spaces);
		fprintf(parse, "NUM_OR_ID\n");
		fprintf(interFile, "%s = %.*s\n", tempvar = newname(), yyleng, yytext );
		int condition_check_digit=1;
		int loop_var=0;
		for(;loop_var<yyleng;loop_var++)
		{
			if(!isdigit(*(yytext+loop_var)))
			{
				condition_check_digit=0;
				break;
			}
		}
		if(condition_check_digit){
			fprintf(assFile, "MVI %c, %.*s\n",REG[tempvar[1]-'0'], yyleng,  yytext);
		} else {
			if(strcmp(tempvar, "t0") == 0){
				fprintf(assFile, "LDA _%.*s\n",yyleng,  yytext);
			} else {
				fprintf(assFile, "MOV H, A\nPUSH H\nLDA _%.*s\nMOV %c, A\nPOP H\nMOV A, H\n",yyleng,  yytext,REG[tempvar[1]-'0']);
			}
		}
		// char assignment[100];
		// int loop_var1=0;
		// for(;loop_var1<yyleng;loop_var1++)assignment[loop_var1]=*(yytext+loop_var1);
		// assignment[loop_var1]='\0';
		// if(!isdigit(assignment[0]))
	 //    {
	 //    		int f=0;
	 //    		for(int i=0;i<id_counter;i++)
	 //    		{
	 //    			if(strcmp(ids[i],assignment)==0)
	 //    			{
	 //    				f=1;
	 //    				break;
	 //    			}
	 //    		}
	 //    		if (f==0)
	 //    		{
	 //    			fprintf( stderr, "Unassigned Variable\n");
	 //    			exit(1);
	 //    		}
	 //    }
		advance();
	}
	else if( match(LP) )
	{
		print_spaces(spaces);
		fprintf(parse, "LP\n");
		advance();
		tempvar = expression(spaces);
		if( match(RP) )
		{
			print_spaces(spaces);
		    fprintf(parse, "RP\n");
			advance();
		}
		else
			fprintf(stderr, "%d: Mismatched parenthesis\n", yylineno );
	}
	else
	fprintf( stderr, "%d: Number or identifier expected\n", yylineno );

	return tempvar;
}

#include <stdarg.h>
#define MAXFIRST 16
#define SYNCH    SEMI

int legal_lookahead( int first_arg , ...)
{
	/* Simple error detection and recovery. Arguments are a 0-terminated list of
	 * those tokens that can legitimately come next in the input. If the list is
	 * empty, the end of file must come next. Print an error message if
	 * necessary. Error recovery is performed by discarding all input symbols
	 * until one that's in the input list is found
	 *
	 * Return true if there's no error or if we recovered from the error,
	 * false if we can't recover.
	 */

	va_list     args;
	int     tok;
	int     lookaheads[MAXFIRST], *p = lookaheads, *current;
	int     error_printed = 0;
	int     rval          = 0;

	va_start( args, first_arg );

	if( !first_arg )
	{
		if( match(EOI) )
			rval = 1;
	}
	else
	{
		*p++ = first_arg;
		while( (tok = va_arg(args, int)) && p < &lookaheads[MAXFIRST] )
			*p++ = tok;

		while( !match( SYNCH ) ) {
			for( current = lookaheads; current < p ; ++current )
			if( match( *current ) )
			{
				rval = 1;
				goto exit;
			}

			if( !error_printed ){
				fprintf( stderr, "Line %d: Syntax error\n", yylineno );
				error_printed = 1;
			}
			advance();
	   }
	}

exit:
	va_end( args );
	return rval;
}