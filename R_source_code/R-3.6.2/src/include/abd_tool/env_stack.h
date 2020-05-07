
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
    envStack->funcObj = ABD_NOT_FOUND;
}

void freeEnv(ABD_ENV_STACK *env)
{
    free(env);
}

ABD_ENV_STACK *memAllocEnvStack()
{
    return (ABD_ENV_STACK *)malloc(sizeof(ABD_ENV_STACK));
}

void envPush(SEXP newRho, ABD_OBJECT *funcObj)
{
    ABD_ENV_STACK *newEnv = memAllocEnvStack();
    if (isVerbose())
    {
        puts("Env pushed to the stack...");
        printf("Old env: %s\n", envToStr(envStack->rho));
        printf("New env: %s\n", envToStr(newRho));
    }
    newEnv->rho = newRho;
    newEnv->funcObj = funcObj;
    newEnv->prev = envStack;
    envStack = newEnv;
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
    if (isVerbose())
    {
        puts("Env popped from the stack...");
        printf("Pop'ed env: %s\n", envToStr(envStack->rho));
        printf("Curr env: %s\n", envToStr(envStack->prev->rho));
    }
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
