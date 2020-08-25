#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <abd_tool/json_helpers_defn.h>
#include <abd_tool/obj_manager_defn.h>
#include <abd_tool/event_manager_defn.h>
#include <abd_tool/settings_manager_defn.h>
#include <unistd.h>
#include <ctype.h>

FILE *openFile(char *filePath, char *mode)
{
    return fopen(filePath, mode);
}

int closeFile(FILE *outputFile)
{
    return fclose(outputFile);
}
char *getScriptPath()
{
    SEXP e, tmp, retValue;
    ParseStatus status;
    char *fullPath;
    char *cmd1 = "cmdArgs <- commandArgs(trailingOnly = FALSE)";
    char *cmd2 = "needle <- \"--file=\"";
    char *cmd3 = "match <- grep(needle, cmdArgs)";
    char *cmd4 = "length(match)";
    char *cmd6 = "normalizePath(sub(needle, \"\", cmdArgs[match]))";

    char *cmd8 = "normalizePath(sys.frames()[[1]]$ofile)";

    PROTECT(tmp = mkString(cmd1));
    PROTECT(e = R_ParseVector(tmp, -1, &status, R_NilValue));
    PROTECT(retValue = R_tryEval(VECTOR_ELT(e, 0), R_GlobalEnv, NULL));

    tmp = mkString(cmd2);
    e = R_ParseVector(tmp, -1, &status, R_NilValue);
    retValue = R_tryEval(VECTOR_ELT(e, 0), R_GlobalEnv, NULL);

    tmp = mkString(cmd3);
    e = R_ParseVector(tmp, -1, &status, R_NilValue);
    retValue = R_tryEval(VECTOR_ELT(e, 0), R_GlobalEnv, NULL);

    tmp = mkString(cmd4);
    e = R_ParseVector(tmp, -1, &status, R_NilValue);
    retValue = R_tryEval(VECTOR_ELT(e, 0), R_GlobalEnv, NULL);

    if (TYPEOF(retValue) == INTSXP)
    {
        if (INTEGER(retValue)[0] > 0)
            tmp = mkString(cmd6);
        else
            tmp = mkString(cmd8);

        e = R_ParseVector(tmp, -1, &status, R_NilValue);
        retValue = R_tryEval(VECTOR_ELT(e, 0), R_GlobalEnv, NULL);
    }
    UNPROTECT(3);

    int pathSize = strlen(CHAR(asChar(retValue)));
    fullPath = memAllocForString(pathSize);
    copyStr(fullPath, CHAR(asChar(retValue)), pathSize);
    return fullPath;
}
void writeCharByCharToFile(FILE *out, char *string, int withComma)
{
    if (withComma)
        fprintf(out, ",");

    fprintf(out, "\"");

    for (int i = 0; string[i] != '\n' && string[i] != '\r' && string[i] != '\0'; i++)
    {
        if (!isprint(string[i]))
            continue;
        if (string[i] == '\"')
            fprintf(out, "\\'");
        else if (string[i] == '\'')
            fprintf(out, "\\'");
        else
            fprintf(out, "%c", string[i]);
    }

    fprintf(out, "\"");
}

void dupScript()
{
    char *dupPath = getJSpath("code");
    char *oriPath = getScriptPath();
    char readLine[1024];
    int first = 1;
    char *retJunk;
    FILE *ori;
    FILE *dup;
    if ((ori = openFile(oriPath, FILE_OPEN_READ)) != FILE_NOT_FOUND)
    {
        if ((dup = openFile(dupPath, FILE_OPEN_WRITE)) != FILE_NOT_FOUND)
        {
            rewind(ori);
            rewind(dup);
            fprintf(dup, "code=JSON.parse('[");
            retJunk = fgets(readLine, sizeof(readLine), ori);
            writeCharByCharToFile(dup, readLine, 0);
            while (retJunk = fgets(readLine, sizeof(readLine), ori))
                writeCharByCharToFile(dup, readLine, 1);
            fprintf(dup, "]')");
            closeFile(dup);
        }
        closeFile(ori);
    }
}
char *getStrFromIndent(JSON_INDENT indent)
{
    if (indent == INDENT_0)
        return "";
    char *ret = (char *)malloc((indent + 1) * sizeof(char));
    ret[0] = '\0';
    for (int i = 0; i < indent; i++)
        strcat(ret, "\t");
    ret[indent] = '\0';
    return ret;
}
char *getStrForType(SEXPTYPE type)
{
    switch (type)
    {
    case REALSXP:
        return "REALSXP";
    case INTSXP:
        return "INTSXP";
    case STRSXP:
        return "STRSXP";
    default:
        return "";
        break;
    }
}

int baseVecSize;
void *baseVecValues;

SEXPTYPE prevType;
void writeVectorValues(FILE *out, int indent, SEXPTYPE type, void *vector, int nElements, FILE *dispOut)
{
    fprintf(out, "[");
    fprintf(dispOut, "[");
    for (int i = 0; i < nElements; i++)
    {
        switch (type)
        {
        case REALSXP:
            fprintf(out, "%.2f%s", ((double *)vector)[i], (i + 1 == nElements) ? "" : ",");
            fprintf(dispOut, "%.2f%s", ((double *)vector)[i], (i + 1 == nElements) ? "" : ",");
            break;
        case INTSXP:
            fprintf(out, "%d%s", ((int *)vector)[i], (i + 1 == nElements) ? "" : ",");
            fprintf(dispOut, "%d%s", ((int *)vector)[i], (i + 1 == nElements) ? "" : ",");
            break;
        case STRSXP:
            fprintf(out, "\"%s\"%s", ((char **)vector)[i], (i + 1 == nElements) ? "" : ",");
            fprintf(dispOut, "\"%s\"%s", ((char **)vector)[i], (i + 1 == nElements) ? "" : ",");
            break;
        default:
            break;
        }
    }
    fprintf(out, "]");
    fprintf(dispOut, "]");
}

