#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/env_stack_defn.h>
#include <Rembedded.h>
#include <R_ext/Parse.h>

void initEventsReg()
{
    eventsReg = ABD_EVENT_NOT_FOUND;
    eventsRegTail = ABD_EVENT_NOT_FOUND;
    lastRetValue = ABD_EVENT_NOT_FOUND;
    eventsReg = createMainEvent();
    eventsRegTail = eventsReg;
    waitingElseIF = 0;
}

ABD_EVENT *initBaseEvent(ABD_EVENT *newBaseEvent)
{
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

ABD_EVENT_ARG *memAllocEventArg()
{
    return (ABD_EVENT_ARG *)malloc(sizeof(ABD_EVENT_ARG));
}

ABD_RET_EVENT *memAllocRetEvent()
{
    return (ABD_RET_EVENT *)malloc(sizeof(ABD_RET_EVENT));
}
IF_EXPRESSION *memAllocIfExp()
{
    return (IF_EXPRESSION *)malloc(sizeof(IF_EXPRESSION));
}

ABD_EVENT_ARG *setArgValues(ABD_EVENT_ARG *currArg, ABD_OBJECT *objPtr, char *rcvdName, ABD_OBJECT_MOD *objValue)
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
    case INTSXP:
        puts("integer vectors");
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
        char *passedName = isNull(TAG(passedArgs)) ? "" : CHAR(PRINTNAME(TAG(passedArgs)));
        char *rcvdName = isNull(TAG(receivedArgs)) ? "" : CHAR(PRINTNAME(TAG(receivedArgs)));
    rollback:
        switch (TYPEOF(symbol))
        {
        case REALSXP:
            //printf("\nCASE REAL Passed name: %s\nReceived name: %s\nValue: %f\n\n", passedName, rcvdName, REAL(symbol)[0]);
            objectFound = memAllocBaseObj();
            setObjBaseValues(objectFound, "NA", getCurrentEnv());
            objectFound->id = -1;
            objectFound->usages = 1;
            objValue = addEmptyModToObj(objectFound, getObjStructType(symbol));
            objValue->id = -1;
            objValue = processByType(symbol, objValue, 0);
            break;

        case SYMSXP:
        {
            //its a variable
            passedName = CHAR(PRINTNAME(symbol));
            //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
            objectFound = findObj(cmnObjReg, passedName, getCurrentEnv());
            if (objectFound == ABD_OBJECT_NOT_FOUND)
            {
                //object not in registry
                SEXP rStrName = mkString(passedName);
                //request R for the object (not hardcoded)
                SEXP symbValue = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
                objectFound = memAllocBaseObj();
                setObjBaseValues(objectFound, passedName, getCurrentEnv());
                objectFound->id = -2;
                objectFound->usages = 1;
                objValue = addEmptyModToObj(objectFound, getObjStructType(symbol));
                objValue = processByType(symbValue, objValue, 0);
            }
            else
            {
                objValue = objectFound->modList;
            }

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

        currentArg = setArgValues(currentArg, objectFound, rcvdName, objValue);
        objectFound = ABD_OBJECT_NOT_FOUND;
        objValue = ABD_OBJECT_NOT_FOUND;
    }
    return argsList;
}

int inCollection(char *check)
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
        retMod->value.vec_value->idxs = (int *)malloc(sizeof(int));
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
}

