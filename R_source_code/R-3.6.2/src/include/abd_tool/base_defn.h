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
    typedef enum abd_state{
        ABD_ENABLE = 1,
        ABD_DISABLE = 0
    }ABD_STATE;

    typedef enum abd_search{
        ABD_EXIST = 1,
        ABD_NOT_EXIST = 0
    }ABD_SEARCH;
    
  static ABD_STATE watcherState = ABD_DISABLE;
  static int st =0;
  //constants
  #define ABD_NOT_FOUND NULL
#endif


/*
    General tool methods prototypes are declared below
*/
void ABD_HELP();
void START_WATCHER();
void STOP_WATCHER();
ABD_STATE isRunning();
void regVarChange(SEXP, SEXP, SEXP);
ABD_SEARCH regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs, SEXP receivedArgs);
void regFunReturn(SEXP lhs, SEXP rho, SEXP val);
ABD_SEARCH checkToReg(SEXP rho);
void printEventReg();
void saveIdxChanges(int nIdxs, int * idxChanges);