void *getNewVectorFromType(SEXPTYPE newType, SEXPTYPE oldType, void *vector, int vectorSize)
{
    void *newVector;
    switch (oldType)
    {
    case INTSXP:
        switch (newType)
        {
        case REALSXP:
            newVector = memAllocDoubleVector(vectorSize);
            for (int i = 0; i < vectorSize; i++)
                ((double *)newVector)[i] = (double)(((int *)vector)[i]);
            break;
        case STRSXP:
            //convert int to string here
            break;
        default:
            break;
        }
        break;

    case REALSXP:
        switch (newType)
        {
        case STRSXP:
            //convert REAL to string here
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return newVector;
}

void writeVector(FILE *out, ABD_VEC_OBJ *vecObj, FILE *dispOut)
{
    SEXPTYPE type = vecObj->type;

    if (vecObj->idxChange)
    {
        fprintf(out, "\n%s\"dataType\" : \"%s\",", getStrFromIndent(INDENT_5), getStrForType(type));
        fprintf(out, "\n%s\"vecMod\" : true,", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"numMods\" : %d,", getStrFromIndent(INDENT_5), vecObj->nCols);
        fprintf(out, "\n%s\"mods\" : [", getStrFromIndent(INDENT_5));

        fprintf(dispOut, "\"dataType\" : \"%s\",", getStrForType(type));
        fprintf(dispOut, "\"vecMod\" : true,");
        fprintf(dispOut, "\"numMods\" : %d,", vecObj->nCols);
        fprintf(dispOut, "\"mods\" : [");

        // if (prevType != type)
        //     baseVecValues = getNewVectorFromType(type, prevType, baseVecValues, baseVecSize);

        for (int i = 0; i < vecObj->nCols; i++)
        {
            int idx = vecObj->idxs[i];
            fprintf(out, "\n%s{\"index\" : %d, ", getStrFromIndent(INDENT_6), idx);
            fprintf(dispOut, "{ \"index\" : %d,", idx);
            switch (type)
            {
            case REALSXP:
                fprintf(out, "\"newValue\" : %.2f", ((double *)vecObj->vector)[i]);
                fprintf(dispOut, "\"newValue\" : %.2f", ((double *)vecObj->vector)[i]);
                // ((double *)baseVecValues)[idx] = ((double *)vecObj->vector)[i];
                break;
            case INTSXP:
                fprintf(out, "\"newValue\" : %d", ((int *)vecObj->vector)[i]);
                fprintf(dispOut, "\"newValue\" : %d", ((int *)vecObj->vector)[i]);
                // ((int *)baseVecValues)[idx] = ((int *)vecObj->vector)[i];
                break;
            case STRSXP:
                fprintf(out, "\"newValue\" : \"%s\"", ((char **)vecObj->vector)[i]);
                fprintf(dispOut, "\"newValue\" : \"%s\"", ((char **)vecObj->vector)[i]);
                // ((char **)baseVecValues)[idx] = ((char **)vecObj->vector)[i];
                break;
            default:
                break;
            }
            fprintf(out, "}");
            fprintf(dispOut, "}");
            if (i + 1 != vecObj->nCols)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        }
        // fprintf(out, ",");
        // fprintf(dispOut, ",");
        //save the changed vector
        // fprintf(out, "\n%s", getStrFromIndent(INDENT_6));
        // writeVectorValues(out, INDENT_7, type, baseVecValues, baseVecSize, dispOut);
        fprintf(out, "\n%s]", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "]");
        prevType = type;
    }
    else
    {
        baseVecSize = vecObj->nCols;
        baseVecValues = vecObj->vector;
        prevType = type;
        fprintf(out, "\n%s\"dataType\" : \"%s\",", getStrFromIndent(INDENT_5), getStrForType(type));
        fprintf(dispOut, "\"dataType\" : \"%s\",", getStrForType(type));
        fprintf(out, "\n%s\"vecMod\" : false,", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"vecMod\" : false,");

        fprintf(out, "\n%s\"nElements\" : %d,", getStrFromIndent(INDENT_5), vecObj->nCols);
        fprintf(dispOut, "\"nElements\" : %d,", vecObj->nCols);
        fprintf(out, "\n%s\"values\" : ", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"values\" : ");
        writeVectorValues(out, INDENT_5, type, baseVecValues, baseVecSize, dispOut);
    }
}

void writeDataFrame(FILE *out, ABD_FRAME_OBJ *frameObj, FILE *dispOut)
{
    int nCols = frameObj->nCols;
    if (frameObj->cellChange)
    {

        fprintf(out, "\n%s\"dataType\" : \"Multiple\",", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"dataType\" : \"Multiple\",");
        fprintf(out, "\n%s\"frameMod\" : true,", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"frameMod\" : true,");
        fprintf(out, "\n%s\"numMods\" : %d,", getStrFromIndent(INDENT_5), nCols);
        fprintf(dispOut, "\"numMods\" : %d,", nCols);
        fprintf(out, "\n%s\"mods\" : [", getStrFromIndent(INDENT_5), nCols);
        fprintf(dispOut, "\"mods\" : [", nCols);

        for (int c = 0; c<nCols; c++) {
            ABD_VEC_OBJ * changedCol = frameObj->cols[c];
            fprintf(out, "\n%s[", getStrFromIndent(INDENT_6));
            fprintf(dispOut, "[");
            fprintf(out, "\n%s%d,", getStrFromIndent(INDENT_7), frameObj->changedVecs[c]);
            fprintf(dispOut, "%d,", frameObj->changedVecs[c]);
            for (int r = 0; r < changedCol->nCols; r++)
            {
                int idx = changedCol->idxs[r];
                fprintf(out, "\n%s{\"index\" : %d, ", getStrFromIndent(INDENT_7), idx);
                fprintf(dispOut, "{ \"index\" : %d,", idx);
                switch (changedCol->type)
                {
                case REALSXP:
                    fprintf(out, "\"newValue\" : %.2f", ((double *)changedCol->vector)[r]);
                    fprintf(dispOut, "\"newValue\" : %.2f", ((double *)changedCol->vector)[r]);
                    // ((double *)baseVecValues)[idx] = ((double *)vecObj->vector)[i];
                    break;
                case INTSXP:
                    fprintf(out, "\"newValue\" : %d", ((int *)changedCol->vector)[r]);
                    fprintf(dispOut, "\"newValue\" : %d", ((int *)changedCol->vector)[r]);
                    // ((int *)baseVecValues)[idx] = ((int *)vecObj->vector)[i];
                    break;
                case STRSXP:
                    fprintf(out, "\"newValue\" : \"%s\"", ((char **)changedCol->vector)[r]);
                    fprintf(dispOut, "\"newValue\" : \"%s\"", ((char **)changedCol->vector)[r]);
                    // ((char **)baseVecValues)[idx] = ((char **)vecObj->vector)[i];
                    break;
                default:
                    break;
                }
                fprintf(out, "}");
                fprintf(dispOut, "}");
                if (r + 1 != changedCol->nCols)
                {
                    fprintf(out, ",");
                    fprintf(dispOut, ",");
                }
            }
            fprintf(out, "\n%s]", getStrFromIndent(INDENT_6));
            fprintf(dispOut, "]");

            if (c + 1 < nCols)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }

        }
        fprintf(out, "\n%s]", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "]");

    }
    else
    {
        fprintf(out, "\n%s\"dataType\" : \"Multiple\",", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"dataType\" : \"Multiple\",");
        fprintf(out, "\n%s\"frameMod\" : false,", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"frameMod\" : false,");
        fprintf(out, "\n%s\"nCols\" : %d,", getStrFromIndent(INDENT_5), nCols);
        fprintf(dispOut, "\"nCols\" : %d,", nCols);


        fprintf(out, "\n%s\"cols\" : [", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"cols\" : [");
        for (int i = 0; i < nCols; i++)
        {
            ABD_VEC_OBJ *currVec = frameObj->cols[i];

            writeVectorValues(out, INDENT_7, currVec->type, currVec->vector, currVec->nCols, dispOut);

            if (i + 1 < nCols)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        }
        fprintf(out, "]");
        fprintf(dispOut, "]");
    }
}

void writeObjModsToFile(FILE *out, ABD_OBJECT_MOD *listStart, FILE *dispOut)
{
    ABD_OBJECT_MOD *currMod = listStart;
    do
    {
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_4), currMod->id);
        fprintf(out, "\n%s\"structType\" : ", getStrFromIndent(INDENT_5));

        fprintf(dispOut, "\"%d\" : {", currMod->id);
        fprintf(dispOut, "\"structType\" : ");
        switch (currMod->valueType)
        {
        case ABD_VECTOR:
            fprintf(out, "\"Vector\",");
            fprintf(dispOut, "\"Vector\",");
            writeVector(out, currMod->value.vec_value, dispOut);
            break;
        case ABD_MATRIX:
            break;
        case ABD_FRAME:
            fprintf(out, "\"DataFrame\",");
            fprintf(dispOut, "\"DataFrame\",");
            writeDataFrame(out, currMod->value.frame_value, dispOut);
            break;
        default:
            puts("default in persist");
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
        fprintf(out, "\n%s\"modList\" : {", getStrFromIndent(INDENT_3));

        fprintf(dispOut, ",");
        fprintf(dispOut, "\"modList\" : {");

        writeObjModsToFile(out, obj->modListStart, dispOut);

        fprintf(out, "\n%s}", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "}");
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

void writeArgStrVector(FILE *out, ABD_VEC_OBJ *vecObj, JSON_INDENT indent, FILE *dispOut)
{
    if (vecObj->nCols == 1)
    {
        fprintf(out, "\n%s\"value\" : \"%s\"", getStrFromIndent(indent), ((char **)vecObj->vector)[0]);
        fprintf(dispOut, "\"value\" : \"%s\"", ((char **)vecObj->vector)[0]);
    }
    else
    {
        fprintf(out, "\n%s\"value\" : [", getStrFromIndent(indent));
        fprintf(dispOut, "\"value\" : [");
        for (int i = 0; i < vecObj->nCols; i++)
        {
            fprintf(out, "\"%s\"%s", ((char **)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
            fprintf(dispOut, "\"%s\"%s", ((char **)vecObj->vector)[i], (i + 1 == vecObj->nCols) ? "" : ",");
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
        writeArgStrVector(out, vecObj, indent, dispOut);
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
        fprintf(dispOut, "{");

        fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_5), currArg->fromObj->id);
        fprintf(out, "\n%s\"fromState\" : %d,", getStrFromIndent(INDENT_5), currArg->passedValue->id);
        fprintf(out, "\n%s\"toId\" : %d,", getStrFromIndent(INDENT_5), currArg->toObj->id);
        fprintf(out, "\n%s\"toState\" : %d", getStrFromIndent(INDENT_5), currArg->rcvdValue->id);

        fprintf(dispOut, "\"fromId\" : %d,", currArg->fromObj->id);
        fprintf(dispOut, "\"fromState\" : %d,", currArg->passedValue->id);
        fprintf(dispOut, "\"toId\" : %d,", currArg->toObj->id);
        fprintf(dispOut, "\"toState\" : %d", currArg->rcvdValue->id);

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
            fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_3), obj->id);
            fprintf(out, "\n%s\"fromState\" : %d,", getStrFromIndent(INDENT_3), event->fromState->id);

            fprintf(dispOut, "\"ABD\",");
            fprintf(dispOut, "\"fromId\" : %d,", obj->id);
            fprintf(dispOut, "\"fromState\" : %d,", event->fromState->id);
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
    fprintf(out, "\n%s\"exprStr\" : \"%s\",", getStrFromIndent(INDENT_3), event->exprStr);
    fprintf(out, "\n%s\"expressions\" : {", getStrFromIndent(INDENT_3));

    fprintf(dispOut, "\"result\" : %.4f,", event->globalResult);
    fprintf(dispOut, "\"exprStr\" : \"%s\",", event->exprStr);
    fprintf(dispOut, "\"expressions\" : {");

    writeExpression(out, event->expr, dispOut);

    fprintf(out, "\n%s}", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "}");
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
    fprintf(out, "\n%s\"toEnv\" : \"%s\",", getStrFromIndent(INDENT_3), envToStr(retEvent->toEnv));
    fprintf(out, "\n%s\"toId\" : %d,", getStrFromIndent(INDENT_3), (retEvent->toObj == ABD_OBJECT_NOT_FOUND) ? -1 : retEvent->toObj->id);
    fprintf(dispOut, "\"toEnv\" : \"%s\",", envToStr(retEvent->toEnv));
    fprintf(dispOut, "\"toId\" : %d,", (retEvent->toObj == ABD_OBJECT_NOT_FOUND) ? -1 : retEvent->toObj->id);
    if (retEvent->toObj != ABD_OBJECT_NOT_FOUND)
    {
        // returned to an object
        fprintf(out, "\n%s\"toState\" : %d", getStrFromIndent(INDENT_3), retEvent->retValue->id);
        fprintf(dispOut, "\"toState\" : %d", retEvent->retValue->id);
    }
    else
        // return not assigned
        writeArgValueToFile(out, retEvent->retValue, INDENT_3, dispOut);
}

void saveVecEvent(FILE *out, ABD_VEC_EVENT *vecEvent, FILE *dispOut)
{
    fprintf(out, "\n%s\"size\" : %d,", getStrFromIndent(INDENT_3), vecEvent->nElements);
    fprintf(out, "\n%s\"toObj\" : %s,", getStrFromIndent(INDENT_3), (vecEvent->toObj == ABD_OBJECT_NOT_FOUND) ? "false" : "true");
    fprintf(out, "\n%s\"fromObj\" : %d,", getStrFromIndent(INDENT_3), (vecEvent->fromObj == ABD_OBJECT_NOT_FOUND) ? 0 : vecEvent->fromObj->id);
    fprintf(out, "\n%s\"fromState\" : %d,", getStrFromIndent(INDENT_3), (vecEvent->fromState == ABD_OBJECT_NOT_FOUND) ? 0 : vecEvent->fromState->id);
    fprintf(out, "\n%s\"rangeL\" : %d,", getStrFromIndent(INDENT_3), vecEvent->rangeL);
    fprintf(out, "\n%s\"rangeR\" : %d", getStrFromIndent(INDENT_3), vecEvent->rangeR);

    fprintf(dispOut, "\"size\" : %d,", vecEvent->nElements);
    fprintf(dispOut, "\"toObj\" : %s,", (vecEvent->toObj == ABD_OBJECT_NOT_FOUND) ? "false" : "true");
    fprintf(dispOut, "\"fromObj\" : %d,", (vecEvent->fromObj == ABD_OBJECT_NOT_FOUND) ? 0 : vecEvent->fromObj->id);
    fprintf(dispOut, "\"fromState\" : %d,", (vecEvent->fromState == ABD_OBJECT_NOT_FOUND) ? 0 : vecEvent->fromState->id);
    fprintf(dispOut, "\"rangeL\" : %d,", vecEvent->rangeL);
    fprintf(dispOut, "\"rangeR\" : %d", vecEvent->rangeR);
}

void writeIFobj(FILE *out, char *sideStr, IF_ABD_OBJ *obj, FILE *dispOut)
{

    fprintf(out, "\n%s\"%sObjId\" : %d,", getStrFromIndent(INDENT_5), sideStr, obj->objPtr->id);
    fprintf(dispOut, "\"%sObjId\" : %d,", sideStr, obj->objPtr->id);

    switch (obj->objValue->valueType)
    {
    case ABD_VECTOR:
    {
        if (obj->objPtr->id != -1)
        {
            fprintf(out, "\n%s\"%sWithIndex\" : %d,", getStrFromIndent(INDENT_5), sideStr, obj->objValue->value.vec_value->idxs[0]);
            fprintf(dispOut, "\"%sWithIndex\" : %d,", sideStr, obj->objValue->value.vec_value->idxs[0]);
            if (obj->objPtr->id > 0)
            {
                fprintf(out, "\n%s\"%sObjState\" : %d,", getStrFromIndent(INDENT_5), sideStr, obj->objValue->id);
                fprintf(dispOut, "\"%sObjState\" : %d,", sideStr, obj->objValue->id);
            }
            else
            {
                fprintf(out, "\n%s\"%sObjName\" : \"%s\",", getStrFromIndent(INDENT_5), sideStr, obj->objPtr->name);
                fprintf(dispOut, "\"%sObjName\" : \"%s\",", sideStr, obj->objPtr->name);
            }
        }
        if (obj->objPtr->id < 0)
        {
            switch (obj->objValue->value.vec_value->type)
            {
            case REALSXP:
                fprintf(out, "\n%s\"%sValue\" : %.2f,", getStrFromIndent(INDENT_5), sideStr, ((double *)obj->objValue->value.vec_value->vector)[0]);
                fprintf(dispOut, "\"%sValue\" : %.2f,", sideStr, ((double *)obj->objValue->value.vec_value->vector)[0]);
                break;
            case INTSXP:
                fprintf(out, "\n%s\"%sValue\" : %d,", getStrFromIndent(INDENT_5), sideStr, ((int *)obj->objValue->value.vec_value->vector)[0]);
                fprintf(dispOut, "\"%sValue\" : %d,", sideStr, ((int *)obj->objValue->value.vec_value->vector)[0]);
                break;
            case STRSXP:
                fprintf(out, "\n%s\"%sValue\" : \"%s\",", getStrFromIndent(INDENT_5), sideStr, ((char **)obj->objValue->value.vec_value->vector)[0]);
                fprintf(dispOut, "\"%sValue\" : \"%s\",", sideStr, ((char **)obj->objValue->value.vec_value->vector)[0]);
                break;
            default:
                break;
            }
        }
    }
    break;
    case ABD_MATRIX:
        break;
    default:
        break;
    }
}

void writeExpression(FILE *out, IF_EXPRESSION *expr, FILE *dispOut)
{
    short root = 0;

    if (expr->left_type == IF_EXPR)
        writeExpression(out, expr->left_data, dispOut);
    else
        root = 1;

    if (expr->right_type == IF_EXPR)
        writeExpression(out, expr->right_data, dispOut);
    else
        root += 2;

    /*
        root == 0 -> expr OP expr ((a>b) == (b<a)) [in this case the center]
        root == 1 -> reached end of left side
        root == 2 -> reached end of right side
        root == 3 -> reach end of both sides
    */
    switch (root)
    {
    case 1:
    {
        // left is obj
        //right is expr
        IF_ABD_OBJ *leftObj = ((IF_ABD_OBJ *)expr->left_data);
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_4), expr->exprId);
        fprintf(out, "\n%s\"isConfined\" : %s,", getStrFromIndent(INDENT_5), expr->isConfined ? "true" : "false");

        fprintf(dispOut, "\"%d\" : {", expr->exprId);
        fprintf(dispOut, "\"isConfined\" : %s,", expr->isConfined ? "true" : "false");

        writeIFobj(out, "l", leftObj, dispOut);
        IF_EXPRESSION *rightExp = ((IF_EXPRESSION *)expr->right_data);
        fprintf(out, "\n%s\"rType\" : \"expr\",", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"rExpId\" : \"%d\",", getStrFromIndent(INDENT_5), ((IF_EXPRESSION *)expr->right_data)->exprId);
        fprintf(out, "\n%s\"op\" : \"%s\",", getStrFromIndent(INDENT_5), expr->operator);
        fprintf(out, "\n%s\"result\" : \"%.2f\"", getStrFromIndent(INDENT_5), expr->result);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));

        fprintf(dispOut, "\"rType\" : \"expr\",");
        fprintf(dispOut, "\"rExpId\" : \"%d\",", ((IF_EXPRESSION *)expr->right_data)->exprId);
        fprintf(dispOut, "\"op\" : \"%s\",", expr->operator);
        fprintf(dispOut, "\"result\" : \"%.2f\"", expr->result);
        fprintf(dispOut, "}");
        break;
    }
    case 2:
    {
        // right is obj

        IF_EXPRESSION *leftExp = ((IF_EXPRESSION *)expr->left_data);
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_4), expr->exprId);
        fprintf(out, "\n%s\"isConfined\" : %s,", getStrFromIndent(INDENT_5), expr->isConfined ? "true" : "false");
        fprintf(out, "\n%s\"lType\" : \"expr\",", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"lExpId\" : \"%d\",", getStrFromIndent(INDENT_5), ((IF_EXPRESSION *)expr->left_data)->exprId);

        fprintf(dispOut, "\"%d\" : {", expr->exprId);
        fprintf(dispOut, "\"isConfined\" : %s,", expr->isConfined ? "true" : "false");
        fprintf(dispOut, "\"lType\" : \"expr\",");
        fprintf(dispOut, "\"lExpId\" : \"%d\",", ((IF_EXPRESSION *)expr->left_data)->exprId);

        IF_ABD_OBJ *rightObj = ((IF_ABD_OBJ *)expr->right_data);
        fprintf(out, "\n%s\"rType\" : \"obj\",", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"rType\" : \"obj\",");
        writeIFobj(out, "r", rightObj, dispOut);
        // left is expr
        fprintf(out, "\n%s\"op\" : \"%s\",", getStrFromIndent(INDENT_5), expr->operator);
        fprintf(out, "\n%s\"result\" : \"%.2f\"", getStrFromIndent(INDENT_5), expr->result);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));

        fprintf(dispOut, "\"op\" : \"%s\",", expr->operator);
        fprintf(dispOut, "\"result\" : \"%.2f\"", expr->result);
        fprintf(dispOut, "}");
        break;
    }
    case 3:
    {
        IF_ABD_OBJ *leftObj = ((IF_ABD_OBJ *)expr->left_data);
        IF_ABD_OBJ *rightObj = ((IF_ABD_OBJ *)expr->right_data);

        //left obj
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_4), expr->exprId);
        fprintf(out, "\n%s\"isConfined\" : %s,", getStrFromIndent(INDENT_5), expr->isConfined ? "true" : "false");
        fprintf(out, "\n%s\"lType\" : \"obj\",", getStrFromIndent(INDENT_5));

        fprintf(dispOut, "\"%d\" : {", expr->exprId);
        fprintf(dispOut, "\"isConfined\" : %s,", expr->isConfined ? "true" : "false");
        fprintf(dispOut, "\"lType\" : \"obj\",");

        writeIFobj(out, "l", leftObj, dispOut);

        //right obj
        fprintf(out, "\n%s\"rType\" : \"obj\",", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"rType\" : \"obj\",");
        writeIFobj(out, "r", rightObj, dispOut);
        fprintf(out, "\n%s\"op\" : \"%s\",", getStrFromIndent(INDENT_5), expr->operator);
        fprintf(out, "\n%s\"result\" : \"%.2f\"", getStrFromIndent(INDENT_5), expr->result);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));

        fprintf(dispOut, "\"op\" : \"%s\",", expr->operator);
        fprintf(dispOut, "\"result\" : \"%.2f\"", expr->result);
        fprintf(dispOut, "}");
        break;
    }
    case 0:
    {
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_4), expr->exprId);
        fprintf(out, "\n%s\"isConfined\" : %s,", getStrFromIndent(INDENT_5), expr->isConfined ? "true" : "false");
        fprintf(out, "\n%s\"lType\" : \"expr\",", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"lExpId\" : \"%d\",", getStrFromIndent(INDENT_5), ((IF_EXPRESSION *)expr->left_data)->exprId);
        fprintf(out, "\n%s\"rType\" : \"expr\",", getStrFromIndent(INDENT_5));
        fprintf(out, "\n%s\"rExpId\" : \"%d\",", getStrFromIndent(INDENT_5), ((IF_EXPRESSION *)expr->right_data)->exprId);
        fprintf(out, "\n%s\"op\" : \"%s\",", getStrFromIndent(INDENT_5), expr->operator);
        fprintf(out, "\n%s\"result\" : \"%.2f\"", getStrFromIndent(INDENT_5), expr->result);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));

        fprintf(dispOut, "\"%d\" : {", expr->exprId);
        fprintf(dispOut, "\"isConfined\" : %s,", expr->isConfined ? "true" : "false");
        fprintf(dispOut, "\"lType\" : \"expr\",");
        fprintf(dispOut, "\"lExpId\" : \"%d\",", ((IF_EXPRESSION *)expr->left_data)->exprId);
        fprintf(dispOut, "\"rType\" : \"expr\",");
        fprintf(dispOut, "\"rExpId\" : \"%d\",", ((IF_EXPRESSION *)expr->right_data)->exprId);
        fprintf(dispOut, "\"op\" : \"%s\",", expr->operator);
        fprintf(dispOut, "\"result\" : \"%.2f\"", expr->result);
        fprintf(dispOut, "}");
        break;
    }
    default:
        break;
    }
    if (expr->exprId > 1)
    {
        fprintf(out, ",");
        fprintf(dispOut, ",");
    }
}

