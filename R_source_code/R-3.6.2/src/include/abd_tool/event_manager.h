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

void initEventsReg()
{
    eventsReg = ABD_EVENT_NOT_FOUND;
    eventsRegTail = ABD_EVENT_NOT_FOUND;
    lastRetValue = ABD_EVENT_NOT_FOUND;

    /* ARITH VARS */
    lastArithEvent = ABD_EVENT_NOT_FOUND;
    finalArithAns = R_NilValue;
    finalArithCall = R_NilValue;
    arithScriptLn = 0;
    currArithIndex = -1;
    arithResults = ABD_NOT_FOUND;

    /* VEC VARS*/
    vecValues = R_NilValue;

    eventsReg = createMainEvent();
    eventsRegTail = eventsReg;
    waitingElseIF = 0;
    eventCounter = 0;
}

ABD_EVENT *initBaseEvent(ABD_EVENT *newBaseEvent)
{
    newBaseEvent->id = ++eventCounter;
    newBaseEvent->scriptLn = getCurrScriptLn();
    newBaseEvent->data.if_event = ABD_NOT_FOUND;
    newBaseEvent->data.func_event = ABD_NOT_FOUND;
    newBaseEvent->data.ret_event = ABD_NOT_FOUND;
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

ABD_EVENT_ARG *setArgValues(ABD_EVENT_ARG *currArg, ABD_OBJECT *objPtr, const char *rcvdName, ABD_OBJECT_MOD *objValue)
{

    int rcvdNameSize = strlen(rcvdName);

    currArg->objPtr = objPtr;

    currArg->rcvdName = memAllocForString(rcvdNameSize);
    copyStr(currArg->rcvdName, rcvdName, rcvdNameSize);

    currArg->objValue = objValue;
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
ABD_OBJECT_MOD *processVector(SEXP symbolValue, ABD_OBJECT_MOD *mod, int idxChange)
{
    SEXPTYPE modType = TYPEOF(symbolValue);
rollback2:
    switch (modType)
    {
    case CLOSXP:
        break;
    case INTSXP:
        if (idxChange)
            mod = setModValues(mod, symbolValue, intVectorMultiChanges);
        else
            mod = setModValues(mod, symbolValue, createIntVector);
        mod->value.vec_value->type = modType;
        break;
    case REALSXP:
        if (idxChange)
            mod = setModValues(mod, symbolValue, realVectorMultiChanges);
        else
            mod = setModValues(mod, symbolValue, createRealVector);
        mod->value.vec_value->type = modType;
        break;
    case STRSXP:
        puts("character vectors");
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
        puts("list (generic vector");
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
    return mod;
}
ABD_OBJECT_MOD *processByType(SEXP symbolValue, ABD_OBJECT_MOD *mod, int idxChange)
{

    switch (getObjStructType(symbolValue))
    {
    case ABD_VECTOR:
        mod->valueType = ABD_VECTOR;
        mod = processVector(symbolValue, mod, idxChange);
        break;
    case ABD_MATRIX:
        mod->valueType = ABD_MATRIX;
        break;
    default:
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
ABD_EVENT_ARG *processArgs(SEXP passedArgs, SEXP receivedArgs)
{
    ABD_EVENT_ARG *argsList = ABD_NOT_FOUND;
    ABD_EVENT_ARG *currentArg = ABD_NOT_FOUND;
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT_MOD *objValue = ABD_NOT_FOUND;

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
            break;
        case SYMSXP:
        {
            //its a variable
            const char *passedName = CHAR(PRINTNAME(symbol));
            //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
            objectFound = findObj(cmnObjReg, passedName, getCurrentEnv());
            if (objectFound == ABD_OBJECT_NOT_FOUND)
            {
                //object not in registry
                SEXP rStrName = mkString(passedName);
                //request R for the object (not hardcoded)

                SEXP symbValue = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
                objectFound = createUnscopedObj(passedName, -2, -2, symbValue, 0);
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
        currentArg = setArgValues(currentArg, objectFound, rcvdName, objValue);
        objectFound = ABD_OBJECT_NOT_FOUND;
        objValue = ABD_OBJECT_NOT_FOUND;
    }
    return argsList;
}

int inCollection(const char *check)
{
    char *collection[] = {
        "+", "-", "*", "/",
        "==", "!=", "<", ">", "<=", ">=",
        "&", "|", "&&", "||", "!", "["};
    int nCollection = 16;

    for (int i = 0; i < nCollection; i++)
    {
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
        objectFound = findObj(cmnObjReg, objName, getCurrentEnv());
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
    printf("in get result with str: %s\n", expr);
    PROTECT(tmp = mkString(expr));
    PROTECT(e = R_ParseVector(tmp, -1, &status, R_NilValue));
    PROTECT(retValue = R_tryEval(VECTOR_ELT(e, 0), getCurrentEnv(), NULL));

    UNPROTECT(3);
    puts("out get result");
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
IF_EXPRESSION *processIfStmt(SEXP st, int withEval)
{

    IF_EXPRESSION *newExpr = memAllocIfExp();
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
            /* pick from the results array */
            newExpr->result = REAL(arithResults[++currArithIndex])[0];
        }
    }
    setWatcherState(ABD_ENABLE);
    //printf("\nStatement %s\nResult %s\n", stmtStr, (newExpr->result) ? "TRUE" : "FALSE");
    free(stmtStr);
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
ABD_IF_EVENT *findLastElseIf()
{
    ABD_IF_EVENT *currEvent = eventsRegTail->data.if_event;
    while (currEvent->else_if != ABD_EVENT_NOT_FOUND)
        currEvent = currEvent->else_if;
    return currEvent;
}
void setIfEventValues(SEXP statement, Rboolean result)
{
    ABD_IF_EVENT *currEvent = findLastElseIf();

    if (isWaitingElseIf())
    {
        if (statement == R_NilValue)
        {
            //is an else
            currEvent->else_if = memAllocIfEvent();
            currEvent->else_if->else_if = ABD_EVENT_NOT_FOUND;
            currEvent->else_if->expr = ABD_NOT_FOUND;
            currEvent->else_if->globalResult = 1;
            currEvent->else_if->isElse = 1;
            waitingElseIF = 0;
        }
        else
        {
            //is an else if
            currEvent->else_if = memAllocIfEvent();
            currEvent->else_if->expr = processIfStmt(statement, 1);
            currEvent->else_if->isElse = 0;
            currEvent->else_if->globalResult = result;
        }
    }
    else
    {
        currEvent->else_if = ABD_EVENT_NOT_FOUND;
        currEvent->isElse = 0;
        currEvent->globalResult = result;
        currEvent->expr = processIfStmt(statement, 1);
    }
}

void setFuncEventValues(ABD_OBJECT *callingObj, SEXP newRho, SEXP passedArgs, SEXP receivedArgs)
{
    eventsRegTail->data.func_event->toEnv = newRho;
    eventsRegTail->data.func_event->caller = getCurrFuncObj();
    eventsRegTail->data.func_event->called = callingObj;
    eventsRegTail->data.func_event->args = processArgs(passedArgs, receivedArgs);
    envPush(newRho, callingObj);
}

void setRetEventValue(SEXP value)
{
    ABD_OBJECT_MOD *valueABD = memAllocMod();
    valueABD = processByType(value, valueABD, 0);
    valueABD->id = -1;
    eventsRegTail->data.ret_event->toObj = ABD_OBJECT_NOT_FOUND;
    eventsRegTail->data.ret_event->from = getCurrFuncObj();
    eventsRegTail->data.ret_event->retValue = valueABD;
    lastRetValue = value;
}

void setAsgnEventValues(ABD_OBJECT *toObj, SEXP value)
{
}

SEXP getSavedArithAns()
{
    return finalArithAns;
}

void setArithEventValues(SEXP call, SEXP ans, SEXP arg1, SEXP arg2, int withPre)
{
    // PrintIt(call, getCurrentEnv());

    // SEXP _arg1 = CAR(CDR(call));
    // SEXP _arg2 = CAR(CDR(CDR(call)));
    // int withIndex = 0;
    // int opSize = strlen(CHAR(PRINTNAME(CAR(call))));
    // char *operator= memAllocForString(opSize);
    // memset(operator, 0, opSize);
    // copyStr(operator, CHAR(PRINTNAME(CAR(call))), opSize);
    // if (!withPre)
    // {

    //     lastArithAns = ans;
    // }
    // else
    // {
    //     /* one pending ARITH  */
    //     if (lastArithAns == arg1)
    //     {
    //         /* precedence in arg1*/
    //         printf("Arg1 is from last evaluated portion\n");
    //     }
    //     else if (lastArithAns == arg2)
    //     {
    //         /* precedence in arg2 */
    //         printf("Arg2 is from last evaluated portion\n");
    //     }
}

void tmpStoreArith(SEXP call, SEXP ans)
{
    if (arithResults == ABD_NOT_FOUND)
    {
        arithResults = (SEXP *)malloc(sizeof(SEXP) * 10);
        for (int i = 0; i < 10; i++)
            arithResults[i] = R_NilValue;
    }
    arithResults[++currArithIndex] = ans;
    finalArithAns = ans;
    finalArithCall = call;
    arithScriptLn = getCurrScriptLn();
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
        newBaseEvent->data.if_event->else_if = ABD_EVENT_NOT_FOUND;
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
    default:
        break;
    }
    newBaseEvent->env = getCurrentEnv();

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
    }
    newEvent->type = newEventType;
    return newEvent;
}

void storeNewVecValues(SEXP values)
{
    vecValues = values;
}

void clearPendingVars()
{
    /* reset the pending variables values */

    /* Reset ARITH_EVENT*/
    lastArithEvent = ABD_EVENT_NOT_FOUND;
    finalArithAns = R_NilValue;
    finalArithCall = R_NilValue;
    arithScriptLn = 0;

    /* Reset RET_EVENT */
    lastRetEvent = ABD_EVENT_NOT_FOUND;
    lastRetValue = R_NilValue;
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

    lastArithEvent->expr = processIfStmt(finalArithCall, 0);
    eventsRegTail->scriptLn = arithScriptLn;

    if (finalArithAns == rhs)
        return eventsRegTail;

    return ABD_EVENT_NOT_FOUND;
}
ABD_EVENT *checkPendingRet(SEXP rhs, ABD_OBJECT *obj)
{
    /* Pending ret event to assign object */
    if ((lastRetValue == R_NilValue) || (lastRetEvent == ABD_EVENT_NOT_FOUND))
        return ABD_EVENT_NOT_FOUND;

    puts("starting ret");
    lastRetEvent->data.ret_event->toObj = obj;
    puts("freeing value");
    free(lastRetEvent->data.ret_event->retValue);
    puts("setting value");
    lastRetEvent->data.ret_event->retValue = obj->modList;
    puts("returning ret");
    return lastRetEvent;
}
ABD_EVENT *checkPendingVec(SEXP rhs2, SEXP vecVal)
{
    if ((vecValues == R_NilValue) || (vecVal != vecValues))
        return ABD_EVENT_NOT_FOUND;

    /* not null and have the same rhs */
    createNewEvent(VEC_EVENT);

    ABD_VEC_EVENT *vecEvent = eventsRegTail->data.vec_event;
    vecEvent->nElements = Rf_nrows(vecVal);
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
        ABD_OBJECT *auxObj = findObj(cmnObjReg, CHAR(PRINTNAME(obj)), getCurrentEnv());

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

    return eventsRegTail;
}
ABD_EVENT *checkPendings(SEXP call, SEXP rhs, ABD_OBJECT *obj)
{
    /* Check if exist an arithmetic event pending */
    ABD_EVENT *retValue = ABD_EVENT_NOT_FOUND;

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
    }
    else
    {
        /*
            Do not have precedence from another event
            Need to process the rhs (from the call) to parse the origin
        */
        ABD_OBJECT *fromObj = ABD_OBJECT_NOT_FOUND;
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
            printf("rhs -> %s\n", CHAR(PRINTNAME(rhs2)));
            if ((fromObj = findObj(cmnObjReg, CHAR(PRINTNAME(rhs2)), rho)) == ABD_OBJECT_NOT_FOUND)
            {
                //not mapped obj
                withIndex = (withIndex == 0) ? 1 : withIndex;
                fromObj = createUnscopedObj(CHAR(PRINTNAME(rhs2)), -2, -2, R_NilValue, 0);
                currAssign->withIndex = withIndex;
            }
            else
                currAssign->withIndex = withIndex - 1;
        }
        else
        {
            fromObj = createUnscopedObj("NA", -1, -1, R_NilValue, 0);
            currAssign->withIndex = -1;
        }

        currAssign->fromType = ABD_O;
        currAssign->fromObj = fromObj;
    }
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
