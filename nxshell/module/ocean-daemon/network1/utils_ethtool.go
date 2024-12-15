// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/netlink.h>

static __u32 get_ethtool_cmd_speed(const char* iface) {
  int fd;
  __u32 ret = 0;

  struct ifreq ifr;
  strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name));

  struct ethtool_cmd ecmd;
  memset(&ecmd, 0, sizeof(ecmd));
  ecmd.cmd = ETHTOOL_GSET;
  ifr.ifr_data = (void *)&ecmd;

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd < 0) {
    printf("Cannot get control socket");
    return 0;
  }
  if (ioctl(fd, SIOCETHTOOL, &ifr) == 0) {
    ret = (ecmd.speed_hi << 16) | ecmd.speed;
    if (ret == (__u16)(-1) || ret == (__u32)(-1))
      ret = 0;
  }

  close(fd);
  return ret;
}
*/
import "C"
import (
	"errors"
	"fmt"
	"math"
	"syscall"
	"unsafe"
)

// Maximum size of an interface name
const (
	IFNAMSIZ = 16
)

// ioctl ethtool request
const (
	SIOCETHTOOL = 0x8946
)

// ethtool stats related constants.
const (
	ETH_GSTRING_LEN  = 32
	ETH_SS_STATS     = 1
	ETH_SS_FEATURES  = 4
	ETHTOOL_GDRVINFO = 0x00000003
	ETHTOOL_GSTRINGS = 0x0000001b
	ETHTOOL_GSTATS   = 0x0000001d
	// other CMDs from ethtool-copy.h of ethtool-3.5 package
	ETHTOOL_GSET      = 0x00000001 /* Get settings. */
	ETHTOOL_SSET      = 0x00000002 /* Set settings. */
	ETHTOOL_GMSGLVL   = 0x00000007 /* Get driver message level */
	ETHTOOL_SMSGLVL   = 0x00000008 /* Set driver msg level. */
	ETHTOOL_GCHANNELS = 0x0000003c /* Get no of channels */
	ETHTOOL_SCHANNELS = 0x0000003d /* Set no of channels */
	ETHTOOL_GCOALESCE = 0x0000000e /* Get coalesce config */
	/* Get link status for host, i.e. whether the interface *and* the
	 * physical port (if there is one) are up (ethtool_value). */
	ETHTOOL_GLINK         = 0x0000000a
	ETHTOOL_GMODULEINFO   = 0x00000042 /* Get plug-in module information */
	ETHTOOL_GMODULEEEPROM = 0x00000043 /* Get plug-in module eeprom */
	ETHTOOL_GPERMADDR     = 0x00000020
	ETHTOOL_GFEATURES     = 0x0000003a /* Get device offload settings */
	ETHTOOL_SFEATURES     = 0x0000003b /* Change device offload settings */
	ETHTOOL_GFLAGS        = 0x00000025 /* Get flags bitmap(ethtool_value) */
	ETHTOOL_GSSET_INFO    = 0x00000037 /* Get string set info */
)

type ethtoolCmd struct { /* ethtool.c: struct ethtool_cmd */
	Cmd            uint32
	Supported      uint32
	Advertising    uint32
	Speed          uint16
	Duplex         uint8
	Port           uint8
	Phy_address    uint8
	Transceiver    uint8
	Autoneg        uint8
	Mdio_support   uint8
	Maxtxpkt       uint32
	Maxrxpkt       uint32
	Speed_hi       uint16
	Eth_tp_mdix    uint8
	Reserved2      uint8
	Lp_advertising uint32
	Reserved       [2]uint32
}

type ifreq struct {
	ifr_name [IFNAMSIZ]byte
	ifr_data uintptr
}

func sendIOCtl(fd uintptr, ifReq uintptr) error {
	_, _, errno := syscall.Syscall(syscall.SYS_IOCTL, fd, SIOCETHTOOL, ifReq)
	if errno != 0 {
		fmt.Printf("Fail to execute ioctl, the errno is %+v", errno)
		return errors.New("failed execute ioctl")
	}
	return nil
}

func getEthtoolCmdSpeed(intf string) (uint32, error) {
	fd, err := syscall.Socket(syscall.AF_INET, syscall.SOCK_DGRAM, syscall.IPPROTO_IP)
	if err != nil {
		return 0, err
	}
	if fd < 0 {
		fd, err = syscall.Socket(syscall.AF_NETLINK, syscall.SOCK_RAW, syscall.NETLINK_GENERIC)
		if err != nil {
			return 0, err
		}
	}
	if fd < 0 {
		return 0, errors.New("cannot get control socket")
	}
	defer syscall.Close(fd)

	ecmd := &ethtoolCmd{
		Cmd: ETHTOOL_GSET,
	}

	var name [IFNAMSIZ]byte
	copy(name[:], intf)

	ifr := ifreq{
		ifr_name: name,
		ifr_data: uintptr(unsafe.Pointer(ecmd)),
	}

	err = sendIOCtl(uintptr(fd), uintptr(unsafe.Pointer(&ifr)))
	if err != nil {
		return 0, err
	}

	var speedval = (uint32(ecmd.Speed_hi) << 16) |
		(uint32(ecmd.Speed) & 0xffff)
	if speedval == math.MaxUint16 || speedval == math.MaxUint32 {
		speedval = 0
	}

	return speedval, nil
}

func getEthtoolCmdSpeedCgo(intf string) uint32 {
	cName := C.CString(intf)
	ret := uint32(C.get_ethtool_cmd_speed(cName))
	C.free(unsafe.Pointer(cName))

	return ret
}
