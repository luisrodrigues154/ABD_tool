options("keep.source"=TRUE)
run <- function(){
    abd_start()
    
    a <- c(1,2,3,4,5)
    b <- c(11,12,13)
    c <- 1:3

    
    #
    #
    a[c] <- b[c]
    # will create a vector with the new values
    #
    #
    a[c] <- b[3]
    #a única coisa que faz, é gerar um vector para os indexes
    #que vao ser alterados
    #print("-------------")
    #a[2] <- 10
    # não cria nenhum vecotr
    #print("-------------")
    #a[c] <- b
    #print("A values")
    #print(a)
    #não gera vector nenhum
    #print("-------------")
    #a[1:3] <- b[3:1]
    #cria 3 vectores
    #vector 1: os indices usados do vector B 
    #vector 2: os valores de B
    #vector 3: os indices usados do vector A
    #print("-------------")
    #a[1:3] <- 3:1
    #cria 2 vectores
    #vector 1: os novos valores
    #vector 2: os indices usados do vector A

    #print(paste0("a: ", a))

    #abd_stop()

}
run()