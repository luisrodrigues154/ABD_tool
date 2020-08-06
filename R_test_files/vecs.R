options("keep.source"=TRUE)
run <- function(){

	abd_start()

	vec1 <- 1:5
	vec2 <- 9:13
	vec1[1] <- vec2[2]*vec1[1]*3
	abd_stop()

}
run()