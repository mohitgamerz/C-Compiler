#include "lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* yytext = ""; /* Lexeme (not '\0' terminated) */
int yyleng   = 0;  /* Lexeme length.               */
int yylineno = 0;  /* Input line number            */

FILE *lexemeFile;

int lex(void){

     static char input_buffer[1024];
     char        *current;

     current = yytext + yyleng; /* Skip current
                                                                 lexeme        */

     while(1){       /* Get the next one         */
            while(!*current ){
                 /* Get new lines, skipping any leading
                 * white space on the line,
                 * until a nonblank line is found.
                 */

                 current = input_buffer;
                 if(!gets(input_buffer)){
                        *current = '\0' ;

                        return EOI;
                 }
                 ++yylineno;
                 while(isspace(*current))
                        ++current;
            }
            for(; *current; ++current){
                 /* Get the next token */
                 yytext = current;
                 yyleng = 1;
                 switch( *current ){
                     case ';':
                        return SEMI;
                     case '+':
                        return PLUS;
                     case '-':
                        return MINUS;
                     case '*':
                        return TIMES;
                     case '/':
                        return DIV;
                     case '(':
                        return LP;
                     case ')':
                        return RP;
                     case '<':
                        return LT;
                     case '>':
                        return GT;
                        case ':':
                        return COL;
                     case '=':
                        return EQ;
                     case '\n':
                     case '\t':
                     case ' ' :
                        break;
                     default:
                        if(!isalnum(*current))
                             fprintf(stderr, "Not alphanumeric <%c>\n", *current);
                        else{
                         while(isalnum(*current))
                                ++current;
                         yyleng = current - yytext;
                         char subbuff[yyleng+1];
                         memcpy( subbuff, yytext, yyleng );
                         subbuff[yyleng] = '\0';
                         if(strcmp(subbuff, "if") == 0)
                         {
                                return IF;
                         }
                         else if(strcmp(subbuff, "then") == 0)
                         {
                                return THEN;
                         }
                         else if(strcmp(subbuff, "while") == 0)
                         {
                                return WHILE;
                         }
                         else if(strcmp(subbuff, "do") == 0)
                         {
                                return DO;
                         }
                         else if(strcmp(subbuff, "begin") == 0)
                         {
                                return BEGIN;
                         }
                         else if(strcmp(subbuff, "end") == 0)
                         {
                                return END;
                         }
                         return NUM_OR_ID;
                        }
                        break;
                 }
            }
     }
}


static int Lookahead = -1; /* Lookahead token  */

int match(int token){
     /* Return true if "token" matches the
            current lookahead symbol.                */

     if(Lookahead == -1)
            Lookahead = lex();

         if(token == ID && Lookahead == NUM_OR_ID)
        {
             int i;
             char *current = yytext;

             for(i=0;i<yyleng;i++)current++;
                while(isspace(*current))current++;
             if((*current)==':'){
                lexemeFile = fopen("Lexemes.txt", "a");
                if (lexemeFile != NULL) {
                    fprintf(lexemeFile,"<ID> ");
                    fclose(lexemeFile);
                }

                return 1;
            }
             return 0;
        }


    if( token == Lookahead)
    {
        lexemeFile = fopen("Lexemes.txt", "a");
        if (lexemeFile != NULL) {
            switch(token)
            {
                case(0):fprintf(lexemeFile,"<END OF INPUT> ");break;
                case(1):fprintf(lexemeFile,"<SEMI> ");break;
                case(2):fprintf(lexemeFile,"<PLUS> ");break;
                case(3):fprintf(lexemeFile,"<TIMES> ");break;
                case(4):fprintf(lexemeFile,"<MINUS> ");break;
                case(5):fprintf(lexemeFile,"<DIV> ");break;
                case(6):fprintf(lexemeFile,"<LP> ");break;
                case(7):fprintf(lexemeFile,"<RP> ");break;
                case(8):fprintf(lexemeFile,"<NUM_OR_ID> ");break;
                case(9):fprintf(lexemeFile,"<IF> ");break;
                case(10):fprintf(lexemeFile,"<THEN> ");break;
                case(11):fprintf(lexemeFile,"<WHILE> ");break;
                case(12):fprintf(lexemeFile,"<DO> ");break;
                case(13):fprintf(lexemeFile,"<BEGIN> ");break;
                case(14):fprintf(lexemeFile,"<END> ");break;
                case(15):fprintf(lexemeFile,"<GT> ");break;
                case(16):fprintf(lexemeFile,"<LT> ");break;
                case(17):fprintf(lexemeFile,"<EQ> ");break;
                case(18):fprintf(lexemeFile,"<COL> ");break;
                case(19):fprintf(lexemeFile,"<ID> ");break;
            }
            fclose(lexemeFile);
        }
        return 1;
    }else{
        return 0;
    }
}

void advance(void){
/* Advance the lookahead to the next
     input symbol.                               */

        Lookahead = lex();
}