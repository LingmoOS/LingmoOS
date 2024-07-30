#!/bin/sh

linux_image()
{
	if grep -Eq "^LINGMO_LINUX_KERNEL_UIMAGE=y$" ${LINGMO_CONFIG}; then
		echo "uImage"
	elif grep -Eq "^LINGMO_LINUX_KERNEL_IMAGE=y$" ${LINGMO_CONFIG}; then
		echo "Image"
	elif grep -Eq "^LINGMO_LINUX_KERNEL_IMAGEGZ=y$" ${LINGMO_CONFIG}; then
		echo "Image.gz"
	else
		echo "zImage"
	fi
}

generic_getty()
{
	if grep -Eq "^LINGMO_TARGET_GENERIC_GETTY=y$" ${LINGMO_CONFIG}; then
		echo ""
	else
		echo "s/\s*console=\S*//"
	fi
}

PARTUUID="$($HOST_DIR/bin/uuidgen)"

install -d "$TARGET_DIR/boot/extlinux/"

sed -e "$(generic_getty)" \
	-e "s/%LINUXIMAGE%/$(linux_image)/g" \
	-e "s/%PARTUUID%/$PARTUUID/g" \
	"board/orangepi/common/extlinux.conf" > "$TARGET_DIR/boot/extlinux/extlinux.conf"

sed "s/%PARTUUID%/$PARTUUID/g" "board/orangepi/common/genimage.cfg" > "$BINARIES_DIR/genimage.cfg"
