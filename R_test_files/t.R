options("keep.source"=TRUE)
run <- function(){
    a <- 10
    abd_start()
    
    
    b <- c(1,2,3,4,5)

    b[1] <- 20
    b[2] <- 30
    if( (b[3]>a) && (b[1]+3 > 4) ){
        print("else")
    }else if(a>20)
    {
        print("abc")
    }
    else{
        x <- 20
    }

    abd_stop()

}
run()