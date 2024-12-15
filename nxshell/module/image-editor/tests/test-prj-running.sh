export QT_QPA_PLATFORM='offscreen'
rm -r build-ut
rm -r ../build-ut
mkdir ../build-ut
cd ../build-ut
cmake -DCMAKE_BUILD_TYPE=Debug .. -D DOTEST=ON
make -j16
export QTEST_FUNCTION_TIMEOUT='1000000'
workdir=$(cd ../$(dirname $0)/build-ut; pwd)
PROJECT_NAME=image-editor-test #可执行程序的文件名
PROJECT_REALNAME=image-editor  #项目名称
mkdir -p html
mkdir -p coverageResult
mkdir -p tests/test
     echo " ===================CREAT LCOV REPROT==================== "
     mkdir ./CMakeFiles/${PROJECT_REALNAME}.dir
     lcov --directory ./CMakeFiles/${PROJECT_REALNAME}.dir --zerocounters
     ./tests/${PROJECT_NAME} --gtest_output=xml:./report/report_image-editor.xml
     lcov --directory . --capture --output-file ./html/${PROJECT_REALNAME}_Coverage.info

     echo " =================== do filter begin ==================== "
     lcov --remove ./html/${PROJECT_REALNAME}_Coverage.info 'CMakeFiles/${PROJECT_NAME}.dir/image-editor-test_autogen/*/*' '${PROJECT_NAME}_autogen/*/*' 'googletest/*/*' '*/usr/include/*' '*/tests/*' '/usr/local/*' -o ./html/${PROJECT_REALNAME}_Coverage_fileter.info
     echo " =================== do filter end ==================== "

     genhtml -o ./html ./html/${PROJECT_REALNAME}_Coverage_fileter.info
     echo " -- Coverage files have been output to ${CMAKE_BINARY_DIR}/html "
     mv ./html/index.html ./html/cov_${PROJECT_REALNAME}.html
     mv asan.log* asan_${PROJECT_REALNAME}.log
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
exit 0