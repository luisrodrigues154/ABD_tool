options("keep.source" = TRUE)
run <- function(){

    abd_start()
    abd_stop(1)
    
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

    x[1] <- 3
    x[2:4] <- 10
    # repeat{
    #     if(x == 2){
    #         break
    #     }
    #     x <- x + 1
    # }

    abd_stop()
}

run()