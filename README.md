# [ABD_Tool] - "Automatic" Bug Detection for R

This is an academic tool that aims to improve debugging in R scripts based on a one-time run.


# Files
Files for the tool are all contained in a folder under the path:
../R_source_code/R-3.6.2/src/include/abd_tool

## How to run
```
cd ../R_source_code/R-3.6.2
./configure
make
```

- add the folder R_source_code/R-3.6.2/bin to the PATH environment. 
- Run by using Rscript (with code constraints) or using R (combined with source("yourScript.R"))
### Make errors
If make'ing the source code throws an error, some flags need to be added to the ./configure step.

- Ubuntu</br>
 ./configure --without-recommended-packages -with-included-gettext

- MacOs</br>
 ./configure --without-recommended-packages

- Make (both, Ubuntu and MacOs) </br>
 make clean all && make -j4

## How to Use
To use the tool, just use **abd_start()** and **abd_stop()** between the code that needs to be analyzed.

Example:
```
abd_start()
a<-20
(...)
abd_stop()
```

For more information about the functionalities, issue **abd_help()** at the terminal (after running R).
## Code Constraints

Due to R implementation, and to take full advantage of the tool, running the script with Rscript has its constraints.

The code must contain the following structure :
```
options("keep.source"=TRUE)
(function(){
	#your code here
})()
```
**Note 1:** Notice that between the first two lines there's no space. The first line should be at the first line of the script (literally)</br>
**Note 2:** Using the function source("script.R"), there's no need to have this kind of structure.