IF_ABD_OBJ *getAbdIfObj(SEXP symbol, int withIndex)
{
    IF_ABD_OBJ *newObj = memAllocIfAbdObj();
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT_MOD *objValue = ABD_OBJECT_NOT_FOUND;

    switch (TYPEOF(symbol))
    {
    case REALSXP:
        objectFound = memAllocBaseObj();
        setObjBaseValues(objectFound, "NA", getCurrentEnv());
        objectFound->id = -1;
        objectFound->usages = 1;
        objValue = addEmptyModToObj(objectFound, getObjStructType(symbol));
        objValue->id = -1;
        objValue = processByType(symbol, objValue, 0);
        break;
    case SYMSXP:
    {
        //its a variable
        char *objName = CHAR(PRINTNAME(symbol));
        //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
        objectFound = findObj(cmnObjReg, objName, getCurrentEnv());
        if (objectFound == ABD_OBJECT_NOT_FOUND)
        {
            //object not in registry
            SEXP rStrName = mkString(objName);
            //request R for the object (not hardcoded)
            SEXP symbValue = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
            objectFound = memAllocBaseObj();
            setObjBaseValues(objectFound, objName, getCurrentEnv());
            objectFound->id = -2;
            objectFound->usages = 1;
            objValue = addEmptyModToObj(objectFound, getObjStructType(symbol));
            objValue = processByType(symbValue, objValue, 0);
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

SEXP getResult(char *expr)
{
    SEXP e, tmp, retValue;
    ParseStatus status;
    int i;

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

IF_EXPRESSION *processIfStmt(SEXP st)
{

    IF_EXPRESSION *newExpr = memAllocIfExp();
    char *stmtStr = memAllocForString(100);
    memset(stmtStr, 0, 100);
    char *operator;
    if (TYPEOF(st) == LANGSXP)
    {
        newExpr->isConfined = 0;
    procStateLabel:;
        SEXP carSt = CAR(st);
        operator= CHAR(PRINTNAME(carSt));
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
                newExpr->left_data = processIfStmt(lElem);
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
                newExpr->right_data = processIfStmt(rElem);
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
    SEXP result = getResult(stmtStr);
    SEXPTYPE resType = TYPEOF(result);

    if (resType == LGLSXP)
        newExpr->result = LOGICAL(result)[0];
    else if (resType == REALSXP)
        newExpr->result = REAL(result)[0];

    //printf("\nStatement %s\nResult %s\n", stmtStr, (newExpr->result) ? "TRUE" : "FALSE");
    free(stmtStr);
    return newExpr;
}

void printExpression(IF_EXPRESSION *expr)
{

    if (expr->isConfined)
        printf("(");

    if (expr->left_type == IF_EXPR)
        printExpression(expr->left_data);
    else
    {
        ABD_OBJECT_MOD *objValue = ((IF_ABD_OBJ *)expr->left_data)->objValue;
        double value = ((double *)objValue->value.vec_value->vector)[0];
        printf("%.2f", value);
    }

    printf(" %s ", expr->operator);

    if (expr->right_type == IF_EXPR)
        printExpression(expr->right_data);
    else
    {
        ABD_OBJECT_MOD *objValue = ((IF_ABD_OBJ *)expr->right_data)->objValue;
        double value = ((double *)objValue->value.vec_value->vector)[0];
        printf("%.2f", value);
    }

    if (expr->isConfined)
        printf(")");
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
            currEvent->else_if->expr = processIfStmt(statement);
            currEvent->else_if->isElse = 0;
            currEvent->else_if->globalResult = result;
        }
    }
    else
    {
        currEvent->else_if = ABD_EVENT_NOT_FOUND;
        currEvent->isElse = 0;
        currEvent->globalResult = result;
        currEvent->expr = processIfStmt(statement);
    }

    puts("WILL PRINT THE IF");
    printf("%sif(", (waitingElseIF) ? "else " : "");
    printExpression(eventsRegTail->data.if_event->expr);
    printf(")\n");
}

void setFuncEventValues(ABD_OBJECT *callingObj, SEXP newRho, SEXP passedArgs, SEXP receivedArgs)
{
    eventsRegTail->data.func_event->toEnv = newRho;
    eventsRegTail->data.func_event->fromEnv = getCurrentEnv();
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
    eventsRegTail->data.ret_event->retEnv = getCurrentEnv();
    eventsRegTail->data.ret_event->retValue = valueABD;
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
        lastRetEvent = newBaseEvent->data.ret_event;
        break;
    default:
        break;
    }
    newBaseEvent->type = type;
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
    return newEvent;
}

ABD_EVENT *createMainEvent()
{
    return createNewEvent(MAIN_EVENT);
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
