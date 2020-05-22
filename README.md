# "Automatic" Bug Detection in R (ABD_tool)
Tool to help in R debugging

##Tool files
###All files src/include/abd_tool
###Displayer path: ABD_tool\R_source_code\R-3.6.2\src\include\abd_tool\displayer

##Configuration

Ubuntu:
./configure --without-recommended-packages 

if gettext error thrown use flag: --with-included-gettext

MacOs:
./configure --without-recommended-packages 

Make
make clean all && make -j4 
