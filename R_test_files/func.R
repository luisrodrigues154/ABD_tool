options("keep.source"=TRUE)
run <- function(){

    abd_start()
    
    a <- 10
    b <- 20

    sub <- function(x){
        result <- x - 3
        result
    }
    add <- function(x, y){
        result <- x + y
        result <- sub(result)
        result  

    }

    add(a,b)
    vec <- 1:10
    vec2 <- 10:1
    for(c in vec){
        vec2[c] <- c
    }





    abd_stop()

}
run()