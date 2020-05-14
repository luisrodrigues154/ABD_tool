#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/json_helpers_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/settings_manager_defn.h>
#include <unistd.h>

FILE *openFile(char *filePath)
{
    return fopen(filePath, FILE_OPEN_MODE);
}

int closeFile(FILE *outputFile)
{
    return fclose(outputFile);
}

char *getStrFromIndent(JSON_INDENT indent)
{
    if (indent == INDENT_0)
        return "";
    char *ret = (char *)malloc((indent + 1) * sizeof(char));
    ret[0] = '\0';
    for (int i = 0; i < indent; i++)
        strncat(ret, "\t", 1);
    ret[indent] = '\0';
    return ret;
}

void writeVector(FILE *out, ABD_VEC_OBJ *vecObj, FILE *dispOut)
{
    SEXPTYPE type = vecObj->type;
    if (vecObj->idxChange)
    {
        fprintf(out, "\n%s\"vecMod\" : true,", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"numMods\" : %d,", getStrFromIndent(INDENT_5), vecObj->nCols);
        fprintf(out, "\n%s\"mods\" : [", getStrFromIndent(INDENT_5));

        fprintf(dispOut, "\"vecMod\" : true,");
        fprintf(dispOut, "\"numMods\" : %d,", vecObj->nCols);
        fprintf(dispOut, "\"mods\" : [");

        //nCols represent number of modifications
        for (int i = 0; i < vecObj->nCols; i++)
        {
            fprintf(out, "\n%s{ \"index\" : %d, ", getStrFromIndent(INDENT_6), vecObj->idxs[i]);
            fprintf(dispOut, "{ \"index\" : %d,", vecObj->idxs[i]);
            switch (type)
            {
            case REALSXP:
                fprintf(out, "\"newValue\" : %.2f }", ((double *)vecObj->vector)[i]);
                fprintf(dispOut, "\"newValue\" : %.2f }", ((double *)vecObj->vector)[i]);
                break;
            case INTSXP:
                fprintf(out, "\"newValue\" : %d }", ((int *)vecObj->vector)[i]);
                fprintf(dispOut, "\"newValue\" : %d }", ((int *)vecObj->vector)[i]);
                break;
            default:
                break;
            }

            if (i + 1 != vecObj->nCols)
                fprintf(out, ",");
        }
        fprintf(out, "\n%s]", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "]");
    }
    else
    {
        fprintf(out, "\n%s\"vecMod\" : false,", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"vecMod\" : false,");
        if (vecObj->nCols == 1)
        {
            fprintf(out, "\n%s\"value\" : %.2f", getStrFromIndent(INDENT_5), ((double *)vecObj->vector)[0]);
            fprintf(dispOut, "\"value\" : %.2f", ((double *)vecObj->vector)[0]);
        }
        else
        {
            fprintf(out, "\n%s\"nElements\" : %d,", getStrFromIndent(INDENT_5), vecObj->nCols);
            fprintf(out, "\n%s\"vector\" : [", getStrFromIndent(INDENT_5));

            fprintf(dispOut, "\"nElements\" : %d,", vecObj->nCols);
            fprintf(dispOut, "\"vector\" : [");

            for (int i = 0; i < vecObj->nCols; i++)
            {
                switch (type)
                {
                case REALSXP:
                    fprintf(out, "%.2f%s", ((double *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
                    fprintf(dispOut, "%.2f%s", ((double *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
                    break;
                case INTSXP:
                    fprintf(out, "%d%s", ((int *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
                    fprintf(dispOut, "%d%s", ((int *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
                    break;
                default:
                    break;
                }
            }

            fprintf(out, "]");
            fprintf(dispOut, "]");
        }
    }
}

void writeObjModsToFile(FILE *out, ABD_OBJECT_MOD *listStart, FILE *dispOut)
{
    ABD_OBJECT_MOD *currMod = listStart;
    do
    {
        fprintf(out, "\n%s{", getStrFromIndent(INDENT_4));
        fprintf(out, "\n%s\"id\" : %d,", getStrFromIndent(INDENT_4), currMod->id);

        fprintf(dispOut, "{");
        fprintf(dispOut, "\"id\" : %d,", currMod->id);

        switch (currMod->valueType)
        {
        case ABD_VECTOR:
            writeVector(out, currMod->value.vec_value, dispOut);
            break;
        case ABD_MATRIX:
            break;
        default:
            break;
        }

        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));
        fprintf(dispOut, "}");

        currMod = currMod->nextMod;
        if (currMod != ABD_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    } while (currMod != ABD_NOT_FOUND);
}

void writeObjToFile(FILE *out, ABD_OBJECT *obj, FILE *dispOut)
{
    //write first start
    fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_2), obj->id);
    fprintf(out, "\n%s\"name\" : \"%s\",", getStrFromIndent(INDENT_3), obj->name);
    fprintf(out, "\n%s\"createdAt\" : \"%s\",", getStrFromIndent(INDENT_3), obj->createdAt);
    fprintf(out, "\n%s\"env\" : \"%s\",", getStrFromIndent(INDENT_3), envToStr(obj->createdEnv));
    fprintf(out, "\n%s\"usages\" : %d", getStrFromIndent(INDENT_3), obj->usages);

    fprintf(dispOut, "\"%d\" : {", obj->id);
    fprintf(dispOut, "\"name\" : \"%s\",", obj->name);
    fprintf(dispOut, "\"createdAt\" : \"%s\",", obj->createdAt);
    fprintf(dispOut, "\"env\" : \"%s\",", envToStr(obj->createdEnv));
    fprintf(dispOut, "\"usages\" : %d", obj->usages);

    if (obj->modListStart != ABD_NOT_FOUND)
    {
        fprintf(out, ",");
        fprintf(out, "\n%s\"modList\" : [", getStrFromIndent(INDENT_3));

        fprintf(dispOut, ",");
        fprintf(dispOut, "\"modList\" : [");

        writeObjModsToFile(out, obj->modListStart, dispOut);

        fprintf(out, "\n%s]", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "]");
    }

    fprintf(out, "\n%s}", getStrFromIndent(INDENT_2));
    fprintf(dispOut, "}");
}

void saveObjects(FILE *out, FILE *dispOut)
{
    ABD_OBJECT *currObj = cmnObjReg;
    fprintf(out, "{");
    fprintf(dispOut, "var objects=JSON.parse('{");
    if (cmnObjReg != ABD_NOT_FOUND)
    {
        //common objects
        fprintf(out, "\n%s\"commonObj\" : {", getStrFromIndent(INDENT_1));
        fprintf(dispOut, "\"commonObj\" : {");
        do
        {
            writeObjToFile(out, currObj, dispOut);
            currObj = currObj->nextObj;
            if (currObj != ABD_OBJECT_NOT_FOUND)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        } while (currObj != ABD_OBJECT_NOT_FOUND);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_1));
        fprintf(dispOut, "}");
    }
    if (cfObjReg != ABD_NOT_FOUND)
    {
        if (cmnObjReg != ABD_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
        currObj = cfObjReg;
        //code flow objects
        fprintf(out, "\n%s\"codeFlowObj\" : {", getStrFromIndent(INDENT_1));
        fprintf(dispOut, "\"codeFlowObj\" : {");
        do
        {
            writeObjToFile(out, currObj, dispOut);
            currObj = currObj->nextObj;

            if (currObj != ABD_OBJECT_NOT_FOUND)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        } while (currObj != ABD_OBJECT_NOT_FOUND);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_1));
        fprintf(dispOut, "}");
    }

    fprintf(out, "\n}");
    fprintf(dispOut, "}')");
}

//EVENTS HELPERS
void writeArgRealVector(FILE *out, ABD_VEC_OBJ *vecObj, JSON_INDENT indent, FILE *dispOut)
{
    if (vecObj->nCols == 1)
    {
        fprintf(out, "\n%s\"value\" : %.2f", getStrFromIndent(indent), ((double *)vecObj->vector)[0]);
        fprintf(dispOut, "\"value\" : %.2f", ((double *)vecObj->vector)[0]);
    }
    else
    {
        fprintf(out, "\n%s\"value\" : [", getStrFromIndent(indent));
        fprintf(dispOut, "\"value\" : [");
        for (int i = 0; i < vecObj->nCols; i++)
        {
            fprintf(out, "%.2f%s", ((double *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
            fprintf(dispOut, "%.2f%s", ((double *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
        }
        fprintf(out, "]");
        fprintf(dispOut, "]");
    }
}
void writeArgIntVector(FILE *out, ABD_VEC_OBJ *vecObj, JSON_INDENT indent, FILE *dispOut)
{
    if (vecObj->nCols == 1)
    {
        fprintf(out, "\n%s\"value\" : %d", getStrFromIndent(indent), ((int *)vecObj->vector)[0]);
        fprintf(dispOut, "\"value\" : %d", ((int *)vecObj->vector)[0]);
    }
    else
    {
        fprintf(out, "\n%s\"value\" : [", getStrFromIndent(indent));
        fprintf(dispOut, "\"value\" : [");
        for (int i = 0; i < vecObj->nCols; i++)
        {
            fprintf(out, "%d%s", ((int *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
            fprintf(dispOut, "%d%s", ((int *)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
        }
        fprintf(out, "]");
        fprintf(dispOut, "]");
    }
}

void writeArgVectorForType(FILE *out, ABD_VEC_OBJ *vecObj, JSON_INDENT indent, FILE *dispOut)
{
    switch (vecObj->type)
    {
    case INTSXP:
        //vector can be size 1 (single number)
        writeArgIntVector(out, vecObj, indent, dispOut);
        break;
    case REALSXP:
        //vector can be size 1 (single number)
        writeArgRealVector(out, vecObj, indent, dispOut);
        break;
    case STRSXP:
        break;
    case NILSXP:
        fprintf(out, "\n%s\"value\" : \"NoReturn\"", getStrFromIndent(indent));
        fprintf(dispOut, "\"value\" : \"NoReturn\"");
        break;
    default:
        fprintf(out, "\n%s\"value\" : \"unknownValue\"", getStrFromIndent(indent));
        fprintf(dispOut, "\"value\" : \"unknownValue\"");
        break;
    }
}
void writeArgValueToFile(FILE *out, ABD_OBJECT_MOD *value, JSON_INDENT indent, FILE *dispOut)
{
    switch (value->valueType)
    {
    case ABD_VECTOR:
        writeArgVectorForType(out, value->value.vec_value, indent, dispOut);
        break;
    case ABD_MATRIX:
        break;

    default:
        break;
    }
}

void saveFuncArgs(FILE *out, ABD_EVENT_ARG *argsList, FILE *dispOut)
{
    ABD_EVENT_ARG *currArg = argsList;
    do
    {

        fprintf(out, "\n%s{", getStrFromIndent(INDENT_4));
        fprintf(out, "\n%s\"type\" : ", getStrFromIndent(INDENT_5));

        fprintf(dispOut, "{");
        fprintf(dispOut, "\"type\" : ");
        if (currArg->objPtr->id == -1)
        {
            //hardcoded value
            fprintf(out, "\"HC\",");
            fprintf(out, "\n%s\"passedAs\" : \"NA\",", getStrFromIndent(INDENT_5));
            fprintf(out, "\n%s\"receivedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->rcvdName);

            fprintf(dispOut, "\"HC\",");
            fprintf(dispOut, "\"passedAs\" : \"NA\",");
            fprintf(dispOut, "\"receivedAs\" : \"%s\",", currArg->rcvdName);
            writeArgValueToFile(out, currArg->objValue, INDENT_5, dispOut);
        }
        else if (currArg->objPtr->id == -2)
        {
            //object not in registry
            fprintf(out, "\"R\",");
            fprintf(out, "\n%s\"passedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->objPtr->name);
            fprintf(out, "\n%s\"receivedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->rcvdName);

            fprintf(dispOut, "\"R\",");
            fprintf(dispOut, "\"passedAs\" : \"%s\",", currArg->objPtr->name);
            fprintf(dispOut, "\"receivedAs\" : \"%s\",", currArg->rcvdName);
            writeArgValueToFile(out, currArg->objValue, INDENT_5, dispOut);
        }
        else
        {
            //object in registry
            fprintf(out, "\"ABD\",");
            fprintf(out, "\n%s\"objId\" : \"%d\",", getStrFromIndent(INDENT_5), currArg->objPtr->id);
            fprintf(out, "\n%s\"receivedAs\" : \"%s\",", getStrFromIndent(INDENT_5), currArg->rcvdName);
            fprintf(out, "\n%s\"valueId\" : \"%d\"", getStrFromIndent(INDENT_5), currArg->objValue->id);

            fprintf(dispOut, "\"ABD\",");
            fprintf(dispOut, "\"objId\" : \"%d\",", currArg->objPtr->id);
            fprintf(dispOut, "\"receivedAs\" : \"%s\",", currArg->rcvdName);
            fprintf(dispOut, "\"valueId\" : \"%d\"", currArg->objValue->id);
        }

        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));
        fprintf(dispOut, "}");
        currArg = currArg->nextArg;

        if (currArg != ABD_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }

    } while (currArg != ABD_NOT_FOUND);
}
void saveAssignEvent(FILE *out, ABD_ASSIGN_EVENT *event, FILE *dispOut)
{
    fprintf(out, "\n%s\"toObj\" : %d,", getStrFromIndent(INDENT_3), event->toObj->id);
    fprintf(out, "\n%s\"origin\" : \"%s\",", getStrFromIndent(INDENT_3), (event->fromType == ABD_E) ? "event" : "obj");
    fprintf(out, "\n%s\"toState\" : %d,", getStrFromIndent(INDENT_3), (event->value != ABD_OBJECT_NOT_FOUND) ? event->value->id : 0);

    fprintf(dispOut, "\"toObj\" : %d,", event->toObj->id);
    fprintf(dispOut, "\"origin\" : \"%s\",", (event->fromType == ABD_E) ? "event" : "obj");
    fprintf(dispOut, "\"toState\" : %d,", (event->value != ABD_OBJECT_NOT_FOUND) ? event->value->id : 0);

    if (event->fromType == ABD_E)
    {
        /* from event */
        fprintf(out, "\n%s\"fromEvent\" : %d", getStrFromIndent(INDENT_3), ((ABD_EVENT *)event->fromObj)->id);
        fprintf(dispOut, "\"fromEvent\" : %d", ((ABD_EVENT *)event->fromObj)->id);
    }
    else
    {
        ABD_OBJECT *obj = ((ABD_OBJECT *)event->fromObj);
        fprintf(out, "\n%s\"fromObj\" : ", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "\"fromObj\" : ");

        if (obj->id == -1)
        {
            //hardcoded value
            fprintf(out, "\"HC\",");
            fprintf(dispOut, "\"HC\",");
        }
        else if (obj->id == -2)
        {
            //object not in registry
            fprintf(out, "\"R\",");
            fprintf(out, "\n%s\"name\" : \"%s\",", getStrFromIndent(INDENT_3), obj->name);

            fprintf(dispOut, "\"R\",");
            fprintf(dispOut, "\"name\" : \"%s\",", obj->name);
        }
        else
        {
            //object in registry
            fprintf(out, "\"ABD\",");
            fprintf(out, "\n%s\"name\" : \"%s\",", getStrFromIndent(INDENT_3), obj->name);

            fprintf(dispOut, "\"ABD\",");
            fprintf(dispOut, "\"name\" : \"%s\",", obj->name);
        }
        fprintf(out, "\n%s\"withIndex\" : %d", getStrFromIndent(INDENT_3), event->withIndex);
        fprintf(dispOut, "\"withIndex\" : %d", event->withIndex);
    }
}
void saveExpression(FILE *out, int id, IF_EXPRESSION *expr, FILE *dispOut)
{

    if (expr->isConfined)
        fprintf(out, "(");

    if (expr->left_type == IF_EXPR)
        saveExpression(out, id, expr->left_data, dispOut);
    else
    {
        ABD_OBJECT *obj = ((IF_ABD_OBJ *)expr->left_data)->objPtr;
        if (obj->id == -1)
        {
            // hardcoded, get value
            ABD_OBJECT_MOD *objValue = ((IF_ABD_OBJ *)expr->left_data)->objValue;
            fprintf(out, "%.4f", ((double *)objValue->value.vec_value->vector)[0]);
        }
        else
            fprintf(out, "%s", getObjStr((IF_ABD_OBJ *)expr->left_data));
    }

    fprintf(out, " %s ", expr->operator);

    if (expr->right_type == IF_EXPR)
        saveExpression(out, id, expr->right_data, dispOut);
    else
    {
        ABD_OBJECT *obj = ((IF_ABD_OBJ *)expr->right_data)->objPtr;
        if (obj->id == -1)
        {
            // hardcoded, get value
            ABD_OBJECT_MOD *objValue = ((IF_ABD_OBJ *)expr->right_data)->objValue;
            fprintf(out, "%.4f", ((double *)objValue->value.vec_value->vector)[0]);
        }
        else
            fprintf(out, "%s", getObjStr((IF_ABD_OBJ *)expr->right_data));
    }

    if (expr->isConfined)
    {
        fprintf(out, ")");
    }
}

void saveArithEvent(FILE *out, ABD_ARITH_EVENT *event, FILE *dispOut)
{
    fprintf(out, "\n%s\"result\" : %.4f,", getStrFromIndent(INDENT_3), event->globalResult);
    fprintf(out, "\n%s\"expressions\" : [", getStrFromIndent(INDENT_3));

    fprintf(dispOut, "\"result\" : %.4f,", event->globalResult);
    fprintf(dispOut, "\"expressions\" : [");

    saveExpression(out, 0, event->expr, dispOut);

    fprintf(out, "\n%s]", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "]");
}
void saveFuncEvent(FILE *out, ABD_FUNC_EVENT *funcEvent, FILE *dispOut)
{
    fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_3), (funcEvent->caller == ABD_OBJECT_NOT_FOUND) ? 0 : funcEvent->caller->id);
    fprintf(out, "\n%s\"toId\" : %d,", getStrFromIndent(INDENT_3), funcEvent->called->id);
    fprintf(out, "\n%s\"toEnv\" : \"%s\",", getStrFromIndent(INDENT_3), envToStr(funcEvent->toEnv));
    fprintf(out, "\n%s\"args\" : [", getStrFromIndent(INDENT_3));

    fprintf(dispOut, "\"fromId\" : %d,", (funcEvent->caller == ABD_OBJECT_NOT_FOUND) ? 0 : funcEvent->caller->id);
    fprintf(dispOut, "\"toId\" : %d,", funcEvent->called->id);
    fprintf(dispOut, "\"toEnv\" : \"%s\",", envToStr(funcEvent->toEnv));
    fprintf(dispOut, "\"args\" : [");

    if (funcEvent->args == ABD_NOT_FOUND)
    {
        fprintf(out, "]");
        fprintf(dispOut, "]");
    }
    else
    {
        saveFuncArgs(out, funcEvent->args, dispOut);
        fprintf(out, "\n%s]", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "]");
    }
}

void saveRetEvent(FILE *out, ABD_RET_EVENT *retEvent, FILE *dispOut)
{
    fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_3), retEvent->from->id);
    fprintf(out, "\n%s\"toId\" : %d,", getStrFromIndent(INDENT_3), (retEvent->toObj == ABD_OBJECT_NOT_FOUND) ? -1 : retEvent->toObj->id);
    fprintf(dispOut, "\"fromId\" : %d,", retEvent->from->id);
    fprintf(dispOut, "\"toId\" : %d,", (retEvent->toObj == ABD_OBJECT_NOT_FOUND) ? -1 : retEvent->toObj->id);
    if (retEvent->toObj != ABD_OBJECT_NOT_FOUND)
    {
        // returned to an object
        fprintf(out, "\n%s\"valueId\" : %d", getStrFromIndent(INDENT_3), retEvent->retValue->id);
        fprintf(dispOut, "\"valueId\" : %d", retEvent->retValue->id);
    }
    else
        // return not assigned
        writeArgValueToFile(out, retEvent->retValue, INDENT_3, dispOut);
}

void saveEvents(FILE *out, FILE *dispOut)
{
    if (eventsReg == ABD_EVENT_NOT_FOUND)
        //mostlikely will never happen,
        //because it is initialized always with MAIN_EVENT
        return;
    if (eventsRegTail->type == MAIN_EVENT && eventsReg->nextEvent == ABD_EVENT_NOT_FOUND)
        //has no events
        return;

    ABD_EVENT *currEvent = eventsReg->nextEvent;
    fprintf(out, "{");
    fprintf(dispOut, "var events=JSON.parse('{");
    do
    {
        //write to the json file
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_1), currEvent->id);
        fprintf(out, "\n%s\"line\" : %d,", getStrFromIndent(INDENT_2), currEvent->scriptLn);
        fprintf(out, "\n%s\"atEnv\" : \"%s\",", getStrFromIndent(INDENT_2), envToStr(currEvent->env));
        fprintf(out, "\n%s\"type\" : ", getStrFromIndent(INDENT_2));

        fprintf(dispOut, "\"%d\" : {", currEvent->id);
        fprintf(dispOut, "\"line\" : %d,", currEvent->scriptLn);
        fprintf(dispOut, "\"atEnv\" : \"%s\",", envToStr(currEvent->env));
        fprintf(dispOut, "\"type\" : ");

        switch (currEvent->type)
        {
        case IF_EVENT:
            break;
        case FUNC_EVENT:
            fprintf(out, "\"func_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"func_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveFuncEvent(out, currEvent->data.func_event, dispOut);
            break;
        case RET_EVENT:
            fprintf(out, "\"ret_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"ret_event\",\\");
            fprintf(dispOut, "\"data\" : {");
            saveRetEvent(out, currEvent->data.ret_event, dispOut);
            break;
        case ASGN_EVENT:
            fprintf(out, "\"assign_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"assign_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveAssignEvent(out, currEvent->data.asgn_event, dispOut);
            break;
        case ARITH_EVENT:
            fprintf(out, "\"arithmetic_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"arithmetic_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveArithEvent(out, currEvent->data.arith_event, dispOut);
            break;
        case VEC_EVENT:
            fprintf(out, "\"vector_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"vector_event\",");
            fprintf(dispOut, "\"data\" : {");
            break;
        default:
            break;
        }
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_2));
        fprintf(dispOut, "}");
        fprintf(out, "}");
        fprintf(dispOut, "}");

        currEvent = currEvent->nextEvent;
        if (currEvent != ABD_EVENT_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    } while (currEvent != ABD_EVENT_NOT_FOUND);

    fprintf(out, "\n}");
    fprintf(dispOut, "}')");
}

void persistInformation()
{
    FILE *outputFile;
    FILE *dispOutFile;

    outputFile = openFile(getObjPath());
    dispOutFile = openFile(getJSpath("objects"));
    if (outputFile == NULL || dispOutFile == NULL)
    {
        //unable to open file
        //check destination path
        puts("Cannot open objects file... check path");
        return;
    }

    //save both registries
    saveObjects(outputFile, dispOutFile);

    //close objects file
    closeFile(outputFile);
    closeFile(dispOutFile);

    //open events file
    puts("Will persist events...");
    outputFile = openFile(getEventsPath());
    dispOutFile = openFile(getJSpath("events"));
    if (outputFile == NULL || dispOutFile == NULL)
    {
        //unable to open file
        //check destination path
        puts("Cannot open events file... check path");
        return;
    }

    //save events
    saveEvents(outputFile, dispOutFile);

    //close events file
    closeFile(outputFile);
    closeFile(dispOutFile);
}
