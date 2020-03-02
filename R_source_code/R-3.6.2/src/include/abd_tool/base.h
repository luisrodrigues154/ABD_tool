#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/obj_manager.h>
#include <Print.h>

typedef enum abd_state{
    ABD_ENABLE = 1,
    ABD_DISABLE = 0
}ABD_STATE;

static ABD_STATE watcherState = ABD_DISABLE;
static ABD_OBJECT * cmnObjReg, * cfObjReg;
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
    initRegs(cmnObjReg);
    initRegs(cfObjReg);
    watcherState = ABD_ENABLE;
}

void STOP_WATCHER(){
    persistInformation(PERSIST_OBJECTS, cmnObjReg, cfObjReg);
    wipeRegs(cmnObjReg);
    watcherState = ABD_DISABLE;
}
void ABD_HELP(){
    
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
void regVarChange(int callingSite, SEXP lhs, SEXP rhs, SEXP rho){
    if(watcherState){
        if(isEnvironment(rho)){
            switch(TYPEOF(rhs)){
                case CLOSXP:
                    //closures (function objects)
                    cfObjReg = newObjUsage(cfObjReg, lhs, rhs, rho);
                    basicPrint2();
                    break;
                case REALSXP:
                    //real numbers (0, 0.1, etc.)
                    cmnObjReg =  newObjUsage(cmnObjReg, lhs, rhs, rho);
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
void verifySymbol(SEXP lhs, SEXP rho){
    if(watcherState){
        ABD_OBJECT * objFound = findObj(cfObjReg, CHAR(PRINTNAME(lhs)), environmentExtraction(rho));
        printf("Found it? %s\n", (objFound == ABD_OBJECT_NOT_FOUND) ? "no" : "yes");
    }
}


