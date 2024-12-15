#!/bin/bash

echo "==> rename files"
for f in *.xml; do
  mv -vf "${f}" "${f/#org.gnome/com.lingmo.wrap.gnome}" &>/dev/null
done

echo "==> rename gsettings path"
for f in *.xml; do
  sed -e 's=org\.gnome=com.lingmo.wrap.gnome=g' \
      -e 's=/org/gnome=/com/lingmo/wrap/gnome=g' -i "${f}"
done

echo "==> show unmet files"
ls -1 | grep -v lingmo
