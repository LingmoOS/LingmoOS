#!/bin/bash

################################################
#字体管理器
###############################################
deepin_font_manager(){
#{{{
    local font_log=~/.cache/deepin/deepin-font-manager/deepin-font-manager.log
    #关闭自体管理器
    kill_deepin_font_manager(){
    #{{{
        local font_close='N'
        #查找到已经启动自体管理器
        local font_pid=`ps -eo pid,comm | grep 'deepin-font' | awk '{print $1}'`

        if [ $font_pid ] ; then
           read -p "自体管理器已经启动,是否关闭,重新测试? y-Yes/n-No:" font_close

           if [ ! $font_close ] ; then
                font_close='N'
           fi

           if [ $font_close == 'y' ] || [ $fontclose == 'Y' ] ; then
                sudo kill -9  $font_pid
           else
                exit 0
           fi
        fi
        #清空log日志
        if [ -f $font_log ] ; then
            sudo rm -rf $font_log
            sleep 0.2
        fi
    #}}}
    }

    #检查有没有安装自体管理器
    local font_manager=`which deepin-font-manager`

    if [ ! $font_manager ]; then
        echo "没有发现自体管理器被安装"
        exit 0
    else
        echo "发现字体管理器，程序文件:$font_manager"
    fi

    #版本
    local font_version=''
    kill_deepin_font_manager
    local font_version=`$font_manager -v | cut -d ' ' -f 2 | cut -d '+' -f 1`
    $font_manager 1>/dev/null 2>&1 &
    local font_cpu=0
    local font_mem=0
    sleep 0.2
    local icpu=0
    while [ 1 ] ; do
        local font_pid=`ps -eo pid,comm | grep 'deepin-font' | awk '{print $1}'`
        if [ ! $font_pid ] ; then
            break;
        fi
      #  font_top=`top -n1 -p $font_pid | grep "deepin-font-m" | col -xb | tr -s ' '`
      local font_top=`top -n1 -p $font_pid  | grep "deepin-font-m" | col -xb`
      font_top=`echo "$font_top" | sed -e 's/.*Bm\(.*\)Bm.*/\1/g'`
   #   font_cpu=`echo "$font_top" | cut -d ' ' -f 9`
   #   font_mem=`echo "$font_top" | cut -d ' ' -f 10`
      local font_cpu=`echo "$font_top" | awk '{print $9}'`
      local font_mem=`echo "$font_top" | awk '{print $10}'`
        if [ $font_cpu ] && [ $font_mem ] ; then
            icpu=$((icpu+1))
         #   echo "$font_top"
            echo "cpu 采样次数:$icpu,当前cpu使用率:$font_cpu 当前mem使用率: $font_mem"
        fi
        sleep 1
    done
    sleep 0.5
   # clear;
    echo "分析中，请稍后..."
    sleep 0.5
    #分析结果文件
    local font_result=`date +%Y-%m-%d_%H-%M-%S`_font.txt
    if [ -f $font_result ] ; then
        rm -rf $font_result
    fi
    echo "结果存储在: $font_result"

    echo "字体管理器版本: $font_version" >> $font_result
    local font_grab=`grep "[GRABPOINT]" $font_log`
    #一 启动时长
    local font_startup_duration=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0001" | sed -e 's/.*DEEPIN_FONT_MANAGER-0001\(.*\)#.*/\1/g' -e 's/ //g'`
    echo "启动时长: $font_startup_duration 毫秒" >> $font_result

    #二 从启动到加载字体完成时长
    local font_startup_load_duration=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0007" | sed -e 's/.*DEEPIN_FONT_MANAGER-0007\(.*\)#.*/\1/g' -e 's/ //g'`
    echo "启动至加载字体完成时长: $font_startup_load_duration 毫秒" >> $font_result
    #font_startup_tot_duration=$[$font_startup_load_duration + $font_startup_duration];
    #echo "启动时长: $font_startup_tot_duration 毫秒" >> $font_result

    #三 安装
    local font_inst_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0003" | wc -l`
    echo $"安装次数为 $font_inst_times 次" >> $font_result

    # 安装字体个数和安装时长
    local font_inst_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0003"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0003\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    for((i =1;i <= $font_inst_times; i++));do
        local font_inst_number=`echo "$font_inst_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_inst_duration=`echo "$font_inst_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "安装字体数: $font_inst_number 个,安装时长: $font_inst_duration 毫秒"
    done >> $font_result

    #四 导出字体
    local font_export_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0004" | wc -l`
    echo $"导出次数为 $font_export_times 次" >> $font_result
    local font_export_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0004"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0004\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    # 导出字体个数和导出时长
    for((i =1;i <= $font_export_times; i++));do
        local font_export_number=`echo "$font_export_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_export_duration=`echo "$font_export_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "导出字体数: $font_export_number 个,导出时长: $font_export_duration 毫秒"
    done >> $font_result

    #五 删除字体
    local font_delete_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0005" | wc -l`
    echo $"删除次数为 $font_delete_times 次" >> $font_result
    local font_delete_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0005"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0005\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    # 删除字体个数和删除时长
    for((i =1;i <= $font_delete_times; i++));do
        local font_delete_number=`echo "$font_delete_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_delete_duration=`echo "$font_delete_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "删除字体数: $font_delete_number 个,删除时长: $font_delete_duration 毫秒"
    done >> $font_result

    #六 禁用字体
    local font_deactive_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0009" | wc -l`
    echo $"禁用次数为 $font_deactive_times 次" >> $font_result
    local font_deactive_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0009"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0009\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    # 禁用字体个数和禁用时常
    for((i =1;i <= $font_deactive_times; i++));do
        local font_deactive_number=`echo "$font_deactive_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_deactive_duration=`echo "$font_deactive_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "禁用字体数: $font_deactive_number 个,禁用时长: $font_deactive_duration 毫秒"
    done >> $font_result

    #七 启用字体
    local font_active_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0008" | wc -l`
    echo $"启用次数为 $font_active_times 次" >> $font_result
    local font_active_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0008"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0008\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    # 启用字体个数和启用时长
    for((i =1;i <= $font_active_times; i++));do
        local font_active_number=`echo "$font_active_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_active_duration=`echo "$font_active_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "启用字体数: $font_active_number 个,启用时长: $font_active_duration 毫秒"
    done >> $font_result

    #八 收藏字体
    local font_favorite_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0006" | wc -l`
    echo $"收藏次数为 $font_favorite_times 次" >> $font_result
    local font_favorite_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0006"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0006\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    # 收藏字体个数和收藏时长
    for((i =1;i <= $font_favorite_times; i++));do
        local font_favorite_number=`echo "$font_favorite_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_favorite_duration=`echo "$font_favorite_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "收藏字体数: $font_favorite_number 个,收藏时长: $font_favorite_duration 毫秒"
    done >> $font_result

    #九 取消收藏字体
    local font_unfavorite_times=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0010" | wc -l`
    echo $"取消收藏次数为 $font_unfavorite_times 次" >> $font_result
    local font_unfavorite_data=`echo "$font_grab" | grep "DEEPIN_FONT_MANAGER-0010"  | sed -e 's/.*DEEPIN_FONT_MANAGER-0010\(.*\)#.*/\1/g' | sed 's/^[ ]*\(.*\)/\1/g' | nl -w1 -s : -n ln`
    # 取消收藏字体个数和时长
    for((i =1;i <= $font_unfavorite_times; i++));do
        local font_unfavorite_number=`echo "$font_unfavorite_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 1`
        local font_unfavorite_duration=`echo "$font_unfavorite_data" | grep "$i:" | sed -e 's/.*:\(.*\)/\1/g' | cut -d ' ' -f 2`
        echo "取消收藏字体数: $font_unfavorite_number 个,取消收藏时长: $font_unfavorite_duration 毫秒"
    done >> $font_result

    echo  "分析结果:"
    cat < $font_result

    return 0;
#}}}
}

#在程序的GUI界面中关闭程序，脚本会自动统计数据，请不要关闭终端或取消脚本执行
deepin_font_manager;

