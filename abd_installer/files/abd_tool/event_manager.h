#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/env_stack_defn.h>
#include <abd_tool/base_defn.h>
#include <Rembedded.h>
#include <R_ext/Parse.h>

static void PrintIt(SEXP call, SEXP rho)
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

void initEventsReg()
{
    eventsReg = ABD_EVENT_NOT_FOUND;
    eventsRegTail = ABD_EVENT_NOT_FOUND;

    /* ARITH VARS */
    lastArithEvent = ABD_EVENT_NOT_FOUND;
    finalArithAns = R_NilValue;
    finalArithCall = R_NilValue;
    arithScriptLn = 0;
    currArithIndex = -1;
    arithResults = ABD_NOT_FOUND;

    /* VEC VARS*/
    vecValues = R_NilValue;
    auxVecCall = R_NilValue;
    auxVecLine = 0;

    /* possible ret*/
    lastRetValue = ABD_EVENT_NOT_FOUND;
    lastRetEvent = ABD_EVENT_NOT_FOUND;
    possibleRet = R_NilValue;
    possibleRetLine = 0;

    /* data frame vars*/
    frameCall = R_NilValue;
    pendingFrame = FALSE;
    waitingFrameVecs = 0;

    /* registry*/
    eventsReg = createMainEvent();
    eventsRegTail = eventsReg;
    waitingElseIF = 0;
    eventCounter = 0;
}

ABD_EVENT *initBaseEvent(ABD_EVENT *newBaseEvent)
{
    newBaseEvent->id = ++eventCounter;
    newBaseEvent->scriptLn = getCurrScriptLn();
    newBaseEvent->data.if_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.func_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.ret_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.asgn_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.arith_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.for_loop_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.idx_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->data.vec_event = ABD_EVENT_NOT_FOUND;
    newBaseEvent->nextEvent = ABD_EVENT_NOT_FOUND;
    return newBaseEvent;
}

ABD_EVENT *memAllocBaseEvent()
{
    return (ABD_EVENT *)malloc(sizeof(ABD_EVENT));
}
IF_ABD_OBJ *memAllocIfAbdObj()
{
    return (IF_ABD_OBJ *)malloc(sizeof(IF_ABD_OBJ));
}
ABD_IF_EVENT *memAllocIfEvent()
{
    return (ABD_IF_EVENT *)malloc(sizeof(ABD_IF_EVENT));
}
ABD_FUNC_EVENT *memAllocFuncEvent()
{
    return (ABD_FUNC_EVENT *)malloc(sizeof(ABD_FUNC_EVENT));
}

ABD_ASSIGN_EVENT *memAllocAssignEvent()
{
    return (ABD_ASSIGN_EVENT *)malloc(sizeof(ABD_ASSIGN_EVENT));
}
ABD_ARITH_EVENT *memAllocArithEvent()
{
    return (ABD_ARITH_EVENT *)malloc(sizeof(ABD_ARITH_EVENT));
}
ABD_EVENT_ARG *memAllocEventArg()
{
    return (ABD_EVENT_ARG *)malloc(sizeof(ABD_EVENT_ARG));
}

ABD_IDX_CHANGE_EVENT *memAllocIdxEvent()
{
    return (ABD_IDX_CHANGE_EVENT *)malloc(sizeof(ABD_IDX_CHANGE_EVENT));
}

ABD_VEC_EVENT *memAllocVecEvent()
{
    return (ABD_VEC_EVENT *)malloc(sizeof(ABD_VEC_EVENT));
}
ABD_RET_EVENT *memAllocRetEvent()
{
    return (ABD_RET_EVENT *)malloc(sizeof(ABD_RET_EVENT));
}
IF_EXPRESSION *memAllocIfExp()
{
    return (IF_EXPRESSION *)malloc(sizeof(IF_EXPRESSION));
}

ITERATION *memAllocIteration()
{
    return (ITERATION *)malloc(sizeof(ITERATION));
}

ABD_FOR_LOOP_EVENT *memAllocForLoopEvent()
{
    return (ABD_FOR_LOOP_EVENT *)malloc(sizeof(ABD_FOR_LOOP_EVENT));
}

ABD_REPEAT_LOOP_EVENT *memAllocRepeatLoopEvent()
{
    return (ABD_REPEAT_LOOP_EVENT *)malloc(sizeof(ABD_REPEAT_LOOP_EVENT));
}

ABD_WHILE_LOOP_EVENT *memAllocWhileLoopEvent()
{
    return (ABD_WHILE_LOOP_EVENT *)malloc(sizeof(ABD_WHILE_LOOP_EVENT));
}
ABD_FRAME_EVENT *memAllocDataFrameEvent()
{
    return (ABD_FRAME_EVENT *)malloc(sizeof(ABD_FRAME_EVENT));
}
ABD_CELL_CHANGE_EVENT * memAllocCellChangeEvent() {
    return (ABD_CELL_CHANGE_EVENT *)malloc(sizeof(ABD_CELL_CHANGE_EVENT));
}
ITER_EVENT_LIST *memAllocIterEventList()
{
    return (ITER_EVENT_LIST *)malloc(sizeof(ITER_EVENT_LIST));
}

ABD_LOOP_CHAIN *memAllocLoopChain()
{
    return (ABD_LOOP_CHAIN *)malloc(sizeof(ABD_LOOP_CHAIN));
}

ABD_EVENT_ARG *setArgValues(ABD_EVENT_ARG *currArg, ABD_OBJECT *fromObj, ABD_OBJECT *toObj, ABD_OBJECT_MOD *passedValue)
{
    currArg->fromObj = fromObj;
    currArg->toObj = toObj;
    currArg->passedValue = passedValue;
    currArg->rcvdValue = toObj->modList;

    return currArg;
}
SEXP getValueFromPROMSXP(SEXP symbol)
{
    // recurse until we find the real promise, not a promise of a promise
    SEXP env;
    while (TYPEOF(symbol) == PROMSXP)
    {
        env = PRENV(symbol);
        symbol = PREXPR(symbol);
        // If the promise is threaded through multiple functions, we'll
        // get some symbols along the way. If the symbol is bound to a promise
        // keep going on up
        if (TYPEOF(symbol) == SYMSXP)
        {
            SEXP obj = Rf_findVar(symbol, env);
            if (TYPEOF(obj) != PROMSXP)
            {
                return obj;
                break;
            }
            symbol = obj;
        }
    }
    return symbol;
}

/*
    The function below, is also used in object manager.
    This funtion receives the SEXP referent to the symbol and
    calls the setModValues with the last argument being the function
    that will store that specific type

    for instance, if it's a REALSXP, it will alloc a vector determined
    by the symbolValue nRows (which is the number of elements)
    with the function 'createRealVector'.
*/
ABD_VEC_OBJ *processVector(SEXP symbolValue, int idxChange)
{
    SEXPTYPE modType;
    rollback2:
    modType = TYPEOF(symbolValue);

    switch (modType)
    {
    case CLOSXP:
        break;
    case INTSXP:
        if (idxChange)
            return intVectorMultiChanges(symbolValue);
        else
            return createIntVector(symbolValue);
        break;
    case REALSXP:
        if (idxChange)
            return realVectorMultiChanges(symbolValue);
        else
            return createRealVector(symbolValue);

        break;
    case STRSXP:
        if (idxChange)
            return strVectorMultiChanges(symbolValue);
        else
            return createStrVector(symbolValue);

        break;
    case CPLXSXP:
        puts("complex vectors");
        break;
    case LGLSXP:
        puts("logical vectors");
        break;
    case RAWSXP:
        puts("raw vector");
        break;
    case VECSXP:
        puts("list (generic vector)");
        break;
    case PROMSXP:
    {
        symbolValue = getValueFromPROMSXP(symbolValue);
        goto rollback2;
        break;
    }
    default:
        break;
    }
    return ABD_OBJECT_NOT_FOUND;
}
ABD_FRAME_OBJ *processDataFrame(SEXP symbolValue, int idxChange)
{

    if (idxChange)
        return frameMultiChanges(symbolValue);
    else
        return createDataFrame(symbolValue);
}

ABD_OBJECT_MOD *initModAndPopulate(ABD_OBJECT_MOD *newMod, OBJ_STATE remotion, ABD_OBJ_VALUE_TYPE valueType)
{
    newMod->value.frame_value = ABD_OBJECT_NOT_FOUND;
    newMod->value.vec_value = ABD_OBJECT_NOT_FOUND;
    newMod->value.mtrx_value = ABD_OBJECT_NOT_FOUND;
    newMod->valueType = valueType;
    newMod->remotion = remotion;
    return newMod;
}

ABD_OBJECT_MOD *processByType(SEXP symbolValue, ABD_OBJECT_MOD *mod, int idxChange)
{
    switch (getObjStructType(symbolValue))
    {
    case ABD_VECTOR:
        mod = initModAndPopulate(mod, ABD_ALIVE, ABD_VECTOR);
        mod->value.vec_value = processVector(symbolValue, idxChange);
        break;
    case ABD_MATRIX:
        mod = initModAndPopulate(mod, ABD_ALIVE, ABD_MATRIX);
        puts("deal with matrix here");
        printf("Row number %d\nCol number %d\n", nrows(symbolValue), Rf_ncols(symbolValue));
        break;
    case ABD_FRAME:
        mod = initModAndPopulate(mod, ABD_ALIVE, ABD_FRAME);
        mod->value.frame_value = processDataFrame(symbolValue, idxChange);
        break;
    default:
        puts("fell to def");
        break;
    }
    return mod;
}

