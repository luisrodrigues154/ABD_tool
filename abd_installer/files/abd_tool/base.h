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

void storePossibleRet(SEXP value) {
    storeRetValues(value);
}
void setWatcherState(ABD_STATE state) {
    watcherState = state;
}
void abd_verbose(SEXP option) {
    if (TYPEOF(CAR(option)) != REALSXP && TYPEOF(CAR(option)) != NILSXP) {
        messagePrinter("Invalid arguments");
        return;
    }
    ABD_STATE state;
    if (TYPEOF(CAR(option)) != NILSXP) {
        state = (int)REAL(CAR(option))[0];
        if (state != ABD_ENABLE && state != ABD_DISABLE) {
            messagePrinter("Invalid arguments");
            return;
        }
        updateVerboseMode(state);
    }
    else
        state = useVerbose();

    state == ABD_ENABLE ? messagePrinter("Verbose mode ENABLED!") : messagePrinter("Verbose mode DISABLED!");
}
void abd_set_launch(SEXP state) {
    if (TYPEOF(CAR(state)) != REALSXP) {
        messagePrinter("Invalid arguments");
        return;
    }

    int option = (int)REAL(CAR(state))[0];
    if (option != 0 && option != 1) {
        messagePrinter("Invalid arguments");
        return;
    }
    checkSettings();
    setLaunchOption(option);
}
void abd_start(SEXP rho) {
    R_jit_enabled = 0;
    checkSettings();
    initObjsRegs();
    initEnvStack(rho);
    initEventsReg();
    setWatcherState(ABD_ENABLE);
}

void messagePrinter(char *message) {
    printf("\n\t[ABD_TOOL] %s\n", message);
}

void persistAndDisplay(Rboolean useSettings) {
    checkSettings();
    // checkPendings(R_NilValue, R_NilValue, ABD_OBJECT_NOT_FOUND);
    if (useSettings)
        setWatcherState(ABD_DISABLE);
    persistInformation();
    // open the browser with the displayer
    int ret = 0;
    ABD_STATE displayNow = ABD_ENABLE;
    if (useSettings)
        displayNow = launchOnStop();
    if (displayNow == ABD_ENABLE)
        ret = system(getCommand());
}

void abd_stop() {
    if (isRunning())
        persistAndDisplay(TRUE);
}

void abd_path() {
    checkSettings();
    messagePrinter(getFolderPath());
}
void abd_set_path(SEXP args) {
    checkSettings();
    SEXP path = CAR(args);
    SEXP target = CAR(CDR(args));
    if (TYPEOF(path) != STRSXP || TYPEOF(target) != REALSXP) {
        messagePrinter("Invalid Arguments");
        return;
    }
    double target_file = REAL(target)[0];
    const char *path_ = CHAR(STRING_ELT(path, 0));
    if (!checkPath(path_) ||
        (target_file != 0 && target_file != 1 && target_file != 2)) {
        messagePrinter("Invalid arguments (absolute path?)");
        return;
    }

    if (strlen(path_) > 200) {
        messagePrinter("Path exceeds size limit [200]");
        return;
    }
    saveNewPath(path_, (int)target_file);
}
void abd_clear() {
    forceDefaults();
    messagePrinter("Default settings Loaded");
}
Rboolean abd_display() {
    if (!isRunning())
        return FALSE;
    persistAndDisplay(FALSE);
    return TRUE;
}
void printForVerbose(char * message) {
    if (useVerbose())
        messagePrinter(message);
}
void abd_help() {
    checkSettings();
    printf("\n\n\t  \"Automatic\" Bug Detection (ABD) tool usage\n");
    printf("\t##################################################\n");
    printf("\t1 -> Start the watcher: abd_start()\n"); // done
    printf("\t2 -> Stop the watcher: abd_stop()\n");   // done
    printf("\t3 -> Launch displayer on abd_stop: abd_stop(0/1) [0 - No, (D) 1 - "
        "Yes] \n"); // done
    printf("\t4 -> Set output file path: abd_path(\"new/path\", 0/1/2) [0 - "
        "objects, 1 - events, 2 - both]\n");                      // done
    printf("\t5 -> Display current output file path: abd_path()\n"); // done
    printf("\t6 -> Set verbose mode: abd_verbose(0/1) [(D) 0 - No, 1 - Yes]\n");
    printf("\t7 -> Launch displayer at current state (execution hangs until "
        "input): abd_display()\n"); // done
    printf("\t8 -> Load Default settings: abd_clear()\n");
    printf("\t##################################################\n");
    printf("\tNOTES:\n");
    printf("\t\t* Paths must be absolute\n");
    printf("\t\t* Options with (D) indicates default\n");
    printf("\t\t* Settings [3,4,6] are saved for next launches\n");
    printf("\t##################################################\n\n\n");
}

