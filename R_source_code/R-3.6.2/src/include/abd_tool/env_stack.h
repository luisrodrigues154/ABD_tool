
#include <abd_tool/env_stack_defn.h>
#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/event_manager_defn.h>

void initEnvStack(SEXP startingEnv)
{
    envStack = (ABD_ENV_STACK *)malloc(sizeof(ABD_ENV_STACK));
    envStack->rho = startingEnv;
    envStack->prev = ABD_NOT_FOUND;
    envStack->funcObj = ABD_OBJECT_NOT_FOUND;
    envStack->args = ABD_OBJECT_NOT_FOUND;
    envStack->onBranch = FALSE;
    envStack->branchDepth = 0;
    envStack->funCallRegged = FALSE;
    envStack->waitingIdxChange = 0;
    envStack->idxChanges = ABD_OBJECT_NOT_FOUND;
    initialEnv = startingEnv;
}

void clearIdxChanges()
{
    envStack->idxChanges = ABD_OBJECT_NOT_FOUND;
    envStack->waitingIdxChange = 0;
}

IDX_CHANGE *getCurrIdxChanges()
{
    return envStack->idxChanges;
}

int waitingIdxChange()
{
    return envStack->waitingIdxChange;
}

void decrementWaitingIdxChange()
{
    envStack->waitingIdxChange--;
}

void incrementWaitingIdxChange()
{
    envStack->waitingIdxChange++;
}

void initIdxChangeAuxVars()
{

    envStack->idxChanges = (IDX_CHANGE *)malloc(sizeof(IDX_CHANGE));
    envStack->idxChanges->src = R_NilValue;
    envStack->idxChanges->srcVec = 0;
    envStack->idxChanges->discard = 0;
    envStack->idxChanges->destIdxsVec = 0;
    envStack->idxChanges->srcIdxsVec = 0;
    envStack->idxChanges->srcValues = R_NilValue;
    envStack->idxChanges->srcIdxs = R_NilValue;
    envStack->idxChanges->destIdxs = R_NilValue;
    envStack->idxChanges->srcObj = ABD_OBJECT_NOT_FOUND;
}

SEXP getInitialEnv()
{
    return initialEnv;
}
void freeEnv(ABD_ENV_STACK *env)
{
    free(env);
}

void setOnBranch(Rboolean onBranch)
{
    envStack->onBranch = onBranch;
}

void setFunCallRegged(Rboolean state)
{
    envStack->funCallRegged = state;
}

Rboolean getFunCalledFlag()
{
    return envStack->funCallRegged;
}

Rboolean onBranch()
{
    return envStack->onBranch;
}

void incBranchDepth()
{
    envStack->branchDepth++;
}
void decBranchDepth()
{
    envStack->branchDepth--;
}
short getCurrBranchDepth()
{
    return envStack->branchDepth;
}

ABD_ENV_STACK *memAllocEnvStack()
{
    return (ABD_ENV_STACK *)malloc(sizeof(ABD_ENV_STACK));
}

void envPush(SEXP newRho, ABD_OBJECT *funcObj)
{
    ABD_ENV_STACK *newEnv = memAllocEnvStack();

    newEnv->rho = newRho;
    newEnv->funcObj = funcObj;
    newEnv->prev = envStack;
    envStack = newEnv;
    envStack->onBranch = FALSE;
    envStack->branchDepth = 0;
}
char *envToStr(SEXP rho)
{
    const void *vmax = vmaxget();
    static char ch[1000];
    if (rho == R_GlobalEnv)
        sprintf(ch, "GlobalEnv");
    else if (rho == R_BaseEnv)
        sprintf(ch, "BaseEnv");
    else if (rho == R_EmptyEnv)
        sprintf(ch, "EmptyEnv");
    else if (R_IsPackageEnv(rho))
        snprintf(ch, 1000, "%s", translateChar(STRING_ELT(R_PackageEnvName(rho), 0)));
    else
        snprintf(ch, 1000, "%p", (void *)rho);

    return ch;
}
void envPop()
{
    ABD_ENV_STACK *elementToPop = envStack;
    envStack = envStack->prev;
    freeEnv(elementToPop);
}
SEXP getCurrentEnv()
{
    return envStack->rho;
}
ABD_OBJECT *getCurrFuncObj()
{
    return envStack->funcObj;
}
ABD_SEARCH cmpToCurrEnv(SEXP rho)
{
    if (envStack == ABD_ENV_NOT_FOUND)
        return ABD_NOT_EXIST;
    return (envStack->rho == rho) ? ABD_EXIST : ABD_NOT_EXIST;
}
