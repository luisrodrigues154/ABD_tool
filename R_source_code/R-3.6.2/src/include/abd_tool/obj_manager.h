#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/env_stack_defn.h>
#include <Print.h>
#include <ctype.h>
#include <strings.h>

/*
   ####################################
    Memory manipulation implementation
   ####################################
*/

/*
    generic
*/
void initObjsRegs()
{
    //set NULL to initialize
    cmnObjReg = ABD_OBJECT_NOT_FOUND;
    cfObjReg = ABD_OBJECT_NOT_FOUND;
    numCfObj = 0;
    numCmnObj = 0;
}

void wipeObjMods(ABD_OBJECT_MOD *listStart)
{
    ABD_OBJECT_MOD *currMod = listStart;

    do
    {
        ABD_OBJECT_MOD *freeMod = currMod;
        currMod = currMod->nextMod;
        free(freeMod);
    } while (currMod != ABD_NOT_FOUND);
}
void wipeRegs()
{
    //clears registry memory (all nodes)
    //TODO: clear ABD_OBJ_MOD_LIST not done

    //wipe common objects registry
    ABD_OBJECT *currentObject = cmnObjReg;
    while (currentObject != ABD_OBJECT_NOT_FOUND)
    {
        ABD_OBJECT *next = currentObject->nextObj;
        free(currentObject);
        currentObject = next;
    }

    //wipe codeflow objects registry
    currentObject = cfObjReg;
    while (currentObject != ABD_OBJECT_NOT_FOUND)
    {
        ABD_OBJECT *next = currentObject->nextObj;
        free(currentObject);
        currentObject = next;
    }
}

ABD_OBJECT *memAllocBaseObj()
{
    return (ABD_OBJECT *)malloc(sizeof(ABD_OBJECT));
}

char *memAllocForString(int strSize)
{
    //+1 for \0 terminator
    return (char *)malloc(strSize * sizeof(char) + 1);
}

ABD_OBJECT *doSwap(ABD_OBJECT *objReg, ABD_OBJECT *obj, ABD_OBJECT *node_RHS)
{
    //set obj->prev = (node_RHS->prev)
    obj->prevObj = node_RHS->prevObj;

    //verify NULL pointer to prev
    if (node_RHS->prevObj != ABD_OBJECT_NOT_FOUND)
        //RHS NOT HEAD OF LIST
        //set (node_RHS->prev)->next = obj
        (node_RHS->prevObj)->nextObj = obj;
    else
        //RHS HEAD OF THE LIST
        //need to assign the new head of the registry to it
        objReg = obj;

    //set node_RHS->prev = obj
    node_RHS->prevObj = obj;

    //set obj->next = node_RHS
    obj->nextObj = node_RHS;

    return objReg;
}

/*
    CMN_OBJ specifics
*/

ABD_OBJECT_MOD *memAllocMod()
{
    return (ABD_OBJECT_MOD *)malloc(sizeof(ABD_OBJECT_MOD));
}

/*
   ######################################
    Registries Management implementation
   ######################################
*/

/*
    generic
*/
void copyStr(char *dest, const char *src, int strSize)
{
    strncpy(dest, src, strSize * sizeof(char));
    dest[strSize] = '\0';
}