/*
    The function below will run through all the arguments and return the
    head to the linked list that will constitute the arguments list

    CAUTION:
    if the type is 17 (DOTSXP) then we need to do something different
    due to the fact that it is an arbitrary number of arguments
    might apply the same procedure as 17 to the 18 (anysxp)
*/
ABD_EVENT_ARG *processArgs(SEXP passedArgs, SEXP receivedArgs, SEXP newRho, ABD_OBJECT *targetFunc)
{
    ABD_EVENT_ARG *argsList = ABD_NOT_FOUND;
    ABD_EVENT_ARG *currentArg = ABD_NOT_FOUND;
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT_MOD *objValue = ABD_NOT_FOUND;
    SEXP rcvdValue = R_NilValue;
    for (; passedArgs != R_NilValue; passedArgs = CDR(passedArgs), receivedArgs = CDR(receivedArgs))
    {
        if (argsList == ABD_NOT_FOUND)
        {
            // ok first arg to alloc
            // this avoid allocations without args existence
            currentArg = memAllocEventArg();
            currentArg->nextArg = ABD_NOT_FOUND;
            argsList = currentArg;
        }
        else
        {
            currentArg->nextArg = memAllocEventArg();
            currentArg = currentArg->nextArg;
            currentArg->nextArg = ABD_NOT_FOUND;
        }
        SEXP symbol = CDR(CAR(passedArgs));
        /*
            if the argument is an object (symbol), first do a lookup in
            the tool registry, if not found (might not be inside the
            tool scope of action), request R to retrieve the value

            this mitigate the fact that the user might be declaring the
            object before start_watcher() is issued.
        */

        rollback:
        switch (TYPEOF(symbol))
        {
        case REALSXP:
            objectFound = createUnscopedObj("NA", -1, -1, symbol, 0);
            objValue = objectFound->modList;
            rcvdValue = symbol;
            break;
        case SYMSXP:
        {
            //its a variable
            const char *passedName = CHAR(PRINTNAME(symbol));
            SEXP rStrName = mkString(passedName);
            rcvdValue = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
            //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
            objectFound = findCmnObj(passedName, getCurrentEnv());
            if (objectFound == ABD_OBJECT_NOT_FOUND)
            {
                //object not in registry
                //request R for the object (not hardcoded)
                objectFound = createUnscopedObj(passedName, -2, -2, rcvdValue, 0);
            }
            objValue = objectFound->modList;

            break;
        }
        /*case LGLSXP:
            case INTSXP:
                Rprintf("[%d] '%s' %d\n", i+1, rcvdName, INTEGER(symbol)[0]);
                break;
            case CPLXSXP:
                {
                    Rcomplex cpl = COMPLEX(symbol)[0];
                    Rprintf("[%d] '%s' %f + %fi\n", i+1, rcvdName, cpl.r, cpl.i);
                }
                break;
            case STRSXP:
                Rprintf("[%d] '%s' %s\n", i+1, rcvdName,
                    CHAR(STRING_ELT(symbol, 0)));
                break;
            */
        case BCODESXP:
        {
            //extract the promise from the bytecode sxp
            symbol = PREXPR(CAR(passedArgs));
            goto rollback;
            break;
        }
        default:
            break;
            //Rprintf("[%d] '%s' R type\n", i+1, rcvdName);
        }
        const char *rcvdName = isNull(TAG(receivedArgs)) ? "" : CHAR(PRINTNAME(TAG(receivedArgs)));
        ABD_OBJECT *toObj = createLocalVariable(rcvdName, newRho, rcvdValue, targetFunc);
        currentArg = setArgValues(currentArg, objectFound, toObj, objValue);

        objectFound = ABD_OBJECT_NOT_FOUND;
        objValue = ABD_OBJECT_NOT_FOUND;
    }
    return argsList;
}

int inCollection(const char *check)
{
    char *collection[] ={
        "+", "-", "*", "/",
        "==", "!=", "<", ">", "<=", ">=",
        "&", "|", "&&", "||", "!", "[", "**", "^" };
    int nCollection = 18;

    for (int i = 0; i < nCollection; i++)
    {
        //printf("checking: %s == %s\n", check, collection[i]);
        if (strcmp(check, collection[i]) == 0)
            return 1;
    }
    return 0;
}
//returns the id (+1 to avoid idx 0 problems) in the changes vector, otherwise 0 (false)
int inIdxChangesVec(int idx, int nChanges, int *idxChanges)
{
    for (int i = 0; i < nChanges; i++)
    {
        if (idxChanges[i] == idx)
        {
            return i + 1;
        }
    }
    return 0;
}

ABD_OBJECT_MOD *getValuePointer(ABD_OBJECT_MOD *retMod, ABD_OBJECT_MOD *currMod, int found, int findIdx)
{

    switch (currMod->value.vec_value->type)
    {
    case REALSXP:
    {
        retMod->value.vec_value->vector = memAllocDoubleVector(1);

        if (found)
            retMod->value.vec_value->vector = &(((double *)currMod->value.vec_value->vector)[found - 1]);
        else
            retMod->value.vec_value->vector = &(((double *)currMod->value.vec_value->vector)[findIdx]);
        break;
    }

    default:
        break;
    }
    return retMod;
}

ABD_OBJECT_MOD *idxCurrValueVec(ABD_OBJECT_MOD *modList, int findIdx)
{
    ABD_OBJECT_MOD *retMod = memAllocMod();
    ABD_OBJECT_MOD *currMod = modList;
    int found = 0;
    top:;
    if (!currMod->value.vec_value->idxChange || found)
    {
        retMod->id = modList->id;
        retMod->nextMod = ABD_OBJECT_NOT_FOUND;
        retMod->prevMod = ABD_OBJECT_NOT_FOUND;
        retMod->valueType = modList->valueType;
        retMod->value.vec_value = memAllocVecObj();
        retMod->value.vec_value->type = currMod->value.vec_value->type;
        retMod->value.vec_value->idxChange = 0;
        retMod->value.vec_value->nCols = 1;
        retMod->value.vec_value->idxs = memAllocIntVector(1);
        retMod->value.vec_value->idxs[0] = findIdx;
        return getValuePointer(retMod, currMod, found, findIdx);
    }

    do
    {
        if ((found = inIdxChangesVec(findIdx, currMod->value.vec_value->nCols, currMod->value.vec_value->idxs)))
            break;
        currMod = currMod->prevMod;
    } while (!found && currMod->value.vec_value->idxChange);
    goto top;
}

ABD_OBJECT_MOD *findCurrValue(ABD_OBJECT_MOD *modList, int findIdx)
{
    ABD_OBJECT_MOD *retMod = ABD_OBJECT_NOT_FOUND;
    switch (modList->valueType)
    {
    case ABD_VECTOR:
        return idxCurrValueVec(modList, findIdx);
        break;
    case ABD_MATRIX:
        break;

    default:
        return retMod;
    }
    return ABD_OBJECT_NOT_FOUND;
}
ABD_OBJECT_MOD *fakeIdxForUnscoped(ABD_OBJECT_MOD *mod, int index)
{
    switch (mod->valueType)
    {
    case ABD_VECTOR:
        mod->value.vec_value->idxChange = 0;
        mod->value.vec_value->nCols = 1;
        mod->value.vec_value->idxs = memAllocIntVector(1);
        mod->value.vec_value->idxs[0] = index;
        break;
    case ABD_MATRIX:
        break;
    default:
        break;
    }
    return mod;
}
IF_ABD_OBJ *getAbdIfObj(SEXP symbol, int withIndex)
{
    IF_ABD_OBJ *newObj = memAllocIfAbdObj();
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT_MOD *objValue = ABD_OBJECT_NOT_FOUND;

    switch (TYPEOF(symbol))
    {
    case REALSXP:
        objectFound = createUnscopedObj("NA", -1, -1, symbol, 0);
        objValue = objectFound->modList;
        break;
    case SYMSXP:
    {
        //its a variable
        const char *objName = CHAR(PRINTNAME(symbol));
        //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
        objectFound = findCmnObj(objName, getCurrentEnv());
        if (objectFound == ABD_OBJECT_NOT_FOUND)
        {
            //object not in registry
            int nDigits = floor(log10(abs(withIndex + 1))) + 1;
            int nameSize = strlen(objName) + nDigits + 2; //+2 for the []
            char *auxName = memAllocForString(nameSize);
            memset(auxName, 0, nameSize);
            sprintf(auxName, "%s[%d]", objName, withIndex + 1);

            //request R for the object (not hardcoded)
            SEXP symbValue = getResult(auxName);
            objectFound = createUnscopedObj(objName, -2, -2, symbValue, 0);
            objValue = fakeIdxForUnscoped(objectFound->modList, withIndex);
        }
        else
            // get index effective value
            objValue = findCurrValue(objectFound->modList, withIndex);
        break;
    }

    /*case LGLSXP:
        case INTSXP:
            Rprintf("[%d] '%s' %d\n", i+1, rcvdName, INTEGER(symbol)[0]);
            break;
        case CPLXSXP:
            {
                Rcomplex cpl = COMPLEX(symbol)[0];
                Rprintf("[%d] '%s' %f + %fi\n", i+1, rcvdName, cpl.r, cpl.i);
            }
            break;
        case STRSXP:
            Rprintf("[%d] '%s' %s\n", i+1, rcvdName,
                CHAR(STRING_ELT(symbol, 0)));
            break;
        */
    default:
        break;
        //Rprintf("[%d] '%s' R type\n", i+1, rcvdName);
    }
    newObj->objPtr = objectFound;
    newObj->objValue = objValue;

    return newObj;
}

SEXP getResult(const char *expr)
{
    SEXP e, tmp, retValue;
    ParseStatus status;
    //printf("GET_RESULT for [%s]\n", expr);
    PROTECT(tmp = mkString(expr));
    PROTECT(e = R_ParseVector(tmp, -1, &status, R_NilValue));
    PROTECT(retValue = R_tryEval(VECTOR_ELT(e, 0), getCurrentEnv(), NULL));

    UNPROTECT(3);

    return retValue;
}

char *getObjStr(IF_ABD_OBJ *objStruct)
{
    ABD_OBJECT *obj = objStruct->objPtr;
    ABD_OBJECT_MOD *objValue = objStruct->objValue;
    char *objRepr = memAllocForString(20);
    memset(objRepr, 0, 20);
    sprintf(objRepr, "%s", obj->name);

    switch (objValue->valueType)
    {
    case ABD_VECTOR:
    {
        //check if it was a specific index
        if (objValue->value.vec_value->idxs != ABD_OBJECT_NOT_FOUND)
            sprintf(objRepr, "%s[%d]", objRepr, objValue->value.vec_value->idxs[0] + 1);
        break;
    }
    case ABD_MATRIX:
        break;
    default:
        break;
    }
    return objRepr;
}

void mkStrForCmp(IF_EXPRESSION *newExpr, char *stmtStr)
{
    if (newExpr->left_type == IF_EXPR)
        sprintf(stmtStr, "%s%.2f", stmtStr, ((IF_EXPRESSION *)newExpr->left_data)->result);
    else
    {
        ABD_OBJECT *obj = ((IF_ABD_OBJ *)newExpr->left_data)->objPtr;
        if (obj->id == -1)
        {
            // hardcoded, get value
            ABD_OBJECT_MOD *objValue = ((IF_ABD_OBJ *)newExpr->left_data)->objValue;

            sprintf(stmtStr, "%s%.4f", stmtStr, ((double *)objValue->value.vec_value->vector)[0]);
        }
        else
            sprintf(stmtStr, "%s%s", stmtStr, getObjStr((IF_ABD_OBJ *)newExpr->left_data));
    }

    //get the operator
    sprintf(stmtStr, "%s%s", stmtStr, newExpr->operator);
    if (newExpr->right_type == IF_EXPR)
        sprintf(stmtStr, "%s%.2f", stmtStr, ((IF_EXPRESSION *)newExpr->right_data)->result);
    else
    {
        ABD_OBJECT *obj = ((IF_ABD_OBJ *)newExpr->right_data)->objPtr;
        if (obj->id == -1)
        {
            // hardcoded, get value
            ABD_OBJECT_MOD *objValue = ((IF_ABD_OBJ *)newExpr->right_data)->objValue;
            sprintf(stmtStr, "%s%.4f", stmtStr, ((double *)objValue->value.vec_value->vector)[0]);
        }
        else
            sprintf(stmtStr, "%s%s", stmtStr, getObjStr((IF_ABD_OBJ *)newExpr->right_data));
    }
    if (newExpr->isConfined)
        sprintf(stmtStr, "%s)", stmtStr);
}

