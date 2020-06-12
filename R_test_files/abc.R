options("keep.source"=TRUE)
run <- function(){
    b <- 30:20
    abd_start()
    
    f1 <- function(x){
        f1_a <- x*2
        f1_a
    }

    
    c <- 10:5
    a <- 1:10

    a[1:3] <- b[1:3]
    a[1:3] <- c[1]
    a[4] <- f1(10)
    a[5:10] <- 2*(2+1)
    abd_stop()
}

run()