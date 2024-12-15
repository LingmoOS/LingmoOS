// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub_common

import (
	"bytes"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"sort"
	"strconv"
	"strings"

	"github.com/lingmoos/go-lib/encoding/kv"
)

const (
	GrubParamsFile            = "/etc/default/grub"
	OCEANGrubParamsFile         = "/etc/default/grub.d/11_ocean.cfg"
	GfxmodeDetectReadyPath    = "/tmp/lingmo-gfxmode-detect-ready"
	LingmoGfxmodeDetect       = "LINGMO_GFXMODE_DETECT"
	LingmoGfxmodeAdjusted     = "LINGMO_GFXMODE_ADJUSTED"
	LingmoGfxmodeNotSupported = "LINGMO_GFXMODE_NOT_SUPPORTED"
)

func LoadGrubParamsFile(file string) (map[string]string, error) {
	params := make(map[string]string)
	f, err := os.Open(file)
	if err != nil {
		return params, err
	}
	defer f.Close()

	r := kv.NewReader(f)
	r.TrimSpace = kv.TrimLeadingTailingSpace
	r.Comment = '#'
	for {
		pair, err := r.Read()
		if err != nil {
			break
		}
		if pair.Key == "" {
			continue
		}
		params[pair.Key] = pair.Value
	}

	return params, nil
}

func LoadGrubParams() (map[string]string, error) {
	return LoadGrubParamsFile(GrubParamsFile)
}

func LoadOCEANGrubParams() (map[string]string, error) {
	return LoadGrubParamsFile(OCEANGrubParamsFile)
}

func DecodeShellValue(in string) string {
	output, err := exec.Command("/bin/sh", "-c", "echo -n "+in).Output()
	if err != nil {
		// fallback
		return strings.Trim(in, "\"")
	}
	return string(output)
}

type Gfxmode struct {
	Width  int
	Height int
}

func (v Gfxmode) String() string {
	return fmt.Sprintf("%dx%d", v.Width, v.Height)
}

func getBootArgLingmoGfxmode() (string, error) {
	filename := "/proc/cmdline"
	content, err := os.ReadFile(filename)
	if err != nil {
		return "", err
	}
	fields := bytes.Split(content, []byte(" "))
	const prefix = "LINGMO_GFXMODE="
	var result string
	for _, field := range fields {
		if bytes.HasPrefix(field, []byte(prefix)) {
			result = string(bytes.TrimSpace(field[len(prefix):]))
			break
		}
	}

	return result, nil
}

func GetBootArgLingmoGfxmode() (cur Gfxmode, all Gfxmodes, err error) {
	lingmoGfxmode, err := getBootArgLingmoGfxmode()
	if err != nil {
		return
	}

	cur, all, err = parseBootArgLingmoGfxmode(lingmoGfxmode)
	return
}

func parseBootArgLingmoGfxmode(str string) (cur Gfxmode, all Gfxmodes, err error) {
	fields := strings.Split(str, ",")
	if len(fields) < 2 {
		err = errors.New("length of fields < 2")
		return
	}
	var curIdx int
	curIdx, err = strconv.Atoi(string(fields[0]))
	if err != nil {
		return
	}

	for _, field := range fields[1:] {
		var m Gfxmode
		m, err = ParseGfxmode(field)
		if err != nil {
			return
		}

		all = append(all, m)
	}

	if curIdx < 0 || curIdx >= len(all) {
		err = fmt.Errorf("curIdx %d out of range [0,%d]", curIdx, len(all))
		return
	}

	return all[curIdx], all, nil
}

var gfxmodeReg = regexp.MustCompile(`^\d+x\d+$`)

func ParseGfxmode(str string) (Gfxmode, error) {
	if !gfxmodeReg.MatchString(str) {
		return Gfxmode{}, fmt.Errorf("invalid gfxmode %q", str)
	}

	var v Gfxmode
	_, err := fmt.Sscanf(str, "%dx%d", &v.Width, &v.Height)
	if err != nil {
		return Gfxmode{}, err
	}

	return v, nil
}

type Gfxmodes []Gfxmode

func (v Gfxmodes) Len() int {
	return len(v)
}

func (v Gfxmodes) Less(i, j int) bool {
	a := v[i]
	b := v[j]

	return a.Width*a.Height < b.Width*b.Height
}

func (v Gfxmodes) Swap(i, j int) {
	v[i], v[j] = v[j], v[i]
}

func (v Gfxmodes) Add(m Gfxmode) Gfxmodes {
	var found bool
	for _, r0 := range v {
		if r0 == m {
			found = true
			break
		}
	}
	if !found {
		return append(v, m)
	}
	return v
}

func (v Gfxmodes) Max() (max Gfxmode) {
	for _, m := range v {
		if m.Width*m.Height > max.Width*max.Height {
			max = m
		}
	}
	return
}

func (v Gfxmodes) Intersection(v1 Gfxmodes) (result Gfxmodes) {
	dict := make(map[Gfxmode]struct{})
	for _, m := range v {
		dict[m] = struct{}{}
	}

	for _, m := range v1 {
		if _, ok := dict[m]; ok {
			result = append(result, m)
		}
	}
	return
}

func (v Gfxmodes) SortDesc() {
	sort.Sort(sort.Reverse(v))
}

func ShouldFinishGfxmodeDetect(params map[string]string) bool {
	if params[LingmoGfxmodeDetect] == "1" {
		_, err := os.Stat(GfxmodeDetectReadyPath)
		if os.IsNotExist(err) {
			return true
		}
	}
	return false
}

func InGfxmodeDetectionMode(params map[string]string) bool {
	return params[LingmoGfxmodeDetect] == "1"
}

func IsGfxmodeDetectFailed(params map[string]string) bool {
	return params[LingmoGfxmodeDetect] == "2"
}

func HasLingmoGfxmodeMod() bool {
	dir := "/boot/grub"
	for _, platform := range []string{"i386-pc", "x86_64-efi", "arm64-efi", "sw64-efi"} {
		modFile := filepath.Join(dir, platform, "lingmo_gfxmode.mod")
		_, err := os.Stat(modFile)
		if err == nil {
			return true
		}
	}
	return false
}
