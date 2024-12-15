// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
.pragma library

const VpnTypeEnum = Object.freeze({
                                      "l2tp": 0x01,
                                      "pptp": 0x02,
                                      "vpnc": 0x04,
                                      "openvpn": 0x08,
                                      "strongswan": 0x10,
                                      "openconnect": 0x20,
                                      "sstp": 0x40
                                  })
// ip正则表达式
const ipRegExp = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/
const ipv6RegExp = /^((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?$/
// 子网掩码
const maskRegExp = /(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)/
// MAC正则表达式
const macRegExp = /([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})/

function toVpnTypeEnum(vpnKey) {
    let key = vpnKey.substring(31)
    console.log("toVpnTypeEnum", vpnKey, key)
    return VpnTypeEnum.hasOwnProperty(key) ? VpnTypeEnum[key] : 0x01
}

function toVpnKey(vpnType) {
    for (let key in VpnTypeEnum) {
        if (VpnTypeEnum[key] === vpnType) {
            return "org.freedesktop.NetworkManager." + key
        }
    }
    return "org.freedesktop.NetworkManager.l2tp"
}

function removeTrailingNull(str) {
    return str ? str.toString().replace(/\0+$/, '') : ""
}

function numToIp(num) {
    let ips = [0, 0, 0, 0]
    for (var i = 0; i < 4; i++) {
        ips[i] = (num >> (i * 8)) & 255
    }
    return ips.join('.')
}

function ipToNum(ip) {
    console.log("ipToNum----", ip, typeof ip)
    let ips = ip.split('.')
    let cidr = 0
    let ipNum = 0
    if (ips.length !== 4) {
        return 0
    }
    for (let ipStr of ips) {
        let num = parseInt(ipStr, 10)
        ipNum |= ((num & 255) << cidr)
        cidr += 8
    }
    return ipNum
}

function prefixToIp(subnetMask) {
    if (subnetMask < 0 || subnetMask > 32) {
        throw new Error("Subnet mask must be between 0 and 32")
    }

    let maskArray = [255, 255, 255, 255]

    for (var i = 0; i < 4; i++) {
        let byteBits = i * 8 + 8 - subnetMask
        if (byteBits > 0) {
            maskArray[i] = (255 << byteBits) & 255
        }
    }

    return maskArray.join('.')
}

function ipToPrefix(decimalSubnet) {
    let octets = decimalSubnet.split('.')
    let cidr = 0

    for (let octet of octets) {
        let num = parseInt(octet, 10)
        for (var i = 0; i < 8; i++) {
            if ((num & (1 << (7 - i))) !== 0) {
                cidr++
            } else {
                break
            }
        }
        if (cidr === 32)
            break
    }
    return cidr
}

function macToStr(mac) {
    return Array.prototype.map.call(new Uint8Array(mac), x => ('00' + x.toString(16)).toUpperCase().slice(-2)).join(':')
}
function strToMac(str) {
    if (str.length === 0)
        return new Uint8Array()
    let arr = str.split(":")
    let hexArr = arr.join("")
    return new Uint8Array(hexArr.match(/[\da-f]{2}/gi).map(bb => {
                                                               return parseInt(bb, 16)
                                                           })).buffer
}
// 转为ByteArray并以\0结尾
function strToByteArray(data) {
    if (typeof data === 'string') {
        var arr = []
        for (var i = 0; i < data.length; ++i) {
            let charcode = data.charCodeAt(i)
            if (charcode < 0x80) {
                arr.push(charcode)
            } else if (charcode < 0x800) {
                arr.push(0xc0 | (charcode >> 6), 0x80 | (charcode & 0x3f))
            } else if (charcode < 0xd800 || charcode >= 0xe000) {
                arr.push(0xe0 | (charcode >> 12), 0x80 | ((charcode >> 6) & 0x3f), 0x80 | (charcode & 0x3f))
            } else {
                // Handle surrogate pairs (U+10000 to U+10FFFF)
                i++
                charcode = 0x10000 + (((charcode & 0x3ff) << 10) | (str.charCodeAt(i) & 0x3ff))
                arr.push(0xf0 | (charcode >> 18), 0x80 | ((charcode >> 12) & 0x3f), 0x80 | ((charcode >> 6) & 0x3f), 0x80 | (charcode & 0x3f))
            }
        }
        if (arr[arr.length - 1] !== 0) {
            arr.push(0)
        }
        return new Uint8Array(arr).buffer
    } else if (data instanceof ArrayBuffer) {
        const uint8Array = new Uint8Array(data)
        if (uint8Array[uint8Array.length - 1] !== 0) {
            const newUint8Array = new Uint8Array(uint8Array.length + 1)
            newUint8Array.set(uint8Array)
            newUint8Array[newUint8Array.length - 1] = 0
            return newUint8Array.buffer
        } else {
            return data
        }
    }
    return undefined
}