void saveIfEvent(FILE *out, ABD_IF_EVENT *if_event, FILE *dispOut)
{
    fprintf(out, "\n%s\"globalResult\" : %s,", getStrFromIndent(INDENT_3), (if_event->globalResult ? "true" : "false"));
    fprintf(dispOut, "\"globalResult\" : %s,", (if_event->globalResult ? "true" : "false"));

    if (if_event->isElse)
    {
        fprintf(out, "\n%s\"exprStr\" : \"else\",", getStrFromIndent(INDENT_3));
        fprintf(out, "\n%s\"isElseIf\" : false,", getStrFromIndent(INDENT_3));
        fprintf(out, "\n%s\"isElse\" : true", getStrFromIndent(INDENT_3));

        fprintf(dispOut, "\"exprStr\" : \"else\",");
        fprintf(dispOut, "\"isElseIf\" : false,");
        fprintf(dispOut, "\"isElse\" : true");
    }
    else
    {

        if (if_event->isElseIf)
        {
            fprintf(out, "\n%s\"exprStr\" : \"%s\",", getStrFromIndent(INDENT_3), if_event->exprStr);
            fprintf(out, "\n%s\"isElseIf\" : true,", getStrFromIndent(INDENT_3));
            fprintf(out, "\n%s\"isElse\" : false,", getStrFromIndent(INDENT_3));

            fprintf(dispOut, "\"exprStr\" : \"%s\",", if_event->exprStr);
            fprintf(dispOut, "\"isElseIf\" : true,");
            fprintf(dispOut, "\"isElse\" : false,");
        }
        else
        {
            fprintf(out, "\n%s\"exprStr\" : \"%s\",", getStrFromIndent(INDENT_3), if_event->exprStr);
            fprintf(out, "\n%s\"isElseIf\" : false,", getStrFromIndent(INDENT_3));
            fprintf(out, "\n%s\"isElse\" : false,", getStrFromIndent(INDENT_3));

            fprintf(dispOut, "\"exprStr\" : \"%s\",", if_event->exprStr);
            fprintf(dispOut, "\"isElseIf\" : false,");
            fprintf(dispOut, "\"isElse\" : false,");
        }
        fprintf(out, "\n%s\"expressions\" : {", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "\"expressions\" : {");
        writeExpression(out, if_event->expr, dispOut);
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "}");
    }
}

