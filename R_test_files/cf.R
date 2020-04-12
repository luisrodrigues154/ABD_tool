
b <- 40
start_watcher()

f1 <- function(x,y,z){
    f1_a <- 10
    f2_a <- f1_a + x
    f2_a
}
a <- 30
result <- f1(a,b, 10)
stop_watcher()

