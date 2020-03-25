#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/env_stack_defn.h>

void initEventsReg(){
    eventsReg = ABD_EVENT_NOT_FOUND;
    eventsRegTail = ABD_EVENT_NOT_FOUND;
    lastRetValue = ABD_EVENT_NOT_FOUND;
    currFunc = ABD_OBJECT_NOT_FOUND;
    eventsReg = createMainEvent();
    eventsRegTail = eventsReg;
}

ABD_EVENT * initBaseEvent(ABD_EVENT * newBaseEvent){
    newBaseEvent->data.if_event = ABD_NOT_FOUND;
    newBaseEvent->data.func_event = ABD_NOT_FOUND;
    newBaseEvent->data.ret_event = ABD_NOT_FOUND;
    newBaseEvent->nextEvent = ABD_EVENT_NOT_FOUND;
    return newBaseEvent;
}

ABD_EVENT * memAllocBaseEvent(){
    return (ABD_EVENT *) malloc(sizeof(ABD_EVENT));
}


ABD_FUNC_EVENT * memAllocFuncEvent(){
    return (ABD_FUNC_EVENT *) malloc(sizeof(ABD_FUNC_EVENT));
}

ABD_EVENT_ARG * memAllocEventArg(){
    return (ABD_EVENT_ARG *) malloc(sizeof(ABD_EVENT_ARG));
}   

ABD_RET_EVENT * memAllocRetEvent(){
    return (ABD_RET_EVENT *) malloc(sizeof(ABD_RET_EVENT));
}

