#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
PREMAKE_DIR=Premake     # Replace this with your local premake directory
cd $SCRIPT_DIR

OS=`uname -s`
if [ "$OS" == 'Windows' ]; then
    OS="windows"
    ACTION="vs2019"
elif [ "$OS" == 'Darwin' ]; then
    OS="macosx"
    ACTION="xcode4"
    # ACTION="gmake"
elif [ "$OS" == 'Linux' ]; then
    OS="linux"
    ACTION="gmake"
else
    echo "Unknown system $OS!"
    exit 1
fi

# Run Premake (project generation)
(cd $PREMAKE_DIR && ./premake5 --os=$OS $ACTION)

exit 0
