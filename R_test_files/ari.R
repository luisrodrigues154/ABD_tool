options("keep.source"=TRUE)
run <- function(){

    abd_start()
    
    a <- 1:10
    b <- 10:20

    result <- a[1] * b[2]
    abd_stop()

}
run()