void PrintDaCall(SEXP call, SEXP rho) {
    int old_bl = R_BrowseLines,
        blines = asInteger(GetOption1(install("deparse.max.lines")));
    if (blines != NA_INTEGER && blines > 0)
        R_BrowseLines = blines;

    R_PrintData pars;
    PrintInit(&pars, rho);
    PrintValueRec(call, &pars);

    R_BrowseLines = old_bl;
}

void finalizeVarIdxChange(SEXP result, SEXP rho) {

    if (!isRunning())
        return;

    //if not changes regged
    if (getCurrIdxChanges() == ABD_OBJECT_NOT_FOUND && getCurrCellChange() == ABD_OBJECT_NOT_FOUND)
        return;
    Rboolean isCellChange = FALSE;
    if (cmpToCurrEnv(rho) == ABD_NOT_EXIST)
        /* the env does not match, this does not return right away because when
        assigning a cell to a dataframe
        using df[x,y] the rho passed is not the same as the one created (which is
        strange). This tries to mitigate that problem. */
        if (getCurrCellChange() != ABD_NOT_FOUND)
            isCellChange = TRUE;
        else if (getCurrIdxChanges() != ABD_NOT_FOUND)
            isCellChange = FALSE;
        else
            /* The env does not match, and has no pending idx/cell changes */
            return;

    if (getCurrCellChange() != ABD_NOT_FOUND)
        isCellChange = TRUE;
    else
        isCellChange = FALSE;

    if (isCellChange) {
        processVarCellChange(result);
        printForVerbose("Cell change processed");
    }
    else {
        processVarIdxChange(result);
        printForVerbose("Index change processed");
    }
}

void regVarIdxChange(SEXP call, SEXP rho) {

    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    Rboolean isDataFrame = FALSE;

    // printf("var index change... line %d\n", getCurrScriptLn());

    SEXP obj_sexp = CAR(CDR(CAR(CDR(call))));
    ABD_OBJECT *obj_ptr = ABD_OBJECT_NOT_FOUND;
    if (TYPEOF(obj_sexp) == LANGSXP) {
        obj_sexp = CAR(CDR(obj_sexp));
        const char *obj_name = CHAR(PRINTNAME(obj_sexp));
        obj_ptr = findCmnObj(obj_name, rho);
    }
    else {
        const char *obj_name = CHAR(PRINTNAME(obj_sexp));
        obj_ptr = findCmnObj(obj_name, rho);
    }
    if (obj_ptr == ABD_OBJECT_NOT_FOUND)
        return;


    if (obj_ptr->modList->valueType == ABD_FRAME)
        isDataFrame = TRUE;

    if (isDataFrame) {
        preProcessDataFrameCellChange(call, obj_ptr, rho);
        printForVerbose("Cell change detected");
    }
    else {
        preProcessVarIdxChange(call, obj_ptr, rho);
        printForVerbose("Index change detected");
    }

    // now wait ...
}

