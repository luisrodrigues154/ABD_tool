#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/json_helpers_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/settings_manager_defn.h>

FILE * openFile(char * filePath){
    return fopen(filePath, FILE_OPEN_MODE);
}

int closeFile(FILE * outputFile){
    return fclose(outputFile);
}

char * getStrFromIndent(JSON_INDENT indent){
    if(indent == INDENT_0)
        return "";
    char * ret = (char *) malloc((indent+1)*sizeof(char));
    ret[0]='\0';
    for(int i=0; i<indent; i++)
        strncat(ret, "\t", 1);
    ret[indent] = '\0';
    return ret;
}

void writeRealVector(FILE * out, ABD_VEC_OBJ * vecObj, int id){
    fprintf(out, "\n%s\"id\" : %d,", getStrFromIndent(INDENT_5), id);

    if(vecObj->idxChange == 'Y'){
        fprintf(out, "\n%s\"vecMod\" : true,", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"numMods\" : %d,", getStrFromIndent(INDENT_5), vecObj->nCols);
        fprintf(out, "\n%s\"mods\" : [", getStrFromIndent(INDENT_5));
        
        //nCols represent number of modifications
        for(int i =0; i<vecObj->nCols; i++){
            fprintf(out, "\n%s{ \"index\" : %d, ", getStrFromIndent(INDENT_6), vecObj->idxs[i]);
            fprintf(out, "\"newValue\" : %.2f }", ((double *) vecObj->vector)[i]);
            if(i+1 != vecObj->nCols)
                fprintf(out, ",");
        }
        fprintf(out, "\n%s]", getStrFromIndent(INDENT_5));
    }else{
        fprintf(out, "\n%s\"vecMod\" : false,", getStrFromIndent(INDENT_5));
        if(vecObj->nCols == 1){
        fprintf(out, "\n%s\"value\" : %.2f", getStrFromIndent(INDENT_5), ((double *) vecObj->vector)[0]);
        }else{
            fprintf(out, "\n%s\"nElements\" : %d,", getStrFromIndent(INDENT_5), vecObj->nCols);
            fprintf(out, "\n%s\"vector\" : [", getStrFromIndent(INDENT_5));
            for(int i=0; i<vecObj->nCols; i++)
                fprintf(out, "%.2f%s", ((double *) vecObj->vector)[i], (i+1==vecObj->nCols) ? "" : ",");
            fprintf(out, "]");
        }
    }
}

void writeObjModsToFile(FILE * out, ABD_OBJECT_MOD * listStart){
    ABD_OBJECT_MOD * currMod = listStart;
    do{
        fprintf(out, "\n%s{", getStrFromIndent(INDENT_4));
        switch(currMod->type){
            case REALSXP:
                //vector can be size 1 (single number)
                writeRealVector(out, currMod->value.vec_value, currMod->id);
                break;
            case STRSXP:
                break;
        }
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));
        currMod = currMod->nextMod;
        if(currMod != ABD_NOT_FOUND)
            fprintf(out, ",");
    }while(currMod != ABD_NOT_FOUND);
}

void writeObjToFile(FILE * out, ABD_OBJECT * obj){
    //write first start
    fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_2), obj->id);
    fprintf(out, "\n%s\"name\" : \"%s\",",getStrFromIndent(INDENT_3), obj->name);
    fprintf(out, "\n%s\"createdEnv\" : \"%s\",",getStrFromIndent(INDENT_3), environmentExtraction(obj->createdEnv));
    fprintf(out, "\n%s\"usages\" : %d",getStrFromIndent(INDENT_3), obj->usages);
    if(obj->modListStart != ABD_NOT_FOUND){
        fprintf(out, ",");
        fprintf(out, "\n%s\"modList\" : [",getStrFromIndent(INDENT_3));
        writeObjModsToFile(out, obj->modListStart);
        fprintf(out, "\n%s]",getStrFromIndent(INDENT_3));
    }
    fprintf(out, "\n%s}",getStrFromIndent(INDENT_2));
}

