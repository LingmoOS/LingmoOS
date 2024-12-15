#!/bin/bash  

if [ $# -lt 2 ] ; then
echo "USAGE: $0  <input svg dir> <output dci dir>"
echo " e.g.: $0 /usr/share/bloom /tmp/dcis/bloom"
exit 1;
fi

# 定义Ctrl+C信号处理函数
ctrl_c() {
    echo "Ctrl+C pressed. Exiting..."
    exit 1
}

# 捕获Ctrl+C信号，并调用ctrl_c函数
trap ctrl_c SIGINT

in_dir=$1
tmp_in_dir=/tmp/svgs/$(basename $in_dir)
out_dir=$2
echo "$in_dir ==> $out_dir"

if [ ! -d "$tmp_in_dir" ]; then
    mkdir -p "$tmp_in_dir"
fi

set -x

find "$in_dir" -type f -name "*.svg" | while read file; do
    if grep -q "<feGaussianBlur*" "$file"; then
        relative_path=$(realpath --relative-to="$in_dir" "$file")
        target_dir="$tmp_in_dir/$(basename $(dirname $relative_path))"
        if [ ! -d "$target_dir" ]; then
            mkdir -p "$target_dir"
        fi

        cp -n "$file" "$target_dir"
        # if [ $? -ne 0 ]; then
        #      echo "Skipped $file (file already exists in $target_dir)"
        # fi
    fi
done

svg2dci_tool="/usr/libexec/dtk5/DGui/bin/dci-icon-theme"
#svg2dci_tool="/home/mike/src/build/build-dtkgui-Release/tools/dci-icon-theme/dci-icon-theme"

if [ ! -f "$svg2dci_tool" ]; then
    echo "dci-icon-theme not installed"
    sudo apt install libdtkgui5-bin
    svg2dci_tool=$(dpkg -L libdtkgui5-bin | grep dci-icon-theme)
fi 

function version_gt() {
 test "$(echo "$@" | tr " " "\n" | sort -V | head -n 1)" != "$1";
  }

$svg2dci_tool -v 
ver=$($svg2dci_tool -v | cut -d ' ' -f2)
req_ver="0.0.5"

if version_gt $req_ver $ver; then
   echo "requested version >=$req_ver. but found $ver"
   exit
fi

$svg2dci_tool "$tmp_in_dir" -o "$out_dir" -O 3=95