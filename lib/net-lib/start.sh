#!/bin/bash
# Build and run script.
# Author: Igor

 #                          ###  #   #      #    #       ##      #                      
# # # # ### ### ###  ##     #    #      ### ### ###     #   ###     ### ##  ### ###  ## 
### # # #   # # #   # #     ##   #   #  # # # #  #       #  #    #  ##  # # #   ##   #  
# # ### #   ### #   ###     #    ##  ##  ## # #  ##       # ###  ## ### # # ### ### ##  
# #                         #           ###             ##                              

set -eu
MODULE_NAME="NETLIB"


error() {
    echo $1
    exit 1
}


copyright() {
    echo "Aurora Flight Sciences (2018)."
}


usage() {
    echo "[$MODULE_NAME] Usage: start.sh [--help] <command> [<args>]"
    echo ""
    echo "MIDAS_ROOT environmental variable should be defined which indicates"
    echo "the root directory where all dependencies are stored."
    echo ""
    echo "-h --help         Show help information."
    echo "--install         Installs the module."
    echo "--clean           Cleans the module."
    echo ""
    echo "No command can be used to clean XOR install the module."
    echo ""
    echo "Commands:"
    echo "run               Runs the module."
    echo "build             Build the module."
    echo "test              Runs the tests."
    echo ""
    copyright
}


usage_run() {
    echo "[$MODULE_NAME] Usage: start.sh run [--help | <args>]"
    echo "-h --help         Show help information about the run command."
    echo ""
    echo "Runs the module. Default is to run the most recently built target."
    echo ""
    echo "--debug           Runs the module in GDB."
    echo ""
    copyright
}


usage_build() {
    echo "[$MODULE_NAME] Usage: start.sh build [--help | <args>]"
    echo "-h --help         Show help information about the build command."
    echo ""
    echo "Builds the module. Default is to build for desktop."
    echo ""
    echo "--install         Installs the module."
    echo "--clean           Cleans the module."
    echo ""
    echo "Arguments which get translated into CMake options:"
    echo "--tegra           Build for the tegra."
    echo "--rpi             Build for the raspberry pi."
    echo "--debug           Build a debug version."
    echo "--testing         Build with tests."
    echo "--examples        Build with examples."
    echo ""
    copyright
}


usage_test() {
    echo "[$MODULE_NAME] Usage: start.sh test [--help | <args>]"
    echo "-h --help         Show help information about the test command."
    echo ""
    echo "Tests the module using ctest."
    echo ""
    copyright
}


# Build the command line arguments to make start.sh based on the passed input.
# NOTE: deprecated
construct_start_args() {
    local start_args=""
    if [ "$arg_install" = true ]; then
        start_args="$start_args --install"
    fi
    if [ "$arg_clean" = true ]; then
        start_args="$start_args --clean"
    fi
    if [ "$arg_tegra" = true ]; then
        start_args="$start_args --tegra"
    fi
    if [ "$arg_rpi" = true ]; then
        start_args="$start_args --rpi"
    fi
    if [ "$arg_debug" = true ]; then
        start_args="$start_args --debug"
    fi
    if [ "$arg_testing" = true ]; then
        start_args="$start_args --testing"
    fi
    if [ "$arg_examples" = true ]; then
        start_args="$start_args --examples"
    fi
    echo "$start_args"
}


# Build the command line arguments to cmake based on the passed input.
construct_cmake_args() {
    local cmake_args=""

    # Create the arguments from passed user input
    if [ "$arg_tegra" = true ]; then
        cmake_args="$cmake_args -DGLOBAL_FOR_TEGRA=ON"
    fi
    if [ "$arg_rpi" = true ]; then
        cmake_args="$cmake_args -DGLOBAL_FOR_RPI=ON"
    fi
    if [ "$arg_debug" = true ]; then
        cmake_args="$cmake_args -DGLOBAL_WITH_DEBUG=ON"
    fi
    if [ "$arg_testing" = true ]; then
        cmake_args="$cmake_args -DGLOBAL_WITH_TESTING=ON"
    fi
    if [ "$arg_examples" = true ]; then
        cmake_args="$cmake_args -DGLOBAL_WITH_EXAMPLES=ON"
    fi

    # Create the MIDAS_ROOT argument from the environmental variable
    if [ -z "${MIDAS_ROOT-}" ]; then 
        echo "Warning: MIDAS_ROOT not set"
    else
        cmake_args="$cmake_args -DMIDAS_ROOT=$MIDAS_ROOT"
    fi

    echo "$cmake_args"
}


