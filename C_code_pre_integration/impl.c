#include <stdio.h>
#include <stdlib.h>
#include <string.h>





#define ABD_DELETED 1
#define ABD_ALIVE 0
#define ABD_OBJECT_NOT_FOUND NULL


void basicPrint();



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
