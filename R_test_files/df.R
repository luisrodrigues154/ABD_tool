options("keep.source"=TRUE)
run <- function(){
    
    
    df2 <- data.frame("c1" = 6:12, "c2" = 9:15)
    vec <- 10:1
    print(df2)
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
    df <- data.frame("col1" = 1:5, "col2" = 10:6)
    #colValue <- 99

    #df$c1 = df2$c2
    #df$c1[1:2] <- 91
    #df[1] <- df2$c2
    df[,] <- 1:12
    #df[1:3,1:2] <- df2$c2[3:1]
    #df[1:3,1:2] <- df2[]
    #df[1:3,1:2] <- df2$c2
    #df[1:3,1:2] <- df2[2:1]
    #df[1:3,1:2] <- df2[,]
    # df[1:3,1:2] <- df2[,2:1]
    #df[1:3,1:2] <- df2$c2[1:2]
    #df[1:3,1:2] <- df2[2:1,1:1]
    

    #df[1:3,1:2] <- df2[1:3, 1:2]
    #df[1] <- 10 #(assign to all cells in col1)
    #df$col2 <- 10 #(assign to all cells in col1)

    #df <- data.frame(1:20, 20:1, 30:11)
    #df$col2 = 99
    print(df)
    
    # l <- list("a" = 2.5, "b" = TRUE, "c" = 1:3)
    # print(l)
    
    #abd_stop()

}
run()