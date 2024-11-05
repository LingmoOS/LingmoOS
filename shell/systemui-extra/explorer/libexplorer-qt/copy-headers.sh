#!/bin/bash
echo "copy header files"
find $(dirname $0) -name "*.h" | xargs -i cp {} $(dirname $0)/development-files/header-files
