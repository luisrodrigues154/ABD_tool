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
static int waitingElseIF;

/* for loops helper vars */
static short waitingForVecs;
Rboolean forIdxsVec;
Rboolean forValVec;
static int forValPos;

typedef struct loop_chain
{
  short initialBranchDepth;
  ABD_LOOP_TAGS loopType;
  union {
    ABD_WHILE_LOOP_EVENT *whileLoop;
    ABD_FOR_LOOP_EVENT *forLoop;
    ABD_REPEAT_LOOP_EVENT *repeatLoop;
  } loop;
  ITERATION *currIter;
  struct loop_chain *prevLoop;
} ABD_LOOP_CHAIN;

static ABD_LOOP_CHAIN *loopStack;
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
static short exprId;

/* Stores Vector creation values related*/
static SEXP vecValues;
static SEXP auxVecCall;
static int auxVecLine;

/* Stores data frame creation values related */

typedef struct frame_creation
{
  Rboolean srcVec, srcIdxsVec, discard;
  SEXP srcObj;
  SEXP srcVal;
  SEXP srcIdxs;
} FRAME_CREATION;

int numFrameSrcs;
int *waitingFrameIdxs;
FRAME_CREATION **frameSrcs;

static short waitingFrameVecs;
static Rboolean pendingFrame;
static SEXP frameCall;


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
void pushForEvent(ABD_FOR_LOOP_EVENT *newForEvent);
void pushRepeatEvent(ABD_REPEAT_LOOP_EVENT *newRepeatEvent);
void pushWhileEvent(ABD_WHILE_LOOP_EVENT *newWhileLoopEvent);
void pushNewLoop(ABD_LOOP_TAGS type, void *newEvent);
void addEventToCurrentLoop(ABD_EVENT *newEvent);
void popLoopFromStack(ABD_LOOP_TAGS requestingType);
void appendLastEventToLoop(ABD_LOOP_TAGS type);
ABD_VEC_OBJ *processVector(SEXP symbolValue, int idxChange);
Rboolean inLoopByType(ABD_LOOP_TAGS type);
void preProcessDataFrame(SEXP call);
void preProcessVarIdxChange(SEXP call, ABD_OBJECT *  targetObj, SEXP rho);
void preProcessDataFrameCellChange(SEXP call, ABD_OBJECT * target_df, SEXP rho);
void preProcessDataFrameSrc(SEXP call);
void preProcessDataFrameDest(SEXP call);
void preProcessSrc(SEXP call);
void preProcessDest(SEXP call);
int waitingCellChange();
void storeVecForCellChange(SEXP vec);
ABD_OBJECT_MOD *initModAndPopulate(ABD_OBJECT_MOD *newMod, OBJ_STATE remotion, ABD_OBJ_VALUE_TYPE valueType);
ABD_FRAME_OBJ *processDataFrame(SEXP symbolValue, int idxChange);