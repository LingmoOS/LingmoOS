#!/bin/bash


case $1 in 
	lmupdate)
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi
		/usr/share/lingmo-settings/tools/aptlm/aptlm lmupdate 2>&1 | tee /tmp/spark-store-app-lmupdate-log.txt
		IS_LMUPDATE_ERROR=`cat /tmp/spark-store-app-lmupdate-log.txt | grep "E: "`
		echo "$IS_LMUPDATE_ERROR" > /tmp/spark-store-app-lmupdate-status.txt
		chmod 777 /tmp/spark-store-app-lmupdate-status.txt
		chmod 777 /tmp/spark-store-app-lmupdate-log.txt
	;;

	upgradable-list)
		output=$(env LANGUAGE=en_US /usr/bin/apt -c /usr/share/lingmo-settings/tools/apt-fast-conf/aptlm-apt.conf list --upgradable -o Dir::Etc::sourcelist="/usr/share/lingmo-settings/tools/apt-fast-conf/sources.list.d/lingmoos.list" -o Dir::Etc::sourceparts="/dev/null" -o APT::Get::List-Cleanup="0" | awk NR\>1)

		IFS_OLD="$IFS"
		IFS=$'\n'

		for line in $output ; do
			PKG_NAME=$(echo $line | awk -F '/' '{print $1}')
			PKG_NEW_VER=$(echo $line | awk -F ' ' '{print $2}')
			PKG_CUR_VER=$(echo $line | awk -F ' ' '{print $6}' | awk -F ']' '{print $1}')
			echo "${PKG_NAME} ${PKG_NEW_VER} ${PKG_CUR_VER}"
		done

		IFS="$IFS_OLD"
	;;

	upgrade-app)
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi

		/usr/share/lingmo-settings/tools/aptlm/aptlm install "${@:2}" --only-upgrade  2>&1 | tee /tmp/lingmoos-app-upgrade-log.txt
		chmod 777 /tmp/lingmoos-app-upgrade-log.txt
		IS_UPGRADE_ERROR=`cat /tmp/lingmoos-app-upgrade-log.txt | grep "Package manager quit with exit code."`
		echo "$IS_UPGRADE_ERROR" > /tmp/spark-store-app-upgrade-status.txt
	;;
	test-install-app)
if [ "$(id -u)" != "0" ] ; then
	pkexec "$0" "$@"
	exit
fi

try_run_output=$(/usr/share/lingmo-settings/tools/aptlm/aptlm --dry-run install $2)
try_run_ret="$?"

if [ "$try_run_ret" -ne 0 ]
  then
    echo "Package manager quit with exit code.Here is the log" 
    echo "包管理器以错误代码退出.日志如下" 
    echo
    echo -e "${try_run_output}"
    echo "Will try after run /usr/share/lingmo-settings/tools/aptlm/aptlm update"
    echo "将会在/usr/share/lingmo-settings/tools/aptlm/aptlm update之后再次尝试"
    /usr/share/lingmo-settings/tools/aptlm/aptlm update
    echo ----------------------------------------------------------------------------
	try_run_output=$(/usr/share/lingmo-settings/tools/aptlm/aptlm --dry-run install $2)
	try_run_ret="$?"
  		if [ "$try_run_ret" -ne 0 ]
  		then
  		  echo "Package manager quit with exit code.Here is the log" 
   		 echo "包管理器以错误代码退出.日志如下" 
   		 echo
    		echo -e "${try_run_output}"
    		exit "$try_run_ret"
 		 fi

fi
	exit 0
	;;
	
	clean-log)

	rm -f /tmp/spark-store-app-lmupdate-status.txt /tmp/spark-store-app-lmupdate-log.txt /tmp/lingmoos-app-upgrade-log.txt /tmp/spark-store-app-upgrade-status.txt
	;;
esac
