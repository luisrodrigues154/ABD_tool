options("keep.source"=TRUE)
run <- function(){
    a <- 1
    abd_start()
    
    b <- 10
    c <- 30
    vec2 <- c(1,2,3,4)
    vec <- 1:10

    vec[2] <- as.integer(999)
    print("############")
    vec2[1] <- 9999

    abd_stop()
}
run()
