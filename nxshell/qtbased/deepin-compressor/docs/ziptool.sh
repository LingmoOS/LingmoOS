#!/bin/bash 

function ergodic(){  
    for file in ` ls $1 `  
    do  
        if [ -d $1"/"$file ]   
        then  
        	ziptool $2 add_dir $1"/"$file
            ergodic $1"/"$file $2
        else  
        	ziptool $2 add_file $1"/"$file $1"/"$file 0 0
            echo "$file"
        fi  
    done  
} 

ergodic $1 $2
