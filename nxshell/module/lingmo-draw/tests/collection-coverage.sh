lcov --directory ./src/CMakeFiles/lingmoDrawBase.dir/ --capture --output-file ./coverageResult/lingmo-draw-test_Coverage.info
echo \ ===================\ do\ filter\ begin\ ====================\ 
lcov --remove ./coverageResult/lingmo-draw-test_Coverage.info '*/lingmoDrawBase_autogen/*' '*/lingmo-draw-test_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '*/gtest/*' -o ./coverageResult/lingmo-draw-test_Coverage.info
echo \ ===================\ do\ filter\ end\ ====================\ 
genhtml -o ./coverageResult/report ./coverageResult/lingmo-draw-test_Coverage.info
