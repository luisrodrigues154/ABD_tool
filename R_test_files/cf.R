options("keep.source"=TRUE)
run <- function(){
    a <- 1
    abd_start()
    f1 <- function(x, y, z){
        result <- y+z
        result <- 40/(10*2)
        result
    }
    #
    
    b <- 10
    c <- 30

    result <- f1(a,b,c)
    print(paste0("result is: ", result))

    abd_stop()
}
run()
