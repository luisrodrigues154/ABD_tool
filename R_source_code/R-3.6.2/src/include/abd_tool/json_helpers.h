#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/json_helpers_defn.h>

FILE * openFile(char * filePath){
    return fopen(filePath, FILE_OPEN_MODE);
}

int closeFile(FILE * outputFile){
    return fclose(outputFile);
}

void initializeJSONstructure(FILE * outputFile, char * indentation){
    fprintf(outputFile, "%s[\n", indentation);
}
void finalizeJSONstructure(FILE * outputFile, char * indentation){
    fprintf(outputFile, "%s]\n", indentation);
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
    
    currentModification = obj->modList;
    
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
    finalizeJSONstructure(outputFile, JSON_INDENT_3);
    
}

void persistObjects(FILE * outputFile, ABD_OBJECT * objectsRegistry){
    if(objectsRegistry == ABD_OBJECT_NOT_FOUND)
        return;
    
    ABD_OBJECT * currentObject = objectsRegistry;

    do{
        fprintf(outputFile, "%s{\n", JSON_INDENT_1);
        writeObjectToFile(outputFile, currentObject);
        
        currentObject = currentObject->nextObj;

        if(currentObject != ABD_OBJECT_NOT_FOUND)
            fprintf(outputFile, "%s},\n", JSON_INDENT_1);
        else
            fprintf(outputFile, "%s}\n", JSON_INDENT_1);
    }while(currentObject != ABD_OBJECT_NOT_FOUND);
}


void persistInformation(int structureToStore, ABD_OBJECT * objectsRegistry){
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
            initializeJSONstructure(outputFile, JSON_INDENT_0);
            persistObjects(outputFile, objectsRegistry);
            finalizeJSONstructure(outputFile, JSON_INDENT_0);
            break;
        case PERSIST_CODE_FLOW:
            break;
        default:
            break;
    }
    closeFile(outputFile);
    
}
