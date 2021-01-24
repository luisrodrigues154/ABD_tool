
#include <abd_tool/env_stack_defn.h>
#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/event_manager_defn.h>

void clearAllVars()
{

    envStack->args = ABD_OBJECT_NOT_FOUND;
    envStack->onBranch = FALSE;
    envStack->branchDepth = 0;
    envStack->funCallRegged = FALSE;
    envStack->waitingIdxChange = 0;
    envStack->waitingCellChange = 0;
    envStack->idxChanges = ABD_OBJECT_NOT_FOUND;
    envStack->cellChanges = ABD_OBJECT_NOT_FOUND;
    envStack->onTmp = FALSE;
    envStack->tmpStore = R_NilValue;
}

void initEnvStack(SEXP startingEnv)
{
    envStack = (ABD_ENV_STACK *)malloc(sizeof(ABD_ENV_STACK));
    envStack->rho = startingEnv;
    clearAllVars();
    envStack->funcObj = ABD_OBJECT_NOT_FOUND;
    envStack->prev = ABD_NOT_FOUND;
    initialEnv = startingEnv;
}

void clearIdxChanges()
{
    if (envStack->idxChanges != ABD_OBJECT_NOT_FOUND)
        free(envStack->idxChanges);
    envStack->idxChanges = ABD_OBJECT_NOT_FOUND;
    envStack->waitingIdxChange = 0;
}

void clearCellChanges()
{
    if (envStack->cellChanges != ABD_OBJECT_NOT_FOUND)
        free(envStack->cellChanges);
    envStack->cellChanges = ABD_OBJECT_NOT_FOUND;
    envStack->waitingCellChange = 0;
}

IDX_CHANGE *getCurrIdxChanges()
{
    if (envStack == ABD_NOT_FOUND)
        return ABD_NOT_FOUND;
    return envStack->idxChanges;
}

CELL_CHANGE *getCurrCellChange()
{
    if (envStack == ABD_NOT_FOUND)
        return ABD_NOT_FOUND;
    return envStack->cellChanges;
}
int waitingIdxChange()
{
    return envStack->waitingIdxChange;
}
int waitingCellChange()
{
    return envStack->waitingCellChange;
}

void decrementWaitingIdxChange()
{
    envStack->waitingIdxChange--;
}

void incrementWaitingIdxChange()
{
    envStack->waitingIdxChange++;
}

void incrementWaitingCellChange()
{
    envStack->waitingCellChange++;
}

void decrementWaitingCellChange()
{
    envStack->waitingCellChange--;
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

void initCellChangeAuxVars()
{
    envStack->cellChanges = (CELL_CHANGE *)malloc(sizeof(CELL_CHANGE));

    //init dest vars
    envStack->cellChanges->waitingRowsVec = FALSE;
    envStack->cellChanges->waitingColsVec = FALSE;
    envStack->cellChanges->toRows = R_NilValue;
    envStack->cellChanges->toCols = R_NilValue;
    envStack->cellChanges->nRows = 0;
    envStack->cellChanges->nCols = 0;

    //init src vars
    envStack->cellChanges->waitingSrcValues = FALSE;
    envStack->cellChanges->waitingSrcRows = FALSE;
    envStack->cellChanges->waitingSrcCols = FALSE;
    envStack->cellChanges->srcValues = R_NilValue;
    envStack->cellChanges->srcRows = R_NilValue;
    envStack->cellChanges->srcCols = R_NilValue;
    envStack->cellChanges->srcSexpObj = R_NilValue;
    envStack->cellChanges->srcNRows = 0;
    envStack->cellChanges->srcNCols = 0;

    envStack->cellChanges->targetObj = ABD_OBJECT_NOT_FOUND;
    envStack->cellChanges->targetCol = ABD_OBJECT_NOT_FOUND;
    envStack->cellChanges->srcObj = ABD_OBJECT_NOT_FOUND;
    envStack->cellChanges->nCellChanges = 0;
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

void forceBranchDepth(short value)
{
    envStack->branchDepth = value;
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

void tmpSwapEnv(SEXP tmpEnv)
{
    envStack->tmpStore = envStack->rho;
    envStack->rho = tmpEnv;
    envStack->onTmp = TRUE;
}

void popTmpEnv()
{
    if (envStack->onTmp)
    {
        envStack->rho = envStack->tmpStore;
        envStack->onTmp = FALSE;
    }
}

void envPush(SEXP newRho, ABD_OBJECT *funcObj)
{
    ABD_ENV_STACK *newEnv = memAllocEnvStack();

    newEnv->rho = newRho;
    newEnv->funcObj = funcObj;
    newEnv->prev = envStack;
    envStack = newEnv;
    clearAllVars();
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
