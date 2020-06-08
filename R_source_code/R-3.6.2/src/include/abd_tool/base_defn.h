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
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
/*
    The needed constants to clear the code and make "things" more
    abstracted and easy to modify
*/
#ifndef loaded_base
#define loaded_base
typedef enum abd_state
{
    ABD_ENABLE = 1,
    ABD_DISABLE = 0
} ABD_STATE;

typedef enum abd_search
{
    ABD_EXIST = 1,
    ABD_NOT_EXIST = 0
} ABD_SEARCH;

static ABD_STATE watcherState = ABD_DISABLE;
static ABD_STATE verbose = ABD_DISABLE;
static int st = 0;
//constants
#define ABD_NOT_FOUND NULL
static SEXP cmp;
#endif

/*
    General tool methods prototypes are declared below
*/
void setWatcherState(ABD_STATE state);
void setVerboseMode(ABD_STATE state);
void abd_help();
void abd_start(SEXP rho);
void abd_stop();
ABD_STATE isRunning();
ABD_STATE isVerbose();

/* Functions used to register events */
void regVarChange(SEXP, SEXP, SEXP, SEXP);
ABD_SEARCH regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs, SEXP receivedArgs);
void regFunRet(SEXP lhs, SEXP rho, SEXP val);
void regVarIdxChange(SEXP call, SEXP rho);
void regIf(SEXP Stmt, Rboolean result, SEXP rho);
void regArith(SEXP call, SEXP ans, SEXP rho);
void storeIsWaitingIf(int isWaiting, SEXP rho);
/* Misc functions */
ABD_SEARCH checkToReg(SEXP rho);
void printEventReg();
void storeCompareResult(SEXP cmpr);
int cmpStoredArithAns(SEXP arg1, SEXP arg2);
void regVecCreation(SEXP call, SEXP vector, SEXP rho);
int getSt();
void storePossibleRet(SEXP promRet);
