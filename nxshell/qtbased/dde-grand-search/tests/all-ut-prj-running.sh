#!/bin/bash

# 定位脚本所在父目录
PROJECT_FOLDER=${1}

TESTS_FOLDER=$PROJECT_FOLDER/tests
SRC_FOLDER=$PROJECT_FOLDER/src

echo "TESTS_FOLDER:" $TESTS_FOLDER
echo "SRC_FOLDER" $SRC_FOLDER

# 定位build_ut
BUILD_DIR=${2}

UT_TESTS_FOLDER=$BUILD_DIR/tests

#运行UT类型 all dde-grand-search libdde-grand-search-daemon
UT_PRJ_TYPE=${3}
UT_TYPE_ALL="all"
UT_TYPE_GRAND_SEARCH="dde-grand-search"
UT_TYPE_LIBGRAND_SEARCH_DAEMON="libdde-grand-search-daemon"
UT_TYPE_PREVIEW_PLUGIN="preview-plugin"
UT_TYPE_GRAND_SEARCH_DOCK_PLUGIN="dde-grand-search-dock-plugin"

REBUILD_PRJ=${4}
REBUILD_TYPE_YES="yes"

#CPU 个数
CPU_NUMBER=${5}
#是否显示报告
SHOW_REPORT=${6}

check_ut_result()
{
  if [ $1 != 0 ]; then
     echo "Error: UT process is broken by: " $2 ",end with: "$1
     exit $1
  fi
}

# 打印当前目录，当前目录应当是 build-ut
echo `pwd`
echo "start dde-grand-search all UT cases:" $UT_PRJ_TYPE

# 下面是编译和工程测试
# 1. 编译工程
mkdir -p $UT_TESTS_FOLDER
cd $UT_TESTS_FOLDER
cmake -DCMAKE_BUILD_TYPE=Debug $TESTS_FOLDER
make -j$CPU_NUMBER


# 2. 运行ut-grand-search工程
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_GRAND_SEARCH" ] ; then
        echo $UT_TYPE_GRAND_SEARCH "test case is running"

	DIR_TEST_GRAND_SEARCH=$UT_TESTS_FOLDER/grand-search
	cd $DIR_TEST_GRAND_SEARCH
	
	extract_path_grand_search="*/src/grand-search/*"
	remove_path_grand_search="*/tests/* */3rdParty/* */build-ut/* *moc_*"
        # report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	$TESTS_FOLDER/ut-target-running.sh $BUILD_DIR dde-grand-search $DIR_TEST_GRAND_SEARCH test-dde-grand-search "$extract_path_grand_search" "$remove_path_grand_search" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_GRAND_SEARCH
fi

# 3. 运行ut-libgrand-search-daemon工程
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_LIBGRAND_SEARCH_DAEMON" ] ; then
        echo $UT_TYPE_LIBGRAND_SEARCH_DAEMON "test case is running"

	DIR_TEST_LIBGRAND_SEARCH_DAEMON=$UT_TESTS_FOLDER/libgrand-search-daemon
	cd $DIR_TEST_LIBGRAND_SEARCH_DAEMON

	extract_path_libgrand_search_daemon="*/src/libgrand-search-daemon/*"
    remove_path_libgrand_search_daemon="*/tests/* */3rdParty/* */build-ut/* *moc_* */anything_interface.*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR libdde-grand-search-daemon $DIR_TEST_LIBGRAND_SEARCH_DAEMON test-libdde-grand-search-daemon "$extract_path_libgrand_search_daemon" "$remove_path_libgrand_search_daemon" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_LIBGRAND_SEARCH_DAEMON
fi

# 4. 运行ut-preview-plugin工程
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_PREVIEW_PLUGIN" ] ; then
        echo $UT_TYPE_PREVIEW_PLUGIN "test case is running"

	DIR_TEST_PREVIEW_PLUGIN=$UT_TESTS_FOLDER/preview-plugin
	cd $DIR_TEST_PREVIEW_PLUGIN

	extract_path_preview_plugin="*/src/preview-plugin/*"
    remove_path_preview_plugin="*/tests/* */3rdParty/* */build-ut/* *moc_* *interface"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR preview-plugin $DIR_TEST_PREVIEW_PLUGIN test-preview-plugin "$extract_path_preview_plugin" "$remove_path_preview_plugin" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_PREVIEW_PLUGIN
fi

# 5. 运行ut-grand-search-dock-plugin工程
if [ "$UT_PRJ_TYPE" = "$UT_TYPE_ALL" ] || [ "$UT_PRJ_TYPE" = "$UT_TYPE_GRAND_SEARCH_DOCK_PLUGIN" ] ; then
        echo $UT_TYPE_GRAND_SEARCH_DOCK_PLUGIN "test case is running"

	DIR_TEST_GRAND_SEARCH_DOCK_PLUGIN=$UT_TESTS_FOLDER/grand-search-dock-plugin
	cd $DIR_TEST_GRAND_SEARCH_DOCK_PLUGIN

	extract_path_grand_search_dock_plugin="*/src/grand-search-dock-plugin/*"
    remove_path_grand_search_dock_plugin="*/tests/* */3rdParty/* */build-ut/* *moc_*"
	# report的文件夹，报告后缀名，编译路径，可执行程序名，正向解析设置，逆向解析设置
	./../../../tests/ut-target-running.sh $BUILD_DIR grand-search-dock-plugin $DIR_TEST_GRAND_SEARCH_DOCK_PLUGIN test-dde-grand-search-dock-plugin "$extract_path_grand_search_dock_plugin" "$remove_path_grand_search_dock_plugin" $SHOW_REPORT
        check_ut_result $? $UT_TYPE_GRAND_SEARCH_DOCK_PLUGIN
fi

echo "end dde-grand-search all UT cases"

exit 0