void saveIdxChangeEvent(FILE *out, ABD_IDX_CHANGE_EVENT *idx_event, FILE *dispOut)
{
    fprintf(out, "\n%s\"toId\" : %d,", getStrFromIndent(INDENT_3), idx_event->toObj->id);
    fprintf(out, "\n%s\"toState\" : %d,", getStrFromIndent(INDENT_3), idx_event->toState->id);
    fprintf(out, "\n%s\"origin\" : \"%s\",", getStrFromIndent(INDENT_3), (idx_event->fromType == ABD_E) ? "event" : "obj");

    fprintf(dispOut, "\"toId\" : %d,", idx_event->toObj->id);
    fprintf(dispOut, "\"toState\" : %d,", idx_event->toState->id);
    fprintf(dispOut, "\"origin\" : \"%s\",", (idx_event->fromType == ABD_E) ? "event" : "obj");

    if (idx_event->fromType == ABD_E)
    {
        //
        fprintf(out, "\n%s\"fromEvent\" : %d", getStrFromIndent(INDENT_3), ((ABD_EVENT *)idx_event->fromObj)->id);
        fprintf(dispOut, "\"fromEvent\" : %d", ((ABD_EVENT *)idx_event->fromObj)->id);
    }
    else
    {

        ABD_OBJECT *obj = ((ABD_OBJECT *)idx_event->fromObj);
        fprintf(out, "\n%s\"fromObj\" : ", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "\"fromObj\" : ");

        if (obj->id == -1)
        {
            //hardcoded value
            fprintf(out, "\"HC\",");
            fprintf(dispOut, "\"HC\",");
            writeArgValueToFile(out, idx_event->fromState, INDENT_3, dispOut);
        }
        else
        {

            if (obj->id == -2)
            {
                //object not in registry
                fprintf(out, "\"R\",");
                fprintf(out, "\n%s\"name\" : \"%s\",", getStrFromIndent(INDENT_3), obj->name);

                fprintf(dispOut, "\"R\",");
                fprintf(dispOut, "\"name\" : \"%s\",", obj->name);
                fprintf(out, "\n%s\"fromIdxs\" : [", getStrFromIndent(INDENT_3));
                fprintf(dispOut, "\"fromIdxs\" : [");

                for (int i = 0; i < idx_event->nIdxs; i++)
                {
                    fprintf(out, "%d", idx_event->fromIdxs[i]);
                    fprintf(dispOut, "%d", idx_event->fromIdxs[i]);
                    if (i + 1 < idx_event->nIdxs)
                    {
                        fprintf(out, ",");
                        fprintf(dispOut, ",");
                    }
                }
                fprintf(out, "],");
                fprintf(dispOut, "],");
                writeArgValueToFile(out, idx_event->fromState, INDENT_3, dispOut);
            }
            else
            {
                //object in registry
                fprintf(out, "\"ABD\",");
                fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_3), obj->id);
                fprintf(out, "\n%s\"fromState\" : %d,", getStrFromIndent(INDENT_3), idx_event->fromState->id);

                fprintf(dispOut, "\"ABD\",");
                fprintf(dispOut, "\"fromId\" : %d,", obj->id);
                fprintf(dispOut, "\"fromState\" : %d,", idx_event->fromState->id);

                fprintf(out, "\n%s\"fromIdxs\" : [", getStrFromIndent(INDENT_3));
                fprintf(dispOut, "\"fromIdxs\" : [");

                for (int i = 0; i < idx_event->nIdxs; i++)
                {
                    fprintf(out, "%d", idx_event->fromIdxs[i]);
                    fprintf(dispOut, "%d", idx_event->fromIdxs[i]);
                    if (i + 1 < idx_event->nIdxs)
                    {
                        fprintf(out, ",");
                        fprintf(dispOut, ",");
                    }
                }
                fprintf(out, "]");
                fprintf(dispOut, "]");
            }
        }
    }
}

