utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

./tests/deepin-editor-test --gtest_output=xml:./report/report.xml

mkdir -p report
lcov -d $workdir -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/src/*' -o ./report/coverage.info

lcov --remove ./report/coverage.info '*/tests/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
