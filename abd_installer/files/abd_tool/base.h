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

void storePossibleRet(SEXP value)
{
    storeRetValues(value);
}
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
    R_jit_enabled = 0;
    checkSettings();
    initObjsRegs();
    initEnvStack(rho);
    initEventsReg();
    setWatcherState(ABD_ENABLE);
    setVerboseMode(ABD_ENABLE);
}

void abd_stop()
{
    if (isRunning())
    {
        checkSettings();
        //checkPendings(R_NilValue, R_NilValue, ABD_OBJECT_NOT_FOUND);
        setWatcherState(ABD_DISABLE);
        persistInformation();
        //open the browser with the displayer
        //int ret = system(getCommand());
    }
}

void abd_help()
{
    checkSettings();
    printf("\n\n\t  \"Automatic\" Bug Detection (ABD) tool usage\n");
    printf("\t##################################################\n");
    printf("\t-> Start the watcher: abd_start()\n");
    printf("\t-> Stop the watcher: abd_stop()\n");
    printf("\t-> Set output file path: abd_setPath(\"your/path\")\n");
    printf("\t-> Display current output file path: abd_path()\n");
    printf("\t##################################################\n\n\n");
}

void PrintDaCall(SEXP call, SEXP rho)
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
void finalizeVarIdxChange(SEXP result, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    //printf("finalize idx change... line %d\n", getCurrScriptLn());

    processVarIdxChange(result);
    // if (inLoopEvent())
    //     addEventToForIteration(eventsRegTail);
}
void regVarIdxChange(SEXP call, SEXP rho)
{

    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    //printf("var index change... line %d\n", getCurrScriptLn());
    initIdxChangeAuxVars();
    //if matrix, toObj will remain langsxp, just a pair of brackets more
    getCurrIdxChanges()->dest = CAR(CDR(CAR(CDR(call))));

    //pre-process
    preProcessVarIdxChange(call, rho);

    /*
        if they are all 0, need to process now, otherwise
        just wait for processIndexChanges() being triggered by the reaching vectors
    */

    //now wait ...
}

void regDataFrameCreation(SEXP call, SEXP rho)
{
    if (!(isRunning() && (cmpToCurrEnv(rho) == ABD_EXIST)))
        return;
    frameCall = call;
    preProcessDataFrame(CDR(call));
}

void regVarChange(SEXP call, SEXP lhs, SEXP rhs, SEXP rho)
{

    if (!(isRunning() && isEnvironment(rho) && (cmpToCurrEnv(rho) == ABD_EXIST)))
        return;
    //printf("var change... line %d\n", getCurrScriptLn());
    // puts("rhs2 V");
    // PrintDaCall(rhs2, getCurrentEnv());
    //print f("typeof lhs %d\n", TYPEOF(rhs));
    /* store the new information for the object */
    ABD_OBJECT *objUsed = newObjUsage(lhs, rhs, rho);

    if (inLoopByType(ABD_FOR) && call == R_NilValue && loopStack->loop.forLoop->iterator == ABD_OBJECT_NOT_FOUND)
        loopStack->loop.forLoop->iterator = objUsed;

    if (TYPEOF(rhs) != CLOSXP)
    {
        //need to extract the rhs from the call
        ABD_ASSIGN_EVENT *currAssign = ABD_EVENT_NOT_FOUND;
        SEXP rhs2 = CAR(CDR(CDR(call)));
        createAsgnEvent(objUsed, rhs, rhs2, rho);
    }
    clearPendingVars();
}

void decrementBranchDepth(SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) && onBranch()))
        return;
    decBranchDepth();
    if (!getCurrBranchDepth())
        setOnBranch(FALSE);
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

void regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs, SEXP receivedArgs)
{

    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    //printf("func call... line %d\n", getCurrScriptLn());
    ABD_OBJECT *objFound = findFuncObj(CHAR(PRINTNAME(lhs)), rho);

    if (objFound == ABD_OBJECT_NOT_FOUND)
        return;

    createNewEvent(FUNC_EVENT);
    setFuncEventValues(objFound, newRho, passedArgs, receivedArgs);
    setFunCallRegged(TRUE);
    // if (inLoopEvent())
    //     addEventToForIteration(eventsRegTail);
}
Rboolean isFunCallRegged()
{
    if (!isRunning())
        return FALSE;
    return getFunCalledFlag();
}

