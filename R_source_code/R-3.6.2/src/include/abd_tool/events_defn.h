

#ifndef loaded_ev
#define loaded_ev
typedef enum abd_event_types
{
    MAIN_EVENT = 0,
    IF_EVENT = 1,
    FUNC_EVENT = 2,
    RET_EVENT = 3,
    ASGN_EVENT = 4,
    ARITH_EVENT = 5,
    VEC_EVENT = 6
} ABD_EVENT_TYPE;

//describe a if statement and its else ifs
typedef enum if_type
{
    IF_EXPR = 0,
    IF_ABD = 1
} IF_DATA_TYPE;

typedef struct if_abd_obj
{
    ABD_OBJECT *objPtr;
    /* 
        the objValue can assume different things depending on the flag that it has inside it.
        the flag itself is idxChange
        if(idxChange)
            - objValue.id translates to the the state the object was used and not the actual modList value
            - the idxs vector will indicate which index was used (always size 1)
            - the vector index will indicate which was the value for that position doing the needed roolbacks
                * the roll is basically, find the effective value for that index based on the history 
                * which means that if, going back until the declaration of the whole vector,
                * there'is no changes to that index, the value assumed is the initially declared one,
                * otherwise the value is the first modifcation found      
    */
    ABD_OBJECT_MOD *objValue;
} IF_ABD_OBJ;

typedef struct if_expr
{
    short exprId;
    char *operator; // '>', '<', '|' , '+', '-' , (...)
    int isConfined; // 1 - True, 0 - False -> indicates if is inside parentheses
    double result;  // 1 - True; 0 - False (if operator is arith then is the result of arith)
    /* 
            in the end, the left_data and right_data will have the type IF_ABD 
        */
    IF_DATA_TYPE left_type;  // indicates the type of data in the left_data var
    IF_DATA_TYPE right_type; // indicates the type of data in the left_data var

    void *left_data;  // left value of expression
    void *right_data; // right value of expression
} IF_EXPRESSION;

typedef struct if_event
{
    short globalResult; // 'T' or 'F' for the statement as a whole
    short isElseIf;
    short isElse;        // 'T' or 'F' indicates if the statement is an ELSE
    IF_EXPRESSION *expr; // if or else if
    char *exprStr;
} ABD_IF_EVENT;

//describe a function call and its arguments
typedef struct abd_event_arg
{
    ABD_OBJECT *fromObj;
    ABD_OBJECT *toObj;
    ABD_OBJECT_MOD *passedValue;
    ABD_OBJECT_MOD *rcvdValue;
    struct abd_event_arg *nextArg;
} ABD_EVENT_ARG;

typedef struct abd_func_event
{
    SEXP toEnv;
    ABD_OBJECT *caller;
    ABD_OBJECT *called;
    ABD_EVENT_ARG *args;
} ABD_FUNC_EVENT;

//describe return events
typedef struct abd_return
{
    SEXP toEnv;
    ABD_OBJECT *toObj;
    ABD_OBJECT_MOD *retValue;
} ABD_RET_EVENT;

//assignment event

typedef enum
{
    ABD_O = 0, /* is an ABD_OBJECT */
    ABD_E = 1  /* is an ABD_EVENT */
} ASSIGN_DATA_TYPE;

typedef struct abd_assign
{
    ABD_OBJECT *toObj;
    ASSIGN_DATA_TYPE fromType;
    /*
        The variable fromObj will contain either a poiter to an ABD_EVENT or an ABD_OBJECT.

        Denoting that, for example, if we have the following expression (ex:1):

        ex 1: a <- 10

        The ABD_OBJECT that *fromObj will point to will have an id -1 indicating that it was
        hardcoded in script.

        In other hand we can have an assignment from a variable that was not declared 
        inside the tool itself, so, in this case (ex:2), the ABD_OBJECT id will be -2.

        ex 2: a <- b

        finally we can have a pointer to an actual mapped object, in this case
        the fromObj ABD_OBJECT will have a id>0 value.

        The pointer value (ABD_OBJECT_MOD *) will point to the state in which the object was used
        and so, to view the actual value in that state, it might need to be reconstructed (vectors/matrices)
    */
    void *fromObj;
    /* 
        The variable below stores the state in which the value was used.
        Exemple:
        - b <- c(1,2,3)
        - b[1] <- 10
        - a <- b

        - value will contain the ABD_OBJECT_MOD referent to the modification which,
            says that, we used b in its state with id=2.
        - For posterior analysis, the vector (currently being [10,2,3]) needs to be reconstructed    

    */
    int withIndex;
    ABD_OBJECT_MOD *value;
    ABD_OBJECT_MOD *fromState;
} ABD_ASSIGN_EVENT;

//arithmetic event

typedef struct arith_event
{
    /* 
        The internals of ABD_IF_EVENT can be reused to save the sake of time and 
        elf size
    
        example:
            a <- (2+3) * 2
    */

    double globalResult; // the final result that
    IF_EXPRESSION *expr; // from example this would be 2+3
} ABD_ARITH_EVENT;

typedef struct vec_event
{
    int nElements;
    /* 
        if created by hand, fromObj = ABD_OBJECT_NOT_FOUND 
        if unscoped, fromOBJ->id = -2
        if legit obj, fromOBJ->id > 0
    */
    ABD_OBJECT *fromObj;
    /* 
        The vars below are useful to express the following: vec[1:3]
        rangeL = 1
        rangeR = 3
     */
    int rangeL; // inclusive
    int rangeR; // inclusive

    /* what was the state when the object was used... (state==ID)*/
    ABD_OBJECT_MOD *fromState;
    ABD_OBJECT *toObj;
} ABD_VEC_EVENT;

typedef struct idx_change_event
{
    ABD_OBJECT *toObj;
    //in the state contains the indexes
    ABD_OBJECT_MOD *toState;
    ABD_OBJECT *fromObj;
    int *fromIdxs;
} ABD_IDX_CHANGE_EVENT;

#define ABD_EVENT_NOT_FOUND NULL
#endif