IF_EXPRESSION *processIfStmt(SEXP st, int withEval)
{

    IF_EXPRESSION *newExpr = memAllocIfExp();
    newExpr->exprId = ++exprId;
    char *stmtStr = memAllocForString(100);
    memset(stmtStr, 0, 100);
    if (TYPEOF(st) == LANGSXP)
    {
        newExpr->isConfined = 0;
        procStateLabel:;
        SEXP carSt = CAR(st);
        const char *operator= CHAR(PRINTNAME(carSt));
        int opSize = strlen(operator);
        newExpr->operator= memAllocForString(opSize);
        copyStr(newExpr->operator, operator, opSize);

        if (inCollection(operator))
        {
            int withIndex = 0;
            SEXP lElem = CAR(CDR(st));
            if (TYPEOF(lElem) == LANGSXP)
            {
                if (strcmp(CHAR(PRINTNAME(CAR(lElem))), "[") == 0)
                {
                    //index change language sxp
                    // a[1]
                    // lElem == symb name
                    // rElem == 1 (index)
                    SEXP val = CAR(CDR(CDR(lElem)));
                    lElem = CAR(CDR(lElem));
                    withIndex = (int)REAL(val)[0] - 1;
                    goto jump1;
                }
                newExpr->left_type = IF_EXPR;
                newExpr->left_data = processIfStmt(lElem, 1);
            }
            else
            {
                jump1:;
                // if we come to (2>3) left side wont be LANGSXP
                // need to process by type
                newExpr->left_type = IF_ABD;
                // alloc new ABD_OBJ and set values (dont forget mod)
                newExpr->left_data = getAbdIfObj(lElem, withIndex);
            }
            withIndex = 0;
            SEXP rElem = CAR(CDR(CDR(st)));
            if (TYPEOF(rElem) == LANGSXP)
            {
                if (strcmp(CHAR(PRINTNAME(CAR(rElem))), "[") == 0)
                {
                    //index change language sxp
                    // a[1]
                    // lElem == symb name
                    // rElem == 1 (index)
                    SEXP val = CAR(CDR(CDR(rElem)));
                    rElem = CAR(CDR(rElem));
                    withIndex = (int)REAL(val)[0] - 1;
                    goto jump2;
                }
                newExpr->right_type = IF_EXPR;
                newExpr->right_data = processIfStmt(rElem, 1);
            }
            else
            {
                jump2:;
                // if we come to (2>3) left side wont be LANGSXP
                // need to process by type
                newExpr->right_type = IF_ABD;
                // alloc new ABD_OBJ and set values (dont forget mod)
                newExpr->right_data = getAbdIfObj(rElem, withIndex);
            }
        }
        else if (strcmp(operator, "(") == 0)
        {
            //confine
            newExpr->isConfined = 1;
            st = CAR(CDR(st));
            sprintf(stmtStr, "(");
            goto procStateLabel;
        }
    }
    else
    {
        // not language expression
        // can be if(a) or if(1), etc...
        puts("just a single number in here...");
    }
    mkStrForCmp(newExpr, stmtStr);
    setWatcherState(ABD_DISABLE);
    if (withEval)
    {
        SEXP result = getResult(stmtStr);
        SEXPTYPE resType = TYPEOF(result);

        if (resType == LGLSXP)
            newExpr->result = LOGICAL(result)[0];
        else if (resType == REALSXP)
            newExpr->result = REAL(result)[0];
    }
    else
    {
        if (lastArithEvent != ABD_EVENT_NOT_FOUND)
        {
            /* pick from the results array */;
            newExpr->result = REAL(arithResults[++currArithIndex])[0];
            /* int arithLen = Rf_length(arithResults[++currArithIndex]);


            //newExpr->resultSize = arithLen;
            if (arithLen > 1)
            {
            }
            else
            {
                printf("statment: %s\n", stmtStr);
                newExpr->result = REAL(arithResults[currArithIndex])[0];
                printf("result: %.4f\n", newExpr->result);
            }
 */
        }
    }
    setWatcherState(ABD_ENABLE);
    //printf("\nStatement %s\nResult %s\n", stmtStr, (newExpr->result) ? "TRUE" : "FALSE");
    puts("before free 1");
    free(stmtStr);
    puts("no error");
    return newExpr;
}

int isWaitingElseIf()
{
    return waitingElseIF;
}
void setIsWaitingIf(int isWaiting)
{
    waitingElseIF = isWaiting;
}

char *getStrForStatement(SEXP s, R_PrintData *data)
{
    int i;
    char *retStr;
    SEXP t = getAttrib(s, R_SrcrefSymbol);
    Rboolean useSrc = data->useSource && isInteger(t);
    if (useSrc)
    {
        PROTECT(t = lang2(R_AsCharacterSymbol, t));
        t = eval(t, R_BaseEnv);
        UNPROTECT(1);
    }
    else
    {
        t = deparse1w(s, 0, data->useSource | DEFAULTDEPARSE);
        R_print = *data; /* Deparsing calls PrintDefaults() */
    }
    PROTECT(t);

    int len = (int)strlen(translateChar(STRING_ELT(t, 0)));
    retStr = memAllocForString(len);
    copyStr(retStr, translateChar(STRING_ELT(t, 0)), len);
    UNPROTECT(1);
    return retStr;
}

void setIfEventValues(SEXP statement, Rboolean result)
{
    ABD_IF_EVENT *currEvent = eventsRegTail->data.if_event;
    R_PrintData pars;

    exprId = 0;
    if (isWaitingElseIf())
    {
        if (statement == R_NilValue)
        {
            //is an else
            currEvent->expr = ABD_NOT_FOUND;
            currEvent->exprStr = ABD_NOT_FOUND;
            currEvent->globalResult = 1;
            currEvent->isElse = 1;
            currEvent->isElseIf = 0;
            waitingElseIF = 0;
        }
        else
        {
            //is an else if
            currEvent->globalResult = result;
            currEvent->isElse = 0;
            currEvent->isElseIf = 1;
            exprId = 0;
            currEvent->expr = processIfStmt(statement, 1);
            PrintInit(&pars, getCurrentEnv());
            currEvent->exprStr = getStrForStatement(statement, &pars);
        }
    }
    else
    {

        currEvent->isElse = 0;
        currEvent->isElseIf = 0;
        currEvent->globalResult = result;
        exprId = 0;
        currEvent->expr = processIfStmt(statement, 1);
        PrintInit(&pars, getCurrentEnv());
        currEvent->exprStr = getStrForStatement(statement, &pars);
    }

    if (result)
    {
        setOnBranch(TRUE);
        incBranchDepth();
        setIsWaitingIf(0);
    }
}

void setIfEventValues2(SEXP statement, Rboolean result)
{
    ABD_IF_EVENT *currEvent = eventsRegTail->data.if_event;
    R_PrintData pars;

    exprId = 0;
    if (isWaitingElseIf())
    {
        if (statement == R_NilValue)
        {
            //is an else
            currEvent->expr = ABD_NOT_FOUND;
            currEvent->exprStr = ABD_NOT_FOUND;
            currEvent->globalResult = 1;
            currEvent->isElse = 1;
            currEvent->isElseIf = 0;
            waitingElseIF = 0;
        }
        else
        {
            //is an else if
            currEvent->globalResult = result;
            currEvent->isElse = 0;
            currEvent->isElseIf = 1;
            exprId = 0;
            currEvent->expr = processIfStmt(statement, 1);
            PrintInit(&pars, getCurrentEnv());
            currEvent->exprStr = getStrForStatement(statement, &pars);
        }
    }
    else
    {

        currEvent->isElse = 0;
        currEvent->isElseIf = 0;
        currEvent->globalResult = result;
        exprId = 0;
        currEvent->expr = processIfStmt(statement, 1);
        PrintInit(&pars, getCurrentEnv());
        currEvent->exprStr = getStrForStatement(statement, &pars);
    }

    // if (result)
    // {
    //     setOnBranch(TRUE);
    //     incBranchDepth();
    //     setIsWaitingIf(0);
    // }
    setIsWaitingIf(0);
}

void setFuncEventValues(ABD_OBJECT *calledObj, SEXP newRho, SEXP passedArgs, SEXP receivedArgs)
{
    eventsRegTail->data.func_event->toEnv = newRho;
    eventsRegTail->data.func_event->caller = getCurrFuncObj();
    eventsRegTail->data.func_event->called = calledObj;
    eventsRegTail->branchDepth = getCurrBranchDepth();
    eventsRegTail->data.func_event->args = processArgs(passedArgs, receivedArgs, newRho, calledObj);
    envPush(newRho, calledObj);
}

void setRetEventValue(SEXP value)
{
    ABD_OBJECT_MOD *valueABD = memAllocMod();

    if (checkRetStored(value) == ABD_EXIST)
    {
        valueABD = processByType(possibleRet, valueABD, 0);

        eventsRegTail->data.ret_event->retValue = valueABD;
        eventsRegTail->scriptLn = possibleRetLine;
    }
    else
    {
        valueABD = processByType(value, valueABD, 0);
        eventsRegTail->data.ret_event->retValue = valueABD;
    }
    valueABD->id = -1;
    eventsRegTail->data.ret_event->toObj = ABD_OBJECT_NOT_FOUND;
    envPop();
    eventsRegTail->data.ret_event->toEnv = getCurrentEnv();
    lastRetValue = value;
}

void setAsgnEventValues(ABD_OBJECT *toObj, SEXP value)
{
}

SEXP getSavedArithAns()
{
    return finalArithAns;
}

void tmpStoreArith(SEXP call, SEXP ans)
{
    if (arithResults == ABD_NOT_FOUND)
    {

        arithResults = (SEXP *)malloc(sizeof(SEXP) * 10);
        for (int i = 0; i < 10; i++)
            arithResults[i] = R_NilValue;
    }
    arithScriptLn = getCurrScriptLn();
    arithResults[++currArithIndex] = ans;
    finalArithAns = ans;
    finalArithCall = call;
}
void verifyBranchDepthIntegrity()
{
    if (loopStack->initialBranchDepth != getCurrBranchDepth())
        forceBranchDepth(loopStack->initialBranchDepth);
}
ABD_EVENT *creaStructsForType(ABD_EVENT *newBaseEvent, ABD_EVENT_TYPE type)
{
    //main event needs nothing but the type
    switch (type)
    {
    case FUNC_EVENT:
        newBaseEvent->data.func_event = memAllocFuncEvent();
        break;
    case IF_EVENT:
        newBaseEvent->data.if_event = memAllocIfEvent();
        break;
    case RET_EVENT:
        newBaseEvent->data.ret_event = memAllocRetEvent();
        lastRetEvent = newBaseEvent;
        break;
    case ASGN_EVENT:
        newBaseEvent->data.asgn_event = memAllocAssignEvent();
        break;
    case ARITH_EVENT:
        newBaseEvent->data.arith_event = memAllocArithEvent();
        lastArithEvent = newBaseEvent->data.arith_event;
        break;
    case VEC_EVENT:
        newBaseEvent->data.vec_event = memAllocVecEvent();
        break;
    case IDX_EVENT:
        newBaseEvent->data.idx_event = memAllocIdxEvent();
        break;
    case FOR_EVENT:
        newBaseEvent->data.for_loop_event = memAllocForLoopEvent();
        break;
    case REPEAT_EVENT:
        newBaseEvent->data.repeat_loop_event = memAllocRepeatLoopEvent();
        break;
    case WHILE_EVENT:
        newBaseEvent->data.while_loop_event = memAllocWhileLoopEvent();
        newBaseEvent->data.while_loop_event->cndtStr = ABD_NOT_FOUND;
        break;
    case FRAME_EVENT:
        newBaseEvent->data.data_frame_event = memAllocDataFrameEvent();
        break;
    case CELL_EVENT:
        newBaseEvent->data.cell_change_event = memAllocCellChangeEvent();
        break;
    case BREAK_EVENT:
    case NEXT_EVENT:
        break;

    default:
        break;
    }
    return newBaseEvent;
}

