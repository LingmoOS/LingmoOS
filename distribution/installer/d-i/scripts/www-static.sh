#!/bin/sh
#
# Create or update basic files under the www directory

set -e

www=$1
if [ -z "$www" -o ! -d "$www" ]; then
  echo "E: www directory not specified or not a directory, aborting"
  exit 1
fi

echo "Updating static contents"
rsync -av www-static/* $www/static/

echo "Creating .htaccess file if required"
if [ ! -f "$www/.htaccess" ]; then
  echo 'AddCharset UTF-8 .txt' > "$www/.htaccess"
fi
