#include <stdio.h>
#include <R.h>
#include <Internal.h>
#include <Rinternals.h>
#include <Defn.h>
#include <stdlib.h>
#include <string.h>

#ifndef loaded_warnerr
#define loaded_warnerr

typedef struct abd_warn{
    int id;
    char * message;
    struct abd_warn * prevWarning;
}ABD_WARNINGS;

typedef struct abd_error{
    int line;
    char * message;
    char * exprStr;
    SEXP atEnv;
    ABD_OBJECT * atFunc;
}ABD_ERRORS;

static ABD_ERRORS * currError;
static ABD_WARNINGS * currWarning;
static int warnCount;

#define ABD_ERR_DEF "No call found"
#endif

void initVars();
int getWarnCount();

// ----------------
void clearWarnings();
void storeWarningSignal(const char * message);
ABD_WARNINGS * memAllocWarning();
ABD_WARNINGS * getWarnings();

void storeErrorSignal(SEXP call, char * message);
ABD_ERRORS * getError();
ABD_ERRORS *  memAllocError();