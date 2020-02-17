#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define JSON_INDENT_1 "\t"
#define JSON_INDENT_2 "\t\t"
#define JSON_INDENT_3 "\t\t\t"
#define JSON_INDENT_4 "\t\t\t\t"
#define JSON_INDENT_5 "\t\t\t\t\t"

#define PERSIST_OBJECTS 1
#define PERSIST_CODE_FLOW 2
#define OBJECTS_FILE_PATH "/home/luis/Desktop/objects.json"
#define FILE_OPEN_MODE "w+"
#define ABD_DELETED 1
#define ABD_ALIVE 0
#define ABD_OBJECT_NOT_FOUND NULL


void basicPrint();

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

ABD_OBJECT * objectsRegistry;

//write to file helpers (TO JSON FORMAT)

FILE * openFile(char * filePath){
    return fopen(filePath, FILE_OPEN_MODE);
}

int closeFile(FILE * outputFile){
    return fclose(outputFile);
}

void initializeJSONstructure(FILE * outputFile){
    fprintf(outputFile, "[\n" );
}
void finalizeJSONstructure(FILE * outputFile){
    fprintf(outputFile, "]\n" );
}

void writeObjectModificationToFile(FILE * outputFile, ABD_OBJECT_MOD * modification){
    fprintf(outputFile, "%s\"instructionNumber\" : \"%d\",\n", JSON_INDENT_5, modification->instructionNumber);
    fprintf(outputFile, "%s\"function\" : \"%s\",\n", JSON_INDENT_5, modification->functionName);
    fprintf(outputFile, "%s\"newValue\" : \"%d\",\n", JSON_INDENT_5, modification->newValue);
    fprintf(outputFile, "%s\"remotion\" : %s\n", JSON_INDENT_5, (modification->remotion == 1) ? "true": "false");
}

void writeObjectToFile(FILE * outputFile, ABD_OBJECT * obj){
    ABD_OBJECT_MOD * currentModification = ABD_OBJECT_NOT_FOUND;

    fprintf(outputFile, "%s\"type\" : \"%d\",\n", JSON_INDENT_2, obj->type);
    fprintf(outputFile, "%s\"name\" : \"%s\",\n", JSON_INDENT_2 ,obj->name);
    fprintf(outputFile, "%s\"createdEnv\" : \"%s\",\n", JSON_INDENT_2, obj->createdEnv);
    //start modifications JSON array
    fprintf(outputFile, "%s\"modList\" : [\n", JSON_INDENT_2);
    //start json object
    
    currentModification = obj->ABD_OBJECT_MOD_LIST;
    
    do{
        fprintf(outputFile, "%s{\n", JSON_INDENT_4);
        writeObjectModificationToFile(outputFile, currentModification);
        currentModification = currentModification->nextMod;
        if(currentModification!= ABD_OBJECT_NOT_FOUND)
            //object has more modifications
            fprintf(outputFile, "%s},\n", JSON_INDENT_4);
        else
            fprintf(outputFile, "%s}\n", JSON_INDENT_4);
    }while(currentModification!= ABD_OBJECT_NOT_FOUND);


    
    //closes modifications JSON array
    fprintf(outputFile, "%s]\n", JSON_INDENT_3);
    
}

void persistObjects(FILE * outputFile){
    if(objectsRegistry == ABD_OBJECT_NOT_FOUND)
        return;
    
    ABD_OBJECT * currentObject = objectsRegistry;

    do{
        fprintf(outputFile, "%s{\n", JSON_INDENT_1);
        writeObjectToFile(outputFile, currentObject);
        
        currentObject = currentObject->next_ABD_OBJECT;

        if(currentObject != ABD_OBJECT_NOT_FOUND)
            fprintf(outputFile, "%s},\n", JSON_INDENT_1);
        else
            fprintf(outputFile, "%s}\n", JSON_INDENT_1);
    }while(currentObject != ABD_OBJECT_NOT_FOUND);
}


void persistInformation(int structureToStore){
    FILE * outputFile ;
    switch (structureToStore)
    {
        case PERSIST_OBJECTS:
            outputFile = openFile(OBJECTS_FILE_PATH);
            if(outputFile == NULL){
                //unable to open file
                //check destination path
                puts("Cannot open file");
                return;
            }
            initializeJSONstructure(outputFile);
            persistObjects(outputFile);
            finalizeJSONstructure(outputFile);
            break;
        case PERSIST_CODE_FLOW:
            break;
        default:
            break;
    }
    closeFile(outputFile);
    
}

//object storage implementation