ABD_EVENT_ARG * setArgValues(ABD_EVENT_ARG * currArg, ABD_OBJECT * objPtr, char * rcvdName, ABD_OBJECT_MOD * objValue){
    
    int rcvdNameSize = strlen(rcvdName);

    currArg->objPtr = objPtr;
    
    currArg->rcvdName = memAllocForString(rcvdNameSize);
    copyStr(currArg->rcvdName, rcvdName, rcvdNameSize);

    currArg->objValue = objValue;
    return currArg;
}
SEXP getValueFromPROMSXP(SEXP symbol){
    // recurse until we find the real promise, not a promise of a promise
    SEXP env;
    while(TYPEOF(symbol) == PROMSXP) {
        env = PRENV(symbol);
        symbol = PREXPR(symbol);

        // If the promise is threaded through multiple functions, we'll
        // get some symbols along the way. If the symbol is bound to a promise
        // keep going on up
        if (TYPEOF(symbol) == SYMSXP) {
            SEXP obj = Rf_findVar(symbol, env);

            if (TYPEOF(obj) != PROMSXP){
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
ABD_OBJECT_MOD * processByType(SEXP symbolValue, ABD_OBJECT_MOD * mod){
    rollback2:
    switch (TYPEOF(symbolValue))
    {
        case CLOSXP:
            break;
        case REALSXP:
            mod = setModValues(mod, symbolValue, createRealVector);
                
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
    mod->type = TYPEOF(symbolValue);
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
ABD_EVENT_ARG * processArgs(SEXP passedArgs, SEXP receivedArgs){
    ABD_EVENT_ARG * argsList = ABD_NOT_FOUND;
    ABD_EVENT_ARG * currentArg = ABD_NOT_FOUND;
    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT_MOD * objValue = ABD_NOT_FOUND;

    for(; passedArgs != R_NilValue; passedArgs = CDR(passedArgs), receivedArgs = CDR(receivedArgs)) {
        if(argsList == ABD_NOT_FOUND){
            // ok first arg to alloc
            // this avoid allocations without args existence
            currentArg = memAllocEventArg();
            currentArg->nextArg = ABD_NOT_FOUND;
            argsList = currentArg;
            
        }else{
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
        char * passedName = isNull(TAG(passedArgs)) ? "" : CHAR(PRINTNAME(TAG(passedArgs)));
        char * rcvdName = isNull(TAG(receivedArgs)) ? "" : CHAR(PRINTNAME(TAG(receivedArgs)));
        rollback:
        switch(TYPEOF(symbol)) {
            case REALSXP:
                //printf("\nCASE REAL Passed name: %s\nReceived name: %s\nValue: %f\n\n", passedName, rcvdName, REAL(symbol)[0]);
                objectFound = memAllocBaseObj();
                setObjBaseValues(objectFound, "NA", getCurrentEnv());
                objectFound->id = -1;
                objValue = memAllocMod();
                objValue->id = -1;
                objValue = processByType(symbol, objValue);
                break;

            case SYMSXP:
                {
                    //its a variable
                    passedName = CHAR(PRINTNAME(symbol));
                    //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
                    objectFound = findObj(cmnObjReg, passedName, getCurrentEnv());
                    if(objectFound == ABD_OBJECT_NOT_FOUND){
                        //object not in registry
                        SEXP rStrName = mkString(passedName);
                        //request R for the object (not hardcoded)
                        SEXP symbValue = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
                        objectFound = memAllocBaseObj();
                        setObjBaseValues(objectFound, passedName, getCurrentEnv());
                        objectFound->id = -2;
                        objValue = memAllocMod();
                        objValue = processByType(symbValue, objValue);
                        
                    }else{
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
void printArgsList(ABD_EVENT_ARG * argsList){
    if(argsList == ABD_NOT_FOUND)
        return;
    int count = 0;
    ABD_EVENT_ARG * currArg = argsList;
    do{
        count++;
        printf("Arg %d\n", count);
        printf("Passed name: %s\n", currArg->objPtr->name);
        printf("Received name: %s\n", currArg->rcvdName);
        printf("Value: %f\n", ((double *) currArg->objValue->value.vec_value->vector)[0]);
        puts("--------------------------");
        currArg = currArg->nextArg;
    }while(currArg != ABD_OBJECT_NOT_FOUND);
}
void setFuncEventValues(ABD_OBJECT * callingObj, SEXP newRho, SEXP passedArgs, SEXP receivedArgs){
    eventsRegTail->data.func_event->caller = currFunc;
    eventsRegTail->data.func_event->called = callingObj;
    eventsRegTail->data.func_event->args = processArgs(passedArgs, receivedArgs);
    envPush(newRho, callingObj);
}

void setRetEventValue(SEXP value){
    ABD_OBJECT_MOD * valueABD = memAllocMod();
    valueABD = processByType(value, valueABD);
    valueABD->id = -1;
    eventsRegTail->data.ret_event->toObj = ABD_OBJECT_NOT_FOUND;
    eventsRegTail->data.ret_event->from = getCurrFuncObj();
    eventsRegTail->data.ret_event->retValue = valueABD;

}

ABD_EVENT * creaStructsForType(ABD_EVENT * newBaseEvent, ABD_EVENT_TYPE type){
    //main event needs nothing but the type
    switch (type){
        case FUNC_EVENT:
            newBaseEvent->data.func_event =  memAllocFuncEvent();
            break;
        case IF_EVENT:
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


ABD_EVENT * createNewEvent(ABD_EVENT_TYPE newEventType){
    ABD_EVENT * newEvent = memAllocBaseEvent();
    newEvent = initBaseEvent(newEvent);
    if(newEventType != MAIN_EVENT){
        newEvent = creaStructsForType(newEvent, newEventType);  
        eventsRegTail->nextEvent = newEvent;
        eventsRegTail = eventsRegTail->nextEvent;
    }
    return newEvent;
}

ABD_EVENT * createMainEvent(){
    return createNewEvent(MAIN_EVENT);
}

void eventPrint(ABD_EVENT * event){

    switch(event->type){
        case MAIN_EVENT:
            printf("Running on main event...\n");
            break;

        case FUNC_EVENT:
            printf("Running on a func event...\nFunction name: %s\n", event->data.func_event->called->name);
            break;

        default: printf("None\n");break;

    }
}
