objOut <- 33

start_watcher()

f1 <- function(x, y){
    print("here")
    f1_a <- x+10
}
f2 <- function(x){
    print("executed")
    30
}

main_a <- f2()
f1(main_a, 10)

stop_watcher()