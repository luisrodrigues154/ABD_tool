options("keep.source" = TRUE)
run <- function(){

    abd_start()
    z <- 10:1
    a <- 1:4
    b <- 3:1
    c <- 1:10
    print('--------------')

    f1 <- function(x){
        x <- x*2
        x
    }

    for(x in z){
        if(x<3){
            c[x] <- f1(x)
            c[x] <- 10
        }else{
            c[x] <- x+1
        }
    }
    
    teste <- 1:1000000
    abd_stop()
    # with c() create two vectors
    # vector 1 - indexes
    # vector 2 - values
    #for(x in z[c(1,2,3)]){
        
    #}
    #print("--------------")

    #for(x in z[3:1]){
    #    print("cycle 3")
    #}
    #print("--------------")

    #for(x in 1:3){
    #    print("cycle 4")
    #}



    

    
}

run()