#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <Print.h>

/*
   ####################################
    Memory manipulation implementation
   ####################################
*/

/*
    generic
*/
void initObjsRegs(){
    //set NULL to initialize
    cmnObjReg = ABD_OBJECT_NOT_FOUND;
    cfObjReg = ABD_OBJECT_NOT_FOUND;
    numCfObj = 0;
    numCmnObj = 0;
}

void wipeObjMods(ABD_OBJECT_MOD * listStart){
    ABD_OBJECT_MOD * currMod = listStart;
    
    do{
        ABD_OBJECT_MOD * freeMod = currMod;
        currMod = currMod->nextMod;
        free(freeMod);
    }while(currMod != ABD_NOT_FOUND);

}
void wipeRegs(){
    //clears registry memory (all nodes)
    //TODO: clear ABD_OBJ_MOD_LIST not done
    
    //wipe common objects registry
    ABD_OBJECT * currentObject = cmnObjReg;
    while(currentObject!=ABD_OBJECT_NOT_FOUND){
        ABD_OBJECT * next = currentObject->nextObj;
        free(currentObject);
        currentObject = next;
    }
    
    //wipe codeflow objects registry
    currentObject = cfObjReg;
    while(currentObject!=ABD_OBJECT_NOT_FOUND){
        ABD_OBJECT * next = currentObject->nextObj;
        free(currentObject);
        currentObject = next;
    }
}
ABD_OBJECT * memAllocBaseObj(){
    return (ABD_OBJECT *) malloc(sizeof(ABD_OBJECT));
}

char * memAllocForString(int strSize){
    //+1 for \0 terminator
    return (char *) malloc(strSize * sizeof(char) +1);
}


