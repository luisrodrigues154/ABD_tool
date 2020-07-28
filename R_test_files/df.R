options("keep.source"=TRUE)
run <- function(){
    
    
    abd_start()
    
    # m <- matrix(1:10, nrow=2)
    
    # print(m)
    vec <- 10:40
    a <- 10:5
    b <- 20:25
    print(" ")
    print(" ")
    print(" ")
    
    print("---------------------")
    #df <- data.frame(1:20, 20:1, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11, 30:11)
    df <- data.frame("c1" = a , "c2" = b, "c3" = 5:10)

    #df <- data.frame(1:20, 20:1, 30:11)
    print(df)
    
    # l <- list("a" = 2.5, "b" = TRUE, "c" = 1:3)
    # print(l)
    
    abd_stop()

}
run()