options("keep.source"=TRUE)
run <- function(){
    
    a <- 10:5
    abd_start()
    
    # m <- matrix(1:10, nrow=2)
    
    # print(m)
    
    b <- 20:25
    print(" ")
    print(" ")
    print(" ")
    
    print("---------------------")
    #df <- data.frame(1:20, 20:1, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11)
    df <- data.frame("c1" = a , "c2" = b)

    #df <- data.frame(1:20, 20:1, 30:11)
    print(df)
    
    # l <- list("a" = 2.5, "b" = TRUE, "c" = 1:3)
    # print(l)
    
    abd_stop()

}
run()