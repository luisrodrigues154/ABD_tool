#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>

void initEventsReg(){
    eventsReg = ABD_EVENT_NOT_FOUND;
    eventsRegTail = ABD_EVENT_NOT_FOUND;
    lastRetValue = ABD_EVENT_NOT_FOUND;
    eventsReg = createMainEvent();
    eventsRegTail = eventsReg;
}

ABD_EVENT * initBaseEvent(ABD_EVENT * newBaseEvent){
    newBaseEvent->data.if_event = ABD_NOT_FOUND;
    newBaseEvent->data.func_event = ABD_NOT_FOUND;
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

ABD_EVENT * bindObjToFuncEvent(ABD_EVENT * funcEvent, ABD_OBJECT * obj){
    funcEvent->data.func_event->objPtr = obj;
    
}


void addArgToFuncEvent(SEXP symbol, SEXP value){
    //need to receive a symbol and a value (not just symbol)
    //because the arguments may be named

}


ABD_EVENT * creaStructsForType(ABD_EVENT * newBaseEvent, ABD_EVENT_TYPE type){
    //main event needs nothing but the type
    switch (type){
        case FUNC_EVENT:{
            ABD_FUNC_EVENT * newFuncEvent = memAllocFuncEvent();
            newFuncEvent->args = memAllocEventArg();
            newBaseEvent->data.func_event = newFuncEvent;
            break;
        }
        case IF_EVENT:
            break;
        case RET_EVENT:
            
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
    newEvent = creaStructsForType(newEvent, newEventType);  
    if(newEventType != MAIN_EVENT){
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
            printf("Running on a func event...\nFunction name: %s\n", event->data.func_event->objPtr->name);
            break;

        default: printf("None\n");break;

    }
}
