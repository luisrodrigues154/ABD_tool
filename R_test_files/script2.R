options("keep.source"=TRUE)
run <- function(){
abd_start()

f1 <-function(){
    f1_a <- 10
    f1_a
}
a <- f1()
b <- f1()

abd_stop()
}

run()