noop() {

    # Clean it
    if [ "$arg_clean" = true ]; then
        echo "Cleaning $MODULE_NAME"
        clean

    # Install it
    elif [ "$arg_install" = true ]; then
        echo "Installing $MODULE_NAME"
        install

    fi
}


run() {
    echo "Running $MODULE_NAME" 

    if [ "$arg_debug" = true ]; then
        echo "Library $MODULE_NAME cannot be run in GDB"
    else
        echo "Library $MODULE_NAME cannot be run"
    fi
}


run_test() {
    echo "Starting $MODULE_NAME tests" 
    cd $dir_build || error "Error: Could not find build directory"
    ctest -V
    cd $dir_root
}


build() {
    echo "Building $MODULE_NAME"

    # Clean it
    if [ "$arg_clean" = true ]; then
        clean
    fi

    # Run cmake
    build_cmake

    # Run make
    build_make

    # Install it
    if [ "$arg_install" = true ]; then
        install
    fi
}


clean() {
    rm -rf $dir_build
    rm -rf $dir_install
}


build_cmake() {
    cmake_args="$(construct_cmake_args)"
    mkdir -p $dir_build
    cd $dir_build || error "Error: Could not find build directory"
    cmake $cmake_args ..  || error "Error: cmake failed"
    cd $dir_root
}


build_make() {
    cd $dir_build || error "Error: Could not find build directory"
    make -j8 all  || error "Error: make failed"
    cd $dir_root
}


install() {
    mkdir -p $dir_install
    cd $dir_build || error "Error: Could not find build directory"
    make install || error "Error: make install failed"
    cd $dir_root
}


# Root directory form which this script is run
dir_root=$(pwd)
dir_build="$dir_root/build"
dir_install="$dir_root/install"

# Version of the script
version="0.1"

# Variables containing argument flags
arg_install=false
arg_clean=false
arg_help=false
arg_tegra=false
arg_rpi=false
arg_debug=false
arg_testing=false
arg_examples=false

# Array containing the positional arguments
positional=()

# Parse command line arguments
while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
        --install)
        arg_install=true
        shift
        ;;
        --clean)
        arg_clean=true
        shift
        ;;
        --tegra)
        arg_tegra=true
        shift
        ;;
        --rpi)
        arg_rpi=true
        shift
        ;;
        --debug)
        arg_debug=true
        shift
        ;;
        --testing)
        arg_testing=true
        shift
        ;;
         --examples)
        arg_examples=true
        shift
        ;;
        -h|--help)
        arg_help=true
        shift
        ;;
        *)
        positional+=("$1")
        shift
        ;;
    esac
done

if [ "$arg_rpi" = true ]; then
    if [ -z "${MIDAS_ROOT-}" ]; then 
        export PATH="$PATH:$dir_root/external/x-tools/arm-unknown-linux-gnueabihf/bin"
    else
        export PATH="$PATH:$MIDAS_ROOT/x-tools/arm-unknown-linux-gnueabihf/bin"
    fi
fi

if [ ${#positional[@]} -eq 0 ]; then
    if [ "$arg_help" = true ]; then
        usage
    else
        noop
    fi
    exit 0
fi

if [ "${positional[0]}" = "run" ]; then
    if [ "$arg_help" = true ]; then
        usage_run
    else
        run
    fi
fi

if [ "${positional[0]}" = "build" ]; then
    if [ "$arg_help" = true ]; then
        usage_build
    else
        build
    fi
fi

if [ "${positional[0]}" = "rebuild" ]; then
    if [ "$arg_help" = true ]; then
        usage_rebuild
    else
        rebuild
    fi
fi

if [ "${positional[0]}" = "test" ]; then
    if [ "$arg_help" = true ]; then
        usage_test
    else
        run_test
    fi
fi

exit 0
