#!/bin/bash

# 定义宏
ANALOG_INPUT_MIC_CONF="/usr/share/pulseaudio/alsa-mixer/paths/analog-input-mic.conf"
ANALOG_INPUT_LINEIN_CONF="/usr/share/pulseaudio/alsa-mixer/paths/analog-input-linein.conf"
ANALOG_INPUT_REAR_MIC_CONF="/usr/share/pulseaudio/alsa-mixer/paths/analog-input-rear-mic.conf"
ANALOG_INPUT_FRONT_MIC_CONF="/usr/share/pulseaudio/alsa-mixer/paths/analog-input-front-mic.conf"
ANALOG_INPUT_INTERNAL_MIC_CONF="/usr/share/pulseaudio/alsa-mixer/paths/analog-input-internal-mic.conf"

MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-mic-1.conf"
MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-mic-2.conf"
MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-mic-3.conf"
LINEIN_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-linein-1.conf"
LINEIN_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-linein-2.conf"
LINEIN_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-linein-3.conf"
REAR_MIC_BOOST_0="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-rear-mic-0.conf"
REAR_MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-rear-mic-1.conf"
REAR_MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-rear-mic-2.conf"
REAR_MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-rear-mic-3.conf"
FRONT_MIC_BOOST_0="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-front-mic-0.conf"
FRONT_MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-front-mic-1.conf"
FRONT_MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-front-mic-2.conf"
FRONT_MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-front-mic-3.conf"
INTERNAL_MIC_BOOST_1="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-internal-mic-1.conf"
INTERNAL_MIC_BOOST_2="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-internal-mic-2.conf"
INTERNAL_MIC_BOOST_3="/usr/share/pulseaudio/alsa-mixer/paths/lingmo-paths/analog-input-internal-mic-3.conf"

# 定义软链接函数
ukmedia_symlink() {
	local target_file=$1
	local link_name=$2

	# 检查软链接是否存在
	if [ -e "$link_name" ]; then
		echo "Removing existing symlink: $link_name"
		rm "$link_name"
	fi

	# 创建软链接
	ln -s "$target_file" "$link_name"
	echo "Soft link created: $link_name ---> $target_file"
}

# 定义获取Vendor ID函数(声卡唯一性)
get_vendor_id() {
	cat /proc/asound/card*/codec#0 |grep 'Vendor Id'
}

# 定义获取Subsystem ID函数(同款声卡可能存在不同,厂商会做客制化处理)
get_subsystem_id() {
	cat /proc/asound/card*/codec#0 |grep 'Subsystem Id'
}

# 定义获取system product name函数
get_system_product_name() {
	dmidecode -s system-product-name
}

get_baseboard_product_name() {
	dmidecode -s baseboard-product-name
}

# 判断逻辑 1.声卡codec 2.整机型号system-product-name
vendor_id=$(get_vendor_id)
subsystem_id=$(get_subsystem_id)
system_product_name=$(get_system_product_name)
baseboard_product_name=$(get_baseboard_product_name)


if [[ "$vendor_id" =~ "0x10ec0269" ]]; then

        echo "Realtek ALC269VC or ALC269VB"

        if [ "$baseboard_product_name" == "B52-ZK1" ]; then
                echo "创智成 B52-ZK1: Internal = 1"
                ukmedia_symlink "$INTERNAL_MIC_BOOST_1" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
	
	elif [ "$baseboard_product_name" == "MAXHUB-NZ61-6780A" ]; then
                echo "MAXHUB-NZ61: Internal = 1"
                ukmedia_symlink "$INTERNAL_MIC_BOOST_1" "$ANALOG_INPUT_INTERNAL_MIC_CONF"

        elif [ "$baseboard_product_name" == "Phytium D3000 Desktop Board" ]; then
                echo "创智成F301 Phytium D3000, Front = 1, Rear = 1"
                ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
                ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"

	elif [ "$system_product_name" == "BM6J80" ]; then
                echo "宝信创BM6J80: Internal = 2"
                ukmedia_symlink "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
        fi

        exit
fi

if [[ "$vendor_id" =~ "0x10ec0623" ]]; then

        echo "Realtek ALC263"

        if [ "$baseboard_product_name" == "LXKT-ZX-KX7000" ]; then
                echo "Lenovo KaiTian M90Z: Front = 1, Rear = 1"
                ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
                ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"
        fi

        exit
fi

if [[ "$vendor_id" =~ "0x10ec0662" ]]; then

        echo "Realtek ALC662 rev3"

        if [ "$baseboard_product_name" == "LM-LS3A4000-7A1000-1w-V01-pc_A2005" ]; then
                echo "Lenovo KaiTian M90Z: Front = 0, Rear = 2"
                ukmedia_symlink "$REAR_MIC_BOOST_2" "$ANALOG_INPUT_REAR_MIC_CONF"

	elif [ "$baseboard_product_name" == "THTF-FTD2000-1W-MBX-F20ER100" ]; then
                echo "飞腾一体机 超越AF31-T1, Front = 1, Rear = 1"
                ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
                ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"
        fi

        exit
