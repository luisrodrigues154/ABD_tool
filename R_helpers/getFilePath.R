full.fpath <- tryCatch(normalizePath(parent.frame(2)$ofile),  # works when using source
               error=function(e) # works when using R CMD
                     normalizePath(unlist(strsplit(commandArgs()[grep('^--file=', commandArgs())], '='))[2]))
filePath <- dirname(full.fpath)
args <- commandArgs()
fileArg <- args[grepl("^--file.*\\.R", args)]
fileName <- gsub("^.*?=","",fileArg)
fullPath <- paste0(filePath, "/", fileName)