

#ifndef loaded_ev
    #define loaded_ev
    typedef enum abd_event_types {
        MAIN_EVENT = 0,
        IF_EVENT = 1,
        FUNC_EVENT = 2,
        RET_EVENT = 3
    }ABD_EVENT_TYPE;

    //event structures

    //describe a if statement and its else ifs 
    typedef enum if_type {
        IF_EXPR = 0,
        IF_ABD = 1
    }IF_DATA_TYPE;

    typedef struct if_abd_obj{
        ABD_OBJECT * objPtr;
        ABD_OBJECT_MOD * objValue;
    }IF_ABD_OBJ;
    
    typedef struct if_expr{
        char * operator; // '>', '<', '|' , '+', '-' , (...)
        int isConfined; // 1 - True, 0 - False -> indicates if is inside parentheses
        double result; // 1 - True; 0 - False (if operator is arith then is the result of arith)
        
        /* 
            in the end, the left_data and right_data will have the type IF_ABD 
        */
        IF_DATA_TYPE left_type; // indicates the type of data in the left_data var
        IF_DATA_TYPE right_type;// indicates the type of data in the left_data var

        void * left_data; // left value of expression
        void * right_data; // right value of expression
    }IF_EXPRESSION;


    typedef struct if_event{
        int globalResult; // 'T' or 'F' for the statement as a whole
        IF_EXPRESSION * expr; // if or else if
        struct if_content * else_if; // pointer to an else if that was originated from this if
        int reachedElse; // 'T' or 'F'
    }ABD_IF_EVENT;

    //describe a function call and its arguments
    typedef struct abd_event_arg{
        ABD_OBJECT * objPtr;
        char * rcvdName;
        ABD_OBJECT_MOD * objValue;
        struct abd_event_arg * nextArg;
    }ABD_EVENT_ARG;

    
    typedef struct abd_func_event{
        ABD_OBJECT * caller;
        ABD_OBJECT * called;
        ABD_EVENT_ARG * args;
    }ABD_FUNC_EVENT;

    //describe return events
    typedef struct abd_return{
        ABD_OBJECT * from;
        ABD_OBJECT * toObj;
        ABD_OBJECT_MOD * retValue;
    }ABD_RET_EVENT;

    #define ABD_EVENT_NOT_FOUND NULL
#endif    