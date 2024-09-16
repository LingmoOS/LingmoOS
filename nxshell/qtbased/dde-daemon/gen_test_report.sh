#!/bin/bash

# 在这里设置依赖目录，依赖目录不做统计，该目录在不同项目不一致，dde-api是gobuild
#gThirdpartPath="vendor"
gThirdpartPath="gopath"
#gThirdpartPath="gobuild"
# 设置忽略目录列表，列表的目录及其子目录不统计
gIgnorePaths=("gobuild" "gopath" "vendor")


# 生成的测试报告文件
gReportDir=./cover_report
gHTMLFile=${gReportDir}/index.html
gDeleteCoverageData=1    # 1 - 删除单元测试日志和覆盖率中间数据, 0 - 不删除


####################################################################################

if [ $# == 1 ] && [ "$1" == "clean" ]; then
    rm ${gReportDir} -rf
    echo "clean"
    exit
fi

gFuncHit=0
gFuncTotal=0
gRowHit=0
gRowTotal=0
gTestCaseNumTotal=0
gTestCaseNumFailed=0

function getGOPath() {
    echo "$(pwd):$(pwd)/${gThirdpartPath}:${GOPATH}"
}

# 依赖目录不做统计
function getThirdpartPath() {
     echo "./${gThirdpartPath}/"
}

# 获取需要统计的目录
function getTestPath() {
    currentDirectory=$(pwd)
    cmd=""
    for dir in "${gIgnorePaths[@]}"
    do
        cmd=$cmd" | grep -v ^_"$currentDirectory/$dir".*"
    done
    cmd="go list ./..."$cmd
    ret=$(eval $cmd)
    echo ${ret//"_"$currentDirectory/.}
}

# 获取因缺少测试文件导致会遗漏的目录（包）的列表
function getGoDirect() {

    arrDirGo=()
    while read -r line
    do
        arrDirGo[${#arrDirGo[*]}]=$line
    done <<< "$(find ./ -name '*.go' | sed 's/\(.*\)\/[^\/]*.go$/\1/' | sort -u)"

    arrDirGOTest=()
    while read -r line
    do
        arrDirGOTest[${#arrDirGOTest[*]}]=$line
    done <<< "$(find ./ -name '*.go' | sed 's/\(.*\)\/[^\/]*_test.go$/\1/' | sort -u)"

    for dirGo in "${arrDirGo[@]}"
    do
        isFind=0
        for dirGoTest in "${arrDirGOTest[@]}"
        do
            if [ "${dirGo}" == "${dirGoTest}" ]; then
                isFind=1
            fi
        done
        strRet=$(echo "$dirGo" | grep "^$(getThirdpartPath).*")
        if [ "$strRet" != "" ]; then
            isFind=1
        fi
        if [ $isFind == 0 ]; then
            echo "$dirGo"
        fi
    done
}

# 单元测试用例数统计
function TestCasesNum() {
    # 官方gotest
    gTestCaseNumTotal=$(grep -c '^=== RUN[^\/]*$' ${gReportDir}/test.log)
    gTestCaseNumFailed=$(grep -c '^--- FAIL[^\/]*$' ${gReportDir}/test.log)

    # 第三方测试框架
    TestCasesNumByGoCheck
    TestCasesNumByGoConvey

    if [ ${gDeleteCoverageData} == 1 ]; then
        rm ${gReportDir}/test.log -f
    fi
}

# 单元测试用例数统计 - 测试框架gocheck(gopkg.in/check)
function TestCasesNumByGoCheck() {
    while read -r line
    do
        # OK: 5 passed, 2 skipped
        # OOPS: 3 passed, 1 FAILED
        # skipped当作无效用例不统计，有失败的时候，前缀变成OOPS
        if [ "$line" == "" ]; then
            continue
        fi
        success=$(echo $line | grep -Eo ' [^ ]* passed' | tr -cd "(0-9)")
        if [ "$success" != "" ]; then
            gTestCaseNumTotal=$((gTestCaseNumTotal+success))
        fi
        failed=$(echo $line | grep -Eo ' [^ ]* FAILED' | tr -cd "(0-9)")
        if [ "$failed" != "" ]; then
            gTestCaseNumTotal=$((gTestCaseNumTotal+failed))
            gTestCaseNumFailed=$((gTestCaseNumFailed+failed))
            # 如果当前Suite有失败，那么testing.T也会判断失败，避免重复计算，所以减1
            ((gTestCaseNumFailed--))
        fi
        # 每一个匹配到的行代表一个Suite，每个Suite对应一个testing.T，这个testing.T只用于转换，不是实际用例，所以减1
        ((gTestCaseNumTotal--))
    done <<< $(grep '^O[KO][P:][S ].*' ${gReportDir}/test.log)
}

# 单元测试用例数统计 - 测试框架goconvey(github.com/smartystreets/goconvey)
function TestCasesNumByGoConvey() {
    isInTest=0 # 逐行解析，以此记录进入一次gotest的testing.T
    failedNum=0
    isConvey=0 # 判断此次的testing.T是否goconvey
    isConveyFailed=0 # 判断此次goconvey是否有失败的用例
    echo "" >> ${gReportDir}/test.log
    while read -r line
    do
        # === RUN   TestReader
        #   Test Reader ✔✔
        #     Get Pid ✔✔✔✔
        #     ReadAll ✔✔
        # 8 total assertions
        #   Test ReadAll error ✘
        # Failures:
        #   * /home/feng08/code/go/src/go-lib/encoding/kv/reader_test.go
        # 9 total assertions
        #   Test Read shell vars ✔✔✔✔✔✔✔✔
        # 17 total assertions
        # --- FAIL: TestReader (0.00s)
        if [ "$line" == "" ]; then
            continue
        fi
        startHit=$(echo $line | grep -c '^=== RUN[^\/]*$')
        if [ $startHit == 1 ]; then
            isInTest=1
            continue
        fi
        endHit=$(echo $line | grep -c '^--- [^\/]*$')
        if [ $endHit == 1 ]; then
            isInTest=0
            if [ $isConvey == 1 ]; then
                # testing.T也会判断为一个用例，避免重复计算
                ((gTestCaseNumTotal--))
                isConvey=0
            fi
            if [ $isConveyFailed == 1 ]; then
                # testing.T也会判断失败，避免重复计算
                ((gTestCaseNumFailed--))
                isConveyFailed=0
            fi
            continue
        fi
        if [ $isInTest == 0 ]; then
            continue
        fi 
        failedHit=$(echo $line | grep -c '✘')
        if [ $failedHit == 1 ]; then
            ((failedNum++))
            continue
        fi
        conveyHit=$(echo $line | grep -c '^[0-9]* total assertions$')
        if [ $conveyHit == 1 ]; then
            # 每个该行对应一个goconvey用例
            isConvey=1
            ((gTestCaseNumTotal++))
            if [ $failedNum != 0 ]; then
                isConveyFailed=1
                ((gTestCaseNumFailed++))
            fi
            failedNum=0
            continue
        fi
    done < ${gReportDir}/test.log
}

# 单元测试用例数统计
function TestCasesNumByTestFile() {
    gTestCaseNumTotal=0
    while read -r file
    do
        GoConveyNum=0
        GoCheckNum=0
        GoTestNum=0
        while read -r line
        do
            # 去掉开头空格和制表符
            trimLine=$(awk '{sub(/^[\t ]*/,"");print}' <<< $line)
            if [ "${trimLine:0:2}" == "//" ] || [ "${trimLine:0:2}" == "/*" ]; then
                # 注释行跳过
                continue
            fi
            GoConveyHit=$(echo $line | grep -c '^Convey(.*func(.*$')
            GoConveyNum=$((GoConveyNum+GoConveyHit))
            GoCheckHit=$(echo $line | grep -c '^func (.*\*C\.C).*$')
            GoCheckNum=$((GoCheckNum+GoCheckHit))
            GoTestHit=$(echo $line | grep -c '^func .*\*testing\.T.*$')
            GoTestNum=$((GoTestNum+GoTestHit))
        done < $file
        if [ $GoConveyNum != 0 ] || [ $GoCheckNum != 0 ]; then
            # 在gocheck或者goconvey测试框架下gotest函数只是入口函数不是实际用例
            GoTestNum=0
        fi
        gTestCaseNumTotal=$((gTestCaseNumTotal+GoCheckNum+GoConveyNum+GoTestNum))
    done <<< $(find ./ -type f -name "*_test.goo")

    echo $gTestCaseNumTotal
    echo $gTestCaseNumFailed
}

# 开始单元测试
function makeTest() {
    mkdir -pv $gReportDir

    arrSupplementDir=()
    while read -r line
    do
        if [ "$line" != "" ]; then
            arrSupplementDir[${#arrSupplementDir[*]}]=$line
        fi
    done <<< "$(getGoDirect)"

    for supplementDir in "${arrSupplementDir[@]}"
    do
        supplementFile=${supplementDir}"/auto_generate_temp_go_test.go"
        name=$(grep "^package .*" "${supplementDir}"/*.go | head -n 1 | cut -d ":" -f 2)
        # echo "supplement:""${supplementDir}"" - ""${name}"
        echo "${name}" >> "$supplementFile"
    done
    #exit
    echo "gotest dir:$(getTestPath)"

    env GOPATH="$(getGOPath)" go test $(getTestPath)  -v -cover  -coverprofile=${gReportDir}/coverage.data | tee ${gReportDir}/test.log
    env GOPATH="$(getGOPath)" go tool cover -func=${gReportDir}/coverage.data -o ${gReportDir}/coverage.txt
    
    if [ ${gDeleteCoverageData} == 0 ]; then
        # 下面命令可以生成查看具体覆盖情况的html文件
        env GOPATH=$(getGOPath) go tool cover -html=${gReportDir}/coverage.data -o ${gReportDir}/coverage.html
    fi

    find ./ -type f -name "auto_generate_temp_go_test.go" -exec rm -rf {} \;

    while read -r line
    do
        arrData=($line)
        if [ ${#arrData[*]} != 3 ]; then
            continue
        fi
        gRowTotal=$((gRowTotal+arrData[1]))

        if [ "${arrData[1]}" != "0" ] && [ "${arrData[2]}" != "0" ]; then
            gRowHit=$((gRowHit+arrData[1]))
        fi
    done  < ${gReportDir}/coverage.data
    if [ ${gDeleteCoverageData} == 1 ]; then
        rm ${gReportDir}/coverage.data -f
    fi

    while read -r line
    do
        if [ ${line:0-5} != "0.0%" ]; then
            ((gFuncHit++))
        fi
        ((gFuncTotal++))
    done  < ${gReportDir}/coverage.txt
    # 去掉最后一行统计
    ((gFuncHit--))
    ((gFuncTotal--))

    TestCasesNum
}

# 生成html测试报告
function genHtml() {
    echo "generate html report."
    curdir=$(pwd)
    dirName=${curdir##*/}

    datetime=$(date +"%Y/%m/%d %H:%M:%S")
    echo "<!DOCTYPE html>"  > $gHTMLFile
    echo "<html>" >> $gHTMLFile
    echo "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head>"  >> $gHTMLFile
    echo "<body><h2>$dirName code coverage report - gotest[$datetime]</h2>" >> $gHTMLFile
    echo "<hr/>" >> $gHTMLFile

    testCaseNumSuccess=$((gTestCaseNumTotal-gTestCaseNumFailed))
    testCaseFailedCoverage=0
    if [ $gTestCaseNumTotal -ne 0 ];then
        testCaseFailedCoverage=$(awk -v v1=$gTestCaseNumFailed -v v2=$gTestCaseNumTotal 'BEGIN{printf "%.2f",v1*100/v2}')
    fi
    echo "<p style=\"font-size:18px\">测试用例统计:</p>" >> $gHTMLFile
    echo "<table border=\"1\" bordercolor=\"#000000\" width=\"1000\"  style=\"BORDER-COLLAPSE: collapse\" >" >> $gHTMLFile
    echo "<tr style=\"color:White\" bgColor=#0066CC><th>测试用例总数</th><th>失败用例数</th><th>成功用例数</th><th>失败率</th></tr>" >> $gHTMLFile
    echo "<tr align=\"center\" ><td>${gTestCaseNumTotal}</td><td>${gTestCaseNumFailed}</td><td>${testCaseNumSuccess}</td><td>$testCaseFailedCoverage%</td></tr>" >> $gHTMLFile
    echo "</table>" >> $gHTMLFile

    rowCoverage=0
    if [ $gRowTotal -ne 0 ];then
        rowCoverage=$(awk -v v1=$gRowHit -v v2=$gRowTotal 'BEGIN{printf "%.2f",v1*100/v2}')
    fi
    funcCoverage=0
    if [ $gFuncTotal -ne 0 ];then
        funcCoverage=$(awk -v v1=$gFuncHit -v v2=$gFuncTotal 'BEGIN{printf "%.2f",v1*100/v2}')
    fi
    echo "<p style=\"font-size:18px\">覆盖率统计:</p>" >> $gHTMLFile
    echo "<table border=\"1\" bordercolor=\"#000000\" width=\"1000\"  style=\"BORDER-COLLAPSE: collapse\" >" >> $gHTMLFile
    echo "<tr style=\"color:White\" bgColor=#0066CC><th></th><th>Hit</th><th>Total</th><th>Coverage</th></tr>" >> $gHTMLFile
    echo "<tr align=\"center\" ><td>Lines</td><td>$gRowHit</td><td>$gRowTotal</td><td>$rowCoverage%</td></tr>" >> $gHTMLFile
    echo "<tr align=\"center\" ><td>Functions</td><td>$gFuncHit</td><td>$gFuncTotal</td><td>$funcCoverage%</td></tr>" >> $gHTMLFile
    echo "</table>" >> $gHTMLFile

    echo "<p style=\"font-size:18px\">每个函数的覆盖率:</p>" >> $gHTMLFile
    echo "<table border=\"1\" bordercolor=\"#000000\" width=\"1000\"  style=\"BORDER-COLLAPSE: collapse\" >" >> $gHTMLFile
    echo "<tr style=\"color:White\" bgColor=#0066CC><th>go文件</th><th>函数</th><th>覆盖率</th></tr>" >> $gHTMLFile
    while read line  || [[ -n ${line} ]]
    do
        arrLine=($line)
        if [ "${arrLine[0]}" == "total:" ];then
            continue
        fi
        filePath=${arrLine[0]#*$dirName}
        if [ "${filePath:0:1}" != "/"  ]; then
            filePath="/$filePath"
        fi
        echo "<tr align=\"center\" ><td>$dirName$filePath</td><td>${arrLine[1]}</td><td>${arrLine[2]}</td></tr>" >> $gHTMLFile
    done  < ${gReportDir}/coverage.txt

    echo "</table></body></html>" >> $gHTMLFile
    if [ ${gDeleteCoverageData} == 1 ]; then
        rm ${gReportDir}/coverage.txt -f
    fi
}

makeTest
genHtml
