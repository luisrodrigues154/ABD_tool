/*
    In this file are declared the methods definitions as well as needed macros
    The implementation of the below declared methods is implemented at
    the file: 
        base.h



*/

/* 
    The needed libraries to this work
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    The needed constants to clear the code and make "things" more
    abstracted and easy to modify
*/






/*
    General tool methods prototypes are declared below
*/

void ABD_HELP();
void START_WATCHER();
void STOP_WATCHER();
void regVarChange(int, SEXP, SEXP, SEXP);
void verAndReg(SEXP lhs, SEXP rho);
void regCodeFlowChange();
