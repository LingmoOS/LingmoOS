#!/bin/bash

set -e
set -o pipefail

export info_version_string=
export mate_data_add_string=
export os_version_name=
export sign_position=

# 拷贝配置文件
do_copy_config() {
  # 使用绝对路径，方便在 /target 中加载
  cp utils /tmp

  # sp1 使用 automatic
  if grep -q automatic /proc/cmdline; then
    [[ -f /cdrom/ky-installer.cfg ]] && cp /cdrom/ky-installer.cfg /tmp
  # 990/9A0 使用此设置，ple-mode 审核模式，test-mode 全自动安装模式
  elif grep -qE 'ple-mode|test-mode' /proc/cmdline; then
    [[ -f /cdrom/kyple-installer.cfg ]] && cp /cdrom/kyple-installer.cfg /tmp/ky-installer.cfg
  else
    cp /usr/share/lingmo-os-installer/ky-installer.cfg /tmp
  fi
}

do_copy_config

. /tmp/utils


# 运行 prepare 脚本
do_prepare() {
  mkdir -p /var/log/installer

  run_scripts_dir prepare

  if [[ -d /cdrom/hooks/prepare ]]; then
    run_scripts_dir /cdrom/hooks/prepare
  fi
}

# 运行 in_chroot 脚本
do_run_target() {
  mkdir -p /target/${IN_TARGET}
  cp utils /tmp/ky-installer.cfg /target/tmp
  cp -r k.sh in_chroot /target/${IN_TARGET}

  chroot /target bash ${IN_TARGET}/k.sh >>${LOG_FILE} 2>&1
}

# 运行 after 脚本
do_after() {
  run_scripts_dir after

  if [[ -d /cdrom/hooks/after ]]; then
    run_scripts_dir /cdrom/hooks/after
  fi
}

# 执行 post 脚本
do_cdrom_post_actions() {
  if [[ -f /cdrom/.lingmo-post-actions ]] && [[ "${is_ghost}" != "true" ]]; then
    cp /cdrom/.lingmo-post-actions /target/${IN_TARGET}
    chroot /target bash ${IN_TARGET}/.lingmo-post-actions >>${LOG_FILE} 2>&1
  fi

  if [[ -f /cdrom/.lingmo-post-actions-nochroot ]] && [[ "${is_ghost}" != "true" ]]; then
    bash /cdrom/.lingmo-post-actions-nochroot >>${LOG_FILE} 2>&1
  fi
}

# 拷贝日志
do_copy_log() {
  msg "正在拷贝安装日志..."
  cp -r /var/log/installer/ /target/var/log/
}

# 出厂备份
do_factory_backup() {
  umount /target/backup
  msg "正在进行系统备份..."
  ## 工厂备份
  /usr/bin/backup-auto --factorybackup /target /backup || true
  msg "系统备份完成"
}

do_install_success() {
  if [[ "${is_990_9a0}" == "true" ]]; then
    mkdir -p /target/boot/efi
    touch /target/boot/efi/INSTALL_SUCCESS
  fi
}

handle_factory_backup() {
  factory_backup=$(get_value factory-backup)
  if [[ -f /target/etc/.bootinfo ]]; then
    if grep -q 'factory-backup' /proc/cmdline; then
      do_factory_backup
    elif [[ $factory_backup -eq 1 ]]; then
      do_factory_backup
    fi
  fi
}

do_finished() {
  echo 'finished'
  touch /tmp/z-finished-install
}

rm_deploy_systemroot_usr_config() {
  cat "/usr/share/lingmo-os-installer/scripts/ostree_system_usr_etc_config" | while read filename
  do
    echo "$filename"
	realPathFilename="$1$filename"
	[ -f $realPathFilename ] && rm $realPathFilename
  done

}

