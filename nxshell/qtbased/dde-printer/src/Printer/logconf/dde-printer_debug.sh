#!/bin/bash

# A script to configure the module's debug log level.
#
# Note: Please do not modify this script directly,
# as modifying this script will invalidate this script.
set -e

app_id="dde-printer"
configuration_id="org.deepin.dde.printer"

# 获取当前用户的UID
# current_uid=$(id -u)

# 判断是否是Root权限
# if [ "$current_uid" -ne 0 ]; then
# 	echo "You need to have root privileges to run this script."
# 	exit 1
# fi

for x in "$@"; do
	case $x in
	debug=*)
		debug=${x#debug=}
		;;
	state)
		state=y
		;;
	test)
		test=y
		;;
	esac
done

get_level_from_rules() {
	local str="$1"
	str=${str//\"/}

	# 将字符串按分号进行分割成数组
	#IFS=';' read -ra str_array <<< "$str"
	str_array=(${str//;/ })

	# 从数组的最后一个元素开始逐个检查
	for ((i = ${#str_array[@]} - 1; i >= 0; i--)); do
		# 判断元素是否包含 '=true'
		if [[ ${str_array[i]} == *=true ]]; then
			# 提取 '.' 后面的字符串
			tmp_str=${str_array[i]}
			tmp_array=(${tmp_str//\./ })
			col=${#tmp_array[@]}
			result=${tmp_array[col - 1]}
			break
		fi
	done
	# 去掉 =true
	result=${result//=true/}
	# 输出结果
	echo $result
}

case "${debug}" in
"debug")
	dde-dconfig --set -a $app_id -r $configuration_id -k log_rules -v "*dde-printer*.debug=true" #按需改变分类、添加子模块规则
	exit 0
	;;
"info")
	dde-dconfig --set -a $app_id -r $configuration_id -k log_rules -v "*.debug=false;*dde-printer*.info=true" #按需改变分类、添加子模块规则
	exit 0
	;;
"warning")
	dde-dconfig --set -a $app_id -r $configuration_id -k log_rules -v "*.debug=false;*.info=false;*dde-printer*.warning=true" #按需改变分类、添加子模块规则
	exit 0
	;;
esac

if [ "$state" == "y" ]; then
  log_rules=$(dde-dconfig --get -a $app_id -r $configuration_id -k log_rules) # 规则最后一位必须为true,默认全局warning为true
  get_level_from_rules $log_rules
fi
