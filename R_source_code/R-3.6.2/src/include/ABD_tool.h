
#include <stdio.h>
#include <Internal.h>
#include <ABD_tool_DEF.h>
static int watcherActivated = 0;

void START_WATCHER(){
    watcherActivated = 1;
}

void STOP_WATCHER(){
    watcherActivated = 0;
}
void ABD_HELP(){
    
}
void regVarChange(SEXP variable, SEXP newValue){
    if(watcherActivated){
        Rprintf("Some variable modified...\n");
        Rprintf("Variable: %s\n", CHAR(variable));
        Rprintf("New Value: %.0f\n", REAL(newValue)[0]);        
    }
    
}