void regDataFrameCreation(SEXP call, SEXP rho) {
    if (!(isRunning() && (cmpToCurrEnv(rho) == ABD_EXIST)))
        return;
    frameCall = call;
    preProcessDataFrame(CDR(call));
    printForVerbose("Data Frame creation detected");
}

void regVarChange(SEXP call, SEXP lhs, SEXP rhs, SEXP rho) {

    if (!(isRunning() && isEnvironment(rho) && (cmpToCurrEnv(rho) == ABD_EXIST)))
        return;

    ABD_OBJECT *objUsed = newObjUsage(lhs, rhs, rho);

    if (inLoopByType(ABD_FOR) && call == R_NilValue &&
        loopStack->loop.forLoop->iterator == ABD_OBJECT_NOT_FOUND)
        loopStack->loop.forLoop->iterator = objUsed;

    if (TYPEOF(rhs) != CLOSXP) {
        // need to extract the rhs from the call
        ABD_ASSIGN_EVENT *currAssign = ABD_EVENT_NOT_FOUND;
        SEXP rhs2 = CAR(CDR(CDR(call)));
        createAsgnEvent(objUsed, rhs, rhs2, rho);
    }
    printForVerbose("New object usage detected");
    clearPendingVars();
}

void decrementBranchDepth(SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) && onBranch()))
        return;
    decBranchDepth();
    if (!getCurrBranchDepth())
        setOnBranch(FALSE);
}

/*
    The function below will verify if R is trying to execute a closure defined
   by the user. This verification is done because if the user did not declared
   an object with that symbol name, then, the function being called should not
   be tracked.
*/
ABD_SEARCH checkToReg(SEXP rho) {
    if (!isRunning())
        return ABD_NOT_EXIST;
    return cmpToCurrEnv(rho);
}

void regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs,
    SEXP receivedArgs) {

    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    // printf("func call... line %d\n", getCurrScriptLn());
    ABD_OBJECT *objFound = findFuncObj(CHAR(PRINTNAME(lhs)), rho);

    if (objFound == ABD_OBJECT_NOT_FOUND)
        return;

    createNewEvent(FUNC_EVENT);
    setFuncEventValues(objFound, newRho, passedArgs, receivedArgs);
    setFunCallRegged(TRUE);
    printForVerbose("Function call detected");
    // if (inLoopEvent())
    //     addEventToForIteration(eventsRegTail);
}
Rboolean isFunCallRegged() {
    if (!isRunning())
        return FALSE;
    return getFunCalledFlag();
}

void regVecCreation(SEXP call, SEXP vector, SEXP rho) {

    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    if (waitingCellChange()) {
        storeVecForCellChange(vector);
        if (!waitingCellChange()) {
            if (getCurrCellChange()->toRows == R_NilValue ||
                getCurrCellChange()->toCols == R_NilValue) {
                finalizeVarIdxChange(R_NilValue, getCurrentEnv());
            }
        }
        return;
    }

    if (waitingFrameVecs) {
        storeVecDataFrameEvent(vector);
        return;
    }
    if (waitingForVecs) {
        storeVecForEvent(vector);

        if (!waitingForVecs)
            finalizeForEventProcessing();
        return;
    }

    if (waitingIdxChange()) {
        if (toDiscard()) {
            decrementWaitingIdxChange();
            return;
        }
        storeVecForIdxChange(vector);
        return;
    }
    checkPendings(R_NilValue, R_NilValue, ABD_OBJECT_NOT_FOUND);
    storeNewVecValues(vector, call);
}

void regIf(SEXP Stmt, Rboolean result, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    // printf("if statement ... line %d\n", getCurrScriptLn());
    createNewEvent(IF_EVENT);
    setIfEventValues(Stmt, result);
    clearPendingVars();
    printForVerbose("Branch detected");
}

/* FOR LOOP CALLABLES*/
void regForLoopStart(SEXP call, SEXP enumerator, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    clearPendingVars();
    ABD_FOR_LOOP_EVENT *newForLoopEvent =
        createNewEvent(FOR_EVENT)->data.for_loop_event;
    setForEventValues(call, newForLoopEvent, enumerator);
    printForVerbose("For loop start detected");
}
void regForLoopIteration(int iterId, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_FOR)))
        return;

    createNewLoopIteration(iterId, ABD_FOR);
}

