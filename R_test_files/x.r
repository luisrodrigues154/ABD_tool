options("keep.source" = TRUE)
run <- function(){

    abd_start()
    z <- 600:1
    
    print('--------------')

    f1 <- function(x){
        x <- x*2
        x
    }
    print("-------------------------")
    
    c <- 1:20
    c[1] <- 2+3
    for(x in z){
        print("---")
        if(x == 599){
            print("will next")
            next
        }
        if(x == 598){
            break
        }

        if(x<3){
            c[x] <- f1(x)
            c[x] <- 10
        }else{
            c[x] <- x+1
        }
        
    }
    z[1:3] <- f1(2)
    b <- 4+2*(90-60)
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