void saveForLoopEvent(FILE *out, ABD_FOR_LOOP_EVENT *forEvent, FILE *dispOut)
{
    ITERATION *currIter = forEvent->itList;
    fprintf(out, "\n%s\"estimatedIter\" : %d,", getStrFromIndent(INDENT_3), forEvent->estIterNumber);
    fprintf(out, "\n%s\"iterCounter\" : %d,", getStrFromIndent(INDENT_3), forEvent->iterCounter);
    fprintf(out, "\n%s\"lastEventId\" : %d,", getStrFromIndent(INDENT_3), forEvent->lastEvent->id);
    fprintf(out, "\n%s\"iteratorId\" : %d,", getStrFromIndent(INDENT_3), forEvent->iterator->id);
    fprintf(out, "\n%s\"enumeratorId\" : %d,", getStrFromIndent(INDENT_3), forEvent->enumerator->id);
    fprintf(out, "\n%s\"enumeratorState\" : %d,", getStrFromIndent(INDENT_3), forEvent->enumState->id);
    fprintf(out, "\n%s\"fromIdxs\" : [", getStrFromIndent(INDENT_3));

    fprintf(dispOut, "\"estimatedIter\" : %d,", forEvent->estIterNumber);
    fprintf(dispOut, "\"iterCounter\" : %d,", forEvent->iterCounter);
    fprintf(dispOut, "\"lastEventId\" : %d,", forEvent->lastEvent->id);
    fprintf(dispOut, "\"iteratorId\" : %d,", forEvent->iterator->id);
    fprintf(dispOut, "\"enumeratorId\" : %d,", forEvent->enumerator->id);
    fprintf(dispOut, "\"enumeratorState\" : %d,", forEvent->enumState->id);
    fprintf(dispOut, "\"fromIdxs\" : [");

    for (int i = 0; i < forEvent->estIterNumber; i++)
    {
        fprintf(out, "%d", forEvent->fromIdxs[i]);
        fprintf(dispOut, "%d", forEvent->fromIdxs[i]);
        if (i + 1 < forEvent->estIterNumber)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    }

    fprintf(out, "],");
    fprintf(out, "\n%s\"iterations\" : {", getStrFromIndent(INDENT_3));

    fprintf(dispOut, "],");
    fprintf(dispOut, "\"iterations\" : {");

    while (currIter != ABD_NOT_FOUND)
    {
        fprintf(out, "\n%s\"%d\" : {", getStrFromIndent(INDENT_4), currIter->iterId);
        fprintf(out, "\n%s\"iteratorState\" : %d,", getStrFromIndent(INDENT_5), currIter->iteratorState->id);
        fprintf(out, "\n%s\"events\" : [", getStrFromIndent(INDENT_5));

        fprintf(dispOut, "\"%d\" : {", currIter->iterId);
        fprintf(dispOut, "\"iteratorState\" : %d,", currIter->iteratorState->id);
        fprintf(dispOut, "\"events\" : [");

        ITER_EVENT_LIST *currEventList = currIter->eventsList;
        while (currEventList != NULL)
        {
            fprintf(out, "%d", currEventList->event->id);
            fprintf(dispOut, "%d", currEventList->event->id);
            currEventList = currEventList->nextEvent;
            if (currEventList != ABD_EVENT_NOT_FOUND)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        }

        fprintf(out, "]");
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));

        fprintf(dispOut, "]");
        fprintf(dispOut, "}");

        currIter = currIter->nextIter;
        if (currIter != ABD_EVENT_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    }
    fprintf(out, "\n%s}", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "}");
}

