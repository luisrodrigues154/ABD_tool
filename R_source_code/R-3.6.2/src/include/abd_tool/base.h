#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/obj_manager.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/event_manager.h>
#include <abd_tool/json_helpers_defn.h>
#include <abd_tool/json_helpers.h>
#include <abd_tool/env_stack.h>
#include <abd_tool/env_stack_defn.h>
#include <abd_tool/settings_manager_defn.h>
#include <Print.h>
#include <Defn.h>
#include <math.h>

/*
    Methods to start and stop the tool, helper function also here

    -> start method:
        # start the information collection
        # Wipe the objectsRegistry

    -> stop method:
        # stop the information collection
        # save the objectsRegistry to a JSON with path specified at
            File: json_helpers_defn.h
            Constant: OBJECTS_FILE_PATH
        # wipe all the ABD_OBJECT_MOD list for all the ABD_OBJECT's saved DLL
*/
void setWatcherState(ABD_STATE state)
{
    watcherState = state;
}
void setVerboseMode(ABD_STATE state)
{
    verbose = state;
}
void abd_start(SEXP rho)
{
    checkSettings();
    initObjsRegs();
    initEnvStack(rho);
    initEventsReg();
    setWatcherState(ABD_ENABLE);
    setVerboseMode(ABD_ENABLE);
}
void mkCopiesToDisplayer()
{
}

void abd_stop()
{
    if (isRunning())
    {
        checkSettings();
        setWatcherState(ABD_DISABLE);
        persistInformation();

        //open the browser
        system(getCommand());
    }
}

void abd_help()
{
    //checkSettings();
    // printf("\n\n\t  \"Automatic\" Bug Detection (ABD) tool usage\n");
    // printf("\t##################################################\n");
    // printf("\t-> Start the watcher: abd_start()\n");
    // printf("\t-> Stop the watcher: abd_stop()\n");
    // printf("\t-> Set output file path: abd_setPath(\"your/path\")\n");
    // printf("\t-> Display current output file path: abd_path()\n");
    // printf("\t##################################################\n\n\n");
    //
}

void prepVarIdxChange(SEXP var)
{
    prepForIdxChange(var);
}

void regVarIdxChange(SEXP indexes, SEXP newValues, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && waitingIdxChange))
        return;

    storeIdxChange(indexes);
    printf("Index Changed at line %d\n", getCurrScriptLn());
    newObjUsage(R_NilValue, newValues, rho);
}
static void PrintDaCall(SEXP call, SEXP rho)
{
    int old_bl = R_BrowseLines,
        blines = asInteger(GetOption1(install("deparse.max.lines")));
    if (blines != NA_INTEGER && blines > 0)
        R_BrowseLines = blines;

    R_PrintData pars;
    PrintInit(&pars, rho);
    PrintValueRec(call, &pars);

    R_BrowseLines = old_bl;
}

void regVarChange(SEXP call, SEXP lhs, SEXP rhs, SEXP rho)
{

    if (!(isRunning() && isEnvironment(rho) && (cmpToCurrEnv(rho) == ABD_EXIST)))
        return;
    //need to extract the rhs from the call
    ABD_ASSIGN_EVENT *currAssign = ABD_EVENT_NOT_FOUND;
    SEXP rhs2 = CAR(CDR(CDR(call)));
    // puts("rhs2 V");
    // PrintDaCall(rhs2, getCurrentEnv());

    /* store the new information for the object */
    ABD_OBJECT *objUsed = newObjUsage(lhs, rhs, rho);

    createAsgnEvent(objUsed, rhs, rhs2, rho);

    clearPendingVars();
}

/*
    The function below will verify if R is trying to execute a closure defined by the user.
    This verification is done because if the user did not declared an object with that symbol
    name, then, the function being called should not be tracked.
*/
ABD_SEARCH checkToReg(SEXP rho)
{
    if (!isRunning())
        return ABD_NOT_EXIST;
    return cmpToCurrEnv(rho);
}

ABD_SEARCH regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs, SEXP receivedArgs)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return ABD_NOT_EXIST;

    ABD_OBJECT *objFound = findFuncObj(CHAR(PRINTNAME(lhs)), rho);
    if (objFound == ABD_OBJECT_NOT_FOUND)
        return ABD_NOT_EXIST;

    //printf("FunCall at line %d\n", getCurrScriptLn());

    createNewEvent(FUNC_EVENT);
    setFuncEventValues(objFound, newRho, passedArgs, receivedArgs);

    return ABD_EXIST;
}

void regVecCreation(SEXP vector, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    storeNewVecValues(vector);
}

void regIf(SEXP Stmt, Rboolean result, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    //printf("If stmt at line %d\n", getCurrScriptLn());
    if (!isWaitingElseIf())
        createNewEvent(IF_EVENT);
    setIfEventValues(Stmt, result);
}

void regArith(SEXP call, SEXP ans, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    tmpStoreArith(call, ans);
}

void storeIsWaitingIf(int isWaiting, SEXP rho)
{
    if (isRunning() && cmpToCurrEnv(rho) == ABD_EXIST)
        setIsWaitingIf(isWaiting);
}

void regFunRet(SEXP lhs, SEXP rho, SEXP val)
{
    createNewEvent(RET_EVENT);
    setRetEventValue(val);
    envPop();
}

ABD_STATE isRunning()
{
    return watcherState;
}
ABD_STATE isVerbose()
{
    return verbose;
}

void printFormatedString(const char *strToDisplay)
{
}

void storeCompareResult(SEXP cmpr)
{
    puts("Storing");
    cmp = cmpr;
}

int getSt()
{
    return st;
}
int cmpStoredArithAns(SEXP arg1, SEXP arg2)
{
    if (arg1 == getSavedArithAns() || arg2 == getSavedArithAns())
        return 1;

    return 0;
}