code=JSON.parse('["options(\'keep.source\'=TRUE)","run <- function(){","","    abd_start()","","    a <- 1","    b <- 2","    ","    ","","    f1 <- function(x,y){","        f1_a <- x","        f1_b <- y","        f1_b","    }","","    f2 <- function(z,b){","        a<-z","        b<-b","        c <- 10","        c","    }","","    ret2 <- f1(a,b)","","    #f2(10,11)","","    abd_stop()","}","","run()"]')