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
#include <abd_tool/env_stack.h>
#include <abd_tool/env_stack_defn.h>
#include <Print.h>

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

void printMods(ABD_OBJECT * obj){
    ABD_OBJECT_MOD * currMod = obj->modList;
    do{
        if(currMod->type == REALSXP){
            for(int i=0; i<currMod->value.vec_value->nCols; i++)
                printf("mod %i  Value %f\n", i, ((double *) currMod->value.vec_value->vector)[i]);
        }
        currMod = currMod->prevMod;
    }while(currMod != NULL);
}
void basicPrint(){
   if(cmnObjReg == ABD_OBJECT_NOT_FOUND)
        printf("REG NULL\n");
    else{
        ABD_OBJECT * currentObj = cmnObjReg;
        do{
            printf("name: %s\n", currentObj->name);
            printMods(currentObj);
            currentObj = currentObj->nextObj;
        }while(currentObj!=NULL);
    }
}

void START_WATCHER(){
    initObjsRegs();
    initEventsReg();
    initEnvStack();
    watcherState = ABD_ENABLE;
}

void STOP_WATCHER(){
    persistInformation(PERSIST_OBJECTS, cmnObjReg, cfObjReg);
    //wipeRegs(cmnObjReg);
    watcherState = ABD_DISABLE;
    printEventReg();
}
void ABD_HELP(){
    /*printf("\n\n\t  \"Automatic Bug Detection\" (ABD) tool usage\n");
    printf("\t###############################################\n");
    printf("\t-> Start the watcher: start_watcher()\n");
    printf("\t-> Stop the watcher: stop_watcher()\n");
    printf("\t-> Set output file: abd_setPath(\"your/path\")\n");
    printf("\t-> Display current output path: abd_path()\n");
    printf("\t###############################################\n\n\n");*/
    basicPrint();
}


void basicPrint2(){
   if(cfObjReg == ABD_OBJECT_NOT_FOUND)
        printf("REG NULL\n");
    else{
        ABD_OBJECT * currentObj =cfObjReg;
        do{
            printf("name: %s\n", currentObj->name);
            currentObj = currentObj->nextObj;
        }while(currentObj!=NULL);
    }
}

void regVarChange(SEXP lhs, SEXP rhs, SEXP rho){
    if(watcherState){
        if(isEnvironment(rho)){
            newObjUsage(lhs,rhs,rho);
        }
    }
}


/*
    The function below will verify if R is trying to execute a closure defined by the user.
    This verification is done because if the user did not declared an object with that symbol
    name, then, the function being called should not be tracked.
*/

ABD_SEARCH checkToReg(SEXP rho){
    return cmpToCurrEnv(rho);
}

ABD_EVENT_ARG * createArgEntry(ABD_OBJECT * objPtr, char * rcvdName, ABD_OBJECT_MOD * objValue){
    ABD_EVENT_ARG * newArg = memAllocEventArg();
    int rcvdNameSize = strlen(rcvdName);

    newArg->objPtr = objPtr;
    
    newArg->rcvdName = memAllocForString(rcvdNameSize);
    copyStr(newArg->rcvdName, rcvdName, rcvdNameSize);

    newArg->objValue = objValue;
    
    return newArg;
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

    for(int i = 0; passedArgs != R_NilValue; i++, passedArgs = CDR(passedArgs), receivedArgs = CDR(receivedArgs)) {
        if(argsList == ABD_NOT_FOUND){
            // ok first arg to alloc
            // this avoid allocations without args existence
            currentArg = memAllocEventArg();
            argsList = currentArg;
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
        
        switch(TYPEOF(symbol)) {
            case REALSXP:
                printf("\nPassed name: %s\nReceived name: %s\nValue: %f\n\n", passedName, rcvdName, REAL(symbol)[0]);
                break;

            case SYMSXP:
                {
                    passedName = CHAR(PRINTNAME(symbol));
                    printf("\nPassed name: %s\nReceived name: %s\n", passedName, rcvdName);
                    ABD_OBJECT * objectFound = findObj(cmnObjReg, passedName, getCurrentEnv());
                    if(objectFound == ABD_OBJECT_NOT_FOUND){
                        //object not in registry
                        SEXP rStrName = mkString(passedName);
                        SEXP value = findVar(installChar(STRING_ELT(rStrName, 0)), getCurrentEnv());
                        objectFound = memAllocBaseObj();
                        setObjBaseValues(objectFound, passedName, getCurrentEnv());
                        ABD_OBJECT_MOD * sexpToABDconvert = memAllocMod();

                        //TODO: 
                        // create a unlinked object mod to translate the SEXP passed
                        // as argument to ABD_tool 'language'
                        // it'll do as the modifcation registry (identify the type)
                        // and call specific methods to store the information with
                        // functions passed as argument to then be executed.
                        
                    }

                }
                break;
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
                Rprintf("[%d] '%s' R type\n", i+1, rcvdName);
        }
    
    }
    
    return argsList;
}


ABD_SEARCH regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs, SEXP receivedArgs){
    if(watcherState){
        ABD_OBJECT * objFound = findObj(cfObjReg, CHAR(PRINTNAME(lhs)), rho);
        if(objFound == ABD_OBJECT_NOT_FOUND)
            return ABD_NOT_EXIST;

        ABD_EVENT_ARG * argsList = processArgs(passedArgs, receivedArgs);
        ABD_EVENT * newEvent = createNewEvent(FUNC_EVENT);
        bindObjToFuncEvent(newEvent, objFound);        
        envPush(newRho);
        return ABD_EXIST;
    }
    return ABD_NOT_EXIST;
}


void regFunReturn(SEXP lhs, SEXP rho, SEXP val){
    createNewEvent(RET_EVENT);
    envPop();
}



void printEventReg(){
    printf("Will print events reg...\n");
    ABD_EVENT * currentEvent = eventsReg;
    do{
        switch (currentEvent->type)
        {
        case MAIN_EVENT:
            puts("\tmain event...");
            break;
        case FUNC_EVENT:
            puts("\tfunc event...");
            break;
        case RET_EVENT:
            puts("\tret event...");
            break;
        default:
            break;
        }
        currentEvent = currentEvent->nextEvent;
    }while(currentEvent!=ABD_EVENT_NOT_FOUND);
    printf("Events reg printed...\n");
}