void saveRepeatLoopEvent(FILE *out, ABD_REPEAT_LOOP_EVENT *repeatEvent, FILE *dispOut)
{
    ITERATION *currIter = repeatEvent->itList;

    fprintf(out, "\n%s\"iterCounter\" : %d,", getStrFromIndent(INDENT_3), repeatEvent->iterCounter);
    fprintf(out, "\n%s\"lastEventId\" : %d,", getStrFromIndent(INDENT_3), repeatEvent->lastEvent->id);

    fprintf(dispOut, "\"iterCounter\" : %d,", repeatEvent->iterCounter);
    fprintf(dispOut, "\"lastEventId\" : %d,", repeatEvent->lastEvent->id);

    fprintf(out, "\n%s\"iterations\" : {", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "\"iterations\" : {");

    while (currIter != ABD_NOT_FOUND)
    {
        fprintf(out, "\n%s\"%d\" : [", getStrFromIndent(INDENT_4), currIter->iterId);
        fprintf(dispOut, "\"%d\" : [", currIter->iterId);
        ITER_EVENT_LIST *currEventList = currIter->eventsList;
        while (currEventList != NULL)
        {
            fprintf(out, "%d", currEventList->event->id);
            fprintf(dispOut, "%d", currEventList->event->id);
            currEventList = currEventList->nextEvent;
            if (currEventList != ABD_EVENT_NOT_FOUND)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        }

        fprintf(out, "]");
        fprintf(dispOut, "]");

        currIter = currIter->nextIter;
        if (currIter != ABD_EVENT_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    }
    fprintf(out, "\n%s}", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "}");
}

void saveWhileLoopEvent(FILE *out, ABD_WHILE_LOOP_EVENT *whileEvent, FILE *dispOut)
{
    ITERATION *currIter = whileEvent->itList;

    fprintf(out, "\n%s\"iterCounter\" : %d,", getStrFromIndent(INDENT_3), whileEvent->iterCounter);
    fprintf(out, "\n%s\"lastEventId\" : %d,", getStrFromIndent(INDENT_3), whileEvent->lastEvent->id);
    fprintf(out, "\n%s\"statement\" : \"%s\",", getStrFromIndent(INDENT_3), whileEvent->cndtStr);

    fprintf(dispOut, "\"iterCounter\" : %d,", whileEvent->iterCounter);
    fprintf(dispOut, "\"lastEventId\" : %d,", whileEvent->lastEvent->id);
    fprintf(dispOut, "\"statement\" : \"%s\",", whileEvent->cndtStr);

    fprintf(out, "\n%s\"iterations\" : {", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "\"iterations\" : {");

    while (currIter != ABD_NOT_FOUND)
    {
        fprintf(out, "\n%s\"%d\" : [", getStrFromIndent(INDENT_4), currIter->iterId);
        fprintf(dispOut, "\"%d\" : [", currIter->iterId);
        ITER_EVENT_LIST *currEventList = currIter->eventsList;
        while (currEventList != NULL)
        {
            fprintf(out, "%d", currEventList->event->id);
            fprintf(dispOut, "%d", currEventList->event->id);
            currEventList = currEventList->nextEvent;
            if (currEventList != ABD_EVENT_NOT_FOUND)
            {
                fprintf(out, ",");
                fprintf(dispOut, ",");
            }
        }

        fprintf(out, "]");
        fprintf(dispOut, "]");

        currIter = currIter->nextIter;
        if (currIter != ABD_EVENT_NOT_FOUND)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    }
    fprintf(out, "\n%s}", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "}");
}
void saveDataFrameEvent(FILE *out, ABD_FRAME_EVENT *frameEvent, FILE *dispOut)
{
    fprintf(out, "\n%s\"numCols\" : %d,", getStrFromIndent(INDENT_3), frameEvent->nCols);
    fprintf(dispOut, "\"numCols\" : %d,", frameEvent->nCols);

    fprintf(out, "\n%s\"srcs\" : [", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "\"srcs\" : [");
    for (int i = 0; i < frameEvent->nCols; i++)
    {
        fprintf(out, "\n%s{", getStrFromIndent(INDENT_4));
        fprintf(dispOut, "{");
        fprintf(out, "\n%s\"colName\" : \"%s\",", getStrFromIndent(INDENT_5), frameEvent->colNames[i]);
        fprintf(dispOut, "\"colName\" : \"%s\",", frameEvent->colNames[i]);

        fprintf(out, "\n%s\"objId\" : %d,", getStrFromIndent(INDENT_5), frameEvent->srcObjs[i]->id);
        fprintf(dispOut, "\"objId\" : %d,", frameEvent->srcObjs[i]->id);

        if (frameEvent->srcObjs[i]->id > 0)
        {
            fprintf(out, "\n%s\"objState\" : %d,", getStrFromIndent(INDENT_5), frameEvent->srcStates[i]->id);
            fprintf(dispOut, "\"objState\" : %d,", frameEvent->srcStates[i]->id);
        }
        else
        {
            if (frameEvent->srcObjs[i]->id == -2)
            {
                fprintf(out, "\n%s\"name\" : \"%s\",", getStrFromIndent(INDENT_5), frameEvent->srcObjs[i]->name);
                fprintf(dispOut, "\"name\" : \"%s\",", frameEvent->srcObjs[i]->name);
            }
            fprintf(out, "\n%s\"values\" : ", getStrFromIndent(INDENT_5));
            fprintf(dispOut, "\"values\" : ");
            writeVectorValues(out, INDENT_0, frameEvent->srcStates[i]->value.vec_value->type, frameEvent->srcStates[i]->value.vec_value->vector, frameEvent->srcStates[i]->value.vec_value->nCols, dispOut);
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }

        fprintf(out, "\n%s\"idxs\" : [", getStrFromIndent(INDENT_5));
        fprintf(dispOut, "\"idxs\" : [", frameEvent->srcStates[i]->id);

        int numIdxs = frameEvent->numIdxs[i][0];

        if (numIdxs > 0)
        {
            int *idxs = frameEvent->fromIdxs[i];
            for (int j = 0; j < numIdxs; j++)
            {
                int val = idxs[j];
                fprintf(out, "%d", val);
                fprintf(dispOut, "%d", val);
                if (j + 1 < numIdxs)
                {
                    fprintf(out, ",");
                    fprintf(dispOut, ",");
                }
            }
        }
        fprintf(out, "]", getStrFromIndent(INDENT_4));
        fprintf(dispOut, "]");
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_4));
        fprintf(dispOut, "}");

        if (i + 1 < frameEvent->nCols)
        {
            fprintf(out, ",");
            fprintf(dispOut, ",");
        }
    }

    fprintf(out, "\n%s]", getStrFromIndent(INDENT_3));
    fprintf(dispOut, "]");
}

