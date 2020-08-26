options("keep.source" = TRUE)
run <- function(){

    abd_start()
    abd_stop(0)

    v1 <- 99995:99999
    v2 <- 99999:99995
    result <- v1[1] + v2[1]
    if((v1[1] + v2[1] - 1) >= 99){
        v1[1] <- 99
        v2[1] <- 1
    }else{
        v1[1] <- 1
        v2[1] <- 10
    }

    abd_stop()
}

run()