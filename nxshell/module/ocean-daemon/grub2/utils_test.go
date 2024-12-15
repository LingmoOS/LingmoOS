// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package grub2

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/stretchr/testify/require"
)

func Test_replaceAndbackupDir(t *testing.T) {
	tmpDirA, err := getTempDir()
	require.NoError(t, err)

	testFile := filepath.Join(tmpDirA, "test")
	dataA := []byte{'a'}
	dataB := []byte{'b'}
	err = os.WriteFile(testFile, dataA, 0755)
	require.NoError(t, err)

	tmpDirB, err := getTempDir()
	require.NoError(t, err)

	err = os.WriteFile(filepath.Join(tmpDirB, "test"), dataB, 0755)
	require.NoError(t, err)

	err = replaceAndBackupDir(tmpDirA, tmpDirB)
	require.NoError(t, err)

	get, err := os.ReadFile(testFile)
	require.NoError(t, err)
	require.Equal(t, dataB, get)

	bakDir := tmpDirA + ".bak"
	get, err = os.ReadFile(filepath.Join(bakDir, "test"))
	require.NoError(t, err)
	require.Equal(t, dataA, get)

	_ = os.RemoveAll(bakDir)
	_ = os.RemoveAll(tmpDirA)
}

func Test_copyBgSource(t *testing.T) {
	src := "testdata/theme/lingmo"
	dst, err := getTempDir()
	require.NoError(t, err)

	err = copyBgSource(src, dst)
	require.NoError(t, err)
	require.FileExists(t, filepath.Join(dst, "background_source"))

	_ = os.RemoveAll(dst)
}
