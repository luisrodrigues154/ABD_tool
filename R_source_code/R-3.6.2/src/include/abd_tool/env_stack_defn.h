#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/obj_manager_defn.h>

#ifndef loaded_stack
    #define loaded_stack
       
    typedef struct abd_env_stack{
        SEXP rho;
        ABD_OBJECT * funcObj;
        struct abd_env_stack * prev;
    }ABD_ENV_STACK;

    //control the function calls and restrict defineVars() that are not intended to be registered
    static ABD_ENV_STACK * envStack;

    //constants
    #define ABD_ENV_NOT_FOUND NULL
#endif


void initEnvStack();
void freeEnv(ABD_ENV_STACK * env);
ABD_ENV_STACK * memAllocEnvStack();
void envPush(SEXP newRho, ABD_OBJECT * funcObj);
void envPop();
ABD_SEARCH cmpToCurrEnv(SEXP rho);
SEXP getCurrentEnv();
ABD_OBJECT * getCurrFuncObj();