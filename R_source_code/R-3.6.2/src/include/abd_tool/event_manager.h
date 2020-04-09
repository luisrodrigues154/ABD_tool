#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/env_stack_defn.h>
 #include <Rembedded.h>
 #include <R_ext/Parse.h>
 
void initEventsReg(){
    eventsReg = ABD_EVENT_NOT_FOUND;
    eventsRegTail = ABD_EVENT_NOT_FOUND;
    lastRetValue = ABD_EVENT_NOT_FOUND;
    currFunc = ABD_OBJECT_NOT_FOUND;
    eventsReg = createMainEvent();
    eventsRegTail = eventsReg;
    waitingElseIF = 0;
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
IF_ABD_OBJ * memAllocIfAbdObj(){
    return (IF_ABD_OBJ *) malloc(sizeof(IF_ABD_OBJ));
}
ABD_IF_EVENT * memAllocIfEvent(){
    return (ABD_IF_EVENT *) malloc(sizeof(ABD_IF_EVENT));
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
IF_EXPRESSION * memAllocIfExp(){
    return (IF_EXPRESSION *) malloc(sizeof(IF_EXPRESSION));
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





// void printArgsList(ABD_EVENT_ARG * argsList){
//     if(argsList == ABD_NOT_FOUND)
//         return;
//     int count = 0;
//     ABD_EVENT_ARG * currArg = argsList;
//     do{
//         count++;
//         printf("Arg %d\n", count);
//         printf("Passed name: %s\n", currArg->objPtr->name);
//         printf("Received name: %s\n", currArg->rcvdName);
//         printf("Value: %f\n", ((double *) currArg->objValue->value.vec_value->vector)[0]);
//         puts("--------------------------");
//         currArg = currArg->nextArg;
//     }while(currArg != ABD_OBJECT_NOT_FOUND);
// }

int inCollection(char * check){
	char * collection [] = {
            "+", "-", "*", "/",
			"==", "!=", "<", ">", "<=", ">=",
			"&", "|", "&&", "||", "!", "[" };
    int nCollection = 16;

    for(int i=0; i<nCollection; i++){
        if(strcmp(check, collection[i]) == 0)
            return 1;
    }
    return 0;
}

void * rebuildVector(ABD_OBJECT * obj){

}

IF_ABD_OBJ * getAbdIfObjIndexed(SEXP symbol, SEXP index){

}
IF_ABD_OBJ * getAbdIfObj(SEXP symbol){
    IF_ABD_OBJ * newObj = memAllocIfAbdObj();
    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT_MOD * objValue = ABD_OBJECT_NOT_FOUND;

    switch(TYPEOF(symbol)) {
        case REALSXP:
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
                char * objName = CHAR(PRINTNAME(symbol));
                //printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
                objectFound = findObj(cmnObjReg, objName, getCurrentEnv());
                if(objectFound == ABD_OBJECT_NOT_FOUND){
                    //object not in registry
                    SEXP rStrName = mkString(objName);
                    //request R for the object (not hardcoded)
                    SEXP symbValue = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
                    objectFound = memAllocBaseObj();
                    setObjBaseValues(objectFound, objName, getCurrentEnv());
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
        default: 
            break;
            //Rprintf("[%d] '%s' R type\n", i+1, rcvdName);
    }
    newObj->objPtr = objectFound;
    newObj->objValue = objValue;

    return newObj;
}
//
// (1+a)
SEXP calculateResult(char * expr){
    SEXP e, tmp, retValue;
    ParseStatus status;
    int i;
    
    PROTECT(tmp = mkString(expr));
    PROTECT(e = R_ParseVector(tmp, -1, &status, R_NilValue));
    PROTECT(retValue = R_tryEval(VECTOR_ELT(e,0), R_GlobalEnv, NULL));

    UNPROTECT(3);

    return retValue;
}

IF_EXPRESSION * processIfStmt(SEXP st){

	IF_EXPRESSION * newExpr = memAllocIfExp();
    char asChar[100];
	if(TYPEOF(st) == LANGSXP){
		newExpr->isConfined = 0;
        
		procStateLabel:;
		SEXP carSt = CAR(st);
		char * operator = CHAR(PRINTNAME(carSt));
		int opSize = strlen(operator);
		newExpr->operator = memAllocForString(opSize);
		copyStr(newExpr->operator, operator, opSize);
        
		if(inCollection(operator)){
            if(strcmp(operator, "[") == 0){
                // a[1]
                // lElem == symb name
                // rElem == 1 (index)
            }else{

            }

            int objIndex = 0;
			SEXP lElem = CAR(CDR(st));
			if(TYPEOF(lElem) == LANGSXP){
				
                char * auxOp = CHAR(PRINTNAME(CAR(lElem)));
                if(strcmp(operator, "[") == 0){
                    objIndex = 1;
                    goto jump;
                }
                newExpr->left_type = IF_EXPR;
				newExpr->left_data = processIfStmt(lElem);
			}else{
                jump:;
				// if we come to (2>3) left side wont be LANGSXP
				// need to process by type
				newExpr->left_type = IF_ABD;

				// alloc new ABD_OBJ and set values (dont forget mod)
                newExpr->left_data = getAbdIfObj(lElem);				
			}
			
			SEXP rElem = CAR(CDR(CDR(st)));
            printf("TYPEOF rElem %d\n", TYPEOF(rElem));
			if(TYPEOF(rElem) == LANGSXP){
				newExpr->right_type = IF_EXPR;
                puts("will enter from rElem");
				newExpr->right_data = processIfStmt(rElem);
			}else{
				// if we come to (2>3) left side wont be LANGSXP
				// need to process by type
				newExpr->right_type = IF_ABD;
				
				// alloc new ABD_OBJ and set values (dont forget mod)
				newExpr->right_data = getAbdIfObj(rElem);
			}
            
            if(newExpr->left_type == IF_EXPR){
                sprintf(asChar, "%s%.2f", asChar, ((IF_EXPRESSION *) newExpr->left_data)->result);
            }else{
                ABD_OBJECT * obj = ((IF_ABD_OBJ *) newExpr->left_data)->objPtr;
                if(obj->id == -1){
                    // hardcoded, get value
                    ABD_OBJECT_MOD * objValue = ((IF_ABD_OBJ *) newExpr->left_data)->objValue;
                    sprintf(asChar, "%s%.4f", asChar, ((double *) objValue->value.vec_value->vector)[0]);
                }else
                    sprintf(asChar, "%s%s", asChar, obj->name);
            }
            
            //get the operator  
            sprintf(asChar, "%s%s", asChar, operator);
            
            if(newExpr->right_type == IF_EXPR){
                sprintf(asChar, "%s%.2f", asChar, ((IF_EXPRESSION *) newExpr->right_data)->result);
            }else{
                ABD_OBJECT * obj = ((IF_ABD_OBJ *) newExpr->right_data)->objPtr;
                if(obj->id == -1){
                    // hardcoded, get value
                    ABD_OBJECT_MOD * objValue = ((IF_ABD_OBJ *) newExpr->right_data)->objValue;
                    sprintf(asChar, "%s%.4f", asChar, ((double *) objValue->value.vec_value->vector)[0]);
                }else
                    sprintf(asChar, "%s%s", asChar, obj->name);
            }
            if(newExpr->isConfined)
                sprintf(asChar, "%s)", asChar);
		}else if(strcmp(operator, "(") == 0){
			//confine
			newExpr->isConfined = 1;
			st = CAR(CDR(st));
            sprintf(asChar, "(");
			goto procStateLabel;
		}
		
	}else{
		// not language expression
		// can be if(a) or if(1), etc...
		puts("just a single number in here...");
	}
    SEXP result = calculateResult(asChar);
    switch (TYPEOF(result))
    {
        case LGLSXP: newExpr->result = LOGICAL(result)[0]; break;
        case REALSXP: newExpr->result = REAL(result)[0];   break;
        default: break;
    }
        
    
    printf("\nStatement %s\nResult %s\n", asChar, (newExpr->result) ? "TRUE" : "FALSE");
	return newExpr;
}

void printExpression(IF_EXPRESSION * expr){	
	
	if(expr->isConfined)
		printf("(");

	if(expr->left_type == IF_EXPR)
		printExpression(expr->left_data);
	else{
        ABD_OBJECT_MOD * objValue = ((IF_ABD_OBJ *) expr->left_data)->objValue;
        double value = ((double *) objValue->value.vec_value->vector)[0];
        printf("%.2f", value);
    }
		

	printf(" %s ", expr->operator);
	
	if(expr->right_type == IF_EXPR)
		printExpression(expr->right_data);
	else
    {
        ABD_OBJECT_MOD * objValue = ((IF_ABD_OBJ *) expr->right_data)->objValue;
        double value = ((double *) objValue->value.vec_value->vector)[0];
        printf("%.2f", value);
    }
	

	if(expr->isConfined)
		printf(")");
	
}
int isWaitingElseIf(){
    return waitingElseIF;
}
void setIsWaiting(int isWaiting){
    waitingElseIF = isWaiting;
}
ABD_IF_EVENT * findLastElseIf(){
    ABD_IF_EVENT * currEvent = eventsRegTail->data.if_event;
    while(currEvent->else_if != ABD_EVENT_NOT_FOUND)
        currEvent = currEvent->else_if;
    return currEvent;
}
void setIfEventValues(SEXP statement, Rboolean result){
    ABD_IF_EVENT * currEvent = findLastElseIf();
    
    if(isWaitingElseIf()){
        if(statement == R_NilValue){
            //is an else
            puts("regging else");
            currEvent->else_if = memAllocIfExp();
            currEvent->else_if->else_if = ABD_EVENT_NOT_FOUND;
            currEvent->else_if->expr = ABD_NOT_FOUND;
            currEvent->else_if->globalResult = 1;
            currEvent->else_if->isElse = 1;
            waitingElseIF = 0;
        }else{
            //is an else if
            puts("reging an else if");
            currEvent->else_if = processIfStmt(statement);
            currEvent->else_if->isElse = 0;
            currEvent->else_if->globalResult = result;
        }
    }else{
        puts("reging an if");
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
