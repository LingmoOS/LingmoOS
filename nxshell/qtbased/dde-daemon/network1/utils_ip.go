// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

import (
	"encoding/binary"
	"fmt"
	"strconv"
	"strings"
	"unicode"
	"unsafe"
)

const (
	macAddrZero  = "00:00:00:00:00:00"
	ipv4Zero     = "0.0.0.0" //nolint
	ipv6AddrZero = "0000:0000:0000:0000:0000:0000:0000:0000"
)

func ntohl(n uint32) uint32 {
	return binary.BigEndian.Uint32((*(*[4]byte)(unsafe.Pointer(&n)))[:])
}

func htonl(n uint32) uint32 {
	bytes := make([]byte, 4)
	binary.BigEndian.PutUint32(bytes, n)
	return *(*uint32)(unsafe.Pointer(&bytes[0]))
}

// ipToUint32 将 IPv4 字符串转换为 uint32。
// 因此处只处理 NetworkManager 返回的数据，故不处理八进制与十六进制字符串，
// 并假定所有传入字符串都是合法的 IP。
func ipToUint32(ip string) uint32 {
	var ret uint32
	var part uint32 = 1
	var val uint32 = 0
	var base uint32 = 10

	for _, c := range ip {
		if unicode.IsDigit(c) {
			val = val*base + uint32(c-'0')
			continue
		}

		if c == '.' {
			ret |= (val << ((4 - part) * 8))
			val = 0
			part++
		}
	}

	ret |= (val << ((4 - part) * 8))

	return ret
}

const ipv4MaxSize = 15

// uint32ToIP 将 uint32 转为 IPv4 字符串。
func uint32ToIP(l uint32) string {
	var sb strings.Builder
	sb.Grow(ipv4MaxSize)

	for p := 3; p >= 0; p-- {
		// 右移后转为 byte（保留最后 8 位）
		b := byte(l >> (uint(p) * 8))

		sb.WriteString(strconv.FormatUint(uint64(b), 10))
		if p != 0 {
			sb.WriteRune('.')
		}
	}

	return sb.String()
}

// []byte{0,0,0,0,0,0} -> "00:00:00:00:00:00"
func convertMacAddressToString(v []byte) (macAddr string) {
	if len(v) != 6 {
		macAddr = macAddrZero
		logger.Error("machine address is invalid", v)
		return
	}
	macAddr = fmt.Sprintf("%02X:%02X:%02X:%02X:%02X:%02X", v[0], v[1], v[2], v[3], v[4], v[5])
	return
}

// "00:00:00:00:00:00" -> []byte{0,0,0,0,0,0}
func convertMacAddressToArrayByte(v string) (macAddr []byte) {
	macAddr, err := convertMacAddressToArrayByteCheck(v)
	if err != nil {
		logger.Error(err)
	}
	return
}

func convertMacAddressToArrayByteCheck(v string) (macAddr []byte, err error) {
	macAddr = make([]byte, 6)
	a := strings.Split(v, ":")
	if len(a) != 6 {
		err = fmt.Errorf("machine address is invalid %s", v)
		return
	}
	for i := 0; i < 6; i++ {
		if len(a[i]) != 2 {
			err = fmt.Errorf("machine address is invalid %s", v)
			return
		}
		var n uint64
		n, err = strconv.ParseUint(a[i], 16, 8)
		if err != nil {
			err = fmt.Errorf("machine address is invalid %s", v)
			return
		}
		macAddr[i] = byte(n)
	}
	return
}

// 24 -> "255.255.255.0"(string format)
func convertIpv4PrefixToNetMask(prefix uint32) (maskAddress string) {
	mask := uint32(0xFFFFFFFF << (32 - prefix))
	maskAddress = uint32ToIP(mask)
	return
}

// "255.255.255.0"(string format) -> 24
func convertIpv4NetMaskToPrefix(maskAddress string) (prefix uint32) {
	prefix, err := convertIpv4NetMaskToPrefixCheck(maskAddress)
	if err != nil {
		logger.Error(err)
	}
	return
}

