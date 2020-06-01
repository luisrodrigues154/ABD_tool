options("keep.source"=TRUE)
run <- function(){

    abd_start()

    j <- 30
    a <- 1
    j <- 34
    j <- a
    j <- 32
    j <- j
    j <- 22:27
    b <- 2
    j <- 1:40
    

    f1 <- function(x,y,z,t,r,j){
        f1_a <- x
        f1_b <- y
    }

    f2 <- function(z,b){
        a<-z
        b<-b
        c <- 10
    }

    f3 <- function(xx){
        a <- xx
        b <- a
        c <- f2(a,b)
    }
    ret2 <- f1(a,b,1,2,3,4)

    result <- f2(10,11)

    result3 <- f3(10)
    abd_stop()
}

run()