ABD_EVENT *createNewEvent(ABD_EVENT_TYPE newEventType)
{
    ABD_EVENT *newEvent = memAllocBaseEvent();
    newEvent = initBaseEvent(newEvent);
    if (newEventType != MAIN_EVENT)
    {
        newEvent = creaStructsForType(newEvent, newEventType);
        eventsRegTail->nextEvent = newEvent;
        eventsRegTail = eventsRegTail->nextEvent;
        if (loopStack != ABD_NOT_FOUND)
            addEventToCurrentLoop(newEvent);
    }
    newEvent->branchDepth = getCurrBranchDepth();
    newEvent->atFunc = getCurrFuncObj();
    newEvent->env = getCurrentEnv();
    newEvent->type = newEventType;
    return newEvent;
}

void storeNewVecValues(SEXP values, SEXP call)
{
    vecValues = values;
    auxVecCall = call;
    auxVecLine = getCurrScriptLn();
}

void clearPendingVars()
{
    /* reset the pending variables values */
    /* Reset RET_EVENT */
    lastRetEvent = ABD_EVENT_NOT_FOUND;
    lastRetValue = R_NilValue;

    /* reset vec */
    vecValues = R_NilValue;
    auxVecCall = R_NilValue;
    auxVecLine = 0;

    /* ARITH VARS */
    lastArithEvent = ABD_EVENT_NOT_FOUND;
    finalArithAns = R_NilValue;
    finalArithCall = R_NilValue;
    arithScriptLn = 0;
    currArithIndex = -1;
    arithResults = ABD_NOT_FOUND;
    waitingElseIF = 0;

    frameCall = R_NilValue;
    pendingFrame = FALSE;
    waitingFrameVecs = 0;

    clearIdxChanges();
    clearCellChanges();
}

ABD_EVENT *checkPendingArith(SEXP rhs)
{
    /* Pending arithmetic event to register */

    /* check if the values are NULL, if so, no pending, return NULL*/
    if ((finalArithCall == R_NilValue && finalArithAns == R_NilValue))
        return ABD_EVENT_NOT_FOUND;

    /*
        check if the answer from the arith is being used, if not, create the event and return NULL
        otherwise return the lastArithEvent
    */

    createNewEvent(ARITH_EVENT);
    lastArithEvent->globalResult = REAL(finalArithAns)[0];
    currArithIndex = -1;
    exprId = 0;
    lastArithEvent->expr = processIfStmt(finalArithCall, 0);
    eventsRegTail->scriptLn = arithScriptLn;
    R_PrintData pars;
    PrintInit(&pars, getCurrentEnv());
    lastArithEvent->exprStr = getStrForStatement(finalArithCall, &pars);


    if (finalArithAns == rhs)
        return eventsRegTail;

    if (getCurrCellChange() != ABD_OBJECT_NOT_FOUND)
        return eventsRegTail;

    return ABD_EVENT_NOT_FOUND;
}
ABD_EVENT *checkPendingRet(SEXP rhs, ABD_OBJECT *obj)
{
    /* Pending ret event to assign object */
    if ((lastRetValue == R_NilValue) || (lastRetEvent == ABD_EVENT_NOT_FOUND) || (obj == ABD_OBJECT_NOT_FOUND))
        return ABD_EVENT_NOT_FOUND;

    lastRetEvent->data.ret_event->toObj = obj;
    puts("freeing lastRet");
    free(lastRetEvent->data.ret_event->retValue);
    puts("done");
    lastRetEvent->data.ret_event->retValue = obj->modList;

    return lastRetEvent;
}
int toDiscard()
{
    return getCurrIdxChanges()->discard;
}

void storeVecForIdxChange(SEXP vec)
{
    //
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    if (idxChanges->srcIdxsVec)
    {
        idxChanges->srcIdxs = vec;
        idxChanges->srcIdxsVec = 0;
        decrementWaitingIdxChange();
        return;
    }

    if (idxChanges->srcVec)
    {
        idxChanges->srcValues = vec;
        idxChanges->srcVec = 0;
        decrementWaitingIdxChange();
        return;
    }

    if (idxChanges->destIdxsVec)
    {
        idxChanges->destIdxs = vec;
        idxChanges->destIdxsVec = 0;
        decrementWaitingIdxChange();
        return;
    }
}

void storeVecForCellChange(SEXP vec) {

    CELL_CHANGE * cellChange = getCurrCellChange();

    if (cellChange->waitingSrcCols) {
        cellChange->srcCols = vec;
        cellChange->waitingSrcCols = FALSE;
        cellChange->srcNCols = Rf_length(vec);
        decrementWaitingCellChange();
        return;
    }

    if (cellChange->waitingSrcRows) {
        cellChange->srcRows = vec;
        cellChange->waitingSrcRows = FALSE;
        cellChange->srcNRows = Rf_length(vec);
        decrementWaitingCellChange();
        return;
    }

    if (cellChange->waitingSrcValues) {
        cellChange->srcValues = vec;
        cellChange->waitingSrcValues = FALSE;
        decrementWaitingCellChange();
        return;
    }

    if (cellChange->waitingColsVec) {
        cellChange->toCols = vec;
        cellChange->waitingColsVec = FALSE;
        cellChange->nCols = Rf_length(vec);
        decrementWaitingCellChange();
        return;
    }


    if (cellChange->waitingRowsVec) {
        cellChange->toRows = vec;
        cellChange->waitingRowsVec = FALSE;
        cellChange->nRows = Rf_length(vec);
        decrementWaitingCellChange();
        return;
    }
}

void preProcessDest(SEXP call)
{
    //process destination
    SEXP destIdxs = CAR(CDR(CDR(CAR(CDR(call)))));
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    if (TYPEOF(destIdxs) == LANGSXP)
    {

        if ((strcmp(CHAR(PRINTNAME(CAR(destIdxs))), ":") == 0) || (strcmp(CHAR(PRINTNAME(CAR(destIdxs))), "c")))
        {
            //just to know that this will create a vector with the indexes being changed
            idxChanges->destIdxsVec = 1;
            incrementWaitingIdxChange();
        }
    }
    else
    {
        if (!toDiscard())
        {
            if (TYPEOF(destIdxs) == SYMSXP)
                //symbol refering to index
                //get the symbol
                idxChanges->destIdxs = getResult(CHAR(PRINTNAME(destIdxs)));
            else
                idxChanges->destIdxs = destIdxs;
        }
        idxChanges->nIdxChanges = Rf_length(idxChanges->destIdxs);
    }
}

void preProcessSrc(SEXP call)
{
    SEXP initialRhs = CAR(CDR(CDR(call)));
    SEXP rhs = CAR(CDR(CDR(call)));
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    idxChanges->src = R_NilValue;
    rollback:
    if (TYPEOF(rhs) == LANGSXP)
    {
        const char *rhsChar = CHAR(PRINTNAME(CAR(rhs)));

        if (strcmp(rhsChar, "[") == 0)
        {
            //uses another object content
            idxChanges->src = CAR(CDR(rhs));
            rhs = CAR(CDR(CDR(rhs)));
            goto rollback;
        }

        if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0))
        {
            //will need to wait for a vector of indexes
            if (idxChanges->src != R_NilValue)
            {
                idxChanges->srcIdxsVec = 1;
                incrementWaitingIdxChange();
            }
            //     printf("used a range or a combine from obj [%s]\n", CHAR(PRINTNAME(srcObj)));

            idxChanges->srcVec = 1;
            incrementWaitingIdxChange();
        }
    }
    else
    {
        //now try to find the src object in the registry
        if (TYPEOF(rhs) == SYMSXP)
        {
            SEXP srcIdxs = getResult(CHAR(PRINTNAME(rhs)));
            if (idxChanges->src != R_NilValue)
            {

                if (Rf_length(srcIdxs) > 1)
                {
                    idxChanges->srcVec = 1;
                    incrementWaitingIdxChange();
                }

                //means that rhs was treated because of brackets in it ex: b[idxs]
                //if the idxs.len > 1 will create a vector
                idxChanges->srcIdxs = srcIdxs;
            }
            else
            {
                //rhs did not got into the langsxp brances, so, this is the actual obj
                //will not create vector regardless of the src obj length
                idxChanges->src = rhs;
                idxChanges->srcValues = srcIdxs;
            }
        }
        else
        {
            //we'll assume that's a harcoded value for the index
            if (idxChanges->src != R_NilValue)
            {
                int index = (int)REAL(rhs)[0];
                int nDigits = floor(log10(abs(index))) + 1;
                int nameSize = strlen(CHAR(PRINTNAME(idxChanges->src))) + nDigits + 2; //+2 for the []
                char *requestValue = memAllocForString(nameSize);
                memset(requestValue, 0, nameSize);
                sprintf(requestValue, "%s[%d]", CHAR(PRINTNAME(idxChanges->src)), index);
                idxChanges->srcValues = getResult(requestValue);
                idxChanges->srcIdxs = rhs;
                free(requestValue);
            }
            else
                idxChanges->srcValues = rhs;
        }
    }
}

void preProcessDataFrameDest(SEXP call)
{

    SEXP currEnv = getCurrentEnv();
    CELL_CHANGE * cellChanges = getCurrCellChange();
    SEXP lhs = CAR(CDR(call));
    SEXP cols = R_NilValue;
    SEXP rows = R_NilValue;
    R_PrintData pars;
    PrintInit(&pars, currEnv);
    const char * lhs_str = getStrForStatement(lhs, &pars);

    Rboolean hasDollar = FALSE, hasBracket = FALSE;

    hasDollar = (strstr(lhs_str, "$") != ABD_NOT_FOUND);
    hasBracket = (strstr(lhs_str, "[") != ABD_NOT_FOUND);

    if (hasDollar && hasBracket) {
        //has dollar and bracket
        //will have column and row (specific cell or range of cells in column)
        cols = CAR(CDR(CDR(CAR(CDR(lhs)))));
        rows = CAR(CDR(CDR(lhs)));
        cellChanges->nCols = 1;
    }
    else if (hasDollar) {
        rows = R_NilValue;
        cols = CAR(CDR(CDR(lhs)));
        cellChanges->nCols = 1;
        //will change an entire column because there is no bracket to limit it
    }
    else {
        if (strstr(lhs_str, ",") != ABD_NOT_FOUND) {
            //contains comma, has a row and a column
            rows = CAR(CDR(CDR(lhs)));
            if (TYPEOF(rows) == SYMSXP)
                if (strcmp(CHAR(PRINTNAME(rows)), "") == 0)
                    rows = R_NilValue;
            cols = CAR(CDR(CDR(CDR(lhs))));
            if (TYPEOF(cols) == SYMSXP)
                if (strcmp(CHAR(PRINTNAME(cols)), "") == 0)
                    cols = R_NilValue;
        }
        else {
            //does not contains comma, will modify the entire column
            rows = R_NilValue;
            cols = CAR(CDR(CDR(lhs)));
            if (TYPEOF(cols) == SYMSXP)
                if (strcmp(CHAR(PRINTNAME(cols)), "") == 0)
                    cols = R_NilValue;
        }
    }

    if (rows != R_NilValue) {
        SEXPTYPE rowType = TYPEOF(rows);
        switch (rowType) {
        case LANGSXP: {
            //range
            SEXP tester = CAR(rows);
            if ((strcmp(CHAR(PRINTNAME(tester)), ":") == 0) || (strcmp(CHAR(PRINTNAME(tester)), "c"))) {
                cellChanges->waitingRowsVec = TRUE;
                incrementWaitingCellChange();
            }
            break;
        }
        case SYMSXP:
            //symbol
            cellChanges->toRows = rows;
            break;
        default:
            //hardcoded values
            cellChanges->toRows = rows;
            cellChanges->nRows = 1;
            break;
        }
    }
    else {
        cellChanges->nRows = -1;
    }

    if (cols != R_NilValue) {
        SEXPTYPE colType = TYPEOF(cols);
        switch (colType) {
        case LANGSXP: {
            //range
            SEXP tester = CAR(cols);
            if ((strcmp(CHAR(PRINTNAME(tester)), ":") == 0) || (strcmp(CHAR(PRINTNAME(tester)), "c"))) {
                cellChanges->waitingColsVec = TRUE;
                incrementWaitingCellChange();
            }
            break;
        }
        case SYMSXP:
            //symbol
            cellChanges->toCols = cols;

            break;
        default:
            //hardcoded values
            cellChanges->toCols = cols;
            cellChanges->nCols = 1;
            break;
        }

    }
    else {
        cellChanges->nCols = -1;
    }

}

