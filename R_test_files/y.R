options("keep.source" = TRUE)
run <- function(){

    abd_start()
    z <- 600:1
    
    print('--------------')

    f1 <- function(x){
        x <- x*2
        x
    }
    print("-------------------------")
    # i <- 1
    # while (i < 6) {
    #     print(i)
    #     i = i+1
    # }


    x <- 1
    z <- 1
    repeat {
        
        
        repeat {
            print(z)
            if(z==2){
                break
            }
            z <- z+1
        }
        x <- x+1
        if (x == 7){
            break
        }else if(x==5){
            next
        }
    }

    abd_stop()
}

run()