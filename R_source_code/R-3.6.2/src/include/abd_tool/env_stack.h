
#include <abd_tool/env_stack_defn.h>
#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/event_manager_defn.h>

void initEnvStack(){
    envStack = (ABD_ENV_STACK *) malloc(sizeof(ABD_ENV_STACK));
    envStack->rho = R_GlobalEnv;
    envStack->prev = ABD_NOT_FOUND;
    envStack->funcObj = ABD_NOT_FOUND;
}

void freeEnv(ABD_ENV_STACK * env){
    free(env);
}

ABD_ENV_STACK * memAllocEnvStack(){
    return (ABD_ENV_STACK * ) malloc(sizeof(ABD_ENV_STACK));
}

void envPush(SEXP newRho, ABD_OBJECT * funcObj){
    ABD_ENV_STACK * newEnv = memAllocEnvStack();
    newEnv->rho = newRho;
    newEnv->funcObj = funcObj;
    newEnv->prev = envStack;
    envStack = newEnv;
}

void envPop(){
    ABD_ENV_STACK * elementToPop = envStack;
    envStack = envStack->prev;
    freeEnv(elementToPop);
}
SEXP getCurrentEnv(){
    return envStack->rho;
}
ABD_OBJECT * getCurrFuncObj(){
    return envStack->funcObj;
}
ABD_SEARCH cmpToCurrEnv(SEXP rho){
    if(envStack == ABD_ENV_NOT_FOUND)
        return ABD_NOT_EXIST;
    return (envStack->rho == rho) ? ABD_EXIST : ABD_NOT_EXIST;
}

