// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package network

type ipv4AddressesWrapper []ipv4AddressWrapper
type ipv4AddressWrapper struct {
	Address string
	Mask    string
	Gateway string
}

// Ipv6AddressesWrapper
type ipv6AddressesWrapper []ipv6AddressWrapper
type ipv6AddressWrapper struct {
	Address string
	Prefix  uint32
	Gateway string
}

// Ipv6Addresses is an array of (byte array, uint32, byte array)
type ipv6Addresses []ipv6Address
type ipv6Address struct {
	Address []byte
	Prefix  uint32
	Gateway []byte
}

// ipv6Routes is an array of (byte array, uint32, byte array, uint32)
type ipv6Route struct {
	Address []byte
	Prefix  uint32
	NextHop []byte
	Metric  uint32
}
type ipv6Routes []ipv6Route