void regForLoopFinish(SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST && inLoopByType(ABD_FOR)))
        return;

    appendLastEventToLoop(ABD_FOR);
    popLoopFromStack(ABD_FOR);
    printForVerbose("For loop finish detected");
}

/* REPEAT LOOP CALLABLES*/

void regRepeatLoopStart(SEXP call, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    ABD_REPEAT_LOOP_EVENT *newRepeatLoopEvent =
        createNewEvent(REPEAT_EVENT)->data.repeat_loop_event;
    pushNewLoop(ABD_REPEAT, newRepeatLoopEvent);
    printForVerbose("Repeat loop start detected");
}

void regRepeatLoopIteration(int iterId, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST &&
        inLoopByType(ABD_REPEAT)))
        return;

    createNewLoopIteration(iterId, ABD_REPEAT);
}

void regRepeatLoopFinish(SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST &&
        inLoopByType(ABD_REPEAT)))
        return;

    appendLastEventToLoop(ABD_REPEAT);
    popLoopFromStack(ABD_REPEAT);
    printForVerbose("Repeat loop finish detected");
}

/* WHILE LOOP CALLABLES*/

void regWhileLoopStart(SEXP args, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;
    R_PrintData pars;
    ABD_WHILE_LOOP_EVENT *newWhileLoopEvent =
        createNewEvent(WHILE_EVENT)->data.while_loop_event;
    pushNewLoop(ABD_WHILE, newWhileLoopEvent);
    printForVerbose("While loop start detected");
}

void regWhileLoopCondition(SEXP stmt, Rboolean result, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST &&
        inLoopByType(ABD_WHILE)))
        return;

    createNewEvent(IF_EVENT);
    setIfEventValues2(stmt, result);

    if (loopStack->loop.whileLoop->cndtStr == ABD_NOT_FOUND)
        loopStack->loop.whileLoop->cndtStr = eventsRegTail->data.if_event->exprStr;

    clearPendingVars();
}

void regWhileLoopIteration(int iterId, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST &&
        inLoopByType(ABD_WHILE)))
        return;

    createNewLoopIteration(iterId, ABD_WHILE);
    verifyBranchDepthIntegrity();
}

void regWhileLoopFinish(SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST &&
        inLoopByType(ABD_WHILE)))
        return;

    appendLastEventToLoop(ABD_WHILE);
    popLoopFromStack(ABD_WHILE);
    printForVerbose("While loop finish detected");
}

/* LOOP misc*/
void doLoopJump(ABD_LOOP_TAGS jumpType, ABD_LOOP_TAGS requestingLoopType,
    SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST &&
        inLoopByType(requestingLoopType)))
        return;
    if (jumpType == ABD_NEXT)
        createNewEvent(NEXT_EVENT);
    else
        createNewEvent(BREAK_EVENT);
}

void regArith(SEXP call, SEXP ans, SEXP rho) {
    if (!(isRunning() && cmpToCurrEnv(rho) == ABD_EXIST))
        return;

    tmpStoreArith(call, ans);
    printForVerbose("Arithmetic operation detected");
}

void storeIsWaitingIf(int isWaiting, SEXP rho) {
    if (isRunning() && cmpToCurrEnv(rho) == ABD_EXIST)
        setIsWaitingIf(isWaiting);
}

void regFunRet(SEXP lhs, SEXP rho, SEXP val) {
    // printf("return... line %d\n", getCurrScriptLn());
    createNewEvent(RET_EVENT);
    setRetEventValue(val);
    printForVerbose("Function return detected");
    // if (inLoopEvent())
    //     addEventToForIteration(eventsRegTail);
}

ABD_STATE isRunning() {
    return watcherState;
}