void saveObjects(FILE * out){
    ABD_OBJECT * currObj = cmnObjReg;
    fprintf(out, "{");
    if(cmnObjReg != ABD_NOT_FOUND){
        //common objects
        fprintf(out, "\n%s\"commonObj\" : {", getStrFromIndent(INDENT_1));
        do{
            writeObjToFile(out, currObj);
            currObj = currObj->nextObj;
            
            if(currObj != ABD_OBJECT_NOT_FOUND)
                fprintf(out, ",");
        }while(currObj != ABD_OBJECT_NOT_FOUND);
        fprintf(out, "\n%s}",getStrFromIndent(INDENT_1));
    }    
    if(cfObjReg!= ABD_NOT_FOUND){
        if(cmnObjReg != ABD_NOT_FOUND)
            fprintf(out, ",");
        currObj = cfObjReg;
        //code flow objects
        fprintf(out, "\n%s\"codeFlowObj\" : {", getStrFromIndent(INDENT_1));
        do{
            writeObjToFile(out, currObj);
            currObj = currObj->nextObj;
            
            if(currObj != ABD_OBJECT_NOT_FOUND)
                fprintf(out, ",");
        }while(currObj != ABD_OBJECT_NOT_FOUND);
        fprintf(out, "\n%s}",getStrFromIndent(INDENT_1));
    }

    fprintf(out, "\n}");
}

//EVENTS HELPERS
void writeArgVector(FILE * out, ABD_VEC_OBJ * vecObj, JSON_INDENT indent){
    if(vecObj->nCols == 1){
        fprintf(out, "\n%s\"value\" : %.2f", getStrFromIndent(indent), ((double *) vecObj->vector)[0]);
    }else{
        fprintf(out, "\n%s\"value\" : [", getStrFromIndent(indent));
        for(int i=0; i<vecObj->nCols; i++)
            fprintf(out, "%.2f%s", ((double *) vecObj->vector)[i], (i+1==vecObj->nCols) ? "" : ",");
        fprintf(out, "]");
    }
}
void writeArgValueToFile(FILE * out, ABD_OBJECT_MOD * value, JSON_INDENT indent){
    switch(value->type){
        case REALSXP:
            //vector can be size 1 (single number)
            writeArgVector(out, value->value.vec_value, indent);
            break;
        case STRSXP:
            break;
        case NILSXP:
            fprintf(out, "\n%s\"value\" : \"NoReturn\"", getStrFromIndent(indent));
            break;
        default:
            fprintf(out, "\n%s\"value\" : \"unknownValue\"", getStrFromIndent(indent));
            break;
    }
}

