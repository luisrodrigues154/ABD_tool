#!/usr/bin/python3
import sys
import os
import shutil
from checksumdir import dirhash


def checkBasePath(base_path) -> bool:
    print("")
    printProgress("Verifying paths ")
    print("base: {} ...".format(base_path), end="")
    main_folder = base_path + "/src/main"
    include_folder = base_path + "/src/include"
    if(os.path.isdir(base_path)):
        print("OK!")
        print("/main: {} ...".format(main_folder), end="")
        if(os.path.isdir(main_folder)):
            print("OK!")
            print("/include: {} ...".format(include_folder), end="")
            if(os.path.isdir(include_folder)):
                print("OK!")
                return True
            else:
                print("ERROR!")
        else:
            print("ERROR!")
    else:
        print("ERROR!")
    return False


def printProgress(message):
    print("[TASK] {}".format(message))


def printError(message):
    print("[ERROR] {}".format(message))


def checkSum(path):
    printProgress("Check sum verification")
    expected_hash = "53ebf8316d75d279cfb28443c773ef475d22dee9"
    rcvd_hash = dirhash(path, 'sha1')
    print("Expected hash: {}".format(expected_hash))
    print("Generated hash: {}".format(rcvd_hash))
    if(expected_hash == rcvd_hash):
        return True
    return False


def printSuccess(message):
    print("[SUCCESS] {}".format(message))


def checkArgs(args):
    args_len = len(args)

    if(args_len < 2):
        print("")
        printError("Invalid Arguments")
        printUsage()
        sys.exit()

    args_map = {
        "-h": printHelp,
        "-r": printRequirements,
        "-p": None,
        "-c": checkSum,
        "-m": None,
        "-f": None
    }

    rcvd_path = ""
    for i in range(1, len(args)):
        if(args[i] in args_map):
            if(args[i] == "-h"):
                args_map[args[i]]()
            elif(args[i] == "-r"):
                args_map[args[i]]()
            elif(args[i] == "-p"):
                try:
                    rcvd_path = args[i+1]
                    if(checkBasePath(rcvd_path)):
                        printSuccess("Path verified")
                        print("")
                        if("-c" not in args and "-f" not in args):
                            if(checkSum(rcvd_path)):
                                printSuccess("Hashes match!")
                                print("")
                            else:
                                printError("Hashes do not match!!!")
                                sys.exit()
                        else:
                            printProgress("Bypassing Check Sum verification")
                    else:
                        printError("Invalid PATH")
                        sys.exit()
                except IndexError:
                    print("")
                    printError("Invalid Arguments")
                    print("")
                    printUsage()
                    sys.exit()
    return rcvd_path


def printHelp():
    print("")
    print("Available commands")
    print("\t-h : Display this information")
    print("\t-r : Display the requirements to use ABD")
    print("\t-p : Specifies the R sources path")
    print("\t-c : Bypass checksum verification")
    print("\t-m : Configure and make after installation")
    print("\t-f : Force installation (does not check hashes)")
    print("")


def printRequirements():
    print("")
    print("Requirements to use installer:")
    print("-> python 3")
    print("-> R source code (version 4.0.2 tested)\n\t(link: https://github.com/luisrodrigues154/ABD_tool/R_source\n\tOR https://cran.r-project.org/src/base/R-4/R-4.0.2.tar.gz)")
    print("-> R required dependencies (Verify before using -m option)")
    print("\n")


def printUsage():
    print("")
    print("Usage: python abd_inst.py -p path/to/r_source/")
    print("Issue -h to help")
    print("")


def printBanner():
    print("")
    print("Welcome to ABD_tool installer!!")


def patch(f_map, base_path, force):
    if(base_path == ""):
        return False

    script_dir = os.path.dirname(__file__)

    for file in f_map:

        src_file = os.path.join(script_dir, './files/{}'.format(file))
        dest_file = "{}{}{}".format(base_path, f_map[file], file)
        if(file == "abd_tool"):
            if(os.path.isdir(dest_file)):
                if(not force):
                    printSuccess("ABD_tool already installed")
                    return True
                else:
                    os.system("rm -rf {}".format(dest_file))

        try:
            if(os.path.isdir(src_file)):
                printProgress("Installing {}".format(file))
                os.makedirs(os.path.dirname(dest_file), exist_ok=True)
                shutil.copytree(src_file, dest_file)
                printSuccess("Installed!")
                print("")
            else:
                printProgress("Patching {}".format(file))
                shutil.copyfile(src_file, dest_file)
                printSuccess("Patched!")
                print("")
        except OSError:
            printError("File {}".format(file))

    return True


def printAction(message):
    print("[ACTION] {}".format(message), end="")


def doMake(path):
    if(path == ""):
        return False
    rsync_packages = "{}/tools/".format(path)
    os.chdir(rsync_packages)
    print("")
    printProgress("Configuring and making source files")
    print("")
    printProgress("Syncing R-recommended packages")
    printAction("<press any key to continue>")
    input()
    os.system("./rsync-recommended")

    os.chdir(path)
    print("")
    printProgress("Configuring Source files")
    printAction("<press any key to continue>")
    input()
    os.system("./configure")
    printSuccess("Configuration COMPLETED")
    print("")
    printProgress("Making")
    printAction("<press any key to continue>")
    input()
    os.chdir(path)
    print("curr dir {}".format(os.getcwd()))
    os.system("make")
    print("")
    printSuccess("make COMPLETED")
    print("")
    printSuccess("Installation completed")

    while True:
        printAction("Add R and Rscript to PATH variable?? [N/y] ")
        ans = input()
        valid = ["Y", "y"]
        if(ans in valid):
            print("")
            addToPath(path)
            break
        elif(ans == "n" or ans == "N" or ans == ""):
            break
        else:
            printError("Invalid option!")
    return True


def addToPath(path):
    print("")
    printProgress("Adding R/Rscript to PATH varaible")
    bashrc_source = "source ~/.bashrc"
    bin_path = "{}/bin/".format(path)
    path_export = 'export PATH=\"{}:$PATH\"'.format(bin_path)
    cmd = 'echo \'{}\' >> ~/.bashrc'.format(path_export)
    os.system(cmd)
    os.system(bashrc_source)
    printSuccess("PATH variable updated")
    print("")


if __name__ == "__main__":
    printBanner()
    base_path = checkArgs(sys.argv)
    f_map = {
        "abd_tool": "/src/include/",
        "Internal.h": "/src/include/",
        "eval.c": "/src/main/",
        "names.c": "/src/main/",
        "version.c": "/src/main/",
        "arithmetic.c": "/src/main/",
        "seq.c": "/src/main/",
        "bind.c": "/src/main/",
        "subassign.c": "/src/main/",
        "subset.c": "/src/main/"

    }
    force = False
    if("-f" in sys.argv):
        force = True
    result = patch(f_map, base_path, force)
    if(result):
        printSuccess("Intallation successfully finished!!")
        if("-m" in sys.argv):
            doMake(base_path)
        print("")
