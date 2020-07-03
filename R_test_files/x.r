options("keep.source" = TRUE)
run <- function(){

    abd_start()
    z <- 5:1
    zz <- 1:2
    
    b <- 999999
    c <- 1:20

    f1 <- function(x){
        f1_x <- x+10
        f1_x
    }

    print("-------------------------")
    for(x in z){
        for(xx in zz){
            idx <- x+1
            if(x==1 && xx ==2){
                c[idx] <- f1(xx)
            }
        }
    }
    #z[1:3] <- f1(2)
    
    # teste <- 1:1000000


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