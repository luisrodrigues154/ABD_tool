options("keep.source"=TRUE)
run <- function(){
    abd_start()
    a <- 20
    a <- 40
    x <- a

    f1 <- function(x){
        f1_a <- x
        f1_a
    }
    #
    z <- f1(10) 
    z2 <- f1(12)
    b <- c(1,2,3,4)
    abd_stop()

}

run()
