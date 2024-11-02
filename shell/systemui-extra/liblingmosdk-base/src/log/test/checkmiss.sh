#!/bin/bash

function testmiss ()
{
	let count=1
	oldIFS=${IFS}
	IFS=$'\n'
	let missline=0
	let flag=0
	for line in `cat $1`;do
		value=`echo ${line} | awk '{print $NF}'`
		while [ ${value} -ne ${count} ];do
			echo "missing ${count}" >> /tmp/logtest-missing-${TID[${index} - 1]}.rec
			echo "TID - ${TID[${index} - 1]} : ${count} missmatch with ${count}"
			((count ++))
			((missline ++))
		done
		if [ $flag -eq 0 ];then
			echo ""
			((flag ++))
		fi
		printf "\rTID - ${TID[${index} - 1]} : ${count} matched."
		((count ++))
	done
	return missline
}

LOGFILE=/home/lingmo/.log/test-pressure.log

TID=(`awk -F '[]-]' '{print $5}' ${LOGFILE} | sort -n | uniq`)

oldIFS=${IFS}

for index in `seq ${#TID[@]}`;do
	grep ${TID[${index} - 1]} ${LOGFILE} > /tmp/logtest-${TID[${index} - 1]}.log
	testmiss "/tmp/logtest-${TID[${index} - 1]}.log" &
done

wait
