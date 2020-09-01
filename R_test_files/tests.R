options("keep.source" = TRUE)
run <- function(){
    abd_verbose(1)
    abd_stop(0)
    abd_start()

    w = rnorm(500,0,1)  # 500 N(0,1) variates
    print(w)
    v = filter(w, sides=2, rep(1/3,3))
    print(v)
    abd_stop()
}
run()