void preProcessDataFrameSrc(SEXP call)
{
    /*
        Src can be many many things:
        -> other data frame (need to be the exact same size as the dest)
        -> group of vectors c(v1, v2, ..., vn)
            * c(1:3, a, c(3:5)) <- can be all this
        -> HC value(s)
            * x; 1; 1:3, c(1,2,3)

        we can test right away the rows and cols
        -> if CAR(CDR(CDR(RHS))) == SYMSXP and == ""
            -> its an empty bracket, means that:
                -> if not dollar seen : all the frame
                -> if dollar seen: all the rows
        -> if LANGSXP check
            -> if CAR == ":" will wait for something
            -> Then if that is the last and not dollar sign found, it is the cols and all the rows are used
            -> if not the last the it is the rows and the next is the cols
    */
    SEXP rhs = CAR(CDR(CDR(call)));
    SEXP currEnv = getCurrentEnv();
    CELL_CHANGE * cellChanges = getCurrCellChange();
    SEXPTYPE rhsType = TYPEOF(rhs);

    if (rhsType == LANGSXP) {
        const char *rhsChar = CHAR(PRINTNAME(CAR(rhs)));
        if (strcmp(rhsChar, "$") == 0)
        {

            cellChanges->srcSexpObj = CAR(CDR(rhs));
            cellChanges->srcCols = CAR(CDR(CDR(rhs)));
            cellChanges->srcNCols = 1;
            const char * colName = CHAR(asChar(cellChanges->srcCols));
            const char * objName = CHAR(PRINTNAME(cellChanges->srcSexpObj));
            int nameSize = strlen(colName) + strlen(objName) + 2;
            char *requestValue = memAllocForString(nameSize);
            memset(requestValue, 0, nameSize);
            sprintf(requestValue, "%s$%s", objName, colName);
            cellChanges->srcValues = getResult(requestValue);
            cellChanges->srcNRows = -1;

        }
        else if (strcmp(rhsChar, "[") == 0)
        {
            SEXP testForDollar = CAR(CDR(rhs));
            Rboolean dollarSeen = FALSE;
            SEXP rows = R_NilValue;
            SEXP cols = R_NilValue;
            SEXPTYPE rowsType = NILSXP, colsType = NILSXP;

            if (TYPEOF(testForDollar) == LANGSXP) {
                if (strcmp(CHAR(PRINTNAME(CAR(testForDollar))), "$") == 0) {
                    // extract the object here
                    cellChanges->srcSexpObj = CAR(CDR(testForDollar));
                    cols = CAR(CDR(CDR(testForDollar)));
                    colsType = TYPEOF(cols);
                    dollarSeen = TRUE;
                }
            }
            else
                cellChanges->srcSexpObj = testForDollar;

            SEXP srcVals = getResult(CHAR(PRINTNAME(cellChanges->srcSexpObj)));
            if (Rf_isFrame(srcVals)) {
                if (!dollarSeen) {
                    /*
                        dollar not seen, need to check if two arguments passed
                        this arguments can be:
                        -> [] <- all structure
                        -> [val] <- this val will inform columns. NOT ROWS
                        -> [,] <- ALL rows and cols
                        -> [,val] <- all rows, cols == val
                        -> [val,] <- rows == val, all cols
                        -> [val,val] <- rows == val, cols == val
                     */
                    rows = CAR(CDR(CDR(rhs)));
                    rowsType = TYPEOF(rows);
                    cols = CAR(CDR(CDR(CDR(rhs))));
                    colsType = TYPEOF(cols);
                    if (colsType == NILSXP) {
                        cols = rows;
                        colsType = TYPEOF(cols);
                        rowsType = NILSXP;
                        rows = R_NilValue;
                        cellChanges->srcRows = R_NilValue;
                    }


                }
                else {
                    rows = CAR(CDR(CDR(rhs)));
                    rowsType = TYPEOF(rows);
                }


                switch (colsType) {
                case NILSXP: break;
                case LANGSXP: {
                    const char *rhsChar = CHAR(PRINTNAME(CAR(cols)));
                    if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0))
                    {
                        cellChanges->waitingSrcCols = TRUE;
                        incrementWaitingCellChange();
                    }
                    break;
                }
                case SYMSXP:
                    if (strcmp(CHAR(PRINTNAME(cols)), "") == 0)
                        cols = R_NilValue;
                default:
                    cellChanges->srcCols = cols;
                    cellChanges->srcNCols = 1;
                    break;
                }

                if (rowsType != NILSXP) {
                    switch (rowsType) {
                    case NILSXP: break;
                    case LANGSXP: {
                        const char *rhsChar = CHAR(PRINTNAME(CAR(rows)));
                        if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0))
                        {
                            cellChanges->waitingSrcRows = TRUE;
                            incrementWaitingCellChange();
                            if (dollarSeen) {
                                cellChanges->waitingSrcValues = TRUE;
                                incrementWaitingCellChange();
                            }

                        }
                        break;
                    }
                    case SYMSXP:
                        if (strcmp(CHAR(PRINTNAME(rows)), "") == 0)
                            rows = R_NilValue;
                    default:
                        cellChanges->srcRows = rows;
                        break;
                    }
                }

                if ((!cellChanges->waitingSrcValues) && (cellChanges->srcValues == R_NilValue)) {
                    R_PrintData pars;
                    PrintInit(&pars, getCurrentEnv());
                    cellChanges->srcValues = getResult(getStrForStatement(rhs, &pars));
                    cellChanges->srcNCols = cellChanges->srcNRows = -1;

                }


            }
            else if (Rf_isVector(srcVals)) {
                rhs = CAR(CDR(CDR(rhs)));
                if (TYPEOF(rhs) == LANGSXP)
                {
                    const char *rhsChar = CHAR(PRINTNAME(CAR(rhs)));

                    if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0))
                    {

                        if (dollarSeen)
                            cellChanges->waitingSrcRows = TRUE;
                        else
                            cellChanges->waitingSrcCols= TRUE;

                        incrementWaitingCellChange();
                        cellChanges->waitingSrcValues = TRUE;
                        incrementWaitingCellChange();
                    }
                }
                else
                {
                    puts("in else");
                    //TODO: does not treat df[x,y] <- vec[symbol]
                    int index = (int)REAL(rhs)[0];
                    int nDigits = floor(log10(abs(index))) + 1;
                    int nameSize = strlen(CHAR(PRINTNAME(cellChanges->srcSexpObj))) + nDigits + 2; //+2 for the []
                    char *requestValue = memAllocForString(nameSize);
                    memset(requestValue, 0, nameSize);
                    sprintf(requestValue, "%s[%d]", CHAR(PRINTNAME(cellChanges->srcSexpObj)), index);
                    cellChanges->srcValues = getResult(requestValue);
                    cellChanges->srcNRows = 1;

                    cellChanges->srcCols = rhs;
                    free(requestValue);
                }
            }
        }

        else {
            //check hardcoded vector
            const char *rhsChar = CHAR(PRINTNAME(CAR(rhs)));
            if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0)) {
                cellChanges->waitingSrcValues = TRUE;
                cellChanges->srcNCols = -1;
                cellChanges->srcNRows = 0;
                incrementWaitingCellChange();
            }
        }

    }
    else {
        //symbol or harcoded
        if (rhsType == SYMSXP) {
            //symbol
            cellChanges->srcSexpObj = rhs;
            cellChanges->srcValues = getResult(CHAR(PRINTNAME(rhs)));
            cellChanges->srcNCols = Rf_length(cellChanges->srcValues);
            cellChanges->srcNRows = 1;
        }
        else {
            //hc value
            cellChanges->srcSexpObj = R_NilValue;
            cellChanges->srcValues = rhs;
            cellChanges->srcNCols = 1;
            cellChanges->srcNRows = 1;
        }
    }

    puts("\n\n\n\n");
}

void preProcessDataFrameCellChange(SEXP call, ABD_OBJECT *  targetObj, SEXP rho)
{
    CELL_CHANGE * cellChange = ABD_NOT_FOUND;
    setWatcherState(ABD_DISABLE);
    initCellChangeAuxVars();
    cellChange = getCurrCellChange();
    cellChange->targetObj = targetObj;
    cellChange->scriptLn = getCurrScriptLn();
    preProcessDataFrameDest(call);

    /* Src can be a data frame too!!!! */
    preProcessDataFrameSrc(call);

    /*
     to support the statement df[,cols] or df[rows, ] we need to check here the following:
     -> not waiting for vectors
     -> toRows == NilValue or toCols == NilValues
     if true, call finalize from here.
     */
    setWatcherState(ABD_ENABLE);
    if ((!cellChange->waitingSrcValues) && (!cellChange->waitingSrcCols) && (!cellChange->waitingSrcRows) && (!cellChange->waitingRowsVec) && (!cellChange->waitingColsVec) && (cellChange->toRows == R_NilValue || cellChange->toCols == R_NilValue)) {
        finalizeVarIdxChange(R_NilValue, getCurrentEnv());
    }
}

void preProcessVarIdxChange(SEXP call, ABD_OBJECT * targetObj, SEXP rho)
{
    initIdxChangeAuxVars();
    getCurrIdxChanges()->destObj = targetObj;
    preProcessDest(call);
    preProcessSrc(call);
}

