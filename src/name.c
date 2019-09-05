#include <stdio.h>
#include <stdlib.h>
#include "lex.h"

char  *Names[] = { "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7" };   
char  **Namep  = Names;   
int expressionLabelCounter = 0;
int ifthenLabelCounter = 0;
int loopLabelCounter = 0;

char  *newname()   
{   
    if( Namep >= &Names[ sizeof(Names)/sizeof(*Names) ] )   
    {   
        fprintf( stderr, "%d: Expression too complex\n", yylineno );   
        exit( 1 );   
    }   
   
    return( *Namep++ );   
}   
   
void freename(s)   
char    *s;   
{   
    if( Namep > Names )   
    *--Namep = s;   
    else   
    fprintf(stderr, "%d: (Internal error) Name stack underflow\n",   
                                yylineno );   
}   

int getCompareLabel(){
    return expressionLabelCounter++;
}

int getIfThenLabel(){
    return ifthenLabelCounter++;
}

int getLoopLabel(){
    return loopLabelCounter++;
}
