// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package image_effect

import (
	"bufio"
	"bytes"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"github.com/linuxdeepin/go-lib/utils"
)

func getOutputFile(effect, filename string) (outputFile string) {
	outputDir := filepath.Join(cacheDir, effect)
	md5sum, _ := utils.SumStrMd5(filename)
	outputFile = filepath.Join(outputDir, md5sum+filepath.Ext(filename))
	return
}

func modTimeEqual(t1, t2 time.Time) bool {
	return t1.Unix() == t2.Unix() &&
		(t1.Nanosecond()/1000) == (t2.Nanosecond()/1000)
}

func setFileModTime(filename string, t time.Time) error {
	now := time.Now()
	return os.Chtimes(filename, now, t)
}

func runCmdRedirectStdOut(userName, outputFile string, cmdline, envVars []string) error {
	args := append([]string{"-u", userName, "--"}, cmdline...)
	logger.Debugf("$ runuser %s > %q", strings.Join(args, " "), outputFile)

	cmd := exec.Command("runuser", args...)
	cmd.Env = append(os.Environ(), envVars...)
	var errBuf bytes.Buffer
	cmd.Stderr = &errBuf
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return err
	}
	err = cmd.Start()
	if err != nil {
		return err
	}

	fh, err := os.Create(outputFile)
	if err != nil {
		return err
	}
	defer func() {
		err = fh.Close()
		if err != nil {
			logger.Warning(err)
		}
	}()
	bufWriter := bufio.NewWriter(fh)
	var n int64
	n, err = io.Copy(bufWriter, stdout)
	logger.Debugf("copy %d bytes", n)
	if err != nil {
		return err
	}
	err = bufWriter.Flush()
	if err != nil {
		return err
	}
	err = cmd.Wait()
	if len(errBuf.Bytes()) > 0 {
		logger.Warningf("cmd stderr: %s", errBuf.Bytes())
	}
	if err != nil {
		return err
	}

	return nil

}
