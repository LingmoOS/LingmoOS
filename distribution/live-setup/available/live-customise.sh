#!/bin/bash
# customise script for live-wrapper, passed to vmdebootstrap as part
# of the live build

set -e

# Allow the chroot to work on /tmp, independent of TMPDIR from the host
unset TMPDIR

rootdir=$1

# common needs rootdir to already be defined.
. /usr/share/vmdebootstrap/common/customise.lib

trap cleanup 0

mount_support
disable_daemons

mv ${rootdir}/etc/resolv.conf ${rootdir}/etc/resolv.conf.bak
cat /etc/resolv.conf > ${rootdir}/etc/resolv.conf

prepare_apt_source "${LWR_MIRROR}" "${LWR_DISTRIBUTION}"

# Temporary fix for #843983
chroot ${rootdir} chmod 755 /

for PKG in ${FIRMWARE_PKGS}; do
    echo "$PKG        $PKG/license/accepted       boolean true" | \
	chroot ${rootdir} debconf-set-selections
done

chroot ${rootdir} apt-get -q -y install eatmydata

chroot ${rootdir} apt-get -q -y install initramfs-tools live-boot live-config ${LWR_TASK_PACKAGES} ${LWR_EXTRA_PACKAGES} ${LWR_FIRMWARE_PACKAGES} task-laptop task-english libnss-myhostname >> vmdebootstrap.log 2>&1

# Work out what extra packages we need for the installer to work Need
# to run these one at a time, as some of them may conflict if we ask
# apo to install them all together (e.g. grub-efi-$ARCH and grub-pc)
if [ "${LWR_BASE_DEBS}"x != ""x ] ; then
    for PKG in ${LWR_BASE_DEBS}; do
	chroot ${rootdir} apt-get -q -s -u install --reinstall $PKG | awk '/^Inst/ {print $2}' >> base_debs.$PKG.list
    done
    sort -u base_debs.*.list > base_debs.list
    rm -f base_debs.*.list
fi

# We told vmdebootstrap to lock the root password, which will have set
# an (encrypted) password field of "!*" in /etc/shadow. That will
# confuse user-setup-udeb later on such that we won't get a root
# password set if we do an installation. Quick hack fix for now is to
# change that. Later on, let's get user-setup-udeb fixed to handle
# this properly too.
sed -i '/root/s,!,,g' ${rootdir}/etc/shadow

chroot ${rootdir} apt-get -y -q remove --purge eatmydata

# Find all the packages included
export COLUMNS=500
chroot ${rootdir} dpkg -l | awk '/^ii/ {printf "%s %s\n",$2,$3}' > packages.list

# Grab source URLs for all the packages
cat > ${rootdir}//list-sources <<EOF
#!/bin/sh
export COLUMNS=500
for PKG in \$(dpkg -l | awk '/^ii/ {printf "%s ",\$2}'); do
    apt-get source -qq --print-uris \$PKG
done
EOF
chmod +x ${rootdir}/list-sources
chroot ${rootdir} /list-sources > sources.list
rm -f ${rootdir}/list-sources

echo "blacklist bochs-drm" > $rootdir/etc/modprobe.d/qemu-blacklist.conf

replace_apt_source

# Remove stale resolv.conf
rm ${rootdir}/etc/resolv.conf

# Replace stale fstab with incorrect entries with empty one
echo > ${rootdir}/etc/fstab

# Move base.list to sources.list so that we don't end up with duplicate
# sources on live media (we probably need a less roundabout way of handeling
# all of this)
mv ${rootdir}/etc/apt/sources.list.d/base.list ${rootdir}/etc/apt/sources.list

