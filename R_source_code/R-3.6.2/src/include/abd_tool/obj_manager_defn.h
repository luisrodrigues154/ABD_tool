
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
    typedef enum obj_state {
        ABD_ALIVE,
        ABD_DELETED
    }OBJ_STATE;

    typedef struct abd_obj_mod{
        int newValue;
        OBJ_STATE remotion;
        struct abd_obj_mod * prevMod;
        struct abd_obj_mod * nextMod;
    }ABD_OBJECT_MOD;


    typedef struct abd_obj{
        SEXPTYPE type;
        char * name;
        unsigned int usages;
        OBJ_STATE state;
        char * createdEnv;
        //on CF_OBJ will be NULL
        ABD_OBJECT_MOD * modList;
        //pointer to the end of the list (first mod) 
        //will require double linked list to then go up (to persist)
        ABD_OBJECT_MOD * modListStart;
        //pointers to neighbours
        struct abd_obj * prevObj;
        struct abd_obj * nextObj;
    }ABD_OBJECT;

    typedef enum abd_storage{
        ABD_CF_STORAGE,
        ABD_CMN_STORAGE
    }OBJ_STORAGE;

    static ABD_OBJECT * cmnObjReg, * cfObjReg;
    static ABD_OBJECT * cmnObjRegTail, * cmnObjRegTail; 


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
ABD_OBJECT * memAllocBaseObj();
char * memAllocForString(int strSize);
ABD_OBJECT * doSwap(ABD_OBJECT * objReg, ABD_OBJECT * obj, ABD_OBJECT * node_RHS);
void copyStr(char * dest, char * src, int strSize);
/*
    CMN_OBJ specifics
*/
ABD_OBJECT_MOD * memAllocMod();

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
void setObjBaseValues(ABD_OBJECT * obj, char * name,SEXPTYPE type, char * createdEnv);
ABD_OBJECT * addEmptyObjToReg(ABD_OBJECT * objReg);
ABD_OBJECT * findObj(ABD_OBJECT * objReg, char * name, char * createdEnv);
ABD_OBJECT * findRHS(ABD_OBJECT * objReg, ABD_OBJECT * obj);
void changeNeighbours(ABD_OBJECT * obj);
ABD_OBJECT * rankObjByUsages(ABD_OBJECT * objReg, ABD_OBJECT * obj);
void newCmnObjUsage(SEXP lhs, SEXP rhs, SEXP rho);
void newCfObjUsage(SEXP lhs, SEXP rhs, SEXP rho);
char * environmentExtraction(SEXP rho);

/*
    CMN_OBJ specifics
*/
ABD_OBJECT_MOD * setModValues(ABD_OBJECT_MOD * newModification, int newValue, OBJ_STATE remotion);
ABD_OBJECT_MOD * addModToObj(ABD_OBJECT * obj);
ABD_OBJECT_MOD * addEmptyModToObj(ABD_OBJECT * obj);
/*
    CF_OBJ specifics
*/