ABD_OBJECT * allocMemoryForObject(){
    return (ABD_OBJECT *) malloc(sizeof(ABD_OBJECT));
}
ABD_OBJECT_MOD * allocMemoryForObjectModification(){
    return (ABD_OBJECT_MOD *) malloc(sizeof(ABD_OBJECT_MOD));
}

void setBaseValuesForNewObject(ABD_OBJECT * obj, char * name, unsigned short type, char * createdEnv, int value){
    obj->type = type;
    obj->name = (char *) malloc(strlen(name)*sizeof(char));
    strncpy(obj->name, name, strlen(name)*sizeof(char));
    obj->usages = 0;
    obj->removed = 0;
    obj->createdEnv = (char *) malloc(strlen(createdEnv)*sizeof(char));
    strncpy(obj->createdEnv, createdEnv, strlen(createdEnv)*sizeof(char));
    obj->ABD_OBJECT_MOD_LIST = ABD_OBJECT_NOT_FOUND;
}

void setValuesForNewModification(ABD_OBJECT * obj, ABD_OBJECT_MOD * newModification, int instructionNumber, char * functionName, int newValue, unsigned short remotion){
    //increment usages
    obj->usages++;
    newModification->functionName = (char *) malloc(strlen(functionName)*sizeof(char));
    strncpy(newModification->functionName, functionName, strlen(functionName)*sizeof(char));
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
    // Before doing the swap it is needed to save the references that the node stores.
    //This way the list can maintain coerence
    ABD_OBJECT * node_RHS = findNode_RHS(obj);
    if(node_RHS == ABD_OBJECT_NOT_FOUND)
        return;
    changeNeighbours(obj);
    doSwap(obj, node_RHS);
}

void objectUsage(unsigned int instructionNumber, unsigned short type, char * name, int value, char * functionName, char * createdEnv, unsigned short remotion){
    ABD_OBJECT * objectFound = objectLookUp(name);
    if(objectFound == ABD_OBJECT_NOT_FOUND){
        //object not found
        //create object
        objectFound = addObjectToRegistry();
        setBaseValuesForNewObject(objectFound, name, type, createdEnv, value);
    }

    //object found, add modification and then set the values
    setValuesForNewModification(objectFound, addModificationToObjectRegistry(objectFound), instructionNumber,functionName, value, remotion);

    //TODO
    //depois de fazer set, aplicar algoritmo de swap
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



void main(){
    initializeRegistry();
    
    char name[] = "variable1";
    char name2[] = "variable2";
    char name3[] = "variable3";
    
    objectUsage(1, 1, name, 10, "Main", "GlobalEnv", 0);
    objectUsage(1, 1, name, 20, "Main", "GlobalEnv", 0);
    objectUsage(1, 1, name, 30, "Main", "GlobalEnv", 0);

    objectUsage(2, 1, name2, 20, "Main", "GlobalEnv", 0);

    objectUsage(3, 1, name3, 30, "Main", "GlobalEnv", 0);
    objectUsage(3, 1, name3, 40, "Main", "GlobalEnv", 0);
    objectUsage(3, 1, name3, 40, "Main", "GlobalEnv", 0);
    objectUsage(3, 1, name3, 40, "Main", "GlobalEnv", 0);

    //final order 3 1 2
    basicPrint();

    puts("To store information press any");
    getchar();

    persistInformation(PERSIST_OBJECTS);

    wipeRegistry();
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
        printf("prev name: %s\n", (currentObject->prev_ABD_OBJECT != NULL) ? currentObject->prev_ABD_OBJECT->name : "NULL");
        printf("next name: %s\n", (currentObject->next_ABD_OBJECT != NULL) ? currentObject->next_ABD_OBJECT->name : "NULL");
        puts("----------------------------");
        /*if(currentObject->ABD_OBJECT_MOD_LIST!=ABD_OBJECT_NOT_FOUND)
            //printModifications(currentObject);
        printf("type: %d\n", obj->type);
        
        printf("nMods: %d\n", obj->nMods);
        printf("createdEnv: %s\n", obj->createdEnv);
        printf("Removed? %s\n", (obj->removed==ABD_DELETED) ? "yes": "no" );
        printf("Has modifications? %s\n", (obj->ABD_OBJECT_MOD_LIST != NULL) ? "yes" : "no");
        printf("Am i the head of the objects? %s (previous name %s)\n", (obj->prev_ABD_OBJECT == NULL) ? "yes" : "no", 
        (obj->prev_ABD_OBJECT != NULL) ? obj->prev_ABD_OBJECT->name : obj->name);
        printf("Exist more objects? %s\n", (obj->next_ABD_OBJECT != NULL) ? "yes" : "no");*/

        currentObject = currentObject->next_ABD_OBJECT;

    }
    //printf("\nObjects printed -> %d\n", count);
    puts("");
    puts("");
}
