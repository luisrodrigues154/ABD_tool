#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/events_defn.h>

//event types
#ifndef loaded_em
  #define loaded_em
  
  //EVENTS LIST
  typedef struct abd_event{
      ABD_EVENT_TYPE type;
      union{
          ABD_IF_EVENT * if_event;
          ABD_FUNC_EVENT * func_event;
      }data;
      struct abd_event * nextEvent;
  }ABD_EVENT;

  static SEXP lastRetValue;
  static ABD_EVENT * eventsReg;
  static ABD_EVENT * eventsRegTail;
#endif



//PROTOS

ABD_EVENT * setDataUnionToNULL(ABD_EVENT * event);
ABD_EVENT * initBaseEvent(ABD_EVENT * newBaseEvent);
ABD_EVENT * memAllocBaseEvent();
ABD_FUNC_EVENT * memAllocFuncEvent();
ABD_EVENT * creaStructsForType(ABD_EVENT * newBaseEvent, ABD_EVENT_TYPE type, void * obj);
ABD_EVENT * createNewEvent(ABD_EVENT_TYPE newEventType, void * obj);
ABD_EVENT * createMainEvent();
void eventPrint(ABD_EVENT * event);