void setObjBaseValues(ABD_OBJECT *obj, const char *name, SEXP createdEnv)
{
    int nameSize = strlen(name);
    obj->name = memAllocForString(nameSize);
    copyStr(obj->name, name, nameSize);

    obj->usages = 0;
    obj->state = ABD_ALIVE;

    obj->createdEnv = createdEnv;
    ABD_OBJECT *createdAt = getCurrFuncObj();
    if (createdAt == ABD_OBJECT_NOT_FOUND)
    {
        int mainSize = strlen("main");
        obj->createdAt = memAllocForString(mainSize);
        copyStr(obj->createdAt, "main", mainSize);
    }
    else
        obj->createdAt = getCurrFuncObj()->name;

    obj->modListStart = ABD_OBJECT_NOT_FOUND;
    obj->modList = ABD_OBJECT_NOT_FOUND;
}
ABD_OBJECT *addEmptyObjToReg(ABD_OBJECT *objReg)
{
    if (objReg == ABD_OBJECT_NOT_FOUND)
    {
        //empty registry
        objReg = memAllocBaseObj();
        objReg->nextObj = ABD_OBJECT_NOT_FOUND;
        objReg->prevObj = ABD_OBJECT_NOT_FOUND;
        return objReg;
    }
    ABD_OBJECT *auxObject = objReg;
    while (auxObject->nextObj != ABD_OBJECT_NOT_FOUND)
        auxObject = auxObject->nextObj;

    //auxObject->next_ABD_OBJECT is empty
    ABD_OBJECT *newObject = auxObject->nextObj = memAllocBaseObj();

    //set DLList pointers
    newObject->prevObj = auxObject;
    newObject->nextObj = ABD_OBJECT_NOT_FOUND;

    return newObject;
}
ABD_OBJECT *findFuncObj(const char *name, SEXP callingEnv)
{
    if (cfObjReg == ABD_OBJECT_NOT_FOUND)
        //registry empty, alloc
        return ABD_OBJECT_NOT_FOUND;

    //registry have elements
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT *currentObject = cfObjReg;
    unsigned short found = 0;
    do
    {
        // line below mitigates recursion and calling functions declared
        // in the global env as well as declared in function scope

        if ((strncmp(currentObject->name, name, strlen(name) * sizeof(char)) == 0) && (currentObject->createdEnv == getInitialEnv() || currentObject->createdEnv == callingEnv))
        {
            objectFound = currentObject;
            break;
        }

        currentObject = currentObject->nextObj;
    } while (currentObject != ABD_OBJECT_NOT_FOUND);

    return objectFound;
}
ABD_OBJECT *findCmnObj(const char *name, SEXP createdEnv)
{
    if (cmnObjReg == ABD_OBJECT_NOT_FOUND)
        //registry empty, alloc
        return ABD_OBJECT_NOT_FOUND;

    //registry have elements
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT *currentObject = cmnObjReg;
    unsigned short result = 0, result2 = 0;
    unsigned short found = 0;
    do
    {
        if ((currentObject->createdEnv == createdEnv) && (strcmp(currentObject->name, name) == 0))
        {
            objectFound = currentObject;
            break;
        }
        currentObject = currentObject->nextObj;
    } while (currentObject != ABD_OBJECT_NOT_FOUND);

    return objectFound;
}

ABD_OBJECT *findRHS(ABD_OBJECT *objReg, ABD_OBJECT *obj)
{
    //verify if already top of the list
    if (obj->prevObj == ABD_OBJECT_NOT_FOUND)
        return ABD_OBJECT_NOT_FOUND;

    //verify if already well ranked (before.usages > node.usages)
    if ((obj->prevObj)->usages >= obj->usages)
        return ABD_OBJECT_NOT_FOUND;

    //find the node which will stay immediately after the node being ranked
    ABD_OBJECT *node_RHS = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT *auxNode = obj->prevObj;
    unsigned short found = 0;
    do
    {
        if (auxNode->usages < obj->usages)
        {
            //need to move
            node_RHS = auxNode;
            auxNode = auxNode->prevObj;
        }
        else
            found = 1;

        if (auxNode == ABD_OBJECT_NOT_FOUND)
            //left side will be the HEAD
            //right side will be node_RHS
            found = 1;

    } while (found != 1);

    return node_RHS;
}
void changeNeighbours(ABD_OBJECT *obj)
{
    //prev obj NULL
    //prev NULL

    if (obj->prevObj != ABD_OBJECT_NOT_FOUND)
        (obj->prevObj)->nextObj = obj->nextObj;

    //check if last in registry
    if (obj->nextObj != ABD_OBJECT_NOT_FOUND)
        (obj->nextObj)->prevObj = obj->prevObj;
}

ABD_OBJECT *rankObjByUsages(ABD_OBJECT *objReg, ABD_OBJECT *obj)
{
    //this function searches for a space in the list where
    //the node before has more usages or is NULL (HEAD)
    //and the node after has less usages than the node being ranked.
    // Before doing the swap it is needed to save the references that the node stores (changeNeighbours).
    //This way the list can maintain coerence
    ABD_OBJECT *node_RHS = findRHS(objReg, obj);
    if (node_RHS != ABD_OBJECT_NOT_FOUND)
    {
        changeNeighbours(obj);
        objReg = doSwap(objReg, obj, node_RHS);
    }

    return objReg;
}

