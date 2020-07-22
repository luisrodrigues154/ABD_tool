# [ABD_Tool] - "Automatic" Bug Detection for R

This is an academic tool that aims to improve debugging in R scripts based on a one-time run.


# Files
Files for the tool are in the folder under the path:
```
../abd_installer/files/abd_tool
```

Files that are patched (replaced) are in the folder under the path:
```
../abd_installer/files/
```

## How to install
Before installing make sure that all the packages that R needs are installed.</br>
If you dont know what packages are needed do the following (this will use the unmodified R files from cran repo that are included in this repository):

### Dependencies verification (not required, but recommended)
```
cd R_source_code
mkdir aux
tar xfvz R-4.0.2.tar.gz -C aux/
cd aux
/tools/rsync-recommended
./configure 
```
R will verify if you have the required packages, if not, an error is thrown with the name of the missing package. Go ahead and install them through your package manager</br>
**Note:** if pcre is not installed, make sure to install version 2 instead of 1 (ver > 4.0 migrated to pcre2) </br>
At this point, when all the packages are installed, install openjdk if not installed. </br>

Now issue:
```
make
```
If it finished without errors, this means that all the base system packages required to run base R are installed. </br>
Now remove the auxiliar files:
```
cd ..
rm -rf aux
tar xfvz R-4.0.2.tar.gz -C /desired/path
```
**Note:** This path will be the final path where the R source files will be stored. </br>

### Installing ABD_Tool

#### Using the installer
Now that the dependencies requirements are met, go ahed and check the help option of the installer. </br>

```
cd abd_installer
python abd_inst.py -h
```

Available commands</br>
        -h : Display this information</br>
        -r : Display the requirements to use ABD</br>
        -p : Specifies the R sources path</br>
        -c : Bypass checksum verification</br>
        -m : Configure and make after installation</br>



```
cd abd_installer
python abd_inst.py -p path/to/r_sources -m
```
**Note:** -p sources the path to the folder where r was extracted, -m starts the make process after installation </br>
**Note 2:** If you made changes to the R files, use -c to bypass checksum verification</br>

During the installation, some key presses are required</br>
At the end, you are prompted to decide if the installer adds R and Rscript to the PATH variable (to be accessible system wide), <b>Default is NO. </b></br>

#### Manually

**Note**: Will assume the path selected before (/desired/path) to be <b>rs</b> (to reduce verbosity)</br>
**Note 2**: Assuming also that current directory is abd_installer
```
cp *.h /rs/src/include/
cp *.c /rs/src/main/
cp -r abd_tool /rs/src/include/
cd /rs/tools/
./rsync-recommended
cd ..
./configure
make

echo "export PATH='/rs/bin/:$PATH'" >> ~/.bashrc
source ~/.bashrc
```
Installation is complete. </br>
Now ABD tool can be used with Rscript (with code constraints) or using R (combined with source("yourScript.R"))</br>

## How to Use
To use the tool, just use **abd_start()** and **abd_stop()** between the code that needs to be analyzed.

Example:
```
abd_start()
a<-20
(...)
abd_stop()
```

For more information about the functionalities, issue **abd_help()** at the terminal (after launching R).
## Code Constraints

Due to R implementation, and to take full advantage of the tool, running the script with Rscript has its constraints.

The code must contain the following structure:
```
options("keep.source"=TRUE)
run <- function(){
	abd_start()
	#more code here
	abd_stop()
}
run()
```
**OR**

```
options("keep.source"=TRUE)
(function(){
	abd_start()
	#more code here
	abd_stop()
})()
```
**Note 1:** Notice that between the first two lines there's no space. The first line should be at the first line of the script (literally)</br>
**Note 2:** Using the function source("script.R"), there's no need to have this kind of structure.