get_ostree_release_file_info() {
	while read os_release_file_line
	do
		if [ "$os_release_file_line" = "[info]" ]; then
			sign_position=0
			continue
		fi

		if [ $sign_position = 0 ] && [ "$os_release_file_line" != "[info]" ]; then
			info_version_string=$os_release_file_line
			sign_position=1
			continue
		fi

		if [ "$os_release_file_line" != "[metadata]" ]; then
			mate_data_add_string="$mate_data_add_string --add-metadata-string=$os_release_file_line"
		fi

	done < "/etc/ostree-release"

	info_version_string=${info_version_string#*version=}
	os_version_name=$(echo "$info_version_string" | awk -F ":" '{print $1}')
	echo "$os_version_name" >> ${LOG_FILE} 2>&1
	echo "$info_version_string" >> ${LOG_FILE} 2>&1
	echo "$mate_data_add_string" >> ${LOG_FILE} 2>&1
}
#拷贝并建立数据链接
create_copy(){
	source_dir=$1
	target_dir=$2
    if [ ! -d "/target$source_dir" ]; then
    	echo "/target$source_dir don't exist"  >>${LOG_FILE} 2>&1
		return 
	fi

	if [ ! -d "$target_dir" ]; then
    	mkdir -p "$target_dir"
    	if [ $? -ne 0 ]; then
        	echo "create $target_dir failed"  >>${LOG_FILE} 2>&1
			return
    	fi
	fi

	cp -r "/target$source_dir"/* "$target_dir"
	if [ $? -ne 0 ]; then
    	echo "copy /target$source_dir failed"  >>${LOG_FILE} 2>&1
    	return
	fi

	rm -rf "/target$source_dir"
	if [ $? -ne 0 ]; then
    	echo "delete /target$source_dir failed"  >>${LOG_FILE} 2>&1
    	return
	fi
	if [ ! -d "/usr/ost-ex$source_dir" ]; then
    	mkdir -p "/usr/ost-ex$source_dir"
    	if [ $? -ne 0 ]; then
        	echo "create /usr/ost-ex$source_dir failed"  >>${LOG_FILE} 2>&1
        	return
    	fi
	fi
	#必须要确保链接建立正常
	ln -s "/usr/ost-ex$source_dir" "/target$source_dir"

	if [ $? -ne 0 ]; then
    	echo "create ln failed"  >>${LOG_FILE} 2>&1
    	return
	fi
	echo "---handle ost-ex end---" >>${LOG_FILE} 2>&1
}
#检测后续服务端需要保持一致的目录
update_data(){

config_file="/target/usr/ost-ex/.ost-update.config"
if [ ! -f "$config_file" ]; then
    echo "$config_file don't exist"  >>${LOG_FILE} 2>&1
	if [ ! -d "/target/usr/ost-ex" ]; then
    	mkdir -p "/target/usr/ost-ex"
    	if [ $? -ne 0 ]; then
        	echo "create /target/usr/ost-ex failed"  >>${LOG_FILE} 2>&1
        	return
    	fi
	fi
	touch $config_file
	echo "/var/lib/dpkg" > $config_file
fi
count=0
while IFS= read -r line; do
    kv_pairs[$count]="$line"
    count=$((count+1))
done < "$config_file"

for ((i=0; i<count; i++)); do
    if [ -L "/target${kv_pairs[$i]}" ]; then
        echo "/target${kv_pairs[$i]} is linked"  >>${LOG_FILE} 2>&1
    else
        echo "/target${kv_pairs[$i]} isn't linked"  >>${LOG_FILE} 2>&1
		#将来拓展其他目录
        if [[ "${kv_pairs[$i]}" == "/var"* ]]; then
            target_dir="/target/usr/ost-ex${kv_pairs[$i]}"
			create_copy ${kv_pairs[$i]} $target_dir
        fi
    fi
done
}
# 运行 prepare 阶段脚本
mkdir -p /var/log/installer

#自动安装自动选择磁盘安装方案
if grep -q automatic /proc/cmdline; then
        devpath=$(get_value devpath)
        if [[ -z $devpath ]]; then
                 auto_disk >>${LOG_FILE}
        else
                 get_disk  >>${LOG_FILE}
        fi
fi


do_prepare >>${LOG_FILE} 2>&1

# 执行 target inchroot 脚本
do_run_target

# 运行 after 阶段脚本
do_after >>${LOG_FILE} 2>&1

# 执行 postactions 脚本
do_cdrom_post_actions

# 拷贝安装日志
do_copy_log

do_install_success

# 处理出厂备份
handle_factory_backup
LANG=zh_CN.UTF-8
ostree_value_installer=$(get_value ostree_value)
if [ "${ostree_value_installer}" = "true" ]; then

	get_ostree_release_file_info

	umount /target/proc
	umount -l /target/sys
	umount /target/dev/pts
	umount /target/dev
	umount /target/run

	# 删除dev下的空设备节点，会导致commit失败
	rm /target/dev/console /target/dev/full /target/dev/null /target/dev/ptmx /target/dev/random /target/dev/tty /target/dev/urandom /target/dev/zero

	# 将etc目录复制到usr目录下
	cp -a /target/etc /target/usr/

	# 删除etc目录
	rm -r /target/etc

	echo "---------->>>>>>>>>000000 1" >>${LOG_FILE} 2>&1

	# 复制内核文件
	kernel_vesion=`uname -r`
	initrd_real_name=`readlink -f /target/boot/initrd.img`
	cp -a "/target/boot/vmlinuz-$kernel_vesion" "/target/usr/lib/modules/$kernel_vesion/vmlinuz"
	cp -a "$initrd_real_name" "/target/usr/lib/modules/$kernel_vesion/initramfs.img"
	#mkdir /tmp/boot-config
	#cp -a "/target/boot/config-*" "/tmp/boot-config/"

	echo "---------->>>>>>>>>000000 2" >>${LOG_FILE} 2>&1

	mkdir /target/sysroot
	# 卸载其余分区，只保留根分区文件
	# 目前卸载data、boot、efi、backup、home、root，后续需要根据分区进行卸载
	umount /target/data
	#umount /target/boot/efi
	umount -l /target/boot
	umount /target/home
	umount /target/backup
	umount /target/root
	umount /target/cdrom
	echo "---------->>>>>>>>>000000 3" >>${LOG_FILE} 2>&1

	# 将根分区底下的home、root、opt、mnt、media、tmp、srv移至var目录，然后将根分区下做成软连接
	#mv /target/home /target/var/
	#ln -s /var/home /target/home

	#mv /target/root /target/var/
	#ln -s /var/root /target/root

	mv /target/opt /target/var/
	ln -s /var/opt /target/opt

	mv /target/mnt /target/var/
	ln -s /var/mnt /target/mnt

	mv /target/media /target/run/
	ln -s /run/media /target/media

	mv /target/tmp /target/var/
	ln -s /var/tmp /target/tmp

	mv /target/srv /target/var/
	ln -s /var/srv /target/srv

	update_data
	
	#部署ostree
	[ -d /tmp/ostree ] || mkdir /tmp/ostree
	#备份用户信息
	cp -a /target/usr/etc /tmp/
	rm_deploy_systemroot_usr_config /target

	data_part=$(get_value part-data)
	efi_part=$(get_value part-efi)
	boot_part=$(get_value part-boot)
	root_part=$(get_value part-root)

	echo "data_part=${data_part}" >>${LOG_FILE} 2>&1
	mount "${data_part}" /tmp/ostree

	[ -d /tmp/ostree/sysroot ] || mkdir /tmp/ostree/sysroot
	echo "---------->>>>>>>>>000000 4" >>${LOG_FILE} 2>&1
	ostree admin init-fs /tmp/ostree/sysroot
	echo "---------->>>>>>>>>000000 5" >>${LOG_FILE} 2>&1
	ln -s /sysroot/ostree /target/ostree
	echo "---------->>>>>>>>>000000 6" >>${LOG_FILE} 2>&1
	ostree admin os-init "$os_version_name" --sysroot=/tmp/ostree/sysroot
	echo "---------->>>>>>>>>000000 7" >>${LOG_FILE} 2>&1

	# 将deploy节点下根文件系统var目录移到sysroot/var目录下
	[ -d /tmp/ostree/sysroot/var ] || mkdir /tmp/ostree/sysroot/var
	rsync -a /target/var/* /tmp/ostree/sysroot/var/
	rm -r /target/var/*

	ostree commit --repo=/tmp/ostree/sysroot/ostree/repo -b "$info_version_string" -s "$info_version_string" $mate_data_add_string /target/ >>${LOG_FILE} 2>&1
	echo "---------->>>>>>>>>000000 8" >>${LOG_FILE} 2>&1

	mount "${boot_part}" /mnt
	#ls /mnt/ | grep -v config- | xargs rm -rf
	for mnt_boot_file in /mnt/*; do
		if echo $mnt_boot_file | grep -v "config-"; then
			echo "mnt_boot_file: $mnt_boot_file" >> ${LOG_FILE} 2>&1
			rm -rf $mnt_boot_file
		fi
	done
	umount /mnt

	mount "${boot_part}" "/tmp/ostree/sysroot/boot"


	# 获取根分区的UUID
	root_uuid=`lsblk -o UUID ${part_root} | grep -v "UUID"`
	cmdline_opention_argr="$root_uuid ro quiet splash security="
	ostree admin deploy "$info_version_string" --os="$os_version_name" --sysroot=/tmp/ostree/sysroot --karg=root="UUID=$cmdline_opention_argr" >>${LOG_FILE} 2>&1
	echo "---------->>>>>>>>>000000 9" >>${LOG_FILE} 2>&1

	# 将第一个部署点作为默认启动项
	ostree admin pin 0 --sysroot=/tmp/ostree/sysroot
	deploy_hashvalue0=`ostree admin status --sysroot=/tmp/ostree/sysroot | head -n 1 | awk -F " " '{print $2}'`
	## 重新部署作为回滚点
	ostree admin deploy "$info_version_string" --retain --os="$os_version_name" --sysroot=/tmp/ostree/sysroot --karg=root="UUID=$cmdline_opention_argr" >>${LOG_FILE} 2>&1

	#删除根文件系统/target 目录
	echo "---------->>>>>>>>>000000 10" >>${LOG_FILE} 2>&1
	deploy_hashvalue=`ostree admin status --sysroot=/tmp/ostree/sysroot | head -n 1 | awk -F " " '{print $2}'`
	echo "deploy_hashvalue---------->>>>>>>>>>$deploy_hashvalue" >>${LOG_FILE} 2>&1

	umount /tmp/ostree/sysroot/boot
	rm -r /target/*
	echo "---------->>>>>>>>>000000 11" >>${LOG_FILE} 2>&1
	# 后续考虑找到对应的deploy节点，修改其权限，使用mv操作
	#rsync -a /tmp/ostree/sysroot/* /target/
	#chattr -i "/tmp/ostree/sysroot/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue"
	for sysroot_dir_file in /tmp/ostree/sysroot/ostree/deploy/$os_version_name/deploy/*; do
		if echo $sysroot_dir_file | grep -v "origin" > /dev/null; then
			chattr -i $sysroot_dir_file
		fi
	done
	mv /tmp/ostree/sysroot/* /target/
	rm -r /tmp/ostree/sysroot/
	cp -a /tmp/etc /target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/
	cp -a /tmp/etc /target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue0/
	#rm_deploy_systemroot_usr_config "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue"
	for sysroot_dir_file in /target/ostree/deploy/$os_version_name/deploy/*; do
    	if echo $sysroot_dir_file | grep -v "origin" > /dev/null; then
        chattr +i $sysroot_dir_file
    	fi
	done
	echo "---------->>>>>>>>>000000 12" >>${LOG_FILE} 2>&1

	efibootdir="/sys/firmware/efi/efivars"

	mount --bind "/target" "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sysroot"
	mount "${boot_part}" "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sysroot/boot"

	if [ -d $efibootdir ]; then
		[ -d /target/boot/efi ] || mkdir /target/boot/efi
		mount "${efi_part}" "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sysroot/boot/efi"
	fi

	mount --bind "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sysroot/boot" "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/boot"
	if [ -d $efibootdir ]; then 
		mount --bind "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sysroot/boot/efi" "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/boot/efi"
	fi

	mount --bind /dev "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/dev"
	mount --bind /dev/pts "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/dev/pts"
	mount --bind /proc "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/proc"
	mount --bind /sys "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sys"

	if [ -d $efibootdir ]; then
		mount --bind /sys/firmware/efi/efivars "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/sys/firmware/efi/efivars"
	fi

	if [ -d $efibootdir ]; then 
		chroot "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue" grub-install >>${LOG_FILE} 2>&1
	else
		dev_path=$(get_value devpath)
		chroot "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue" grub-install $dev_path >>${LOG_FILE} 2>&1
	fi
	chroot "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue" update-grub >>${LOG_FILE} 2>&1
	mv "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/boot/grub/grub.cfg" "/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/boot/loader/grub.cfg"
	ln -s ../loader/grub.cfg /target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue/boot/grub/grub.cfg
	#cp -a "/tmp/boot-config/config-*" "/target/boot/"
	sync
fi

unset info_version_string
unset mate_data_add_string
unset os_version_name
unset sign_position
# 卸载文件系统
umount_all

do_finished
