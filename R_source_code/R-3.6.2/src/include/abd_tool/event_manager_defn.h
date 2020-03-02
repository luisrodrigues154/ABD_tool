#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/obj_manager_defn.h>

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

