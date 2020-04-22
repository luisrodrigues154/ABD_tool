p <- function(x = NULL, max.lines = getOption("deparse.max.lines")){
    print("just got called")
    n <- length(x <- .traceback(x))
    if(n == 0L)
        cat(gettext("No traceback available"), "\n")
    else {
        for(i in 1L:n) {
            xi <- x[[i]]
            label <- paste0(n-i+1L, ": ")
            m <- length(xi)
            ## Find source location (NULL if not available)
            srcloc <- if (!is.null(srcref <- attr(xi, "srcref"))) {
                srcfile <- attr(srcref, "srcfile")
                paste0(" at ", basename(srcfile$filename), "#", srcref[1L])
            }
            ## Truncate deparsed code (destroys attributes of xi)
            if(is.numeric(max.lines) && max.lines > 0L && max.lines < m) {
                xi <- c(xi[seq_len(max.lines)], " ...")
                m <- length(xi)
            }
            if (!is.null(srcloc)) {
                xi[m] <- paste0(xi[m], srcloc)
            }
            if(m > 1)
                label <- c(label, rep(substr("          ", 1L,
                                             nchar(label, type="w")),
                                      m - 1L))
            cat(paste0(label, xi), sep="\n")
        }
    }
}
f <- function(x){
    x^2
    p()
}

f(10)
