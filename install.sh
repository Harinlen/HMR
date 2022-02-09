#!/bin/bash
check_command () {
    echo -n "checking for $1..."
    if ! [ -x "$(command -v $1)" ]; then
        echo "failed"
        echo "$1 is not installed. Please install the $1."
        exit 1
    fi
    echo "$1"
}
# Check whether the target path is existed.
TARGET_PATH=$1
if [ -z $TARGET_PATH ]; then
    echo "Missing install directory."
    exit 1
fi
if ! [ -d $TARGET_PATH ]; then
    echo "Invalid install directory path."
    exit 1
fi
echo "Installing HMR to path $TARGET_PATH"
# Check whether CMake is installed.
check_command cmake
check_command python3
check_command java
check_command gcc
check_command g++
# CMake and build the directory at the target directory.
C_BUILD_DIR="$TARGET_PATH/build"
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
build_cxx() {
    echo "Building $1..."
    # Create and get into build directory.
    if ! [ -d $C_BUILD_DIR ]; then
        echo -n "Creating build directory..."
        mkdir $C_BUILD_DIR
        echo "done"
    fi
    # Build and make.
    echo $C_BUILD_DIR
    cd $C_BUILD_DIR
    pwd
    cmake -DCMAKE_BUILD_TYPE=Release "$SCRIPT_DIR/cxx/$1"
    make -j4
    # Copy the result to target dir.
    mv "$C_BUILD_DIR/$1" "$TARGET_PATH/hmr-$1"
    # Go back to script directory.
    cd $SCRIPT_DIR
    # Remove the build directory.
    rm -rf $C_BUILD_DIR
}
build_cxx correction
build_cxx filter
build_cxx graph
# Copy the external jars to target directory.
echo -n "Copying external jars..."
cp -r $SCRIPT_DIR/py/ext $TARGET_PATH
echo "done"
echo -n "Copying python scripts..."
cp -r $SCRIPT_DIR/py/*.py $TARGET_PATH
echo "done"
echo -n "Copying bootstrap..."
cp -r $SCRIPT_DIR/py/hmr $TARGET_PATH
echo "done"
