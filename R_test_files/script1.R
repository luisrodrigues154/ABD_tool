options("keep.source"=TRUE)
run <- function(){
    abd_stop(1)
    x <- 20
    abd_start()

    j <- 30
    a <- 1
    j <- 34
    j <- a
    j <- 32
    j <- a
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

    f4 <- function(){
        f4_abc <- 10
        f4_abc
    }
    abcd <- (30*2)+10*1
    ret2 <- f1(a,b,1,2,j,4)
    if((ret2 > 10) ||(ret2 > x+10)){
        print("maior")
    }else if(ret2 == 11){
        print("qq coisa")
    }else{
        if(1==1){
            result <- f2(10,11)
        }
    }
    

    j[2:5] <- f3(10)

    f4()

    abd_stop()
}

run()