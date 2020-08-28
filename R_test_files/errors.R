options("keep.source" = TRUE)
run <- function(){
    #df <- data.frame(1:2, 1:3)
    abd_stop(0)
    abd_verbose(1)
    abd_start()

    add <- function(x, y){
        result <- x+y
        result <- test(result)
        result
    }
    v1 <- 1:10
    v2 <- 2:9999

    

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
    df[1:3,2:3] <- df2["c2"]
    result <- add(1,2)
}   

run()