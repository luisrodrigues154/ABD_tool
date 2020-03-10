#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
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


void START_WATCHER(){
    initObjsRegs();
    initEventsReg();
    initEnvStack();
    watcherState = ABD_ENABLE;
}

void STOP_WATCHER(){
    persistInformation(PERSIST_OBJECTS, cmnObjReg, cfObjReg);
    wipeRegs(cmnObjReg);
    watcherState = ABD_DISABLE;
}
void ABD_HELP(){
    printf("\n\n\t  \"Automatic Bug Detection\" (ABD) tool usage\n");
    printf("\t###############################################\n");
    printf("\t-> Start the watcher: start_watcher()\n");
    printf("\t-> Stop the watcher: stop_watcher()\n");
    printf("\t-> Set output file: abd_setPath(\"your/path\")\n");
    printf("\t-> Display current output path: abd_path()\n");
    printf("\t###############################################\n\n\n");
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
void basicPrint(){
   if(cmnObjReg == ABD_OBJECT_NOT_FOUND)
        printf("REG NULL\n");
    else{
        ABD_OBJECT * currentObj = cmnObjReg;
        do{
            printf("name: %s\n", currentObj->name);
            currentObj = currentObj->nextObj;
        }while(currentObj!=NULL);
    }
}
void regVarChange(SEXP lhs, SEXP rhs, SEXP rho){
    if(watcherState){
        if(isEnvironment(rho)){
            switch(TYPEOF(rhs)){
                case CLOSXP:
                    //closures (function objects)
                    newCfObjUsage(lhs, rhs, rho);
                    basicPrint2();
                    break;
                case REALSXP:
                    newCmnObjUsage(lhs, rhs, rho);
                    basicPrint();
                    break;
                default:
                    break;
            }

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



ABD_SEARCH regFunCall(SEXP lhs, SEXP rho, SEXP newRho){
    if(watcherState){
        ABD_OBJECT * objFound = findObj(cfObjReg, CHAR(PRINTNAME(lhs)), environmentExtraction(rho));
        if(objFound == ABD_OBJECT_NOT_FOUND)
            return ABD_NOT_EXIST;

        createNewEvent(FUNC_EVENT, objFound);
        envPush(newRho);
        return ABD_EXIST;
    }
    return ABD_NOT_EXIST;
}


void regFunReturn(SEXP lhs, SEXP rho, SEXP val){
    createNewEvent(RET_EVENT, NULL);
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
