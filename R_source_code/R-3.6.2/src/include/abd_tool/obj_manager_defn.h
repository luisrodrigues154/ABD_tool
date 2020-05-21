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
            if idxChange is y then
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
    union {
        ABD_MTRX_OBJ *mtrx_value;
        ABD_VEC_OBJ *vec_value;
        char *str_value;
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

static int waitingIdxChange;

//vectors
typedef struct changeElem
{
    SEXPTYPE type;
    int idx;
    void *data;
} CHANGE_ELEMENT;

typedef struct
{
    int nIdxChanges;
    int *idxs;
    ABD_OBJECT *varChanged;
} IDX_CHANGE;

static IDX_CHANGE *idxChanges;

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
ABD_OBJECT *findObj(ABD_OBJECT *objReg, const char *name, SEXP createdEnv);
ABD_OBJECT *findRHS(ABD_OBJECT *objReg, ABD_OBJECT *obj);
void changeNeighbours(ABD_OBJECT *obj);
ABD_OBJECT *rankObjByUsages(ABD_OBJECT *objReg, ABD_OBJECT *obj);
ABD_OBJECT *newObjUsage(SEXP lhs, SEXP rhs, SEXP rho);
ABD_OBJECT_MOD *setModValues(ABD_OBJECT_MOD *newModification, SEXP newValue, ABD_OBJECT_MOD *(*func)(ABD_OBJECT_MOD *, SEXP));
ABD_OBJECT_MOD *addEmptyModToObj(ABD_OBJECT *obj, SEXPTYPE type);
ABD_OBJECT_MOD *createRealVector(ABD_OBJECT_MOD *mod, SEXP rhs);
ABD_OBJECT_MOD *createRealVectorIdxChange(ABD_OBJECT_MOD *newMod, SEXP rhs);
ABD_OBJECT *getCmnObj(const char *name, SEXP rho);
ABD_OBJECT *getCfObj(const char *name, SEXP rho);
ABD_OBJECT *findFuncObj(const char *name, SEXP callingEnv);
void prepForIdxChange(SEXP var);