void regVecCreation(SEXP call, SEXP vector, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    // puts("received vec");
    // PrintDaCall(vector, rho);

    if (waitingFrameVecs)
    {
        storeVecDataFrameEvent(vector);
        return;
    }
    if (waitingForVecs)
    {
        storeVecForEvent(vector);

        if (!waitingForVecs)
            finalizeForEventProcessing();
        return;
    }

    if (waitingIdxChange())
    {
        if (toDiscard())
        {
            decrementWaitingIdxChange();
            return;
        }
        storeVecForIdxChange(vector);
        return;
    }
    checkPendings(R_NilValue, R_NilValue, ABD_OBJECT_NOT_FOUND);
    storeNewVecValues(vector, call);
}

void regIf(SEXP Stmt, Rboolean result, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    //printf("if statement ... line %d\n", getCurrScriptLn());
    createNewEvent(IF_EVENT);
    setIfEventValues(Stmt, result);
    clearPendingVars();
}

/* FOR LOOP CALLABLES*/
void regForLoopStart(SEXP call, SEXP enumerator, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    clearPendingVars();
    ABD_FOR_LOOP_EVENT *newForLoopEvent = createNewEvent(FOR_EVENT)->data.for_loop_event;
    setForEventValues(call, newForLoopEvent, enumerator);
}
void regForLoopIteration(int iterId, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_FOR)))
        return;

    createNewLoopIteration(iterId, ABD_FOR);
}

void regForLoopFinish(SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_FOR)))
        return;

    appendLastEventToLoop(ABD_FOR);
    popLoopFromStack(ABD_FOR);
}

/* REPEAT LOOP CALLABLES*/

void regRepeatLoopStart(SEXP call, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    ABD_REPEAT_LOOP_EVENT *newRepeatLoopEvent = createNewEvent(REPEAT_EVENT)->data.repeat_loop_event;
    pushNewLoop(ABD_REPEAT, newRepeatLoopEvent);
}

void regRepeatLoopIteration(int iterId, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_REPEAT)))
        return;

    createNewLoopIteration(iterId, ABD_REPEAT);
}

void regRepeatLoopFinish(SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_REPEAT)))
        return;

    appendLastEventToLoop(ABD_REPEAT);
    popLoopFromStack(ABD_REPEAT);
}

/* WHILE LOOP CALLABLES*/

void regWhileLoopStart(SEXP args, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    R_PrintData pars;
    ABD_WHILE_LOOP_EVENT *newWhileLoopEvent = createNewEvent(WHILE_EVENT)->data.while_loop_event;
    pushNewLoop(ABD_WHILE, newWhileLoopEvent);
}

void regWhileLoopCondition(SEXP stmt, Rboolean result, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_WHILE)))
        return;

    createNewEvent(IF_EVENT);
    setIfEventValues2(stmt, result);

    if (loopStack->loop.whileLoop->cndtStr == ABD_NOT_FOUND)
        loopStack->loop.whileLoop->cndtStr = eventsRegTail->data.if_event->exprStr;

    clearPendingVars();
}

void regWhileLoopIteration(int iterId, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_WHILE)))
        return;

    createNewLoopIteration(iterId, ABD_WHILE);
    verifyBranchDepthIntegrity();
}

void regWhileLoopFinish(SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_WHILE)))
        return;

    appendLastEventToLoop(ABD_WHILE);
    popLoopFromStack(ABD_WHILE);
}

/* LOOP misc*/
void doLoopJump(ABD_LOOP_TAGS jumpType, ABD_LOOP_TAGS requestingLoopType, SEXP rho)
{
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(requestingLoopType)))
        return;
    if (jumpType == ABD_NEXT)
        createNewEvent(NEXT_EVENT);
    else
        createNewEvent(BREAK_EVENT);
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
    //printf("return... line %d\n", getCurrScriptLn());
    createNewEvent(RET_EVENT);
    setRetEventValue(val);
    // if (inLoopEvent())
    //     addEventToForIteration(eventsRegTail);
}

ABD_STATE isRunning()
{
    return watcherState;
}
ABD_STATE isVerbose()
{
    return verbose;
}
