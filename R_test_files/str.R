options("keep.source" = TRUE)
run <- function(){

    abd_start()
    #a <- "string 1"

    b <- c("123", "456")
    b[1] <- "teste"

    
    abd_stop()
}

run()