#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <ABD_tool_defn.h>
#include <ABD_tool_JSON_HELPERS.h>
#include <Print.h>

/*
    Here are some needed variables 

*/
static int watcherActivated = 0;
static ABD_OBJECT * objectsRegistry;


void basicPrint();
/*
    Here is the implementation for the methods that manipulate the structures 
    declared in ABD_tool_defn.h (to store objects modifications)

*/

char * allocMemoryForString(int strSize){
    //+1 for \0 terminator
    return (char *) malloc(strSize * sizeof(char) +1);
}

void copyString(char * dest, char * src, int strSize){
    strncpy(dest, src, strSize*sizeof(char));
    dest[strSize] = '\0';
}
ABD_OBJECT * allocMemoryForObject(){
    return (ABD_OBJECT *) malloc(sizeof(ABD_OBJECT));
}
ABD_OBJECT_MOD * allocMemoryForObjectModification(){
    return (ABD_OBJECT_MOD *) malloc(sizeof(ABD_OBJECT_MOD));
}

void setBaseValuesForNewObject(ABD_OBJECT * obj, char * name, unsigned short type, char * createdEnv, int value){
    int nameSize = strlen(name);
    int createdEnvSize = strlen(createdEnv);
    
    obj->type = type;
    obj->name = allocMemoryForString(nameSize);
    copyString(obj->name, name, nameSize);

    obj->usages = 0;
    obj->removed = ABD_ALIVE;

    obj->createdEnv = allocMemoryForString(createdEnvSize);
    copyString(obj->createdEnv, createdEnv, createdEnvSize);

    obj->ABD_OBJECT_MOD_LIST = ABD_OBJECT_NOT_FOUND;
}

void setValuesForNewModification(ABD_OBJECT * obj, ABD_OBJECT_MOD * newModification, int instructionNumber, char * functionName, int newValue, unsigned short remotion){
    //increment usages
    int fNameSize = strlen(functionName);
    obj->usages++;
    newModification->functionName = allocMemoryForString(fNameSize);
    copyString(newModification->functionName, functionName, fNameSize);

    newModification->newValue = newValue;
    newModification->remotion = remotion;
    newModification->instructionNumber = instructionNumber;
    newModification->nextMod = ABD_OBJECT_NOT_FOUND;
}

ABD_OBJECT_MOD * addModificationToObjectRegistry(ABD_OBJECT * obj){
    
    if(obj->ABD_OBJECT_MOD_LIST == ABD_OBJECT_NOT_FOUND){
        //list empty
        obj->ABD_OBJECT_MOD_LIST = allocMemoryForObjectModification();
        obj->ABD_OBJECT_MOD_LIST->nextMod = ABD_OBJECT_NOT_FOUND;
        return obj->ABD_OBJECT_MOD_LIST;
    }
    
    ABD_OBJECT_MOD * auxModification = obj->ABD_OBJECT_MOD_LIST;
    while(auxModification->nextMod != ABD_OBJECT_NOT_FOUND)
        auxModification= auxModification->nextMod;

    //the last valid registry is stored in the newModification var
    //alloc memory to new node and assign to nextMod
    ABD_OBJECT_MOD * newModification = auxModification->nextMod = allocMemoryForObjectModification();

    //return nextMod (brand new memory chunk)
    return newModification;
        
}

ABD_OBJECT * addObjectToRegistry(){
    
    if(objectsRegistry == ABD_OBJECT_NOT_FOUND){
        //empty registry
        objectsRegistry = allocMemoryForObject();
        objectsRegistry->next_ABD_OBJECT = ABD_OBJECT_NOT_FOUND;
        objectsRegistry->prev_ABD_OBJECT = ABD_OBJECT_NOT_FOUND;
        return objectsRegistry;
    }
    ABD_OBJECT * auxObject = objectsRegistry;
    while(auxObject->next_ABD_OBJECT != ABD_OBJECT_NOT_FOUND)
        auxObject = auxObject->next_ABD_OBJECT;

    //auxObject->next_ABD_OBJECT is empty
    ABD_OBJECT * newObject = auxObject->next_ABD_OBJECT = allocMemoryForObject();
   
    //set DLList pointers
    newObject->prev_ABD_OBJECT = auxObject;
    newObject->next_ABD_OBJECT = ABD_OBJECT_NOT_FOUND;
    
    return newObject;
}

