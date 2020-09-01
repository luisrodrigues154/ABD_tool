options("keep.source" = TRUE)
run <- function(){
    abd_stop(1)
    abd_verbose(1)
    abd_start()
    df <- read.csv(file="file.csv")
    print(df)
    abd_stop()
}   

run()