#!/bin/bash
# Description : dumpTomlFileName
# Version:  20240830 first creator
# Function to clean the input  # Remove "N/A"
# useage: sudo chmod +x dumpTomlFileName.sh  run it  ./dumpTomlFileName.sh


remove_special_chars() {
    local input="$1"
    if [[ "$input" == *"N/A"* ]]; then
        input=""
    fi
    # Remove special characters using sed
    input=$(echo "$input" | sed 's/[-\/'"'"'~!@#$%^&*(){}:;,.\"\\|~`]//g')
    # Return cleaned input
    echo "$input"
}

# Run the dmidecode command and capture the output
output=$(sudo dmidecode -t 1)
# Parse the output using grep and awk
Manufacturer=$(echo "$output" | grep "Manufacturer" | awk -F ": " '{print $2}')
ProductName=$(echo "$output" | grep "Product Name" | awk -F ": " '{print $2}')
Version=$(echo "$output" | grep "Version" | awk -F ": " '{print $2}')

# Remove special characters from each variable
Manufacturer_cleaned=$(remove_special_chars "$Manufacturer")
productname_cleaned=$(remove_special_chars "$ProductName")
version_cleaned=$(remove_special_chars "$Version")

# Construct the TOML file name
tomlFileName="oeminfo_${Manufacturer_cleaned}_${productname_cleaned}_${version_cleaned}"
# Trim whitespace and convert to lowercase
tomlFileName=$(echo "$tomlFileName" | xargs | tr '[:upper:]' '[:lower:]')
# Remove spaces and add .toml extension
tomlFileName=$(echo "$tomlFileName" | tr -d ' ')".toml"
# Display the final TOML file name

# check the file exist 判断文件是否存在
if [ -f "/etc/deepin/hardware/${tomlFileName}" ];then
echo "TOML File Name  /etc/deepin/hardware/${tomlFileName}  exist"
else
echo "TOML File Name:$tomlFileName  please mkdir /etc/deepin/hardware/ , and put in"
fi
