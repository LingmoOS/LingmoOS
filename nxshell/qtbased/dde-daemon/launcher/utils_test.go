// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package launcher

import (
	"os"
	"os/user"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_getAppIdByFilePath(t *testing.T) {
	appDirs := []string{"/usr/share/applications", "/usr/local/share/applications", "/home/test_user/.local/share/applications"}

	id := getAppIdByFilePath("/usr/share/applications/d-feet.desktop", appDirs)
	assert.Equal(t, id, "d-feet")

	id = getAppIdByFilePath("/usr/share/applications/kde4/krita.desktop", appDirs)
	assert.Equal(t, id, "kde4/krita")

	id = getAppIdByFilePath("/usr/local/share/applications/deepin-screenshot.desktop", appDirs)
	assert.Equal(t, id, "deepin-screenshot")

	id = getAppIdByFilePath("/home/test_user/.local/share/applications/space test.desktop", appDirs)
	assert.Equal(t, id, "space test")

	id = getAppIdByFilePath("/other/dir/a.desktop", appDirs)
	assert.Equal(t, id, "")
}

func Test_getUserAppDir(t *testing.T) {
	dataDir := os.Getenv("XDG_DATA_HOME")
	if dataDir == "" {
		home := os.Getenv("HOME")
		if home == "" {
			user, err := user.Current()
			if err != nil {
				// 跳过此测试
				t.Skip()
				return
			}

			home = user.HomeDir
		}

		dataDir = filepath.Join(home, ".local/share")
	}

	assert.Equal(t, getUserAppDir(), filepath.Join(dataDir, "applications"))
}

func Test_runeSliceDiff(t *testing.T) {
	// pop
	popCount, runesPush := runeSliceDiff([]rune("abc"), []rune("abc"))
	assert.Equal(t, popCount, 0)
	assert.Equal(t, len(runesPush), 0)

	popCount, runesPush = runeSliceDiff([]rune("abc"), []rune("abcd"))
	assert.Equal(t, popCount, 1)
	assert.Equal(t, len(runesPush), 0)

	popCount, runesPush = runeSliceDiff([]rune("abc"), []rune("abcde"))
	assert.Equal(t, popCount, 2)
	assert.Equal(t, len(runesPush), 0)

	// push
	popCount, runesPush = runeSliceDiff([]rune("abcd"), []rune("abc"))
	assert.Equal(t, popCount, 0)
	assert.Equal(t, len(runesPush), 1)
	assert.Equal(t, runesPush[0], 'd')

	popCount, runesPush = runeSliceDiff([]rune("abcde"), []rune("abc"))
	assert.Equal(t, popCount, 0)
	assert.Equal(t, len(runesPush), 2)
	assert.Equal(t, runesPush[0], 'd')
	assert.Equal(t, runesPush[1], 'e')

	// pop and push
	popCount, runesPush = runeSliceDiff([]rune("abcd"), []rune("abce"))
	assert.Equal(t, popCount, 1)
	assert.Equal(t, len(runesPush), 1)
	assert.Equal(t, runesPush[0], 'd')

	popCount, runesPush = runeSliceDiff([]rune("deepin"), []rune("deeinp"))
	assert.Equal(t, popCount, 3)
	assert.Equal(t, len(runesPush), 3)
	assert.Equal(t, runesPush[0], 'p')
	assert.Equal(t, runesPush[1], 'i')
	assert.Equal(t, runesPush[2], 'n')
}

func Test_parseFlatpakAppCmdline(t *testing.T) {
	info, err := parseFlatpakAppCmdline(`/usr/bin/flatpak run --branch=master --arch=x86_64 --command=blender --file-forwarding org.blender.Blender @@ %f @@`)
	assert.NoError(t, err)
	assert.Equal(t, info, &flatpakAppInfo{
		name:   "org.blender.Blender",
		arch:   "x86_64",
		branch: "master",
	})
}
