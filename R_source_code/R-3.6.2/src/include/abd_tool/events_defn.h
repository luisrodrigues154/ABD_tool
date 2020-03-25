

#ifndef loaded_ev
    #define loaded_ev
    typedef enum abd_event_types {
        MAIN_EVENT = 0,
        IF_EVENT = 1,
        FUNC_EVENT = 2,
        RET_EVENT = 3
    }ABD_EVENT_TYPE;

    //event types structures
    typedef struct abd_if_event{
        //TODO
        //analyze R behaviour when if'ing
    }ABD_IF_EVENT;

    typedef struct abd_event_arg{
        ABD_OBJECT * objPtr;
        char * rcvdName;
        ABD_OBJECT_MOD * objValue;
        struct abd_event_arg * nextArg;
    }ABD_EVENT_ARG;

    //register function events
    typedef struct abd_func_event{
        ABD_OBJECT * caller;
        ABD_OBJECT * called;
        ABD_EVENT_ARG * args;
    }ABD_FUNC_EVENT;

    //register return events
    typedef struct abd_func_return{
        ABD_OBJECT * from;
        ABD_OBJECT * toObj;
        ABD_OBJECT_MOD * retValue;
    }ABD_RET_EVENT;

    #define ABD_EVENT_NOT_FOUND NULL
#endif    