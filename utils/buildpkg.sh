#!/bin/bash 

function read_dir(){
for file in `ls build`
	do
		if [ -d $file ]
		then
			read_dir $file
		else
			echo $file
			cd build"/"$file
			apt build-dep .
			dpkg-buildpackage -us -uc -j8
			if [ $? -ne 0 ]; then
				echo "nothing"
			else
				pwd
			fi
#			cd ../../
		fi
	done
}
 
read_dir $1
