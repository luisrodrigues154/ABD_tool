#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/obj_manager_defn.h>

#ifndef loaded_stack
#define loaded_stack

/* struct to manage idx changes*/

typedef struct
{
    int srcVec, destIdxsVec, srcIdxsVec, discard;
    int nIdxChanges;
    SEXP srcValues, srcIdxs, destIdxs;
    SEXP src, dest;
    ABD_OBJECT *destObj;
    ABD_OBJECT *srcObj;
} IDX_CHANGE;

typedef struct abd_env_stack
{
    SEXP rho;
    ABD_OBJECT *funcObj;
    ABD_EVENT_ARG *args;
    short branchDepth;
    Rboolean funCallRegged;
    Rboolean onBranch;
    int waitingIdxChange;
    IDX_CHANGE *idxChanges;
    struct abd_env_stack *prev;
} ABD_ENV_STACK;

//control the function calls and restrict defineVars() that are not intended to be registered
static ABD_ENV_STACK *envStack;
static SEXP initialEnv;
//constants
#define ABD_ENV_NOT_FOUND NULL
#endif
SEXP getInitialEnv();
void initEnvStack(SEXP rho);
void freeEnv(ABD_ENV_STACK *env);
ABD_ENV_STACK *memAllocEnvStack();
void envPush(SEXP newRho, ABD_OBJECT *funcObj);
void envPop();
ABD_SEARCH cmpToCurrEnv(SEXP rho);
SEXP getCurrentEnv();
ABD_OBJECT *getCurrFuncObj();
char *envToStr(SEXP rho);
void setOnBranch(Rboolean onBranch);
Rboolean onBranch();
void incBranchDepth();
void decBranchDepth();
short getCurrBranchDepth();
IDX_CHANGE *getCurrIdxChanges();
int waitingIdxChange();
void decrementWaitingIdxChange();
void incrementWaitingIdxChange();
void initIdxChangeAuxVars();
void clearIdxChanges();