ABD_OBJECT * doSwap(ABD_OBJECT * objReg, ABD_OBJECT * obj, ABD_OBJECT * node_RHS){
    //set obj->prev = (node_RHS->prev)
    obj->prevObj = node_RHS->prevObj;

    //verify NULL pointer to prev
    if(node_RHS->prevObj != ABD_OBJECT_NOT_FOUND)
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

ABD_OBJECT_MOD * memAllocMod(){
    return (ABD_OBJECT_MOD *) malloc(sizeof(ABD_OBJECT_MOD));
}

/*
   ######################################
    Registries Management implementation
   ######################################
*/

/*
    generic
*/
void copyStr(char * dest, char * src, int strSize){
    strncpy(dest, src, strSize*sizeof(char));
    dest[strSize] = '\0';
}

void setObjBaseValues(ABD_OBJECT * obj, char * name, SEXP createdEnv){
    int nameSize = strlen(name);
    obj->name = memAllocForString(nameSize);
    copyStr(obj->name, name, nameSize);

    obj->usages = 0;
    obj->state = ABD_ALIVE;

    obj->createdEnv = createdEnv;

    obj->modListStart = ABD_OBJECT_NOT_FOUND;
    obj->modList = ABD_OBJECT_NOT_FOUND;
}
ABD_OBJECT * addEmptyObjToReg(ABD_OBJECT * objReg){
    
    if(objReg == ABD_OBJECT_NOT_FOUND){
        //empty registry
        
        objReg = memAllocBaseObj();
        objReg->nextObj = ABD_OBJECT_NOT_FOUND;
        objReg->prevObj = ABD_OBJECT_NOT_FOUND;

        return objReg;
    }
    ABD_OBJECT * auxObject = objReg;
    while(auxObject->nextObj != ABD_OBJECT_NOT_FOUND)
        auxObject = auxObject->nextObj;

    //auxObject->next_ABD_OBJECT is empty
    ABD_OBJECT * newObject = auxObject->nextObj = memAllocBaseObj();
   
    //set DLList pointers
    newObject->prevObj = auxObject;
    newObject->nextObj = ABD_OBJECT_NOT_FOUND;
    
    return newObject;
}
ABD_OBJECT * findFuncObj(char * name, SEXP  callingEnv){
    if(cfObjReg == ABD_OBJECT_NOT_FOUND)
        //registry empty, alloc
        return ABD_OBJECT_NOT_FOUND;
    
    //registry have elements
    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT * currentObject = cfObjReg;
    unsigned short result = 0, result2 = 0;
    unsigned short found = 0;
    do{
        // line below mitigates recursion and calling functions declared
        // in the global env as well as declared in function scope
        result2 = (currentObject->createdEnv == callingEnv) || (currentObject->createdEnv  == R_GlobalEnv);
        if(result2 == 1){
            result = strncmp(currentObject->name, name, strlen(name)*sizeof(char));
            if(result == 0){
                objectFound = currentObject;
                found = 1;
                break;
            }
        }    
        currentObject = currentObject->nextObj;
    }while(currentObject!=ABD_OBJECT_NOT_FOUND);

    if(found==0)
        //if not found
        return ABD_OBJECT_NOT_FOUND;

    return objectFound;
}
ABD_OBJECT * findObj(ABD_OBJECT * objReg, char * name, SEXP  createdEnv){
    if(objReg == ABD_OBJECT_NOT_FOUND)
        //registry empty, alloc
        return ABD_OBJECT_NOT_FOUND;
    
    //registry have elements
    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT * currentObject = objReg;
    unsigned short result = 0, result2 = 0;
    unsigned short found = 0;
    do{
        result2 = (currentObject->createdEnv == createdEnv);
        if(result2 == 1){
            result = strncmp(currentObject->name, name, strlen(name)*sizeof(char));
            if(result == 0){
                objectFound = currentObject;
                found = 1;
                break;
            }
        }    
        currentObject = currentObject->nextObj;
    }while(currentObject!=ABD_OBJECT_NOT_FOUND);

    if(found==0)
        //if not found (new object)
        return ABD_OBJECT_NOT_FOUND;

    return objectFound;
}

ABD_OBJECT * findRHS(ABD_OBJECT * objReg, ABD_OBJECT * obj){
    //verify if already top of the list
    if(obj->prevObj == ABD_OBJECT_NOT_FOUND)
        return ABD_OBJECT_NOT_FOUND;

    //verify if already well ranked (before.usages > node.usages)
    if((obj->prevObj)->usages >= obj->usages)
        return ABD_OBJECT_NOT_FOUND;

    //find the node which will stay immediately after the node being ranked
    ABD_OBJECT * node_RHS = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT * auxNode = obj->prevObj;
    unsigned short found = 0;
    do{
        if(auxNode->usages < obj->usages){
            //need to move
            node_RHS = auxNode;
            auxNode = auxNode->prevObj;
        }else
            found = 1;
        
        if(auxNode == ABD_OBJECT_NOT_FOUND)
            //left side will be the HEAD
            //right side will be node_RHS
            found = 1;

    }while(found!=1);
   
   return node_RHS;
} 
void changeNeighbours(ABD_OBJECT * obj){
    //prev obj NULL
    //prev NULL

    if(obj->prevObj != ABD_OBJECT_NOT_FOUND)
        (obj->prevObj)->nextObj = obj->nextObj;

    //check if last in registry
    if(obj->nextObj != ABD_OBJECT_NOT_FOUND)
        (obj->nextObj)->prevObj = obj->prevObj;

}


ABD_OBJECT * rankObjByUsages(ABD_OBJECT * objReg, ABD_OBJECT * obj){
    //this function searches for a space in the list where
    //the node before has more usages or is NULL (HEAD)
    //and the node after has less usages than the node being ranked.
    // Before doing the swap it is needed to save the references that the node stores (changeNeighbours).
    //This way the list can maintain coerence
    ABD_OBJECT * node_RHS = findRHS(objReg, obj);
    if(node_RHS != ABD_OBJECT_NOT_FOUND){
        changeNeighbours(obj);
        objReg = doSwap(objReg, obj, node_RHS);
    }    
    
    return objReg;
}

void * getAddressForValue(SEXP rhs){
    switch (TYPEOF(rhs))
    {
    case REALSXP:
        //need to verify
        break;
    
    default:
        break;
    }

}
ABD_OBJECT * getCmnObj(char * name, SEXP rho){
     ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;

    
    if(cmnObjReg == ABD_OBJECT_NOT_FOUND){
        //registry empty, alloc
        cmnObjReg = addEmptyObjToReg(cmnObjReg);
        objectFound = cmnObjReg;
        objectFound->id = ++numCmnObj;
        setObjBaseValues(objectFound, name, rho);
        
    }
    else
        objectFound = findObj(cmnObjReg, name, rho);
    

    if(objectFound == ABD_OBJECT_NOT_FOUND){
        //alloc
        objectFound = addEmptyObjToReg(cmnObjReg);
        objectFound->id = ++numCmnObj;
        setObjBaseValues(objectFound, name, rho);
    }
    return objectFound;
}
ABD_OBJECT * getCfObj(char * name, SEXP rho){
     ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;

    
    if(cfObjReg == ABD_OBJECT_NOT_FOUND){
        //registry empty, alloc    
        cfObjReg = addEmptyObjToReg(cfObjReg);
        objectFound = cfObjReg;
        objectFound->id = ++numCfObj;
        setObjBaseValues(objectFound, name, rho);
    }
    else
        objectFound = findObj(cfObjReg, name, rho);
    

    if(objectFound == ABD_OBJECT_NOT_FOUND){
        //alloc
        objectFound = addEmptyObjToReg(cfObjReg);
        objectFound->id = ++numCfObj;
        setObjBaseValues(objectFound, name, rho);
    }

    return objectFound;
}

//below is used to all (except closures)
void newObjUsage(SEXP lhs, SEXP rhs, SEXP rho){
    unsigned int instructionNumber = 1;
    SEXPTYPE type = TYPEOF(rhs);
    
    //name extraction from lhs
    int nameSize = strlen(CHAR(PRINTNAME(lhs)));
    char * name = memAllocForString(nameSize);
    copyStr(name,CHAR(PRINTNAME(lhs)), nameSize);
    
    ABD_OBJECT * obj = ABD_OBJECT_NOT_FOUND;
    switch (type)
    {
        case CLOSXP:
            //closures (function objects)
            //newCfObjUsage(lhs, rhs, rho);
            //basicPrint2();
            obj = getCfObj(name, rho);
            break;
        case REALSXP:
            //newCmnObjUsage(lhs, rhs, rho);
            //basicPrint();
            obj = getCmnObj(name, rho);
            ABD_OBJECT_MOD * newMod = addEmptyModToObj(obj, type);

            obj->modList = setModValues(newMod, rhs, createRealVector);
            break;
        case STRSXP:
            puts("character vectors");
            break;
        case CPLXSXP:
            puts("complex vectors");
            break;
        case LGLSXP:
            puts("logical vectors");
            break;
        case INTSXP:
            puts("integer vectors");
            if(Rf_isMatrix(rhs)){
                int nCols = Rf_ncols(rhs);
                int nRows = Rf_nrows(rhs);
                printf("DIM\nrows %d\ncols %d\n", nRows, nCols);
            }
            break;
        case RAWSXP:
            puts("raw vector");
            break;
        case VECSXP:
            puts("list (generic vector");
            break;
        default:
            break;
    }
    obj->usages++;
    if(type == CLOSXP)
        cfObjReg = rankObjByUsages(cfObjReg, obj);
    else
        cmnObjReg = rankObjByUsages(cmnObjReg, obj);

    if(rhs == lastRetValue){
        lastRetEvent->toObj = obj;
        free(lastRetEvent->retValue);
        lastRetEvent->retValue = obj->modList;
        lastRetEvent = ABD_EVENT_NOT_FOUND;
        lastRetValue = ABD_NOT_FOUND;
    }
}

char * environmentExtraction(SEXP rho){
    const void *vmax = vmaxget();
    static char ch[1000];
    if (rho == R_GlobalEnv)
	    sprintf(ch, "GlobalEnv");
    else if (rho == R_BaseEnv)
    	sprintf(ch, "Base");
    else if (rho == R_EmptyEnv)
	    sprintf(ch, "EmptyEnv");
    else if (R_IsPackageEnv(rho))
	    snprintf(ch, 1000, "%s",translateChar(STRING_ELT(R_PackageEnvName(rho), 0)));
    else snprintf(ch, 1000, "%p", (void *)rho);

    return ch;
}
/*
    CMN_OBJ specifics
*/
void manageMatrix(){
    puts("arrived");
}
ABD_OBJECT_MOD * initValueUnion(ABD_OBJECT_MOD * newMod){
    newMod->value.mtrx_value = NULL;
    newMod->value.vec_value = NULL;
    newMod->value.str_value = NULL;

    return newMod;
}
ABD_VEC_OBJ * memAllocVecObj(){
    return (ABD_VEC_OBJ *) malloc(sizeof(ABD_VEC_OBJ));
}

double * memAllocDoubleVector(int size){
    return (double *) malloc(size*sizeof(double));
}
ABD_OBJECT_MOD * createRealVector(ABD_OBJECT_MOD * newMod, SEXP rhs){
    int nElements = Rf_nrows(rhs);
    newMod->value.vec_value = memAllocVecObj();
    newMod->value.vec_value->nCols = nElements;
    newMod->value.vec_value->vector = memAllocDoubleVector(nElements);
    
    for(int i=0; i<nElements; i++)
       ((double *) newMod->value.vec_value->vector)[i] = REAL(rhs)[i]; 

    return newMod;
}

ABD_OBJECT_MOD * setModValues(ABD_OBJECT_MOD * newModification, SEXP newValue, ABD_OBJECT_MOD * (*func)(ABD_OBJECT_MOD *,SEXP) ){
    
    if(newValue == ABD_NOT_FOUND){
        newModification->remotion = ABD_DELETED;
        return newModification;
    }
    
    newModification->remotion = ABD_ALIVE;
    
    newModification = (*func)(newModification, newValue);

    return newModification;
}


ABD_OBJECT_MOD * addEmptyModToObj(ABD_OBJECT * obj, SEXPTYPE type){
    
    if(obj->modList == ABD_OBJECT_NOT_FOUND){
        //list empty
        obj->modList = memAllocMod();
        obj->modList->prevMod = ABD_OBJECT_NOT_FOUND;
        obj->modList->nextMod = ABD_OBJECT_NOT_FOUND;
        obj->modListStart = obj->modList;
        
    }else{
        //has modifications
        //alloc and set at modList (head) [fast search]
        ABD_OBJECT_MOD * newMod = memAllocMod();

        obj->modList->nextMod = newMod;
        newMod->prevMod = obj->modList;
        newMod->nextMod = ABD_OBJECT_NOT_FOUND;
        obj->modList = newMod;

    }
    
    obj->modList->type = type;
    obj->modList = initValueUnion(obj->modList);
    obj->modList->id = obj->usages+1;
    return obj->modList;
}