ABD_OBJECT *getCmnObj(const char *name, SEXP rho)
{
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;

    if (cmnObjReg == ABD_OBJECT_NOT_FOUND)
    {
        //registry empty, alloc
        cmnObjReg = addEmptyObjToReg(cmnObjReg);
        objectFound = cmnObjReg;
        objectFound->id = ++numCmnObj;
        setObjBaseValues(objectFound, name, rho);
    }
    else
        objectFound = findCmnObj(name, rho);

    if (objectFound == ABD_OBJECT_NOT_FOUND)
    {
        //alloc
        objectFound = addEmptyObjToReg(cmnObjReg);
        objectFound->id = ++numCmnObj;
        setObjBaseValues(objectFound, name, rho);
    }
    return objectFound;
}
ABD_OBJECT *getCfObj(const char *name, SEXP rho)
{
    ABD_OBJECT *objectFound = ABD_OBJECT_NOT_FOUND;

    if (cfObjReg == ABD_OBJECT_NOT_FOUND)
    {
        //registry empty, alloc
        cfObjReg = addEmptyObjToReg(cfObjReg);
        objectFound = cfObjReg;
        objectFound->id = ++numCfObj;
        setObjBaseValues(objectFound, name, rho);
    }
    else
        objectFound = findFuncObj(name, rho);

    if (objectFound == ABD_OBJECT_NOT_FOUND)
    {
        //alloc
        objectFound = addEmptyObjToReg(cfObjReg);
        objectFound->id = ++numCfObj;
        setObjBaseValues(objectFound, name, rho);
    }

    objectFound->modList = ABD_OBJECT_NOT_FOUND;
    return objectFound;
}

/*
    CMN_OBJ specifics
*/
void manageMatrix()
{
    puts("arrived");
}
ABD_OBJECT_MOD *initValueUnion(ABD_OBJECT_MOD *newMod)
{
    newMod->value.mtrx_value = NULL;
    newMod->value.vec_value = NULL;
    newMod->value.str_value = NULL;

    return newMod;
}
ABD_VEC_OBJ *memAllocVecObj()
{
    return (ABD_VEC_OBJ *)malloc(sizeof(ABD_VEC_OBJ));
}

