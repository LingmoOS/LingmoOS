utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Release .. 
make -j16

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

app_name=deepin-devicemanager-test

mkdir -p report

./deepin-devicemanager/tests/$app_name --gtest_output=xml:./report/report.xml

lcov -d $workdir -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/src/*' -o ./report/coverage.info

lcov --remove ./report/coverage.info '*/tests/*' '*/3rdparty/*' -o  ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
