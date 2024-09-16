lcov --directory ./src/CMakeFiles/deepinDrawBase.dir/ --capture --output-file ./coverageResult/deepin-draw-test_Coverage.info
echo \ ===================\ do\ filter\ begin\ ====================\ 
lcov --remove ./coverageResult/deepin-draw-test_Coverage.info '*/deepinDrawBase_autogen/*' '*/deepin-draw-test_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '*/gtest/*' -o ./coverageResult/deepin-draw-test_Coverage.info
echo \ ===================\ do\ filter\ end\ ====================\ 
genhtml -o ./coverageResult/report ./coverageResult/deepin-draw-test_Coverage.info
