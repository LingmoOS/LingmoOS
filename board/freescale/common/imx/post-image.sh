#!/usr/bin/env bash

#
# dtb_list extracts the list of DTB files from LINGMO_LINUX_KERNEL_INTREE_DTS_NAME
# in ${BR_CONFIG}, then prints the corresponding list of file names for the
# genimage configuration file
#
dtb_list()
{
	local DTB_LIST

	DTB_LIST="$(sed -n 's/^LINGMO_LINUX_KERNEL_INTREE_DTS_NAME="\([\/a-z0-9 \-]*\)"$/\1/p' "${LINGMO_CONFIG}")"

	for dt in $DTB_LIST; do
		echo -n "\"$(basename "${dt}").dtb\", "
	done
}

#
# linux_image extracts the Linux image format from LINGMO_LINUX_KERNEL_UIMAGE in
# ${BR_CONFIG}, then prints the corresponding file name for the genimage
# configuration file
#
linux_image()
{
	if grep -Eq "^LINGMO_LINUX_KERNEL_UIMAGE=y$" "${LINGMO_CONFIG}"; then
		echo "\"uImage\""
	elif grep -Eq "^LINGMO_LINUX_KERNEL_IMAGE=y$" "${LINGMO_CONFIG}"; then
		echo "\"Image\""
	elif grep -Eq "^LINGMO_LINUX_KERNEL_IMAGEGZ=y$" "${LINGMO_CONFIG}"; then
		echo "\"Image.gz\""
	else
		echo "\"zImage\""
	fi
}

genimage_type()
{
	if grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8M=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8MM=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8MN=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8MP=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8X=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8DXL=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx8"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX91=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx9"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX93=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_imx9"
	elif grep -Eq "^LINGMO_LINUX_KERNEL_INSTALL_TARGET=y$" "${LINGMO_CONFIG}"; then
		if grep -Eq "^LINGMO_TARGET_UBOOT_SPL=y$" "${LINGMO_CONFIG}"; then
		    echo "genimage.cfg.template_no_boot_part_spl"
		else
		    echo "genimage.cfg.template_no_boot_part"
		fi
	elif grep -Eq "^LINGMO_TARGET_UBOOT_SPL=y$" "${LINGMO_CONFIG}"; then
		echo "genimage.cfg.template_spl"
	else
		echo "genimage.cfg.template"
	fi
}

imx_offset()
{
	if grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8M=y$" "${LINGMO_CONFIG}"; then
		echo "33K"
	elif grep -Eq "^LINGMO_PACKAGE_FREESCALE_IMX_PLATFORM_IMX8MM=y$" "${LINGMO_CONFIG}"; then
		echo "33K"
	else
		echo "32K"
	fi
}

uboot_image()
{
	if grep -Eq "^LINGMO_TARGET_UBOOT_FORMAT_DTB_IMX=y$" "${LINGMO_CONFIG}"; then
		echo "u-boot-dtb.imx"
	elif grep -Eq "^LINGMO_TARGET_UBOOT_FORMAT_IMX=y$" "${LINGMO_CONFIG}"; then
		echo "u-boot.imx"
	elif grep -Eq "^LINGMO_TARGET_UBOOT_FORMAT_DTB_IMG=y$" "${LINGMO_CONFIG}"; then
	    echo "u-boot-dtb.img"
	elif grep -Eq "^LINGMO_TARGET_UBOOT_FORMAT_IMG=y$" "${LINGMO_CONFIG}"; then
	    echo "u-boot.img"
	fi
}

main()
{
	local FILES IMXOFFSET UBOOTBIN GENIMAGE_CFG GENIMAGE_TMP
	FILES="$(dtb_list) $(linux_image)"
	IMXOFFSET="$(imx_offset)"
	UBOOTBIN="$(uboot_image)"
	GENIMAGE_CFG="$(mktemp --suffix genimage.cfg)"
	GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"

	sed -e "s/%FILES%/${FILES}/" \
		-e "s/%IMXOFFSET%/${IMXOFFSET}/" \
		-e "s/%UBOOTBIN%/${UBOOTBIN}/" \
		"board/freescale/common/imx/$(genimage_type)" > "${GENIMAGE_CFG}"

	rm -rf "${GENIMAGE_TMP}"

	genimage \
		--rootpath "${TARGET_DIR}" \
		--tmppath "${GENIMAGE_TMP}" \
		--inputpath "${BINARIES_DIR}" \
		--outputpath "${BINARIES_DIR}" \
		--config "${GENIMAGE_CFG}"

	rm -f "${GENIMAGE_CFG}"

	exit $?
}

main "$@"
