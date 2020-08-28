#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <Defn.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/warns_errs_manager_defn.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/env_stack_defn.h>
#include <Rembedded.h>
#include <R_ext/Parse.h>

void initVars()
{
    currError = ABD_NOT_FOUND;
    currWarning = ABD_NOT_FOUND;
    warnCount = 0;
}
int getWarnCount()
{
    return warnCount;
}

void storeWarningSignal(const char *message)
{
    if (currWarning == ABD_NOT_FOUND)
    {
        currWarning = memAllocWarning();
        currWarning->prevWarning = ABD_NOT_FOUND;
    }
    else
    {
        ABD_WARNINGS *newWarning = memAllocWarning();
        newWarning->prevWarning = currWarning;
        currWarning = newWarning;
    }
    int msgLen = strlen(message);
    currWarning->message = memAllocForString(msgLen);
    copyStr(currWarning->message, message, msgLen);
    currWarning->id = ++warnCount;
}
ABD_WARNINGS *memAllocWarning()
{
    //
    return (ABD_WARNINGS *)malloc(sizeof(ABD_WARNINGS));
}
void clearWarnings()
{
    currWarning = ABD_NOT_FOUND;
}

ABD_WARNINGS *getWarnings()
{
    //
    ABD_WARNINGS *toRet = currWarning;
    clearWarnings();
    return toRet;
}

void storeErrorSignal(SEXP call, char *message)
{
    //
    currError = memAllocError();
    R_PrintData pars;
    int msgLen = strlen(message);
    currError->atFunc = getCurrFuncObj();
    currError->atEnv = getCurrentEnv();

    if (call != R_NilValue)
    {
        PrintInit(&pars, getCurrentEnv());
        currError->exprStr = getStrForStatement(call, &pars);
    }
    else
    {
        int exprStrLen = strlen(ABD_ERR_DEF);
        currError->exprStr = memAllocForString(exprStrLen);
        copyStr(currError->exprStr, ABD_ERR_DEF, exprStrLen);
    }

    currError->line = getCurrScriptLn();
    currError->message = memAllocForString(msgLen);
    copyStr(currError->message, message, msgLen);
}
ABD_ERRORS *getError()
{
    //
    return currError;
}
ABD_ERRORS *memAllocError()
{
    //
    return (ABD_ERRORS *)malloc(sizeof(ABD_ERRORS));
}