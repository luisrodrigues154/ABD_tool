options("keep.source"=TRUE)
run <- function(){

    abd_start()
    vec2 <- 100:110
    vec <- 30:1
    # m <- matrix(1:10, nrow=2)
    vec[1:20] <- vec2[1:3]
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
    df <- data.frame("c1" = 1:5, "c2" = 10:6, "c3" = 10:6)
    #colValue <- 99
    df2 <- data.frame("c1" = 6:12, "c2" = 9:15 )
    #df$c1 = df2$c2 # col -> col ... need to be same
    #df$c1 <- 91 # value ... applied to all cells
    #df[1:2] <- df2$c2 # col -> col ... need to be same
    # if to all frame, need to be the size of each col or multiple (THROW ERROR if multiple does not fetch)
    #vec[1] = 10*2
    #df[1:3,1:2] <- 10
    #print("_______________")
    #print(df2$c2[6:1])
    #df[1:3,1:2] <- 1:6 #srcNRows and Ncols == destNrows and Ncols
    #df[1:2,1:2] <- 8:1 #all src and src > size dest, copy to dest cells (JUST WARNING)
    df[1:3,2:3] <- df2["c2"] #just col...  srcValues > dest needed cells, copy until end to needed cells sequentially (THROW ERROR)

    #df[1:3,1:2] <- df2[2:1]
    #df[1:3,1:2] <- df2[,]
    #df[1:3,1:2] <- df2[,2:1]
    #df[1:3,1:2] <- df2$c2[1:2]
    #df[1:3,1:2] <- df2[3:1,1:2]


    #df[1:3,1:2] <- df2[1:3, 1:2]
    #df[1] <- 10 #(assign to all cells in col1)
    #df$col2 <- 10 #(assign to all cells in col1)

    #df <- data.frame(1:20, 20:1, 30:11)
    #df$col2 = 99

    print(df)
    # l <- list("a" = 2.5, "b" = TRUE, "c" = 1:3)
    # print(l)

    abd_stop()

}
run()