ABD_OBJECT * objectLookUp(char * name){
    if(objectsRegistry == ABD_OBJECT_NOT_FOUND)
        //registry empty, alloc
        return ABD_OBJECT_NOT_FOUND;
    
    //registry have elements
    ABD_OBJECT * objectFound = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT * currentObject = objectsRegistry;
    unsigned int result = 0;

    do{
        result = strncmp(currentObject->name, name, strlen(name)*sizeof(char));
    
        if(result == 0)
            objectFound = currentObject;
        else if(currentObject->next_ABD_OBJECT == ABD_OBJECT_NOT_FOUND)
            break;
        else
            currentObject = currentObject->next_ABD_OBJECT;
            
    }while((result!=0) && (currentObject!=ABD_OBJECT_NOT_FOUND));

    if(result!=0)
        //if not found (new object)
        return ABD_OBJECT_NOT_FOUND;

    return objectFound;
}

ABD_OBJECT * findNode_RHS(ABD_OBJECT * obj){
    //verify if already top of the list
    if(obj->prev_ABD_OBJECT == ABD_OBJECT_NOT_FOUND)
        return ABD_OBJECT_NOT_FOUND;

    //verify if already well ranked (before.usages > node.usages)
    if((obj->prev_ABD_OBJECT)->usages >= obj->usages)
        return ABD_OBJECT_NOT_FOUND;

    //find the node which will stay immediately after the node being ranked
    ABD_OBJECT * node_RHS = ABD_OBJECT_NOT_FOUND;
    ABD_OBJECT * auxNode = obj->prev_ABD_OBJECT;
    unsigned short found = 0;
    do{
        if(auxNode->usages < obj->usages){
            //need to move
            node_RHS = auxNode;
            auxNode = auxNode->prev_ABD_OBJECT;
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
    if(obj->prev_ABD_OBJECT != ABD_OBJECT_NOT_FOUND)
        (obj->prev_ABD_OBJECT)->next_ABD_OBJECT = obj->next_ABD_OBJECT;

    //check if last in registry
    if(obj->next_ABD_OBJECT != ABD_OBJECT_NOT_FOUND)
        (obj->next_ABD_OBJECT)->prev_ABD_OBJECT = obj->prev_ABD_OBJECT;

}

void doSwap(ABD_OBJECT * obj, ABD_OBJECT * node_RHS){
    //set obj->prev = (node_RHS->prev)
    obj->prev_ABD_OBJECT = node_RHS->prev_ABD_OBJECT;

    //verify NULL pointer to prev
    if(node_RHS->prev_ABD_OBJECT != ABD_OBJECT_NOT_FOUND)
        //RHS NOT HEAD OF LIST
        //set (node_RHS->prev)->next = obj
        (node_RHS->prev_ABD_OBJECT)->next_ABD_OBJECT = obj;
    else
        //RHS HEAD OF THE LIST
        //need to assign the new head of the registry to it
        objectsRegistry = obj;   
    
    //set node_RHS->prev = obj
    node_RHS->prev_ABD_OBJECT = obj;

    //set obj->next = node_RHS
    obj->next_ABD_OBJECT = node_RHS;
}

void rankObjectByUsages(ABD_OBJECT * obj){
    //this function searches for a space in the list where
    //the node before has more usages or is NULL (HEAD)
    //and the node after has less usages than the node being ranked.
    // Before doing the swap it is needed to save the references that the node stores (changeNeighbours).
    //This way the list can maintain coerence
    ABD_OBJECT * node_RHS = findNode_RHS(obj);
    if(node_RHS == ABD_OBJECT_NOT_FOUND)
        return;
    changeNeighbours(obj);
    doSwap(obj, node_RHS);
}

/*
    When an object modification is caught, it is only needed a call to the method below, 
    everything else is automated

    TODO: this needs to be modified so that the only arguments (in the end) should be:
    SEXP lhs - left hand side of operation (object)
    SEXP rhs - right hand side of operation (values)
    SEXP rho - environment
*/

void objectUsage(SEXP lhs, SEXP rhs, SEXP rho){
    unsigned int instructionNumber = 1;
    unsigned short type = 1;
    int value = REAL(rhs)[0]; 
    char * functionName = "main";
    char * createdEnv = "GlobalEnvironment";
    unsigned short remotion = ABD_ALIVE;
    
    //name extraction from lhs
    int nameSize = strlen(CHAR(lhs));
    char * name = allocMemoryForString(nameSize);
    copyString(name, CHAR(lhs), nameSize);


    ABD_OBJECT * objectFound = objectLookUp(name);
    if(objectFound == ABD_OBJECT_NOT_FOUND){
        //object not found
        //create object
        objectFound = addObjectToRegistry();
        setBaseValuesForNewObject(objectFound, name, type, createdEnv, value);
    }

    //object found, add modification and then set the values
    setValuesForNewModification(objectFound, addModificationToObjectRegistry(objectFound), instructionNumber,functionName, value, remotion);

    //rank it
    rankObjectByUsages(objectFound);
}

void wipeRegistry(){
    //clears registry memory (all nodes)
    //TODO: clear ABD_OBJ_MOD_LIST not done
    
    ABD_OBJECT * currentObject = objectsRegistry;
    int count = 0;
    while(currentObject!=ABD_OBJECT_NOT_FOUND){
        count++;
        ABD_OBJECT * next = currentObject->next_ABD_OBJECT;
        free(currentObject);
        currentObject = next;
        //printf("Cleaned... %d\n", count);
    }
}

void initializeRegistry(){
    //set NULL to initialize
    objectsRegistry = ABD_OBJECT_NOT_FOUND;
}


/*
    Methods to start and stop the tool, helper function also here

    -> start method: 
        # start the information collection
        # Wipe the objectsRegistry

    -> stop method:
        # stop the information collection
        # save the objectsRegistry to a JSON with path specified at
            File: ABD_tool_JSON_HELPS_defn.h 
            Constant: OBJECTS_FILE_PATH
        # wipe all the ABD_OBJECT_MOD list for all the ABD_OBJECT's saved DLL
*/

void START_WATCHER(){
    wipeRegistry();
    watcherActivated = 1;
}

void STOP_WATCHER(){
    persistInformation(PERSIST_OBJECTS, objectsRegistry);
    wipeRegistry();
    watcherActivated = 0;
}
void ABD_HELP(){
    
}
void regVarChange(SEXP lhs, SEXP rhs, SEXP rho){
    if(Rf_isEnvironment(rho) == TRUE){
        printf("env name: %s\n", Rf_EncodeEnvironment(rho));
    }
    
    if(watcherActivated){
        printf("name length: %d\n", strlen(CHAR(lhs)));
        objectUsage(lhs, rhs, rho);  
        basicPrint();  
    }
    
}
void printModifications(ABD_OBJECT * obj){
    ABD_OBJECT_MOD * modList = obj->ABD_OBJECT_MOD_LIST;
    
    while(modList != ABD_OBJECT_NOT_FOUND) {
        printf("Instruction: %d\n", modList->instructionNumber);
        printf("Function Name: %s\n", modList->functionName);
        printf("New Value: %d\n", modList->newValue);
        printf("Remotion? %s\n", (modList->remotion == 1) ? "yes": "no");
        puts("----------------------------");
        modList = modList->nextMod;
    }
    puts("");
    puts("");
}
void basicPrint(){
    int count = 0;
    ABD_OBJECT * currentObject = objectsRegistry;
    while(currentObject!=NULL){
        count++;
        puts("");
        puts("----------------------------");
        printf("name: %s\n", currentObject->name);
        printf("usages: %d\n", currentObject->usages);
        /*printf("prev name: %s\n", (currentObject->prev_ABD_OBJECT != NULL) ? currentObject->prev_ABD_OBJECT->name : "NULL");
        printf("next name: %s\n", (currentObject->next_ABD_OBJECT != NULL) ? currentObject->next_ABD_OBJECT->name : "NULL");*/
        printf("createdEnv: %s\n", currentObject->createdEnv);
        printf("Removed? %s\n", (currentObject->removed==ABD_DELETED) ? "yes": "no" );
        printf("Has modifications? %s\n", (currentObject->ABD_OBJECT_MOD_LIST != NULL) ? "yes" : "no");
        printf("Am i the head of the objects? %s (previous name %s)\n", (currentObject->prev_ABD_OBJECT == NULL) ? "yes" : "no", 
                            (currentObject->prev_ABD_OBJECT != NULL) ? currentObject->prev_ABD_OBJECT->name : currentObject->name);
        puts("#############################");
        puts("\t\tModifications");
        if(currentObject->ABD_OBJECT_MOD_LIST!=ABD_OBJECT_NOT_FOUND)
            printModifications(currentObject);
        puts("#############################");
        printf("Exist more objects? %s\n", (currentObject->next_ABD_OBJECT != NULL) ? "yes" : "no");
        puts("----------------------------");
        currentObject = currentObject->next_ABD_OBJECT;

    }
    //printf("\nObjects printed -> %d\n", count);
    puts("");
    puts("");
}


