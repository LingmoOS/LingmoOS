utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

./tests/ocean-calendar-client-test/ocean-calendar-test --gtest_output=xml:./report/report_calendar_client.xml
./tests/ocean-calendar-service-test/ocean-calendar-service-test --gtest_output=xml:./report/report_calendar_service.xml

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report
lcov -d $workdir -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/calendar-basicstruct/*' '*/calendar-client/*' '*/calendar-service/*' '*/schedule-plugin/*' -o ./report/coverage.info
lcov --remove ./report/coverage.info '*/tests/*' -o  ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0