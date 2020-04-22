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
void abd_start()
{
    checkSettings();
    initObjsRegs();
    initEventsReg();
    initEnvStack();
    watcherState = ABD_ENABLE;
}

void abd_stop()
{
    if (isRunning())
    {
        checkSettings();
        watcherState = ABD_DISABLE;
        persistInformation();
    }
}
void abd_help()
{
    // printf("\n\n\t  \"Automatic\" Bug Detection (ABD) tool usage\n");
    // printf("\t##################################################\n");
    // printf("\t-> Start the watcher: abd_start()\n");
    // printf("\t-> Stop the watcher: abd_stop()\n");
    // printf("\t-> Set output file path: abd_setPath(\"your/path\")\n");
    // printf("\t-> Display current output file path: abd_path()\n");
    // printf("\t##################################################\n\n\n");
    //checkSettings();
    // cntx
    // SrcRefState ParseState;
    // ParseState = src
    // printf("At line... %d\n", ParseState.xxlineno);
    st = 1;
}

void prepVarIdxChange(SEXP var)
{
    prepForIdxChange(var);
}
int getCurrScriptLn()
{
    /* If we have a valid srcref, use it */
    SEXP srcref = R_Srcref;
    if (srcref && srcref != R_NilValue)
    {
        if (TYPEOF(srcref) == VECSXP)
            srcref = VECTOR_ELT(srcref, 0);
        SEXP srcfile = getAttrib(srcref, R_SrcfileSymbol);
        if (TYPEOF(srcfile) == ENVSXP)
        {
            SEXP filename = findVar(install("filename"), srcfile);
            if (isString(filename) && length(filename))
            {
                return asInteger(srcref);
            }
        }
    }
    /* default: */
    return 0;
}

void regVarIdxChange(SEXP indexes, SEXP newValues, SEXP rho)
{

    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && waitingIdxChange))
        return;

    storeIdxChange(indexes);
    printf("Index Changed at line %d\n", getCurrScriptLn());
    newObjUsage(R_NilValue, newValues, rho);
}

void regVarChange(SEXP lhs, SEXP rhs, SEXP rho)
{
    if (!(isRunning() && isEnvironment(rho) && (cmpToCurrEnv(rho) == ABD_EXIST)))
        return;

    printf("Assignment at line %d\n", getCurrScriptLn());
    newObjUsage(lhs, rhs, rho);
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

    printf("FunCall at line %d\n", getCurrScriptLn());
    createNewEvent(FUNC_EVENT);
    setFuncEventValues(objFound, newRho, passedArgs, receivedArgs);
    return ABD_EXIST;
}

void regIf(SEXP Stmt, Rboolean result, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    printf("If stmt at line %d\n", getCurrScriptLn());
    if (!isWaitingElseIf())
        createNewEvent(IF_EVENT);
    setIfEventValues(Stmt, result);
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
    lastRetValue = val;
    envPop();
}

ABD_STATE isRunning()
{
    return watcherState;
}

void storeCompareResult(SEXP cmpr)
{
    cmp = cmpr;
}
