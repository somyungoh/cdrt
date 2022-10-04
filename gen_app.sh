#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR

OS=`uname -s`
if [ "$OS" == 'Darwin' ]; then
    OS="macosx"
    ACTION="make config=release"
else
    echo "Not implemented for $OS!"
    exit 1
fi

# Run premake (project generation)
cd Projects/$OS
$ACTION
cd $SCRIPT_DIR

exit 0