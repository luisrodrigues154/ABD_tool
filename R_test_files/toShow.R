options("keep.source" = TRUE)
run <- function(){
    abd_verbose(1)
    #abd_stop(1)

    abd_start()

    v1 <- 4
    v2 <- 1

    addThis <- function(x,y){
        result <- x + y
        result
    }

    subTwo <- function(x){
        result <- x -2
        result
    }

    dumbThing <- function(x, y){
        first <- addThis(x,y)
        second <- subTwo(2)
        result <- first + second
        result
    }

    dumbResult <- dumbThing(v1,1)

    df <- data.frame("col1" = 1:5, "col2" = 10:6, "c3" = 10:6)
    df2 <- data.frame("c1" = 6:12, "c2" = 9:15 )
    if(dumbResult < 5){
        print("smaller")
    }else if(dumbResult > 5){
        print("bigger")
    }else{
        print("oh its 5")
        df[1:3, 2] <- dumbResult
    }

    df[1:3,2:3] <- df2["c2"] 

    finalThing <- addThis(1, "ThisShouldThrowAnError")

    abd_stop()
}
run()