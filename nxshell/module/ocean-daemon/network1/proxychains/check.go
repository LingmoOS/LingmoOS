// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package proxychains

import (
	"net"
	"regexp"
	"strings"
)

var ipv4Reg = regexp.MustCompile(`\d+\.\d+\.\d+\.\d+`)

func validType(type0 string) bool {
	switch type0 {
	case "http", "socks4", "socks5":
		return true
	default:
		return false
	}
}

func validIPv4(ipStr string) bool {
	if !ipv4Reg.MatchString(ipStr) {
		return false
	}

	ip := net.ParseIP(ipStr)
	return ip != nil
}

func validUser(user string) bool {
	return !strings.ContainsAny(user, "\t ")
}

func validPassword(password string) bool {
	return validUser(password)
}
