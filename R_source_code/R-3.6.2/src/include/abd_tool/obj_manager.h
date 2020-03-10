#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/json_helpers.h>
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

void setObjBaseValues(ABD_OBJECT * obj, char * name, SEXPTYPE type, char * createdEnv){
    int nameSize = strlen(name);
    int createdEnvSize = strlen(createdEnv);
    
    obj->type = type;
    obj->name = memAllocForString(nameSize);
    copyStr(obj->name, name, nameSize);

    obj->usages = 0;
    obj->state = ABD_ALIVE;

    obj->createdEnv = memAllocForString(createdEnvSize);
    copyStr(obj->createdEnv, createdEnv, createdEnvSize);

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

ABD_OBJECT * findObj(ABD_OBJECT * objReg, char * name, char * createdEnv){
    if(objReg == ABD_OBJECT_NOT_FOUND)
        //registry empty, alloc
        return ABD_OBJECT_NOT_FOUND;
    
    //registry have elements
    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT * currentObject = objReg;
    unsigned short result = 0, result2 = 0;
    unsigned short found = 0;
    do{
        result2 = strncmp(currentObject->createdEnv, createdEnv, strlen(createdEnv)*sizeof(char));
        if(result2 == 0){
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
void printObj(ABD_OBJECT * currentObj){
    printf("name: %s\n", currentObj->name);
    printf("Prev name: %s\n", (currentObj->prevObj == NULL) ? "NULL" : currentObj->prevObj->name);
    printf("Next name: %s\n", (currentObj->nextObj == NULL) ? "NULL" : currentObj->nextObj->name);
}
void printi(ABD_OBJECT * cmnObjReg){
   if(cmnObjReg == ABD_OBJECT_NOT_FOUND)
        printf("REG NULL\n");
    else{
        ABD_OBJECT * currentObj = cmnObjReg;
        do{
            printObj(currentObj);
            currentObj = currentObj->nextObj;
        }while(currentObj!=NULL);
    }
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
//below is used to all (except closures)
void newCmnObjUsage(SEXP lhs, SEXP rhs, SEXP rho){
    unsigned int instructionNumber = 1;
    SEXPTYPE type = TYPEOF(rhs);
    int value = REAL(rhs)[0]; 
    char * functionName = "main";
    OBJ_STATE remotion = ABD_ALIVE;
    
    //name extraction from lhs
    int nameSize = strlen(CHAR(PRINTNAME(lhs)));
    char * name = memAllocForString(nameSize);
    copyStr(name,CHAR(PRINTNAME(lhs)), nameSize);
    char * createdEnv = environmentExtraction(rho);

    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;

    
    if(cmnObjReg == ABD_OBJECT_NOT_FOUND){
        //registry empty, alloc    
        cmnObjReg = addEmptyObjToReg(cmnObjReg);
        objectFound = cmnObjReg;
        setObjBaseValues(objectFound, name, type, createdEnv);
    }
    else
        objectFound = findObj(cmnObjReg, name, createdEnv);
    

    if(objectFound == ABD_OBJECT_NOT_FOUND){
        //alloc
        objectFound = addEmptyObjToReg(cmnObjReg);
        setObjBaseValues(objectFound, name, type, createdEnv);
    }
    
    objectFound->modList = setModValues(addEmptyModToObj(objectFound), value, remotion);
    objectFound->usages++;

    rankObjByUsages(cmnObjReg, objectFound);
}
//below is used to closures
void newCfObjUsage(SEXP lhs, SEXP rhs, SEXP rho){
    unsigned int instructionNumber = 1;
    SEXPTYPE type = CLOSXP; 
    char * functionName = "main";
    OBJ_STATE remotion = ABD_ALIVE;
    
    //name extraction from lhs
    int nameSize = strlen(CHAR(PRINTNAME(lhs)));
    char * name = memAllocForString(nameSize);
    copyStr(name,CHAR(PRINTNAME(lhs)), nameSize);
    
    char * createdEnv = environmentExtraction(rho);

    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;

    
    if(cfObjReg == ABD_OBJECT_NOT_FOUND){
        //registry empty, alloc    
        cfObjReg = addEmptyObjToReg(cfObjReg);
        objectFound = cfObjReg;
        setObjBaseValues(objectFound, name, type, createdEnv);
    }
    else
        objectFound = findObj(cfObjReg, name, createdEnv);
    

    if(objectFound == ABD_OBJECT_NOT_FOUND){
        //alloc
        objectFound = addEmptyObjToReg(cfObjReg);
        setObjBaseValues(objectFound, name, type, createdEnv);
    }
    
    //setModValues(addEmptyModToObj(objectFound), instructionNumber,functionName, value, remotion);
    objectFound->usages++;
    rankObjByUsages(cfObjReg, objectFound);
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
ABD_OBJECT_MOD * setModValues(ABD_OBJECT_MOD * newModification, int newValue, OBJ_STATE remotion){
    newModification->newValue = newValue;
    newModification->remotion = remotion;
    return newModification;
}


ABD_OBJECT_MOD * addEmptyModToObj(ABD_OBJECT * obj){
    
    if(obj->modList == ABD_OBJECT_NOT_FOUND){
        //list empty
        obj->modList = memAllocMod();
        obj->modList->prevMod = ABD_OBJECT_NOT_FOUND;
        obj->modList->nextMod = ABD_OBJECT_NOT_FOUND;

        obj->modListStart = obj->modList;
        
        
    }else{
        //the last valid registry is stored in the newModification var
        //alloc memory to new node and assign to nextMod
        
        ABD_OBJECT_MOD * newMod = memAllocMod();

        obj->modList->nextMod = newMod;
        newMod->prevMod = obj->modList;
        newMod->nextMod = ABD_OBJECT_NOT_FOUND;
        obj->modList = newMod;
    
    }
    
    return obj->modList;
}

/*
    CF_OBJ specifics
*/