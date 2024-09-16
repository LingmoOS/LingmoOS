// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package status

import "testing"

func TestGetPackageList(t *testing.T) {
	var origList = []string{
		"accountsservice" + _STATUS_DELIM + "amd64",
		"acl" + _STATUS_DELIM + "amd64",
		"acpid" + _STATUS_DELIM + "amd64",
		"adduser" + _STATUS_DELIM + "all",
		"adwaita-icon-theme" + _STATUS_DELIM + "all",
		"alsa-utils" + _STATUS_DELIM + "amd64",
		"apt" + _STATUS_DELIM + "amd64",
		"test" + _STATUS_DELIM,
	}

	infos, err := GetStatusList("testdata/status")
	if err != nil {
		t.Error("Except nil, but got error:", err)
		return
	}
	if len(infos) != len(origList) {
		t.Errorf("Except %d packages, but got %d", len(origList), len(infos))
		return
	}

	pkgList := infos.ListPackage()
	for i := 0; i < len(origList); i++ {
		if origList[i] != pkgList[i] {
			t.Errorf("Except %s, but got %s", origList[i], pkgList[i])
		}
	}
}

var _acpidInfo = PackageStatus{
	Package:       "acpid",
	Status:        "install ok installed",
	Priority:      "optional",
	Section:       "admin",
	InstalledSize: "146",
	Maintainer:    "Debian Acpi Team <pkg-acpi-devel@lists.alioth.debian.org>",
	Architecture:  "amd64",
	Version:       "1:2.0.31-1",
	Contents: []string{
		"Depends: libc6 (>= 2.27), lsb-base (>= 3.2-14), kmod",
		"Recommends: acpi-support-base (>= 0.114-1)",
		"Conffiles:",
		" /etc/default/acpid 5b934527919a9bba89c7978d15e918b3",
		" /etc/init.d/acpid 2ba41d3445b3052d9d2d170b7a9c30dc",
		"Description: Advanced Configuration and Power Interface event daemon",
		" Modern computers support the Advanced Configuration and Power Interface (ACPI)",
		" to allow intelligent power management on your system and to query battery and",
		" configuration status.",
		" .",
		" ACPID is a completely flexible, totally extensible daemon for delivering",
		" ACPI events. It listens on netlink interface (or on the deprecated file",
		" /proc/acpi/event), and when an event occurs, executes programs to handle the",
		" event. The programs it executes are configured through a set of configuration",
		" files, which can be dropped into place by packages or by the admin.",
		"Homepage: http://sourceforge.net/projects/acpid2/",
	},
}

func TestGetPackageStatus(t *testing.T) {
	infos, err := GetStatusList("testdata/status")
	if err != nil {
		t.Error("Except nil, but got error:", err)
		return
	}

	info := infos.Get(_acpidInfo.Package, _acpidInfo.Architecture)
	if !info.Equal(&_acpidInfo) {
		t.Errorf("Except: %#v, but got: %#v", _acpidInfo, info)
	}
}

func TestPackageStatusSave(t *testing.T) {
	data := `Package: acpid
Status: install ok installed
Priority: optional
Section: admin
Installed-Size: 146
Maintainer: Debian Acpi Team <pkg-acpi-devel@lists.alioth.debian.org>
Architecture: amd64
Version: 1:2.0.31-1
Depends: libc6 (>= 2.27), lsb-base (>= 3.2-14), kmod
Recommends: acpi-support-base (>= 0.114-1)
Conffiles:
 /etc/default/acpid 5b934527919a9bba89c7978d15e918b3
 /etc/init.d/acpid 2ba41d3445b3052d9d2d170b7a9c30dc
Description: Advanced Configuration and Power Interface event daemon
 Modern computers support the Advanced Configuration and Power Interface (ACPI)
 to allow intelligent power management on your system and to query battery and
 configuration status.
 .
 ACPID is a completely flexible, totally extensible daemon for delivering
 ACPI events. It listens on netlink interface (or on the deprecated file
 /proc/acpi/event), and when an event occurs, executes programs to handle the
 event. The programs it executes are configured through a set of configuration
 files, which can be dropped into place by packages or by the admin.
Homepage: http://sourceforge.net/projects/acpid2/

`

	content := _acpidInfo.Bytes()
	if string(content) != data {
		t.Errorf("Except `%s`, but got `%s`", data, content)
	}
}
