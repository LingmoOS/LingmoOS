
TODO: 
选择安装方式时，默认高亮到快速安装 

file: 
/etc/network/interfaces 
/etc/resolv.conf 
/etc/udev/rules.d/70-persistent-net.rules 
/etc/apt/sources.list 
/etc/apt/preferences.d 
/etc/papersize 
/etc/apt/trusted.gpg 
/etc/init.d/apparmor 
/etc/default/locale 
/etc/environment 
/etc/locale.gen 
/etc/X11/xorg.conf.failsafe 
/etc/lightdm/lightdm.conf 
/etc/debconf.conf 
/etc/default/keyboard.pre-ubiquity 
/etc/default/keyboard 
/target/etc/initramfs-tools/modules 
/etc/init.d/console-setup 
/etc/console-setup/ckb 
/etc/casper.conf 
/etc/X11/xinit/xinput.d/all_ALL 
/etc/X11/xinit/xinput.d/default 
/etc/X11/xorg.conf 
usr/sbin/update-initramfs 
/boot/grub/locale 

#systemsetting.ini 
[config] 
autologin=0；1自动登录，0密码登录 
devpath=/dev/sda；全盘安装磁盘 
hostname=tr-PC；hostname 
language=zh_CN；语言 
password=gf45dr；登录密码 
timezone=Asia/Shanghai；时区 
username=tr；用户名 
automatic-installation = 1；1全盘安装，0自定义安装 
factory-backup =0；1工厂备份，0否 
enable-swapfile=false; 使用 swapfile，优先级低于引导参数 kyswapfile


[custompartition] 
bootloader=;引导路径 
partitions=；被挂载的分区 

[setting] 
EnableSwap=false；交换分区 
FileSystem="ext4;ext3;ext2;btrfs;jfs;xfs;fat16;fat32;efi;linux-swap;unused"；创建分区页面的文件系统列表 
FileSystemBoot="ext4;vfat"；boot分区文件系统格式 
PartitionMountedPoints=";/;/boot;/home;/tmp;/var;/srv;/opt;/usr/local"；创建分区页面的挂载点 