fi

if [[ "$vendor_id" =~ "0x10ec0897" ]]; then

        echo "Realtek ALC897"

        if [ "$baseboard_product_name" == "CE720F" ]; then
                echo "Inspur CE720F/CF520F: Front = 1, Rear = 1"
                ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
		ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"

	elif [ "$baseboard_product_name" == "GW-001M1A-FTF" ]; then
                echo "长城世恒TD120A2, Front = 1, Rear = 1"
                ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
                ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"

	elif [ "$baseboard_product_name" == "THTF-6780A-1W-JZXETH01" ]; then
		echo "兆芯超越AZ30-T1, Front = 1, Rear = 1"
		ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
		ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"

	elif [ "$baseboard_product_name" == "THTF-6780A-1W-JZXETH01" ]; then
                echo "UNIS D3890 G2c, Front = 1, Rear = 1"
                ukmedia_symlink "$FRONT_MIC_BOOST_1" "$ANALOG_INPUT_FRONT_MIC_CONF"
                ukmedia_symlink "$REAR_MIC_BOOST_1" "$ANALOG_INPUT_REAR_MIC_CONF"

	elif [ "$baseboard_product_name" == "FD2000ZX200MB1" ]; then
                echo "联想开天M70F, Front = 0, Rear = 0"
                ukmedia_symlink "$FRONT_MIC_BOOST_0" "$ANALOG_INPUT_FRONT_MIC_CONF"
                ukmedia_symlink "$REAR_MIC_BOOST_0" "$ANALOG_INPUT_REAR_MIC_CONF"

	fi

        exit
fi

if [[ "$vendor_id" =~ "0x14f15098" ]]; then

        echo "Conexant CX20632"

	ukmedia_symlink "$MIC_BOOST_3" "$ANALOG_INPUT_MIC_CONF"
	ukmedia_symlink "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"

        exit
fi

if [[ "$vendor_id" =~ "0x14f11f86" ]]; then

        echo "Conexant CX11880"

        if [ "$baseboard_product_name" == "LXKT-ZXE-N70" ]; then
                echo "Lenovo KaiTian N80Z/N89Z: Internal = 2, Mic = merge"
		ukmedia_symlink "$MIC_BOOST_3" "$ANALOG_INPUT_MIC_CONF"
                ukmedia_symlink "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
	fi

        exit
fi

if [[ "$vendor_id" =~ "0x14f120d1" ]]; then

        echo "Conexant SN6180"

        if [ "$baseboard_product_name" == "LXKT-ZXEP-X1" ]; then
                echo "Lenovo KaiTian X1: Internal = 2, Mic = 2"
		ukmedia_symlink "$MIC_BOOST_2" "$ANALOG_INPUT_MIC_CONF"
                ukmedia_symlink "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"
        fi

        exit
fi

if [[ "$vendor_id" =~ "0x14f11f87" ]]; then

        echo "Conexant SN6140"
	#根据适配记录，此声卡目前统一使用：Internal Mic Boost = 2, Mic Boost = 2
	ukmedia_symlink "$MIC_BOOST_2" "$ANALOG_INPUT_MIC_CONF"
	ukmedia_symlink "$INTERNAL_MIC_BOOST_2" "$ANALOG_INPUT_INTERNAL_MIC_CONF"

        exit
fi

# Support for HUAWEI
if [ -e "/usr/share/pulseaudio/alsa-mixer/profile-sets/pulse-huawei.conf" ]; then
	if [ -f /etc/pulse/daemon-i2s-huawei.conf ]; then
		mv /etc/pulse/daemon-i2s-huawei.conf /etc/pulse/daemon.conf
	fi
	exit
fi

# Check mavis
if [[ `aplay -l |grep -c "sofhdadsp"` != "0" && `amixer contents |grep -c "Bass Speaker Playback"` != "0" ]]; then
        echo "Mavis"
        sed -i "/SectionDevice.\"Headphones1\"/,+36d" /usr/share/alsa/ucm2/sof-hda-dsp/HiFi.conf
        sed -i "/If.monomic/,+51d" /usr/share/alsa/ucm2/sof-hda-dsp/HiFi.conf
	exit
fi

# Check 40A1
if [[ `dmidecode |grep -c "706-LS3A5000-4-V1.0-B40L-41A1"` != "0" ]]; then
        echo "TR41A1"
	ukmedia_symlink "/usr/share/pulseaudio/alsa-mixer/paths/analog-output-headphones-enabled-AutoMute.conf" "/usr/share/pulseaudio/alsa-mixer/paths/analog-output-headphones.conf"
	exit
fi

# Check UNIS-D3830-G3
if [[ `dmidecode |grep "UNIS D3830 G3" -c` != "0" ]]; then
       echo "UNIS D3830 G3"
       ukmedia_symlink "/usr/share/pulseaudio/alsa-mixer/profile-sets/UNIS-D3830-G3-audio.conf" "/usr/share/pulseaudio/alsa-mixer/profile-sets/alc4080-mh243a-usb-audio.conf"
       exit
fi