double *memAllocDoubleVector(int size)
{
    return (double *)malloc(size * sizeof(double));
}
int *memAllocIntVector(int size)
{
    return (int *)malloc(size * sizeof(int));
}
ABD_OBJECT_MOD *createIntVector(ABD_OBJECT_MOD *newMod, SEXP rhs)
{
    int nElements = Rf_nrows(rhs);
    newMod->value.vec_value = memAllocVecObj();
    newMod->value.vec_value->idxChange = 0;
    newMod->value.vec_value->idxs = ABD_NOT_FOUND;
    newMod->value.vec_value->nCols = nElements;
    newMod->value.vec_value->vector = memAllocIntVector(nElements);

    for (int i = 0; i < nElements; i++)
        ((int *)newMod->value.vec_value->vector)[i] = INTEGER(rhs)[i];

    return newMod;
}
ABD_OBJECT_MOD *createRealVector(ABD_OBJECT_MOD *newMod, SEXP rhs)
{
    int nElements = Rf_nrows(rhs);
    newMod->value.vec_value = memAllocVecObj();
    newMod->value.vec_value->idxChange = 0;
    newMod->value.vec_value->idxs = ABD_NOT_FOUND;
    newMod->value.vec_value->nCols = nElements;
    newMod->value.vec_value->vector = memAllocDoubleVector(nElements);

    for (int i = 0; i < nElements; i++)
        ((double *)newMod->value.vec_value->vector)[i] = REAL(rhs)[i];

    return newMod;
}
ABD_OBJECT_MOD *intVectorMultiChanges(ABD_OBJECT_MOD *newMod, SEXP rhs)
{

    ABD_OBJECT_MOD *firstMod = newMod;
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    newMod->value.vec_value = memAllocVecObj();
    newMod->value.vec_value->idxChange = 1;

    int srcSize = Rf_length(idxChanges->srcValues);
    int destSize = Rf_length(idxChanges->destIdxs);

    newMod->value.vec_value->nCols = idxChanges->nIdxChanges;

    //what indexes changed
    newMod->value.vec_value->idxs = memAllocIntVector(idxChanges->nIdxChanges);

    //the values themselves
    newMod->value.vec_value->vector = memAllocDoubleVector(idxChanges->nIdxChanges);
    int repeater = Rf_length(idxChanges->srcValues);
    int i, j;
    for (i = j = 0; i < idxChanges->nIdxChanges; i++, j++)
    {
        int toIdx = 0;
        if (TYPEOF(idxChanges->destIdxs) == REALSXP)
            toIdx = (int)REAL(idxChanges->destIdxs)[i];
        else
            //intsxp
            toIdx = INTEGER(idxChanges->destIdxs)[i];

        //because R is 1-n, c is 0-n-1
        toIdx--;

        newMod->value.vec_value->idxs[i] = toIdx;

        if (j == repeater)
            j = 0;

        ((int *)newMod->value.vec_value->vector)[i] = INTEGER(rhs)[j];
    }
    return newMod;
}
ABD_OBJECT_MOD *realVectorMultiChanges(ABD_OBJECT_MOD *newMod, SEXP rhs)
{

    ABD_OBJECT_MOD *firstMod = newMod;
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    newMod->value.vec_value = memAllocVecObj();
    newMod->value.vec_value->idxChange = 1;

    int srcSize = Rf_length(idxChanges->srcValues);
    int destSize = Rf_length(idxChanges->destIdxs);
    if (idxChanges->nIdxChanges == 0)
    {
        idxChanges->nIdxChanges = destSize;
    }

    newMod->value.vec_value->nCols = idxChanges->nIdxChanges;

    //what indexes changed
    newMod->value.vec_value->idxs = memAllocIntVector(idxChanges->nIdxChanges);

    //the values themselves
    newMod->value.vec_value->vector = memAllocDoubleVector(idxChanges->nIdxChanges);
    int repeater = Rf_length(idxChanges->srcValues);
    int i, j;
    for (i = j = 0; i < idxChanges->nIdxChanges; i++, j++)
    {
        int toIdx = 0;
        if (TYPEOF(idxChanges->destIdxs) == REALSXP)
            toIdx = (int)REAL(idxChanges->destIdxs)[i];
        else
            //intsxp
            toIdx = INTEGER(idxChanges->destIdxs)[i];

        //because R is 1-n, c is 0-n-1
        toIdx--;

        newMod->value.vec_value->idxs[i] = toIdx;

        if (j == repeater)
            j = 0;

        ((double *)newMod->value.vec_value->vector)[i] = REAL(rhs)[j];
    }
    return newMod;
}

ABD_OBJECT_MOD *setModValues(ABD_OBJECT_MOD *newModification, SEXP newValue, ABD_OBJECT_MOD *(*func)(ABD_OBJECT_MOD *, SEXP))
{
    if (newValue == ABD_NOT_FOUND)
    {
        newModification->remotion = ABD_DELETED;
        return newModification;
    }

    newModification->remotion = ABD_ALIVE;
    newModification = (*func)(newModification, newValue);

    return newModification;
}

int getObjStructType(SEXP symbolValue)
{

    if (isVector(symbolValue))
        return 1;
    else if (isMatrix(symbolValue))
        return 2;
    else if (isFrame(symbolValue))
        return 3;
    else if (isArray(symbolValue))
        return 4;
    return 0;
}

ABD_OBJECT_MOD *addEmptyModToObj(ABD_OBJECT *obj, ABD_OBJ_VALUE_TYPE type)
{

    if (obj->modList == ABD_OBJECT_NOT_FOUND)
    {
        //list empty
        obj->modList = memAllocMod();
        obj->modList->prevMod = ABD_OBJECT_NOT_FOUND;
        obj->modList->nextMod = ABD_OBJECT_NOT_FOUND;
        obj->modListStart = obj->modList;
    }
    else
    {
        //has modifications
        //alloc and set at modList (head) [fast search]
        ABD_OBJECT_MOD *newMod = memAllocMod();

        obj->modList->nextMod = newMod;
        newMod->prevMod = obj->modList;
        newMod->nextMod = ABD_OBJECT_NOT_FOUND;
        obj->modList = newMod;
    }
    obj->modList->valueType = type;
    obj->modList = initValueUnion(obj->modList);
    obj->modList->id = obj->usages + 1;
    return obj->modList;
}

