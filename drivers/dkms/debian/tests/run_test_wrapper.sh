#!/bin/sh

KERNEL_VER=
NO_SIGNING_TOOL=
for kver in $(dpkg-query -W -f '${Package}\n' 'linux-headers-*' | sed s/linux-headers-//)
do
	if [ -d "/lib/modules/$kver/build" ]
	then
		KERNEL_VER=$kver
		grep -q "^CONFIG_MODULE_SIG_HASH=" "/lib/modules/$kver/build/.config" ||
			NO_SIGNING_TOOL="--no-signing-tool"
		break
	fi
done

export KERNEL_VER
bash ./run_test.sh $NO_SIGNING_TOOL
