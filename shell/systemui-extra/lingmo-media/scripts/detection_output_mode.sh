#!/bin/bash
for USER in $(who | grep tty | awk '{print $1}' | sort | uniq);do
  USER_NAME=$USER
#  USER_NAME=$USER
  USER_ID=$(id -u "$USER_NAME")
  PULSE_SERVER="unix:/run/user/"$USER_ID"/pulse/native"
  OUTPUT_MODE="not available"
  FIND_STR=" analog-output"
  sudo -H -u ${USER_NAME} pactl --server $PULSE_SERVER list cards |grep analog-output  > /tmp/lingmo_headphone.tmp
  while read line;do
    index="$line"
    echo $line
    if [ -n "$index" ];then
      OUTPUT_MODE=${index#*usec, }
      OUTPUT_MODE=${OUTPUT_MODE%%)}
      echo "输出模式为"  $OUTPUT_MODE
    fi
    if [[ $line =~ "analog-output-speaker" ]] && [[ $OUTPUT_MODE == "available" ]];then
      exit 0
    elif [[ $line =~ "analog-output-headphone" ]] && [[ $OUTPUT_MODE == "available" ]];then
      exit 1
    fi 
  done < /tmp/lingmo_headphone.tmp
done

rm -rf /tmp/lingmo_headphone.tmp
exit -1