ABD_EVENT *checkPendingVec(SEXP rhs2, SEXP vecVal)
{
    int mFlag = 0;
    if (vecValues == R_NilValue)
        return ABD_EVENT_NOT_FOUND;

    /* not null  */
    if (rhs2 == R_NilValue)
    {
        if (auxVecCall == R_NilValue || auxVecLine == 0)
        {
            return ABD_EVENT_NOT_FOUND;
        }
        mFlag = 1;
        rhs2 = auxVecCall;
        vecVal = vecValues;
        eventsRegTail->scriptLn = auxVecLine;
    }

    createNewEvent(VEC_EVENT);

    ABD_VEC_EVENT *vecEvent = eventsRegTail->data.vec_event;
    vecEvent->nElements = Rf_nrows(vecVal);
    vecEvent->toObj = ABD_OBJECT_NOT_FOUND;
    vecEvent->rangeL = -1001;
    vecEvent->rangeR = -1001;
    SEXP obj = R_NilValue;
    if (TYPEOF(rhs2) == LANGSXP)
    {
        /*
            GOT AN EXPRESSION
            Will treat only:
                -> a[something]
        */

        if (strcmp(CHAR(PRINTNAME(CAR(rhs2))), "[") == 0)
        {
            /* is an index from a variable */
            obj = CAR(CDR(rhs2));
            rhs2 = CAR(CDR(CDR(rhs2)));
        }
        if (strcmp(CHAR(PRINTNAME(CAR(rhs2))), ":") == 0)
        {
            /* range of indexes (raw numbers) */
            SEXP lElem = CAR(CDR(rhs2));

            if (TYPEOF(lElem) == SYMSXP)
                vecEvent->rangeL = asInteger(getResult(CHAR(PRINTNAME(lElem))));
            else if (TYPEOF(lElem) == REALSXP)
                vecEvent->rangeL = asInteger(lElem);
            else
                vecEvent->rangeL = 0;

            SEXP rElem = CAR(CDR(CDR(rhs2)));
            if (TYPEOF(rElem) == SYMSXP)
                vecEvent->rangeR = asInteger(getResult(CHAR(PRINTNAME(rElem))));
            else if (TYPEOF(rElem) == REALSXP)
                vecEvent->rangeR = asInteger(rElem);
            else
                vecEvent->rangeR = 0;
        }
    }

    if (obj != R_NilValue)
    {
        ABD_OBJECT *auxObj = findCmnObj(CHAR(PRINTNAME(obj)), getCurrentEnv());

        if (auxObj == ABD_OBJECT_NOT_FOUND)
        {
            vecEvent->fromObj = createUnscopedObj(CHAR(PRINTNAME(obj)), -2, -2, R_NilValue, 0);
            vecEvent->fromState = ABD_OBJECT_NOT_FOUND;
        }
        else
        {
            vecEvent->fromObj = auxObj;
            vecEvent->fromState = auxObj->modList;
        }
    }
    else
    {
        vecEvent->fromObj = ABD_OBJECT_NOT_FOUND;
        vecEvent->fromState = ABD_OBJECT_NOT_FOUND;
    }
    if (vecVal != vecValues || mFlag)
        return ABD_EVENT_NOT_FOUND;
    else
        return eventsRegTail;
}

ABD_EVENT *checkPendingFrame(SEXP call, SEXP rhs, ABD_OBJECT * usedObj)
{
    if (!(frameCall != R_NilValue && pendingFrame && frameCall == call))
        return ABD_EVENT_NOT_FOUND;
    SEXP currEnv = getCurrentEnv();
    createNewEvent(FRAME_EVENT);
    ABD_FRAME_EVENT *frameEvent = eventsRegTail->data.data_frame_event;

    frameEvent->nCols = numFrameSrcs;
    frameEvent->srcObjs = (ABD_OBJECT **)malloc(sizeof(ABD_OBJECT *) * numFrameSrcs);
    frameEvent->srcStates = (ABD_OBJECT_MOD **)malloc(sizeof(ABD_OBJECT_MOD *) * numFrameSrcs);
    frameEvent->fromIdxs = (int **)malloc(sizeof(int *) * numFrameSrcs);
    frameEvent->colNames = usedObj->modList->value.frame_value->colNames;
    frameEvent->numIdxs = (int **)malloc(sizeof(int *) * numFrameSrcs);


    for (int i = 0; i < numFrameSrcs; i++)
    {
        ABD_OBJECT *usedObj = ABD_OBJECT_NOT_FOUND;
        if (frameSrcs[i]->srcObj != R_NilValue)
        {
            const char *objName = CHAR(PRINTNAME(frameSrcs[i]->srcObj));
            if ((usedObj = findCmnObj(objName, currEnv)) == ABD_OBJECT_NOT_FOUND)
                usedObj = createUnscopedObj(objName, -2, -2, frameSrcs[i]->srcVal, 0);
        }
        else
            usedObj = createUnscopedObj("NA", -1, -1, frameSrcs[i]->srcVal, 0);

        frameEvent->srcObjs[i] = usedObj;
        frameEvent->srcStates[i] = usedObj->modList;

        frameEvent->numIdxs[i] = memAllocIntVector(1);

        if (frameSrcs[i]->srcIdxs == R_NilValue)
        {
            frameEvent->fromIdxs[i] = ABD_NOT_FOUND;
            frameEvent->numIdxs[i][0] = 0;
        }
        else
        {
            int numIdxs = Rf_nrows(frameSrcs[i]->srcIdxs);
            frameEvent->numIdxs[i][0] = numIdxs;
            frameEvent->fromIdxs[i] = memAllocIntVector(numIdxs);

            for (int j = 0; j < numIdxs; j++)
            {
                switch (TYPEOF(frameSrcs[i]->srcIdxs))
                {
                case REALSXP:
                    frameEvent->fromIdxs[i][j] = (int)REAL(frameSrcs[i]->srcIdxs)[j];
                    break;
                case INTSXP:
                    frameEvent->fromIdxs[i][j] = INTEGER(frameSrcs[i]->srcIdxs)[j];
                    break;
                }
                frameEvent->fromIdxs[i][j]--;
            }
        }
    }

    return eventsRegTail;
}

ABD_EVENT *checkPendings(SEXP call, SEXP rhs, ABD_OBJECT *obj)
{
    /* Check if exist an arithmetic event pending */
    ABD_EVENT *retValue = ABD_EVENT_NOT_FOUND;

    retValue = checkPendingFrame(call, rhs, obj);
    if (retValue != ABD_EVENT_NOT_FOUND)
        return retValue;

    retValue = checkPendingArith(rhs);
    if (retValue != ABD_EVENT_NOT_FOUND)
        return retValue;

    retValue = checkPendingRet(rhs, obj);
    if (retValue != ABD_EVENT_NOT_FOUND)
        return retValue;

    retValue = checkPendingVec(call, rhs);
    if (retValue != ABD_EVENT_NOT_FOUND)
        return retValue;


    return retValue;
}

ABD_EVENT *createMainEvent()
{
    return createNewEvent(MAIN_EVENT);
}

ABD_IDX_CHANGE_EVENT *setIdxList(ABD_IDX_CHANGE_EVENT *idxEvent)
{
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    if (idxChanges->srcIdxs == R_NilValue)
    {
        idxEvent->nIdxs = 1;
        idxEvent->fromIdxs = memAllocIntVector(idxEvent->nIdxs);
        idxEvent->fromIdxs[0] = 0;
        return idxEvent;
    }
    int destIdxSize = Rf_length(idxChanges->destIdxs);
    idxEvent->nIdxs = Rf_length(idxChanges->srcIdxs);
    idxEvent->fromIdxs = memAllocIntVector(idxEvent->nIdxs);
    for (int i = 0; i < idxEvent->nIdxs; i++)
    {
        switch (TYPEOF(idxChanges->srcIdxs))
        {
        case REALSXP:
            idxEvent->fromIdxs[i] = (int)REAL(idxChanges->srcIdxs)[i];
            break;
        case INTSXP:
            idxEvent->fromIdxs[i] = INTEGER(idxChanges->srcIdxs)[i];
            break;
        }
        idxEvent->fromIdxs[i]--;
    }

    /* int *srcIdxsUsed = ABD_OBJECT_NOT_FOUND;
    int nSrcIdxs = Rf_length(idxChanges->srcValues);
    srcIdxsUsed = memAllocIntVector(nSrcIdxs); */
    return idxEvent;
}

ABD_CELL_CHANGE_EVENT *setCellsForSrc(ABD_CELL_CHANGE_EVENT *cellChangeEvent)
{
    CELL_CHANGE *cellChanges = getCurrCellChange();
    int c, r;
    cellChangeEvent->nRowsIdxs = cellChanges->srcNRows;
    cellChangeEvent->nColsIdxs = cellChanges->srcNCols;
    cellChangeEvent->rowsIdxs = memAllocIntVector(cellChangeEvent->nRowsIdxs);
    cellChangeEvent->colsIdxs = memAllocIntVector(cellChangeEvent->nColsIdxs);
    cellChangeEvent->srcDims = getObjDim(cellChanges->srcObj);
    Rboolean doSeqCols = FALSE;
    Rboolean doSeqRows = FALSE;

    if (cellChanges->srcRows == R_NilValue)
        doSeqRows = TRUE;
    if (cellChanges->srcCols == R_NilValue)
        doSeqCols = TRUE;

    for (c = 0; c<cellChanges->srcNCols; c++) {
        if (!doSeqCols)
            cellChangeEvent->colsIdxs[c] = getIdxForSEXP(cellChanges->srcObj, cellChanges->srcCols, c);
        else
            cellChangeEvent->colsIdxs[c] = c;


        for (r=0; r<cellChanges->srcNRows; r++) {
            if (!doSeqRows)
            {
                switch (TYPEOF(cellChanges->srcRows))
                {
                case REALSXP:
                    cellChangeEvent->rowsIdxs[r] = (int)REAL(cellChanges->srcRows)[r];
                    break;
                case INTSXP:
                    cellChangeEvent->rowsIdxs[r] = INTEGER(cellChanges->srcRows)[r];
                    break;
                }
            }
            else
                cellChangeEvent->rowsIdxs[r] = r;

        }
    }

    /* int *srcIdxsUsed = ABD_OBJECT_NOT_FOUND;
    int nSrcIdxs = Rf_length(idxChanges->srcValues);
    srcIdxsUsed = memAllocIntVector(nSrcIdxs); */
    return cellChangeEvent;
}

void createIndexChangeEvent(SEXP rhs, ABD_OBJECT *objUsed)
{
    ABD_EVENT *fromEvent = checkPendings(R_NilValue, rhs, objUsed);
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    /* Create the new assignment event */

    createNewEvent(IDX_EVENT);

    /* get the tail from the events registry, to reduce code verbose */
    ABD_IDX_CHANGE_EVENT *currIdxEvent = eventsRegTail->data.idx_event;

    currIdxEvent->toObj = objUsed;
    currIdxEvent->toState = objUsed->modList;
    if (fromEvent != ABD_EVENT_NOT_FOUND)
    {
        /* has precedence from another event */
        currIdxEvent->fromType = ABD_E;
        currIdxEvent->fromObj = fromEvent;
        currIdxEvent->fromState = ABD_OBJECT_NOT_FOUND;
        if (fromEvent->type == VEC_EVENT)
            fromEvent->data.vec_event->toObj = objUsed;
    }
    else
    {
        currIdxEvent->fromState = ABD_OBJECT_NOT_FOUND;
        currIdxEvent->fromType = ABD_O;
        if (idxChanges->src != R_NilValue)
        {

            if ((currIdxEvent->fromObj = findCmnObj(CHAR(PRINTNAME(idxChanges->src)), getCurrentEnv())) == ABD_OBJECT_NOT_FOUND)
                currIdxEvent->fromObj = createUnscopedObj(CHAR(PRINTNAME(idxChanges->src)), -2, -2, idxChanges->srcValues, 0);
            currIdxEvent = setIdxList(currIdxEvent);
        }
        else
        {
            //hardcoded value???
            currIdxEvent->fromObj = createUnscopedObj("NA", -1, -1, idxChanges->srcValues, 0);
        }
        currIdxEvent->fromState = ((ABD_OBJECT *)currIdxEvent->fromObj)->modList;
    }
}

