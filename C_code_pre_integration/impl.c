#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum obj_state
{
    ABD_DELETED,
    ABD_ALIVE
} OBJ_STATE;

typedef enum abd_state
{
    ABD_ENABLE = 1,
    ABD_DISABLE = 0
} ABD_STATE;

typedef struct abd_obj_mod
{
    unsigned int instructionNumber;
    char *functionName;
    int newValue;
    OBJ_STATE remotion;
    struct abd_obj_mod *nextMod;
} ABD_OBJECT_MOD;

typedef struct abd_obj
{
    unsigned short type;
    char *name;
    unsigned int usages;
    OBJ_STATE state;
    char *createdEnv;
    //on CF_OBJ will be NULL
    ABD_OBJECT_MOD *modList;
    //pointers to neighbours
    struct abd_obj *prevObj;
    struct abd_obj *nextObj;
} ABD_OBJECT;

typedef enum abd_storage
{
    ABD_CF_STORAGE,
    ABD_CMN_STORAGE
} OBJ_STORAGE;

#define ABD_OBJECT_NOT_FOUND NULL
static ABD_STATE watcherState = ABD_DISABLE;
static ABD_OBJECT *cmnObjReg, *cfObjReg;
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
ABD_OBJECT *memAllocBaseObj();
char *memAllocForString(int strSize);
void wipeRegs();
void initRegs();
void doSwap(ABD_OBJECT *objReg, ABD_OBJECT *obj, ABD_OBJECT *node_RHS);
void copyStr(char *dest, char *src, int strSize);
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

/*
    generic
*/
void setObjBaseValues(ABD_OBJECT *obj, char *name, unsigned short type, char *createdEnv, int value);
ABD_OBJECT *addEmptyObjToReg(ABD_OBJECT *objReg);
ABD_OBJECT *findCmnObj(char *name, char *createdEnv);
ABD_OBJECT *findRHS(ABD_OBJECT *objReg, ABD_OBJECT *obj);
void changeNeighbours(ABD_OBJECT *obj);
void rankObjByUsages(ABD_OBJECT *objReg, ABD_OBJECT *obj);
//void newObjUsage(SEXP lhs, SEXP rhs, SEXP rho);
void newObjUsage();
//char * envToStr(SEXP rho)

/*
    CMN_OBJ specifics
*/
void setModValues(ABD_OBJECT_MOD *newModification, int instructionNumber, char *functionName, int newValue, OBJ_STATE remotion);
ABD_OBJECT_MOD *addModToObj(ABD_OBJECT *obj);
ABD_OBJECT_MOD *addEmptyModToObj(ABD_OBJECT *obj);
/*
    CF_OBJ specifics
*/

int main()
{
    cmnObjReg = initRegs();
    newObjUsage();
    return 0;
}
