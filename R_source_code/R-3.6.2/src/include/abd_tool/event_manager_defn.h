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
  short branchDepth;
  union {
    ABD_IF_EVENT *if_event;
    ABD_FUNC_EVENT *func_event;
    ABD_RET_EVENT *ret_event;
    ABD_ASSIGN_EVENT *asgn_event;
    ABD_ARITH_EVENT *arith_event;
    ABD_VEC_EVENT *vec_event;
    ABD_IDX_CHANGE_EVENT *idx_event;
  } data;
  ABD_OBJECT *atFunc;
  SEXP env;
  struct abd_event *nextEvent;
} ABD_EVENT;
static int waitingElseIF;

/* struct to manage idx changes*/

static int waitingIdxChange;
typedef struct
{
  int srcVec, destIdxsVec, srcIdxsVec, discard;
  int nIdxChanges;
  SEXP srcValues, srcIdxs, destIdxs;
  SEXP src, dest;
  ABD_OBJECT *destObj;
  ABD_OBJECT *srcObj;
} IDX_CHANGE;

static IDX_CHANGE *idxChanges;

/* Stores Return value related */
static ABD_EVENT *lastRetEvent;
static SEXP lastRetValue;
SEXP possibleRet;
int possibleRetLine;

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
ABD_EVENT_ARG *processArgs(SEXP passedArgs, SEXP receivedArgs, SEXP newRho, ABD_OBJECT *targetFunc);
ABD_OBJECT_MOD *processByType(SEXP symbolValue, ABD_OBJECT_MOD *mod, int);
ABD_EVENT_ARG *setArgValues(ABD_EVENT_ARG *arg, ABD_OBJECT *fromObj, ABD_OBJECT *toObj, ABD_OBJECT_MOD *passedValue);
void setArithEventValues(SEXP call, SEXP ans, SEXP arg1, SEXP arg2, int withPre);
void tmpStoreArith(SEXP call, SEXP ans);
ABD_EVENT *checkPendings(SEXP call, SEXP rhs, ABD_OBJECT *obj);
void setAsgnEventValues(ABD_OBJECT *toObj, SEXP value);
int getCurrScriptLn();
SEXP getResult(const char *expr);
SEXP getSavedArithAns();
void storeVecsForIdxChange(SEXP vec);
int toDiscard();
ABD_SEARCH checkRetStored(SEXP testValue);
void storeRetValues(SEXP value);
void createIndexChangeEvent(SEXP rhs, ABD_OBJECT *objUsed);
void clearPendingVars();