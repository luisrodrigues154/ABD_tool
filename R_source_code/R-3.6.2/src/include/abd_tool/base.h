#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <abd_tool/base_defn.h>
#include <abd_tool/obj_manager.h>
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
    wipeRegistry();
    watcherState = ABD_ENABLE;
}

void STOP_WATCHER(){
    persistInformation(PERSIST_OBJECTS, objectsRegistry);
    wipeRegistry();
    watcherState = ABD_DISABLE;
}
void ABD_HELP(){
    
}
void regVarChange(int callingSite, SEXP lhs, SEXP rhs, SEXP rho){
    
    if(watcherState){
        if(isEnvironment(rho)){
            objectUsage(lhs, rhs, rho);
                       
            //basicPrint();  
        }
    }
    
}