void saveCellChangeEvent(FILE * out, ABD_CELL_CHANGE_EVENT * event, FILE *  dispOut) {
    fprintf(out, "\n%s\"toObj\" : %d,", getStrFromIndent(INDENT_3), event->toObj->id);
    fprintf(out, "\n%s\"toState\" : %d,", getStrFromIndent(INDENT_3), event->toState->id);
    fprintf(out, "\n%s\"origin\" : \"%s\",", getStrFromIndent(INDENT_3), (event->fromType == ABD_E) ? "event" : "obj");

    fprintf(dispOut, "\"toObj\" : %d,", event->toObj->id);
    fprintf(dispOut, "\"toState\" : %d,", event->toState->id);
    fprintf(dispOut, "\"origin\" : \"%s\",", (event->fromType == ABD_E) ? "event" : "obj");

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
            fprintf(out, "\n%s\"fromId\" : %d,", getStrFromIndent(INDENT_3), obj->id);
            fprintf(out, "\n%s\"fromState\" : %d,", getStrFromIndent(INDENT_3), event->fromState->id);

            fprintf(dispOut, "\"ABD\",");
            fprintf(dispOut, "\"fromId\" : %d,", obj->id);
            fprintf(dispOut, "\"fromState\" : %d,", event->fromState->id);
        }
        // void writeVectorValues(FILE *out, int indent, SEXPTYPE type, void *vector, int nElements, FILE *dispOut)

        fprintf(out, "\n%s\"wRows\" : ", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "\"wRows\" :");
        if (event->nRowsIdxs == event->srcDims[0]) {
            fprintf(out, "[],");
            fprintf(dispOut, "[],");
            fprintf(out, "\n%s\"iRows\" : %d", getStrFromIndent(INDENT_3), event->srcDims[0]);
            fprintf(dispOut, "\"iRows\" : %d", event->srcDims[0]);

        }
        else
            writeVectorValues(out, INDENT_0, INTSXP, event->rowsIdxs, event->nRowsIdxs, dispOut);


        fprintf(out, ",");
        fprintf(dispOut, ",");
        fprintf(out, "\n%s\"wCols\" : ", getStrFromIndent(INDENT_3));
        fprintf(dispOut, "\"wCols\" :");
        if (event->nColsIdxs == event->srcDims[1]) {
            fprintf(out, "[]");
            fprintf(dispOut, "[]");
            fprintf(out, "\n%s\"iCols\" : %d", getStrFromIndent(INDENT_3), event->srcDims[1]);
            fprintf(dispOut, "\"iCols\" : %d", event->srcDims[1]);
        }
        else
            writeVectorValues(out, INDENT_0, INTSXP, event->colsIdxs, event->nColsIdxs, dispOut);
    }
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
        fprintf(out, "\n%s\"atFunc\" : %d,", getStrFromIndent(INDENT_2), (currEvent->atFunc == ABD_OBJECT_NOT_FOUND) ? -1 : currEvent->atFunc->id);
        fprintf(out, "\n%s\"atEnv\" : \"%s\",", getStrFromIndent(INDENT_2), envToStr(currEvent->env));
        fprintf(out, "\n%s\"branchDepth\" : %d,", getStrFromIndent(INDENT_2), currEvent->branchDepth);
        fprintf(out, "\n%s\"type\" : ", getStrFromIndent(INDENT_2));

        fprintf(dispOut, "\"%d\" : {", currEvent->id);
        fprintf(dispOut, "\"line\" : %d,", currEvent->scriptLn);
        fprintf(dispOut, "\"atFunc\" : %d,", (currEvent->atFunc == ABD_OBJECT_NOT_FOUND) ? -1 : currEvent->atFunc->id);
        fprintf(dispOut, "\"atEnv\" : \"%s\",", envToStr(currEvent->env));
        fprintf(dispOut, "\"branchDepth\" : %d,", currEvent->branchDepth);
        fprintf(dispOut, "\"type\" : ");

        switch (currEvent->type)
        {
        case IF_EVENT:
            fprintf(out, "\"if_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"if_event\",");
            fprintf(dispOut, "\"data\" : {");

            saveIfEvent(out, currEvent->data.if_event, dispOut);
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
            fprintf(out, "\"arith_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"arith_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveArithEvent(out, currEvent->data.arith_event, dispOut);
            break;
        case VEC_EVENT:
            fprintf(out, "\"vector_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"vector_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveVecEvent(out, currEvent->data.vec_event, dispOut);
            break;
        case IDX_EVENT:
            fprintf(out, "\"idx_change_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"idx_change_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveIdxChangeEvent(out, currEvent->data.idx_event, dispOut);
            break;
        case FOR_EVENT:
            fprintf(out, "\"for_loop_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"for_loop_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveForLoopEvent(out, currEvent->data.for_loop_event, dispOut);
            break;
        case REPEAT_EVENT:
            fprintf(out, "\"repeat_loop_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"repeat_loop_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveRepeatLoopEvent(out, currEvent->data.repeat_loop_event, dispOut);
            break;
        case WHILE_EVENT:
            fprintf(out, "\"while_loop_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"while_loop_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveWhileLoopEvent(out, currEvent->data.while_loop_event, dispOut);
            break;
        case FRAME_EVENT:
            fprintf(out, "\"data_frame_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"data_frame_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveDataFrameEvent(out, currEvent->data.data_frame_event, dispOut);
            break;
        case CELL_EVENT:
            fprintf(out, "\"cell_change_event\",");
            fprintf(out, "\n%s\"data\" : {", getStrFromIndent(INDENT_2));

            fprintf(dispOut, "\"cell_change_event\",");
            fprintf(dispOut, "\"data\" : {");
            saveCellChangeEvent(out, currEvent->data.cell_change_event, dispOut);
            break;
        case BREAK_EVENT:
            fprintf(out, "\"break_event\"");
            fprintf(dispOut, "\"break_event\"");
            goto avoidCurlies;
            break;
        case NEXT_EVENT:
            fprintf(out, "\"next_event\"");
            fprintf(dispOut, "\"next_event\"");
            goto avoidCurlies;
            break;
        default:
            puts("default in json");
            break;
        }
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_2));
        fprintf(dispOut, "}");
        avoidCurlies:;
        fprintf(out, "\n%s}", getStrFromIndent(INDENT_1));
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
    printForVerbose("Persisting collected data");
    FILE *outputFile;
    FILE *dispOutFile;

    dupScript();
    outputFile = openFile(getObjPath(), FILE_OPEN_WRITE);
    dispOutFile = openFile(getJSpath("objects"), FILE_OPEN_WRITE);
    if (outputFile == FILE_NOT_FOUND || dispOutFile == FILE_NOT_FOUND)
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
    outputFile = openFile(getEventsPath(), FILE_OPEN_WRITE);
    dispOutFile = openFile(getJSpath("events"), FILE_OPEN_WRITE);

    if (outputFile == FILE_NOT_FOUND || dispOutFile == FILE_NOT_FOUND)
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
