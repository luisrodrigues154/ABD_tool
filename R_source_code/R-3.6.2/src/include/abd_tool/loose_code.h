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