void createCellChangeEvent(SEXP rhs, ABD_OBJECT *objUsed)
{

    ABD_EVENT *fromEvent = checkPendings(R_NilValue, rhs, objUsed);
    CELL_CHANGE *cellChanges = getCurrCellChange();


    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    /* Create the new assignment event */

    createNewEvent(CELL_EVENT);

    /* get the tail from the events registry, to reduce code verbose */
    ABD_CELL_CHANGE_EVENT *currCellEvent = eventsRegTail->data.cell_change_event;
    eventsRegTail->scriptLn = cellChanges->scriptLn;

    currCellEvent->toObj = objUsed;
    currCellEvent->toState = objUsed->modList;
    puts("creating cell change event");
    if (fromEvent != ABD_EVENT_NOT_FOUND)
    {
        puts("got an event for cellChange");
        /* has precedence from another event */
        currCellEvent->fromType = ABD_E;
        currCellEvent->fromObj = fromEvent;
        currCellEvent->fromState = ABD_OBJECT_NOT_FOUND;
        if (fromEvent->type == VEC_EVENT)
            fromEvent->data.vec_event->toObj = objUsed;
    }
    else
    {
        currCellEvent->fromState = ABD_OBJECT_NOT_FOUND;
        currCellEvent->fromType = ABD_O;
        if (cellChanges->srcSexpObj != R_NilValue)
        {
            const char * srcName = CHAR(PRINTNAME(cellChanges->srcSexpObj));
            if ((currCellEvent->fromObj = findCmnObj(srcName, getCurrentEnv())) == ABD_OBJECT_NOT_FOUND)
                currCellEvent->fromObj = createUnscopedObj(srcName, -2, -2, cellChanges->srcValues, 0);
            cellChanges->srcObj = currCellEvent->fromObj;
            currCellEvent = setCellsForSrc(currCellEvent);
        }
        else
        {
            //hardcoded value???
            currCellEvent->fromObj = createUnscopedObj("NA", -1, -1, cellChanges->srcValues, 0);
        }
        currCellEvent->fromState = ((ABD_OBJECT *)currCellEvent->fromObj)->modList;
    }
}

void createAsgnEvent(ABD_OBJECT *objUsed, SEXP rhs, SEXP rhs2, SEXP rho)
{
    /* check if the value origin */
    ABD_EVENT *fromEvent = checkPendings(rhs2, rhs, objUsed);

    /* Create the new assignment event */

    createNewEvent(ASGN_EVENT);

    /* get the tail from the events registry, to reduce code verbose */
    ABD_ASSIGN_EVENT *currAssign = eventsRegTail->data.asgn_event;
    currAssign->value = objUsed->modList;
    currAssign->toObj = objUsed;
    currAssign->withIndex = -1;
    if (fromEvent != ABD_EVENT_NOT_FOUND)
    {
        /* has precedence from another event */
        currAssign->fromType = ABD_E;
        currAssign->fromObj = fromEvent;
        currAssign->fromState = ABD_OBJECT_NOT_FOUND;
        if (fromEvent->type == VEC_EVENT)
            fromEvent->data.vec_event->toObj = objUsed;
    }
    else
    {
        /*
            Do not have precedence from another event
            Need to process the rhs (from the call) to parse the origin
        */
        ABD_OBJECT *fromObj = ABD_OBJECT_NOT_FOUND;

        if (inLoopByType(ABD_FOR) && loopStack->loop.forLoop->iterator->id == objUsed->id)
        {

            loopStack->currIter->iteratorState = objUsed->modList;
            fromObj = loopStack->loop.forLoop->enumerator;
            currAssign->fromState = loopStack->loop.forLoop->enumState;

            if (loopStack->loop.forLoop->fromIdxs != ABD_NOT_FOUND)
                currAssign->withIndex = loopStack->loop.forLoop->fromIdxs[forValPos];
            else
                currAssign->withIndex = -1;
        }
        else
        {
            int withIndex = 0;
            if (TYPEOF(rhs2) == LANGSXP)
            {
                /*
                    GOT AN EXPRESSION
                    Will treat only:
                        -> a[idx]
                */

                if (strcmp(CHAR(PRINTNAME(CAR(rhs2))), "[") == 0)
                {
                    /* is an index from a variable */
                    withIndex = asInteger(CAR(CDR(CDR(rhs2))));
                    rhs2 = CAR(CDR(rhs2));
                }
            }
            if (TYPEOF(rhs2) == SYMSXP)
            {
                if ((fromObj = findCmnObj(CHAR(PRINTNAME(rhs2)), rho)) == ABD_OBJECT_NOT_FOUND)
                {
                    //not mapped obj
                    withIndex = (withIndex == 0) ? 1 : withIndex;
                    fromObj = createUnscopedObj(CHAR(PRINTNAME(rhs2)), -2, -2, R_NilValue, 0);
                    currAssign->withIndex = withIndex;
                    currAssign->fromState = ABD_OBJECT_NOT_FOUND;
                }
                else
                {
                    currAssign->fromState = fromObj->modList;
                    currAssign->withIndex = withIndex - 1;
                }
            }
            else
            {
                fromObj = createUnscopedObj("NA", -1, -1, R_NilValue, 0);
                currAssign->fromState = ABD_OBJECT_NOT_FOUND;
                currAssign->withIndex = -1;
            }
        }
        currAssign->fromType = ABD_O;
        currAssign->fromObj = fromObj;
    }
}

void parseDataFrameSrcs(SEXP call, int i)
{
    frameSrcs[i] = (FRAME_CREATION *)malloc(sizeof(FRAME_CREATION));
    frameSrcs[i]->srcObj = R_NilValue;
    frameSrcs[i]->srcVal = R_NilValue;
    frameSrcs[i]->srcIdxs = R_NilValue;
    frameSrcs[i]->srcVec = FALSE;
    frameSrcs[i]->srcIdxsVec = FALSE;
    frameSrcs[i]->discard = FALSE;
    waitingFrameIdxs[i] = 0;

    rollback:
    if (TYPEOF(call) == LANGSXP)
    {
        const char *rhsChar = CHAR(PRINTNAME(CAR(call)));

        if (strcmp(rhsChar, "[") == 0)
        {
            //uses object content
            frameSrcs[i]->srcObj = CAR(CDR(call));
            call = CAR(CDR(CDR(call)));
            goto rollback;
        }

        if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0))
        {
            //will need to wait for a vector of indexes
            if (frameSrcs[i]->srcObj != R_NilValue)
            {

                frameSrcs[i]->srcIdxsVec = TRUE;
                // incrementWaitingIdxChange();
                waitingFrameVecs++;
                waitingFrameIdxs[i]++;
            }
            //     printf("used a range or a combine from obj [%s]\n", CHAR(PRINTNAME(srcObj)));

            frameSrcs[i]->srcVec = TRUE;
            // incrementWaitingIdxChange();
            waitingFrameVecs++;
            waitingFrameIdxs[i]++;
        }
    }
    else
    {
        //now try to find the src object in the registry
        if (TYPEOF(call) == SYMSXP)
        {
            SEXP srcIdxs = getResult(CHAR(PRINTNAME(call)));
            if (frameSrcs[i]->srcObj != R_NilValue)
            {

                if (Rf_length(srcIdxs) > 1)
                {
                    frameSrcs[i]->srcVec = TRUE;
                    // incrementWaitingIdxChange();
                    waitingFrameVecs++;
                    waitingFrameIdxs[i]++;
                }

                //means that rhs was treated because of brackets in it ex: b[idxs]
                //if the idxs.len > 1 will create a vector
                frameSrcs[i]->srcIdxs = srcIdxs;
            }
            else
            {
                //rhs did not got into the langsxp brances, so, this is the actual obj
                //will not create vector regardless of the src obj length
                frameSrcs[i]->srcObj = call;
                frameSrcs[i]->srcVal = srcIdxs;
            }
        }
        else
        {
            //we'll assume that's a harcoded value for the index
            if (frameSrcs[i]->srcObj != R_NilValue)
            {
                int index = (int)REAL(call)[0];
                int nDigits = floor(log10(abs(index))) + 1;
                int nameSize = strlen(CHAR(PRINTNAME(frameSrcs[i]->srcObj))) + nDigits + 2; //+2 for the []
                char *requestValue = memAllocForString(nameSize);
                memset(requestValue, 0, nameSize);
                sprintf(requestValue, "%s[%d]", CHAR(PRINTNAME(frameSrcs[i]->srcObj)), index);
                frameSrcs[i]->srcVal = getResult(requestValue);
                frameSrcs[i]->srcIdxs = call;
                free(requestValue);
            }
            else
                frameSrcs[i]->srcVal = call;
        }
    }
}

int getFrameSrcsCount(SEXP call)
{
    int i = 0;
    for (; call != R_NilValue; i++, call = CDR(call))
        ;
    numFrameSrcs = i;
    waitingFrameIdxs = (int *)malloc(sizeof(int) * i);
    return i;
}

void preProcessDataFrame(SEXP call)
{

    int i = 1;
    waitingFrameVecs = 0;
    pendingFrame = TRUE;

    frameSrcs = (FRAME_CREATION **)malloc(sizeof(FRAME_CREATION *) * getFrameSrcsCount(call));
    for (i = 0; call != R_NilValue; i++, call = CDR(call))
        parseDataFrameSrcs(CAR(call), i);
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
                int line = asInteger(srcref);
                return (R_Interactive) ? line : line + 1;
            }
        }
    }
    /* default: */
    // this happens running Rscript or interactively outside of a function
    return 0;
}
void preProcessEnumerator(SEXP enumerator)
{

    SEXP rhs = enumerator;
    loopStack->loop.forLoop->enumSEXP = R_NilValue;
    rollback22:
    if (TYPEOF(rhs) == LANGSXP)
    {
        const char *rhsChar = CHAR(PRINTNAME(CAR(rhs)));
        if (strcmp(rhsChar, "[") == 0)
        {
            //uses another object content
            loopStack->loop.forLoop->enumSEXP = CAR(CDR(rhs));
            rhs = CAR(CDR(CDR(rhs)));
            goto rollback22;
        }

        if ((strcmp(rhsChar, ":") == 0) || (strcmp(rhsChar, "c") == 0))
        {

            if (loopStack->loop.forLoop->enumSEXP != R_NilValue)
            {
                /*
                    use a subset of the symbol
                    this tells that two vectors will be created
                */

                forIdxsVec = TRUE;
                forValVec = TRUE;
                waitingForVecs += 2;
            }
            else
            {
                forIdxsVec = FALSE;
                forValVec = TRUE;
                waitingForVecs++;
            }
        }
    }
    else
    {
        //now try to find the src object in the registry
        if (TYPEOF(rhs) == SYMSXP)
        {
            /*  used the whole symbol as enumerator */
            loopStack->loop.forLoop->enumSEXP = rhs;
        }
    }
}

