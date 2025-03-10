#!/bin/bash

# 定义状态文件路径
STATUS_FILE="get_status.build.sa"

# 定义配置项映射
declare -A CONFIG_MAP=(
    ["software_status"]="AUTO_BUILD_STATUS_LINGMO_PKG"
    ["iso_status"]="AUTO_BUILD_STATUS_LINGMO_ISO"
    ["version"]="BUILD_VERSION"
    ["target"]="BUILD_TARGET"
    ["arch"]="BUILD_ARCH"
    ["start_time"]="BUILD_START_TIME"
    ["end_time"]="BUILD_END_TIME"
    ["duration"]="BUILD_DURATION"
    ["next_build_time"]="NEXT_BUILD_TIME"
)

# 检查参数数量
if [ "$#" -lt 2 ]; then
    echo "用法: $0 [-p <状态类型> <状态值>] [-v <版本号>] [-t <构建目标>] [-a <构建架构>] [-s <起始时间>] [-e <结束时间>] [-d <持续时间>] [-n <下次构建时间>]"
    echo "示例:"
    echo "  $0 -p software_status 0"
    echo "  $0 -v 1.2.3 -t release -a amd64"
    echo "  $0 -s \"2023-10-01T14:30:00Z\" -e \"2023-10-01T15:45:00Z\""
    exit 1
fi

# 如果文件不存在，则创建
if [ ! -f "$STATUS_FILE" ]; then
    touch "$STATUS_FILE"
fi

# 解析参数
while getopts ":p:v:t:a:s:e:d:n:" opt; do
    case $opt in
        p)
            CONFIG_TYPE="$OPTARG"
            if [[ "$CONFIG_TYPE" != "software_status" && "$CONFIG_TYPE" != "iso_status" ]]; then
                echo "错误: 状态类型必须是 software_status 或 iso_status。"
                exit 1
            fi
            CONFIG_KEY="${CONFIG_MAP[$CONFIG_TYPE]}"
            CONFIG_VALUE="${!OPTIND}"
            if [[ ! "$CONFIG_VALUE" =~ ^[0-4]$ ]]; then
                echo "错误: 状态值必须是 0（成功）、1（构建中）、2（失败）、3（等待）或 4（紧急暂停）。"
                exit 1
            fi
            ;;
        v)
            CONFIG_KEY="${CONFIG_MAP["version"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        t)
            CONFIG_KEY="${CONFIG_MAP["target"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        a)
            CONFIG_KEY="${CONFIG_MAP["arch"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        s)
            CONFIG_KEY="${CONFIG_MAP["start_time"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        e)
            CONFIG_KEY="${CONFIG_MAP["end_time"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        d)
            CONFIG_KEY="${CONFIG_MAP["duration"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        n)
            CONFIG_KEY="${CONFIG_MAP["next_build_time"]}"
            CONFIG_VALUE="$OPTARG"
            ;;
        *)
            echo "无效选项: -$OPTARG"
            exit 1
            ;;
    esac

    # 更新配置文件
    if grep -q "^$CONFIG_KEY=" "$STATUS_FILE"; then
        # 如果配置项已存在，则更新其值
        sed -i "s/^$CONFIG_KEY=.*/$CONFIG_KEY=$CONFIG_VALUE/" "$STATUS_FILE"
    else
        # 如果配置项不存在，则追加到文件
        echo "$CONFIG_KEY=$CONFIG_VALUE" >> "$STATUS_FILE"
    fi

    echo "配置已更新: $CONFIG_KEY=$CONFIG_VALUE"
done
