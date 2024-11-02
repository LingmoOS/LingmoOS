#!/bin/bash

set -e

project_path=$(dirname `realpath $0`)
source ${project_path}/common.sh

if [ ! -d ${generate_pic_dir} ]; then
    ${MKDIR} ${generate_pic_dir}
fi

colors=(
    '000000'
    '00D600'
    '1D52E1'
    'E39900'
    'EE0000'
    'FF2F96'
    'FFFFFF'
)

for color in ${colors[@]}; do
cat>${generate_pic_dir}/rect-${color}.svg<<EOF
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <rect
    x="0" y="0"
    rx="4" ry="4"
    width="16" height="16"
    fill="#${color}"
  />
</svg>
EOF
done

for color in ${colors[@]}; do
cat>${generate_pic_dir}/circle-${color}.svg<<EOF
<svg xmlns="http://www.w3.org/2000/svg" version="1.1">
  <circle
    cx="16" cy="16" r="16"
    fill="#${color}"
  />
</svg>
EOF
done