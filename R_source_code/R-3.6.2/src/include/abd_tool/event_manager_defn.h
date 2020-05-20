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
typedef struct abd_event
{
  int id;
  int scriptLn;
  ABD_EVENT_TYPE type;

  union {
    ABD_IF_EVENT *if_event;
    ABD_FUNC_EVENT *func_event;
    ABD_RET_EVENT *ret_event;
    ABD_ASSIGN_EVENT *asgn_event;
    ABD_ARITH_EVENT *arith_event;
    ABD_VEC_EVENT *vec_event;
  } data;
  ABD_OBJECT *atFunc;
  SEXP env;
  struct abd_event *nextEvent;
} ABD_EVENT;
static int waitingElseIF;

/* Stores Return value related */
static ABD_EVENT *lastRetEvent;
static SEXP lastRetValue;

/* Stores Arithmetic values related */
static ABD_ARITH_EVENT *lastArithEvent;
static SEXP finalArithAns;
static SEXP finalArithCall;
static SEXP *arithResults;
static int currArithIndex;
static int arithScriptLn;

/* Stores Vector creation values related*/
static SEXP vecValues;
static SEXP auxVecCall;
static int auxVecLine;

/* Stores events registry */
static ABD_EVENT *eventsReg;
static ABD_EVENT *eventsRegTail;
static int eventCounter;
#endif

//PROTOS
void initEventsReg();
ABD_EVENT *setDataUnionToNULL(ABD_EVENT *event);
ABD_EVENT *initBaseEvent(ABD_EVENT *newBaseEvent);

//event allocation
ABD_EVENT *memAllocBaseEvent();
ABD_FUNC_EVENT *memAllocFuncEvent();
ABD_EVENT_ARG *memAllocEventArg();
ABD_RET_EVENT *memAllocRetEvent();

ABD_EVENT *creaStructsForType(ABD_EVENT *newBaseEvent, ABD_EVENT_TYPE type);

ABD_EVENT *createNewEvent(ABD_EVENT_TYPE newEventType);
ABD_EVENT *createMainEvent();
void eventPrint(ABD_EVENT *event);
void setFuncEventValues(ABD_OBJECT *callingObj, SEXP newRho, SEXP passedArgs, SEXP receivedArgs);
ABD_EVENT_ARG *processArgs(SEXP passedArgs, SEXP receivedArgs);
ABD_OBJECT_MOD *processByType(SEXP symbolValue, ABD_OBJECT_MOD *mod, int);
ABD_EVENT_ARG *setArgValues(ABD_EVENT_ARG *arg, ABD_OBJECT *objPtr, const char *rcvdName, ABD_OBJECT_MOD *objValue);
void setArithEventValues(SEXP call, SEXP ans, SEXP arg1, SEXP arg2, int withPre);
void tmpStoreArith(SEXP call, SEXP ans);
ABD_EVENT *checkPendings(SEXP call, SEXP rhs, ABD_OBJECT *obj);
void setAsgnEventValues(ABD_OBJECT *toObj, SEXP value);
int getCurrScriptLn();
SEXP getResult(const char *expr);
SEXP getSavedArithAns();