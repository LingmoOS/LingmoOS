#!/bin/bash 

function read_dir(){
for file in `ls $1`
	do
		if [ -d $file ]
		then
			read_dir $file
		else
			echo $file
			cd $1"/"$file
			grep -rl -e "lingmo" -e "ocean" -e "lingmoui" -e "Lingmo" -e "LINGMO" -e "LingmoUI" -e "Ocean" -e "OCEAN"
			if [ $? -ne 0 ]; then
				echo "nothing"
			else
				pwd
			fi
			cd ../
		fi
	done
}
 
read_dir $1
