options("keep.source"=TRUE)
run <- function(){
    
    
    abd_start()
    
    # m <- matrix(1:10, nrow=2)
    
    # print(m)
    # vec <- 10:40
    # a <- 11:1
    # b <- 50:70
    print(" ")
    print(" ")
    print(" ")
    
    print("---------------------")
    #df <- data.frame(1:20, 20:1, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11)
    #df <- data.frame( "c2" = b, "c3" = vec[21:1])
    df <- data.frame("c1" = 1:5, "c2" = 10:6)

    df$c1[1] <- 91
    df[1,2] <- 92

    #df <- data.frame(1:20, 20:1, 30:11)
    print(df)
    
    # l <- list("a" = 2.5, "b" = TRUE, "c" = 1:3)
    # print(l)
    
    #abd_stop()

}
run()