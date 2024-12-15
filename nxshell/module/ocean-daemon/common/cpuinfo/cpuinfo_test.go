// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
package cpuinfo

import (
	"strings"
	"testing"
)

func TestCPUInfo(t *testing.T) {

	cpuinfo, err := ReadCPUInfo("testdata/cpuinfo")
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("%+v", cpuinfo)

	if len(cpuinfo.Processors) != 16 {
		t.Fatal("wrong processor number : ", len(cpuinfo.Processors))
	}

	if cpuinfo.NumCore() != 8 {
		t.Fatal("wrong core number", cpuinfo.NumCore())
	}

	if cpuinfo.NumPhysicalCPU() != 1 {
		t.Fatal("wrong physical cpu number", cpuinfo.NumPhysicalCPU())
	}

	cpuinfo, err = ReadCPUInfo("testdata/cpuinfo2")
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("%+v", cpuinfo)

	if len(cpuinfo.Processors) != 8 {
		t.Fatal("wrong processor number : ", len(cpuinfo.Processors))
	}

	if !strings.HasPrefix(cpuinfo.Hardware, "PANGU") {
		t.Fatal("wrong hardware", cpuinfo.Hardware)
	}
}
