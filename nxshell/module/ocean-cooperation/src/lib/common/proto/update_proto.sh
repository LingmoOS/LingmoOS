#!/bin/bash
#Generate protocol command: fbec --cpp --proto --input=simple.fbe --output=.

fbec="../../../../tools/fbec"

FBE_INPUT_FILE=${1:-"message.fbe"}
cppout=${2:-"."}

if [ -f $fbec ]; then
    echo "Generating FBE proto for ${FBE_INPUT_FILE}... out: ${cppout}"
    $fbec --cpp --final --proto --input=${FBE_INPUT_FILE} --output=${cppout}
    # $fbec --kotlin --final --proto --input=${FBE_INPUT_FILE} --output=${cppout}
else
    echo "the fbe is not exist:$fbec"
fi