ABD_OBJECT *createUnscopedObj(const char *name, int objId, int valId, SEXP value, int withIndex)
{
    ABD_OBJECT *hcObj = memAllocBaseObj();
    setObjBaseValues(hcObj, name, getCurrentEnv());
    hcObj->id = objId;
    hcObj->usages = 1;
    if (value != R_NilValue)
    {
        hcObj->modList = addEmptyModToObj(hcObj, getObjStructType(value));
        hcObj->modList->id = valId;
        hcObj->modList = processByType(value, hcObj->modList, withIndex);
    }
    else
        hcObj->modList = ABD_NOT_FOUND;

    return hcObj;
}
ABD_OBJECT *createLocalVariable(const char *name, SEXP rho, SEXP rhs, ABD_OBJECT *createdAt)
{
    ABD_OBJECT *obj = getCmnObj(name, rho);

    obj->createdAt = createdAt->name;
    ABD_OBJECT_MOD *newMod = addEmptyModToObj(obj, getObjStructType(rhs));

    newMod = processByType(rhs, newMod, 0);
    obj->modList = newMod;
    obj->usages++;
    cmnObjReg = rankObjByUsages(cmnObjReg, obj);

    return obj;
}
ABD_OBJECT *newObjUsage(SEXP lhs, SEXP rhs, SEXP rho)
{
    if (!isalpha(CHAR(PRINTNAME(lhs))[0]))
        return ABD_OBJECT_NOT_FOUND;

    //name extraction from lhs
    char *name;

    int nameSize = strlen(CHAR(PRINTNAME(lhs)));
    name = memAllocForString(nameSize);
    copyStr(name, CHAR(PRINTNAME(lhs)), nameSize);

    ABD_OBJECT *obj = ABD_OBJECT_NOT_FOUND;
    if (TYPEOF(rhs) == CLOSXP)
        obj = getCfObj(name, rho);
    else
    {
        ABD_OBJECT_MOD *newMod = ABD_OBJECT_NOT_FOUND;

        obj = getCmnObj(name, rho);

        newMod = addEmptyModToObj(obj, getObjStructType(rhs));

        newMod = processByType(rhs, newMod, 0);
        obj->modList = newMod;
        obj->usages++;
        cmnObjReg = rankObjByUsages(cmnObjReg, obj);
    }

    return obj;
}

void printReg()
{
    ABD_OBJECT *auxObject = cfObjReg;
    while (auxObject != ABD_OBJECT_NOT_FOUND)
    {
        printf("name: %s\n", auxObject->name);
        printf("prev null: %s\n", auxObject->prevObj == ABD_OBJECT_NOT_FOUND ? "yes" : "no");
        printf("next null: %s\n", auxObject->nextObj == ABD_OBJECT_NOT_FOUND ? "yes" : "no");
        auxObject = auxObject->nextObj;
    }
}

void processVarIdxChange(SEXP result)
{
    IDX_CHANGE *idxChanges = getCurrIdxChanges();
    if (idxChanges->srcValues == R_NilValue)
        idxChanges->srcValues = result;

    SEXP rhs = idxChanges->srcValues;
    ABD_OBJECT *obj = idxChanges->destObj = findCmnObj(CHAR(PRINTNAME(idxChanges->dest)), getCurrentEnv());
    if (obj != ABD_OBJECT_NOT_FOUND)
    {
        //dest idxs vector will always say how many changes will be performed
        idxChanges->nIdxChanges = Rf_length(idxChanges->destIdxs);
        //the object was found in the registry, need to process new mod
        ABD_OBJECT_MOD *newMod = ABD_OBJECT_NOT_FOUND;

        newMod = addEmptyModToObj(obj, getObjStructType(rhs));
        newMod = processByType(rhs, newMod, 1);
        obj->modList = newMod;
        obj->usages++;
        cmnObjReg = rankObjByUsages(cmnObjReg, obj);
        createIndexChangeEvent(result, obj);
    }
    clearPendingVars();
}
