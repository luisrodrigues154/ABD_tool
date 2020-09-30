options("keep.source" = TRUE)
run <- function(){
    abd_start()
    start_time <- Sys.time()

    for(i in 1:1000000){
        if(i<1000){
            print("smaller 1")
        }else if(i<10000){
            print("smaller 2")
        }else if(i<400000){
            print("smaller 3")
        }else if(i<600000){
            print("smaller 4")
        }else if(i<700000){
            print("smaller 5")
        }else if(i<900000){
            print("smaller 6")
        }else if(i<1000000){
            print("smaller 7")
        }else{
            print("equal")
        }
    }

    end_time <- Sys.time()
    print(paste0("Start time: ", start_time))
    print(paste0("End time: ", end_time))
    print(paste0("Diff: ", end_time - start_time))
}

run()