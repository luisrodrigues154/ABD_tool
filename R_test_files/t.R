options("keep.source"=TRUE)
run <- function(){
    a <- 20
    abd_start()
    
    f1 <- function(x, y){
        x <- 10
        y <- 30
        x <- f2()
        x
    }

    f2 <- function(){
        f2_a <- 30
        f2_a
    }
    

    b <- c(1,2,3,4,5)

    b[1] <- 20
    b[2] <- 30
    if( (b[3]>a) && (b[1]+3 > 4) ){
        print("else")
    }else if(a>20)
    {
        abcd <- 30
        abcd <- 30
        abcd <- 30
        abcd <- 30
        abcd <- 30
    }
    else{
        blabla <- 10
        if(a>2){
            xxx <- 30
        }
        x <- 20
        x <- 30
        x <- 99
    }

    val1 <- 10000
    result <- f1(val1, xxx)
    abd_stop()

}
run()