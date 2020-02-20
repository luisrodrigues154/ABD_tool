/*
    In this file are declared the methods definitions as well as needed macros
    The implementation of the below declared methods is implemented at
    the file: 
        ABD_tool.h



*/

/* 
    The needed libraries to this work
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    The needed constants to clear the code and make "things" more
    abstracted and easy to modify
*/

#define ABD_DELETED 1
#define ABD_ALIVE 0
#define ABD_OBJECT_NOT_FOUND NULL

/*
    Here are the declaration of the structures that will store 
    the objects needed information as well as their modifications
    
    The ABD_OBJECT_MOD will store each modification made to the object 
    in a linked list mode

    The ABD_OBJECT will be a ordered by usages Double Linked List that will
    store only the necessary information to identify objects

    TODO: do for code flow too

*/
typedef struct abd_obj_mod{
    unsigned int instructionNumber;
    char * functionName;
    int newValue;
    unsigned short remotion; 
    struct abd_obj_mod * nextMod;
}ABD_OBJECT_MOD;


typedef struct abd_obj{
    unsigned short type;
    char * name;
    unsigned int usages;
    unsigned short removed;
    char * createdEnv;
    ABD_OBJECT_MOD * ABD_OBJECT_MOD_LIST;
    struct abd_obj * prev_ABD_OBJECT;
    struct abd_obj * next_ABD_OBJECT;
}ABD_OBJECT;

/*
    The prototypes regarding the manipulation of the structure that store objects  
    are below

*/
ABD_OBJECT * allocMemoryForObject();
ABD_OBJECT_MOD * allocMemoryForObjectModification();
void setBaseValuesForNewObject(ABD_OBJECT * obj, char * name, unsigned short type, char * createdEnv, int value);
void setValuesForNewModification(ABD_OBJECT * obj, ABD_OBJECT_MOD * newModification, int instructionNumber, char * functionName, int newValue, unsigned short remotion);
ABD_OBJECT_MOD * addModificationToObjectRegistry(ABD_OBJECT * obj);
ABD_OBJECT * addObjectToRegistry();
ABD_OBJECT * objectLookUp(char * name);
ABD_OBJECT * findNode_RHS(ABD_OBJECT * obj);
void changeNeighbours(ABD_OBJECT * obj);
void doSwap(ABD_OBJECT * obj, ABD_OBJECT * node_RHS);
void rankObjectByUsages(ABD_OBJECT * obj);
void objectUsage(SEXP lhs, SEXP rhs, SEXP rho);
void wipeRegistry();
void initializeRegistry();
/*
    General tool methods prototypes are declared below
*/

void ABD_HELP();
void START_WATCHER();
void STOP_WATCHER();
void regVarChange(SEXP, SEXP, SEXP);
void regCodeFlowChange();
