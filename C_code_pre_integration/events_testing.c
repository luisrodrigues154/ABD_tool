#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct abd_obj{
    char * name;
}ABD_OBJECT;


//event types
typedef enum abd_event_types {
    MAIN_EVENT, 
    IF_EVENT, 
    FUNC_EVENT 
}ABD_EVENT_TYPE;

//event types structures
typedef struct abd_if_event{
    //TODO
    //analyze R behaviour when if'ing
}ABD_IF_EVENT;

typedef struct abd_func_event{
    ABD_OBJECT * objPtr;    
    int returnType;
}ABD_FUNC_EVENT;


// EVENT LINKAGE ptrs 


//EVENTS LIST
typedef struct abd_event{
    ABD_EVENT_TYPE type;
    union{
        ABD_IF_EVENT * if_event;
        ABD_FUNC_EVENT * func_event;
    }data;
    struct abd_event_ptrs * ptrs;
    struct abd_event * calls;
}ABD_EVENT;

typedef struct abd_event_ptrs{
    ABD_EVENT * prev;
    ABD_EVENT * next;
}ABD_EVENT_PTRS;

//PROTOS
ABD_EVENT_PTRS * allocEventPtrs();
ABD_FUNC_EVENT * allocNewFuncEvent();
ABD_FUNC_EVENT * createFuncEvent(ABD_EVENT * parentEvent, ABD_OBJECT * objectRegistry);
void createStructsForType(ABD_EVENT * parentEvent, ABD_EVENT * newEvent, ABD_EVENT_TYPE eventType, ABD_OBJECT * obj);
void setDataUnionToNULL(ABD_EVENT * newEvent);
void initializeNewEvent(ABD_EVENT * newEvent, ABD_EVENT_TYPE type);
ABD_EVENT * allocNewEvent();
ABD_EVENT * createNewEvent(ABD_EVENT * parentEvent, ABD_EVENT_TYPE eventType, ABD_OBJECT * obj);
ABD_EVENT * initializeEventsRegistry();
void initializeAndSetNewEventPtrs(ABD_EVENT * parentEvent, ABD_EVENT * newEvent);
void bindNewToParentCalls(ABD_EVENT * parentEvent, ABD_EVENT * newEvent);


//VARS
ABD_EVENT * currentEvent = NULL;
ABD_EVENT * mainEventTail = NULL;

int main(){
    ABD_EVENT * eventsRegistry;
    eventsRegistry = initializeEventsRegistry();
    currentEvent = eventsRegistry;
    mainEventTail = eventsRegistry;

    char name[] = "f1";
    
    ABD_OBJECT * funcObject = (ABD_OBJECT *) malloc(sizeof(ABD_OBJECT));
    funcObject->name = (char *) malloc(strlen(name) * sizeof(char));
    strncpy(funcObject->name, name, strlen(name)*sizeof(char));
    funcObject->name[strlen(funcObject->name)] = '\0';
    
    
    currentEvent = createNewEvent(currentEvent, FUNC_EVENT, funcObject);
    // after it returns
    // set current to corresponding
    
    
    return 0;
}

ABD_EVENT_PTRS * allocEventPtrs(){
    return (ABD_EVENT_PTRS *) malloc(sizeof(ABD_EVENT_PTRS));
}

ABD_FUNC_EVENT * allocNewFuncEvent(){
    return (ABD_FUNC_EVENT * ) malloc(sizeof(ABD_FUNC_EVENT));
}

ABD_FUNC_EVENT * createFuncEvent(ABD_EVENT * parentEvent, ABD_OBJECT * objectRegistry){
    ABD_FUNC_EVENT * newFuncEvent = allocNewFuncEvent();
    
    //bound the event to the currently created object in registry
    newFuncEvent->objPtr = objectRegistry;


    newFuncEvent->returnType = 1;
}
void createStructsForType(ABD_EVENT * parentEvent, ABD_EVENT * newEvent, ABD_EVENT_TYPE eventType, ABD_OBJECT * obj){
    
    switch (eventType){
        case FUNC_EVENT:
            newEvent->data.func_event = createFuncEvent(parentEvent, obj);
            break;

        case IF_EVENT:
            break;

        default:
            break;
    }
}
void setDataUnionToNULL(ABD_EVENT * newEvent){
    newEvent->data.func_event = NULL;
    newEvent->data.if_event = NULL;
}


void initializeNewEvent(ABD_EVENT * newEvent, ABD_EVENT_TYPE type){
    newEvent->type = type;
    newEvent->calls = NULL;
    setDataUnionToNULL(newEvent);
}

ABD_EVENT * allocNewEvent() {
    return (ABD_EVENT * ) malloc(sizeof(ABD_EVENT));
}

void initializeAndSetNewEventPtrs(ABD_EVENT * parentEvent, ABD_EVENT * newEvent){
    newEvent->ptrs = allocEventPtrs();
    newEvent->ptrs->prev = parentEvent;
    newEvent->ptrs->next = NULL;

}

void bindNewToParentCalls(ABD_EVENT * parentEvent, ABD_EVENT * newEvent){
    /*
        This will bind the newEvent to the parent (where the event was invoked) tail

        the structure held is

        - All events have calls that start on the pointer calls
        - after the first one, the calls held by that parent will be accessed by
        the structure ptrs.

        for instance:
        * f1() calls f2(), f3() and f4()
        * f2() calls f5()

        structure would be
        f1.calls = f2
        f2.ptrs.next = f3
        f3.ptrs.next = f4

        f2.calls = f5

        (and so on)
    */


    if(parentEvent->calls == NULL){
        //parent has no calls yet
        parentEvent->calls = newEvent;
        return;
    }

    ABD_EVENT * aux = parentEvent->calls;

    while(aux->ptrs->next != NULL)
        aux = aux->ptrs->next;

    //here aux will be the last element of the parent calls 
    //(where ptr->next is NULL thus the end of the list)

    aux->ptrs->next = newEvent;
}

ABD_EVENT * createNewEvent(ABD_EVENT * parentEvent, ABD_EVENT_TYPE eventType, ABD_OBJECT * obj){
    ABD_EVENT * newEvent = allocNewEvent();
    initializeNewEvent(newEvent, eventType);
    initializeAndSetNewEventPtrs(parentEvent, newEvent);
    createStructsForType(parentEvent, newEvent, eventType, obj);

    //bind the newEvent to the parent calls
    bindNewToParentCalls(parentEvent, newEvent);

    return newEvent;
}

ABD_EVENT  * initializeEventsRegistry(){
    ABD_EVENT * eventsReg =allocNewEvent();
    eventsReg->type = MAIN_EVENT;
    eventsReg->ptrs = allocEventPtrs();
    eventsReg->ptrs->prev = NULL;
    eventsReg->ptrs->next = NULL;
    eventsReg->calls = NULL;
}