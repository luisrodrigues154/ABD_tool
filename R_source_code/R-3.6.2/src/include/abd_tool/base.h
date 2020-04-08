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
    checkSettings();
    initObjsRegs();
    initEventsReg();
    initEnvStack();
    watcherState = ABD_ENABLE;
}

void STOP_WATCHER(){
    if(isRunning()){
        checkSettings();
        watcherState = ABD_DISABLE;
        persistInformation();
    }
}
void ABD_HELP(){
    // printf("\n\n\t  \"Automatic\" Bug Detection (ABD) tool usage\n");
    // printf("\t##################################################\n");
    // printf("\t-> Start the watcher: start_watcher()\n");
    // printf("\t-> Stop the watcher: stop_watcher()\n");
    // printf("\t-> Set output file path: abd_setPath(\"your/path\")\n");
    // printf("\t-> Display current output file path: abd_path()\n");
    // printf("\t##################################################\n\n\n");
    //checkSettings();
    
}


void saveIdxChanges(int nIdxs, int * idxChanges){
    commitIdxChanges(nIdxs, idxChanges);
}

void regVarChange(SEXP lhs, SEXP rhs, SEXP rho){
    if(isRunning()){
        if(isEnvironment(rho)){
            
            if(strncmp(CHAR(PRINTNAME(lhs)), "*tmp*", 5) == 0)
               //do not register
               waitingIdxChange = 1;
            printf("Env %s\n", environmentExtraction(rho));
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
    if(!isRunning())
        return ABD_NOT_EXIST;
    return cmpToCurrEnv(rho);
}


ABD_SEARCH regFunCall(SEXP lhs, SEXP rho, SEXP newRho, SEXP passedArgs, SEXP receivedArgs){
    if(isRunning()){
        ABD_OBJECT * objFound = findFuncObj(CHAR(PRINTNAME(lhs)), rho);
        if(objFound == ABD_OBJECT_NOT_FOUND)
            return ABD_NOT_EXIST;

        createNewEvent(FUNC_EVENT);
        setFuncEventValues(objFound, newRho, passedArgs, receivedArgs);
        return ABD_EXIST;
    }
    return ABD_NOT_EXIST;
}

void regIf(SEXP Stmt, Rboolean result){
    if(isRunning()){
        createNewEvent(IF_EVENT);

        setIfEventValues(Stmt, result);
    }
}

void regFunReturn(SEXP lhs, SEXP rho, SEXP val){
    createNewEvent(RET_EVENT);
    setRetEventValue(val);
    lastRetValue = val;
    envPop();
}

ABD_STATE isRunning(){
    return watcherState;
}



void storeCompareResult(SEXP cmpr){
    cmp = cmpr;
}
