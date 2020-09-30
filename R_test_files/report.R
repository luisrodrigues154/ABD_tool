options("keep.source"=TRUE)
run <- function(){

    abd_start()

    add_ <- function(x, y){
        result <- x + y
        result
    }

    sub_ <- function(x, y){
        result <- x - y
        result
    }

    vec <- 1:10
    ari <- (2 * 3) + 2
    obj <- 1
    obj2 <- 2

    result <- add_(obj, obj2)
    result2 <- sub_(obj, obj2)


    abd_stop()

}

run()