void saveFuncArgs(FILE * out, ABD_EVENT_ARG * argsList){
    ABD_EVENT_ARG * currArg = argsList;   
    do{
        
        fprintf(out, "\n%s{", getStrFromIndent(INDENT_4));
        fprintf(out, "\n%s\"type\" : ",getStrFromIndent(INDENT_5));
        if(currArg->objPtr->id == -1){
            //hardcoded value
            fprintf(out, "\"HC\",");
            fprintf(out, "\n%s\"passedAs\" : \"NA\",", getStrFromIndent(INDENT_5));
            fprintf(out, "\n%s\"receivedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->rcvdName);
            writeArgValueToFile(out, currArg->objValue, INDENT_5);

        }else if(currArg->objPtr->id == -2){
            //object not in registry
            fprintf(out, "\"R\",");
            fprintf(out, "\n%s\"passedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->objPtr->name);
            fprintf(out, "\n%s\"receivedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->rcvdName);
            writeArgValueToFile(out, currArg->objValue, INDENT_5);
        }else{
            //object in registry
            fprintf(out, "\"ABD\",");
            fprintf(out, "\n%s\"objId\" : \"%d\",", getStrFromIndent(INDENT_5), currArg->objPtr->id);
            fprintf(out, "\n%s\"receivedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->rcvdName);
            fprintf(out, "\n%s\"valueId\" : \"%d\"", getStrFromIndent(INDENT_5), currArg->objValue->id);
        }
    
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));
        currArg = currArg->nextArg;

        if(currArg != ABD_NOT_FOUND)
            fprintf(out, ",");

    }while(currArg != ABD_NOT_FOUND);
}

void saveFuncEvent(FILE * out, ABD_FUNC_EVENT * funcEvent){
    fprintf(out, "\n%s\"callerId\" : %d,", getStrFromIndent(INDENT_3), (funcEvent->caller == ABD_OBJECT_NOT_FOUND) ? 0: funcEvent->caller->id);
    fprintf(out, "\n%s\"calledId\" : %d,", getStrFromIndent(INDENT_3), funcEvent->called->id);
    fprintf(out, "\n%s\"args\" : [", getStrFromIndent(INDENT_3));
    if(funcEvent->args == ABD_NOT_FOUND)
        fprintf(out, "]");
    else{
        saveFuncArgs(out, funcEvent->args);
        fprintf(out, "\n%s]", getStrFromIndent(INDENT_3));
    }
}

void saveRetEvent(FILE * out, ABD_RET_EVENT * retEvent){
    fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_3), retEvent->from->id);
    fprintf(out, "\n%s\"toId\" : %d,", getStrFromIndent(INDENT_3),(retEvent->toObj == ABD_OBJECT_NOT_FOUND) ? -1 :retEvent->toObj->id);
    if(retEvent->toObj != ABD_OBJECT_NOT_FOUND)
        // returned to an object
        fprintf(out, "\n%s\"valueId\" : %d", getStrFromIndent(INDENT_3),retEvent->retValue->id);
    else
        // return not assigned
        writeArgValueToFile(out, retEvent->retValue, INDENT_3);

}

void saveEvents(FILE * out){
    if(eventsReg == ABD_EVENT_NOT_FOUND)
        //mostlikely will never happen, 
        //because it is initialized always with MAIN_EVENT
        return;
    if(eventsRegTail->type == MAIN_EVENT || eventsReg->nextEvent == ABD_EVENT_NOT_FOUND)
        //has no events
        return;

    ABD_EVENT * currEvent = eventsReg->nextEvent;
    fprintf(out, "[");
    
    do{
        fprintf(out, "\n%s{", getStrFromIndent(INDENT_1));
        fprintf(out, "\n%s\"type\" : ", getStrFromIndent(INDENT_2));
        switch(currEvent->type){
            case IF_EVENT:
                break;
            case FUNC_EVENT:
                fprintf(out, "\"func_event\",");
                fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));
                saveFuncEvent(out, currEvent->data.func_event);
                break;
            case RET_EVENT: 
                fprintf(out, "\"ret_event\",");
                fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));
                saveRetEvent(out, currEvent->data.ret_event);
                break;

            default : break;
        }
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_2));
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_1));

        currEvent = currEvent->nextEvent;
        if(currEvent!=ABD_EVENT_NOT_FOUND)
            fprintf(out, ",");

    }while(currEvent != ABD_EVENT_NOT_FOUND);

    fprintf(out, "\n]");
}

void persistInformation(){
    FILE * outputFile ;

    
    outputFile = openFile(getObjPath());
    if(outputFile == NULL){
        //unable to open file
        //check destination path
        puts("Cannot open objects file... check path");
        return;
    }

    //save both registries
    saveObjects(outputFile);

    //close objects file
    closeFile(outputFile);

    //open events file
    outputFile = openFile(getEventsPath());
    if(outputFile == NULL){
        //unable to open file
        //check destination path
        puts("Cannot open events file... check path");
        return;
    }

    //save events
    saveEvents(outputFile);

    //close events file
    closeFile(outputFile);
    
}
