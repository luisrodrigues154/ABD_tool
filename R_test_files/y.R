options("keep.source" = TRUE)
run <- function(){

    abd_start()

    
    # i <- 1
    # while (i < 6) {
    #     print(i)
    #     i = i+1
    # }
    print("-------------------------")
    # while(x < 6){
    #     x <- x+1
    #     if(x < 4){
    #         next
    #     }
    # }
    x <- 1:10
    z <- 1:10
    for(c in z){
        # x <- x + c  <- ERRO DE ARITH AQUI (maybe so com objs)
        if(c<5000){
            x[c] <- c * 2  
        }
        
    }
    print("finalized")
    # repeat{
    #     if(x == 2){
    #         break
    #     }
    #     x <- x + 1
    # }

    abd_stop()
}

run()