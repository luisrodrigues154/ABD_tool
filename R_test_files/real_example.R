options("keep.source"=TRUE)
run <- function(){

    abd_start()
    start_time <- Sys.time()

    df <- read.csv(file = "file.csv")

    total_profit <- sum(df[,"Total.Profit"])
    print(paste0("Total profit: ", total_profit))

    average_profit <- mean(df[,"Total.Profit"])
    print(paste0("Average profit: ", average_profit))

    min_index <- which.min(df[,"Total.Profit"])
    print("Record with lowest profit: ")
    print(df[min_index, ])

    max_index <- which.max(df[,"Total.Profit"])
    print("Record with highest profit: ")
    print(df[max_index, ])
    
    

    end_time <- Sys.time()
    print(paste0("Start time: ", start_time))
    print(paste0("End time: ", end_time))
    print(paste0("Diff: ", end_time - start_time))
}

run()