options("keep.source" = TRUE)
run <- function(){

    abd_start()

    f1 <- function(x){
        x <- x*2
        x
    }
    
    # i <- 1
    # while (i < 6) {
    #     print(i)
    #     i = i+1
    # }

    
    x <- 1
    y <- 1:3
    print("-------------------------")
    # while(x < 6){
    #     x <- x+1
    #     if(x < 4){
    #         next
    #     }
    # }
    for(c in y){
        # x <- x + c  <- ERRO DE ARITH AQUI (maybe so com objs)
        while(c<4){
            c <- c+1
        }
        x <- c
    }
    # repeat{
    #     if(x == 2){
    #         break
    #     }
    #     x <- x + 1
    # }

    abd_stop()
}

run()