func convertIpv4NetMaskToPrefixCheck(maskAddress string) (prefix uint32, err error) {
	var mask uint32 // network order
	mask = ipToUint32(maskAddress)
	if err != nil {
		return
	}
	foundZerorBit := false
	for i := uint32(0); i < 32; i++ {
		if mask>>(32-i-1)&0x01 == 1 {
			if !foundZerorBit {
				prefix++
			} else {
				err = fmt.Errorf("net mask address is invalid %s", maskAddress)
				return
			}
		} else {
			foundZerorBit = true
			continue
		}
	}
	return
}

// []byte{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -> "0000:0000:0000:0000:0000:0000:0000:0000"
func convertIpv6AddressToString(v []byte) (ipv6Addr string) {
	ipv6Addr, err := convertIpv6AddressToStringCheck(v)
	if err != nil {
		logger.Error(err)
	}
	return
}
func convertIpv6AddressToStringCheck(v []byte) (ipv6Addr string, err error) {
	if len(v) != 16 {
		ipv6Addr = ipv6AddrZero
		err = fmt.Errorf("ipv6 address is invalid %s", v)
		return
	}
	for i := 0; i < 16; i += 2 {
		s := fmt.Sprintf("%02X%02X", v[i], v[i+1])
		if len(ipv6Addr) == 0 {
			ipv6Addr = s
		} else {
			ipv6Addr = ipv6Addr + ":" + s
		}
	}
	return
}

// "0000:0000:0000:0000:0000:0000:0000:0000" -> []byte{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
func convertIpv6AddressToArrayByte(v string) (ipv6Addr []byte) {
	ipv6Addr, err := convertIpv6AddressToArrayByteCheck(v)
	if err != nil {
		logger.Error(err)
	}
	return
}

func convertIpv6AddressToArrayByteCheck(v string) (ipv6Addr []byte, err error) {
	v, err = expandIpv6Address(v)
	if err != nil {
		return
	}
	ipv6Addr = make([]byte, 16)
	a := strings.Split(v, ":")
	if len(a) != 8 {
		err = fmt.Errorf("ipv6 address is invalid %s", v)
		return
	}
	for i := 0; i < 8; i++ {
		s := a[i]
		if len(s) != 4 {
			err = fmt.Errorf("ipv6 address is invalid %s", v)
			return
		}

		var tmpn uint64
		tmpn, err = strconv.ParseUint(s[:2], 16, 8)
		ipv6Addr[i*2] = byte(tmpn)
		if err != nil {
			err = fmt.Errorf("ipv6 address is invalid %s", v)
			return
		}

		tmpn, err = strconv.ParseUint(s[2:], 16, 8)
		if err != nil {
			err = fmt.Errorf("ipv6 address is invalid %s", v)
			return
		}
		ipv6Addr[i*2+1] = byte(tmpn)
	}
	return
}

// expand ipv6 address to standard format, such as
// "0::0" -> "0000:0000:0000:0000:0000:0000:0000:0000"
// "2001:DB8:2de::e13" -> "2001:DB8:2de:0:0:0:0:e13"
// "2001::25de::cade" -> error
func expandIpv6Address(v string) (ipv6Addr string, err error) {
	a1 := strings.Split(v, ":")
	l1 := len(a1)
	if l1 > 8 {
		err = fmt.Errorf("invalid ipv6 address %s", v)
		return
	}

	a2 := strings.Split(v, "::")
	l2 := len(a2)
	if l2 > 2 {
		err = fmt.Errorf("invalid ipv6 address %s", v)
		return
	} else if l2 == 2 {
		// expand "::"
		abbrFields := ":"
		for i := 0; i <= 8-l1; i++ {
			abbrFields += "0000:"
		}
		v = strings.Replace(v, "::", abbrFields, -1)
	}

	// expand ":0:" to ":0000:"
	a1 = strings.Split(v, ":")
	for i, field := range a1 {
		l := len(field)
		if l > 4 {
			err = fmt.Errorf("invalid ipv6 address %s", v)
			return
		} else if l < 4 {
			field = strings.Repeat("0", 4-l) + field
		}
		if i == 0 {
			ipv6Addr = field
		} else {
			ipv6Addr += ":" + field
		}
	}
	return
}