Rboolean inLoopByType(ABD_LOOP_TAGS type)
{
    if (loopStack == ABD_NOT_FOUND)
        return FALSE;
    return loopStack->loopType == type ? TRUE : FALSE;
}

void createNewLoopIteration(int iterId, ABD_LOOP_TAGS type)
{

    ITERATION *currIterList = loopStack->currIter;
    if (currIterList == ABD_EVENT_NOT_FOUND)
    {
        //first iteration
        if (type == ABD_FOR)
        {
            loopStack->loop.forLoop->itList = memAllocIteration();
            loopStack->loop.forLoop->itList->nextIter = ABD_EVENT_NOT_FOUND;
            loopStack->currIter = loopStack->loop.forLoop->itList;
        }
        else if (type == ABD_REPEAT)
        {
            loopStack->loop.repeatLoop->itList = memAllocIteration();
            loopStack->loop.repeatLoop->itList->nextIter = ABD_EVENT_NOT_FOUND;
            loopStack->currIter = loopStack->loop.repeatLoop->itList;
        }
        else if (type == ABD_WHILE)
        {
            loopStack->loop.whileLoop->itList = memAllocIteration();
            loopStack->loop.whileLoop->itList->nextIter = ABD_EVENT_NOT_FOUND;
            loopStack->currIter = loopStack->loop.whileLoop->itList;
        }

        currIterList = loopStack->currIter;
    }
    else
    {

        currIterList->nextIter = memAllocIteration();
        currIterList = currIterList->nextIter;
        currIterList->nextIter = ABD_EVENT_NOT_FOUND;
        loopStack->currIter = currIterList;
    }

    if (type == ABD_FOR)
        loopStack->loop.forLoop->iterCounter++;
    else if (type == ABD_REPEAT)
        loopStack->loop.repeatLoop->iterCounter++;
    else if (type == ABD_WHILE)
        loopStack->loop.whileLoop->iterCounter++;

    loopStack->currIter->eventsList = ABD_EVENT_NOT_FOUND;
    loopStack->currIter->eventsListTail = ABD_EVENT_NOT_FOUND;
    loopStack->currIter->iterId = ++iterId;
}

void pushNewLoop(ABD_LOOP_TAGS type, void *newEvent)
{

    if (loopStack == ABD_NOT_FOUND)
    {
        loopStack = memAllocLoopChain();
        loopStack->prevLoop = ABD_NOT_FOUND;
    }
    else
    {
        ABD_LOOP_CHAIN *newLoop = memAllocLoopChain();
        newLoop->prevLoop = loopStack;
        loopStack = newLoop;
    }

    loopStack->loopType = type;
    waitingForVecs = 0;
    loopStack->initialBranchDepth = getCurrBranchDepth();
    loopStack->currIter = ABD_EVENT_NOT_FOUND;
    if (type == ABD_FOR)
        pushForEvent((ABD_FOR_LOOP_EVENT *)newEvent);
    else if (type == ABD_REPEAT)
        pushWhileEvent((ABD_WHILE_LOOP_EVENT *)newEvent);
    else if (type == ABD_WHILE)
        pushRepeatEvent((ABD_REPEAT_LOOP_EVENT *)newEvent);
}

void pushForEvent(ABD_FOR_LOOP_EVENT *newForEvent)
{
    loopStack->loop.forLoop = newForEvent;

    forIdxsVec = FALSE;
    forValVec = FALSE;

    loopStack->loop.forLoop->enumSEXP = R_NilValue;
    loopStack->loop.forLoop->idxVec = R_NilValue;
    loopStack->loop.forLoop->valVec = R_NilValue;
    loopStack->loop.forLoop->iterCounter = 0;
    loopStack->loop.forLoop->estIterNumber = 0;
    loopStack->loop.forLoop->iterator = ABD_OBJECT_NOT_FOUND;
    loopStack->loop.forLoop->enumerator = ABD_OBJECT_NOT_FOUND;
    loopStack->loop.forLoop->enumState = ABD_OBJECT_NOT_FOUND;
    loopStack->loop.forLoop->itList = ABD_NOT_FOUND;
    loopStack->loop.forLoop->lastEvent = ABD_EVENT_NOT_FOUND;
    loopStack->loop.forLoop->fromIdxs = ABD_NOT_FOUND;
}

void pushRepeatEvent(ABD_REPEAT_LOOP_EVENT *newRepeatEvent)
{

    loopStack->loop.repeatLoop = newRepeatEvent;
    loopStack->loop.repeatLoop->iterCounter = 0;
    loopStack->loop.repeatLoop->itList = ABD_NOT_FOUND;
    loopStack->loop.repeatLoop->lastEvent = ABD_EVENT_NOT_FOUND;
}

void pushWhileEvent(ABD_WHILE_LOOP_EVENT *newWhileLoopEvent)
{
    loopStack->loop.whileLoop = newWhileLoopEvent;
    loopStack->loop.whileLoop->iterCounter = 0;
    loopStack->loop.whileLoop->itList = ABD_NOT_FOUND;
    loopStack->loop.whileLoop->lastEvent = ABD_EVENT_NOT_FOUND;
}

void addEventToCurrentLoop(ABD_EVENT *newEvent)
{
    if (loopStack->currIter == ABD_NOT_FOUND)
        return;

    ITERATION *currIter = loopStack->currIter;
    if (currIter->eventsListTail == ABD_EVENT_NOT_FOUND)
    {
        currIter->eventsList = memAllocIterEventList();
        currIter->eventsListTail = currIter->eventsList;
        currIter->eventsList->nextEvent = ABD_EVENT_NOT_FOUND;
    }
    else
    {
        currIter->eventsListTail->nextEvent = memAllocIterEventList();
        currIter->eventsListTail = currIter->eventsListTail->nextEvent;
        currIter->eventsListTail->nextEvent = ABD_EVENT_NOT_FOUND;
    }
    currIter->eventsListTail->event = newEvent;
}

void popLoopFromStack(ABD_LOOP_TAGS requestingType)
{
    if (loopStack->loopType != requestingType)
        return;

    ABD_LOOP_CHAIN *toPop = loopStack;
    loopStack = toPop->prevLoop;
    forceBranchDepth(toPop->initialBranchDepth);
    free(toPop);
}

void appendLastEventToLoop(ABD_LOOP_TAGS type)
{
    if (type == ABD_FOR)
        loopStack->loop.forLoop->lastEvent = eventsRegTail;
    else if (type == ABD_REPEAT)
        loopStack->loop.repeatLoop->lastEvent = eventsRegTail;
    else if (type == ABD_WHILE)
        loopStack->loop.whileLoop->lastEvent = eventsRegTail;
}

void finalizeForEventProcessing()
{
    ABD_FOR_LOOP_EVENT *currFor = loopStack->loop.forLoop;
    if (currFor->enumSEXP != R_NilValue)
    {
        // used symbol as values source
        ABD_OBJECT *foundObj = findCmnObj(CHAR(PRINTNAME(currFor->enumSEXP)), getCurrentEnv());

        if (currFor->idxVec == R_NilValue)
        {
            //sourced all the symbol (no values stored)
            currFor->valVec = getResult(CHAR(PRINTNAME(currFor->enumSEXP)));
            if (foundObj == ABD_OBJECT_NOT_FOUND)
                foundObj = createUnscopedObj(CHAR(PRINTNAME(currFor->enumSEXP)), -2, -2, currFor->valVec, 0);
            currFor->estIterNumber = Rf_length(currFor->valVec);
            currFor->fromIdxs = memAllocIntVector(currFor->estIterNumber);
            for (int i = 0; i < currFor->estIterNumber; i++)
                currFor->fromIdxs[i] = i;
        }
        else
        {
            // sourced part of the symbol (have values and idx)
            currFor->estIterNumber = Rf_length(currFor->idxVec);
            if (foundObj == ABD_OBJECT_NOT_FOUND)
            {
                foundObj = createUnscopedObj(CHAR(PRINTNAME(currFor->enumSEXP)), -2, -2, currFor->valVec, 0);
            }

            currFor->fromIdxs = memAllocIntVector(currFor->estIterNumber);
            for (int i = 0; i < currFor->estIterNumber; i++)
            {

                if (TYPEOF(currFor->idxVec) == REALSXP)
                    currFor->fromIdxs[i] = ((int)(REAL(currFor->idxVec)[i])) - 1;
                else if (TYPEOF(currFor->idxVec) == INTSXP)
                    currFor->fromIdxs[i] = INTEGER(currFor->idxVec)[i] - 1;
            }
        }

        currFor->enumerator = foundObj;
        currFor->enumState = foundObj->modList;
    }
    else
    {
        // declared vector on the fly (harcoded, we got values but not idxs)
        currFor->enumerator = createUnscopedObj("NA", -1, -1, currFor->valVec, 0);
        currFor->estIterNumber = Rf_length(currFor->valVec);
        currFor->enumState = currFor->enumerator->modList;
        currFor->fromIdxs = memAllocIntVector(currFor->estIterNumber);
        for (int i = 0; i < currFor->estIterNumber; i++)
            currFor->fromIdxs[i] = i;
    }
}

void storeVecDataFrameEvent(SEXP vec)
{

    int i;
    for (i = 0; i < numFrameSrcs; i++)
        if (waitingFrameIdxs[i] != 0)
        {
            waitingFrameIdxs[i]--;
            break;
        }

    if (frameSrcs[i]->srcIdxsVec)
    {
        frameSrcs[i]->srcIdxs = vec;
        frameSrcs[i]->srcIdxsVec = FALSE;
        waitingFrameVecs--;
        return;
    }

    if (frameSrcs[i]->srcVec)
    {
        frameSrcs[i]->srcVal = vec;
        frameSrcs[i]->srcVec = FALSE;
        waitingFrameVecs--;
        return;
    }
}

void storeVecForEvent(SEXP vec)
{
    //
    if (forIdxsVec)
    {
        loopStack->loop.forLoop->idxVec = vec;
        forIdxsVec = FALSE;
        waitingForVecs--;
        return;
    }

    if (forValVec)
    {
        loopStack->loop.forLoop->valVec = vec;
        forValVec = FALSE;
        waitingForVecs--;
        return;
    }
}

void setForEventValues(SEXP call, ABD_FOR_LOOP_EVENT *newForEvent, SEXP enumerator)
{

    pushNewLoop(ABD_FOR, newForEvent);
    preProcessEnumerator(enumerator);

    /* printf("iterator... TYPE %d\n", TYPEOF(iterator));
    PrintIt(iterator, getCurrentEnv());

    printf("enumerator... TYPE %d\n", TYPEOF(enumerator));
    PrintIt(enumerator, getCurrentEnv());
 */
    if (!waitingForVecs)
        finalizeForEventProcessing();
}

ABD_SEARCH checkRetStored(SEXP testValue)
{
    return (testValue == possibleRet) ? ABD_EXIST : ABD_NOT_EXIST;
}

void storeRetValues(SEXP value)
{
    possibleRetLine = getCurrScriptLn();
    possibleRet = value;
}

void eventPrint(ABD_EVENT *event)
{

    switch (event->type)
    {
    case MAIN_EVENT:
        printf("Running on main event...\n");
        break;

    case FUNC_EVENT:
        printf("Running on a func event...\nFunction name: %s\n", event->data.func_event->called->name);
        break;

    default:
        printf("None\n");
        break;
    }
}
