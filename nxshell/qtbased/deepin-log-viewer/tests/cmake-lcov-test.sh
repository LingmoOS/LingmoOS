utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j16

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

app_name=deepin-log-viewer-test

mkdir -p report

./tests/$app_name --gtest_output=xml:./report/report.xml

lcov -d $workdir -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/application/*' '*/logViewerAuth/*' '*/logViewerService/*' '*/logViewerTruncate/*' -o ./report/coverage.info

lcov --remove ./report/coverage.info '*/tests/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
