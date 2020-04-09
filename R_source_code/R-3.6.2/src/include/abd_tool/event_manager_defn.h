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
          ABD_RET_EVENT * ret_event;
      }data;
      struct abd_event * nextEvent;
  }ABD_EVENT;
  static int waitingElseIF;
  static ABD_OBJECT * currFunc;
  static ABD_RET_EVENT * lastRetEvent;
  static SEXP lastRetValue;
  static ABD_EVENT * eventsReg;
  static ABD_EVENT * eventsRegTail;
#endif



//PROTOS
void initEventsReg();
ABD_EVENT * setDataUnionToNULL(ABD_EVENT * event);
ABD_EVENT * initBaseEvent(ABD_EVENT * newBaseEvent);

//event allocation
ABD_EVENT * memAllocBaseEvent();
ABD_FUNC_EVENT * memAllocFuncEvent();
ABD_EVENT_ARG * memAllocEventArg();
ABD_RET_EVENT * memAllocRetEvent();

ABD_EVENT * creaStructsForType(ABD_EVENT * newBaseEvent, ABD_EVENT_TYPE type);

ABD_EVENT * createNewEvent(ABD_EVENT_TYPE newEventType);
ABD_EVENT * createMainEvent();
void eventPrint(ABD_EVENT * event);
void setFuncEventValues(ABD_OBJECT * callingObj, SEXP newRho, SEXP passedArgs, SEXP receivedArgs);
ABD_EVENT_ARG * processArgs(SEXP passedArgs, SEXP receivedArgs);
ABD_OBJECT_MOD * processByType(SEXP symbolValue, ABD_OBJECT_MOD * mod);
ABD_EVENT_ARG * setArgValues(ABD_EVENT_ARG * arg, ABD_OBJECT * objPtr, char * rcvdName, ABD_OBJECT_MOD * objValue);
