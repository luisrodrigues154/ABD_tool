#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Rinternals.h>
#include <R.h>
#include <Internal.h>
/*
    Here are the declaration of the structures that will store
    the objects needed information as well as their modifications

    The ABD_OBJECT_MOD will store each modification made to the object
    in a linked list mode

    The ABD_OBJECT will be a ordered by usage Double Linked List that will
    store only the necessary information to identify objects

    TODO: do for code flow too

*/

#ifndef loaded_om
#define loaded_om
typedef enum obj_state
{
    ABD_ALIVE,
    ABD_DELETED
} OBJ_STATE;

typedef enum obj_data_typ
{
    ABD_VECTOR = 1,
    ABD_MATRIX = 2,
    ABD_FRAME = 3,
    ABD_ARRAY = 4
} ABD_OBJ_VALUE_TYPE;

typedef struct abd_vec_obj
{
    SEXPTYPE type;
    int idxChange;
    /*
        if is idxChange then
            nCols will represent how many indexes changed
            and vector will be the new value for that index
        else
            a new vector is allocated and nCols will represent
            what the name says (the number of columns in the vector)
            while vector will be the vector totality (1,2,3, ...)
    */
    int nCols;
    int *idxs;
    void *vector;
} ABD_VEC_OBJ;

typedef struct abd_frame_obj
{
    /*
        if is idxChange then
            nCols will represent how many vectors changed
            the *changedVecs will contain the indexes of the changed vectors
            **cols will contain the vector with the change itself similar to the
            ABD_VECOBJ
        else
            **cols contains pointers to ABD_VEC_OBJ that represent the frame columns
            nCols the number of columns
    */
    int idxChange;
    int nCols;
    int *changedVecs;
    ABD_VEC_OBJ **cols;
} ABD_FRAME_OBJ;

typedef struct abd_mtrx_obj
{
    SEXP type;
    int nRows;
    int nCols;
    void *matrix;
} ABD_MTRX_OBJ;

typedef struct abd_obj_mod
{
    int id;
    ABD_OBJ_VALUE_TYPE valueType;
    union
    {
        ABD_MTRX_OBJ *mtrx_value;
        ABD_VEC_OBJ *vec_value;
        ABD_FRAME_OBJ *frame_value;
    } value;
    OBJ_STATE remotion;
    struct abd_obj_mod *prevMod;
    struct abd_obj_mod *nextMod;
} ABD_OBJECT_MOD;

typedef struct abd_obj
{
    int id;
    char *name;
    unsigned int usages;
    OBJ_STATE state;
    SEXP createdEnv;
    char *createdAt;
    //on CF_OBJ will be NULL
    ABD_OBJECT_MOD *modList;
    //pointer to the end of the list (first mod)
    //will require double linked list to then go up (to persist)
    ABD_OBJECT_MOD *modListStart;
    //pointers to neighbours
    struct abd_obj *prevObj;
    struct abd_obj *nextObj;
} ABD_OBJECT;

typedef enum abd_storage
{
    ABD_CF_STORAGE,
    ABD_CMN_STORAGE
} OBJ_STORAGE;

static ABD_OBJECT *cmnObjReg, *cfObjReg;
static ABD_OBJECT *cmnObjRegTail, *cmnObjRegTail;

static int numCfObj;
static int numCmnObj;

#define ABD_OBJECT_NOT_FOUND NULL
#endif
/*
    The prototypes regarding the manipulation of
    the structure that store common objects are below

*/

/*
   ####################################
    Memory manipulation prototypes
   ####################################
*/

/*
    generic
*/
void initObjsRegs();
ABD_OBJECT *memAllocBaseObj();
char *memAllocForString(int strSize);
ABD_OBJECT *doSwap(ABD_OBJECT *objReg, ABD_OBJECT *obj, ABD_OBJECT *node_RHS);
void copyStr(char *dest, const char *src, int strSize);
/*
    CMN_OBJ specifics
*/
ABD_OBJECT_MOD *memAllocMod();

/*
    CF_OBJ specifics
*/

/*
   ####################################
    Registries Management prototypes
   ####################################
*/
ABD_OBJECT *createLocalVariable(const char *name, SEXP rho, SEXP rhs, ABD_OBJECT *createdAt);
void setObjBaseValues(ABD_OBJECT *obj, const char *name, SEXP createdEnv);
ABD_OBJECT *addEmptyObjToReg(ABD_OBJECT *objReg);
ABD_OBJECT *findCmnObj(const char *name, SEXP createdEnv);
ABD_OBJECT *findRHS(ABD_OBJECT *objReg, ABD_OBJECT *obj);
void changeNeighbours(ABD_OBJECT *obj);
ABD_OBJECT *rankObjByUsages(ABD_OBJECT *objReg, ABD_OBJECT *obj);
ABD_OBJECT *newObjUsage(SEXP lhs, SEXP rhs, SEXP rho);

ABD_OBJECT_MOD *addEmptyModToObj(ABD_OBJECT *obj, SEXPTYPE type);
ABD_VEC_OBJ *createRealVector(SEXP rhs);
ABD_OBJECT *getCmnObj(const char *name, SEXP rho);
ABD_OBJECT *getCfObj(const char *name, SEXP rho);
ABD_OBJECT *findFuncObj(const char *name, SEXP callingEnv);
void processVarIdxChange(SEXP rhs);
ABD_OBJECT *createUnscopedObj(const char *name, int objId, int valId, SEXP value, int withIndex);
ABD_FRAME_OBJ *memAllocFrameObj();