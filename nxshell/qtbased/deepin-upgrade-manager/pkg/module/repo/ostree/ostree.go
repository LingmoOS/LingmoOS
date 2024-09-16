// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package ostree

import (
	"deepin-upgrade-manager/pkg/module/repo/branch"
	"deepin-upgrade-manager/pkg/module/util"
	"encoding/base64"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"
)

type OSTree struct {
	repoDir string
}

func NewRepo(repoDir string) (*OSTree, error) {
	return &OSTree{
		repoDir: repoDir,
	}, nil
}

func (repo *OSTree) Init() error {
	_, err := doAction([]string{"init", "--repo=" + repo.repoDir})
	return err
}

func (repo *OSTree) Exist(branchName string) bool {
	refs, err := repo.listRefs()
	if err != nil {
		return false
	}
	for _, ref := range refs {
		if ref == branchName {
			return true
		}
	}
	return false
}

func (repo *OSTree) Last() (string, error) {
	list, err := repo.listRefs()
	if err != nil {
		return "", err
	}
	if len(list) == 0 {
		return "", nil
	}
	return list[0], nil
}

func (repo *OSTree) First() (string, error) {
	list, err := repo.listRefs()
	if err != nil {
		return "", err
	}
	if len(list) == 0 {
		return "", nil
	}
	return list[len(list)-1], nil
}

func (repo *OSTree) List() (branch.BranchList, error) {
	refs, err := repo.listRefs()
	if err != nil {
		return nil, err
	}

	return refs, nil
}

func (repo *OSTree) ListByName(branchName string,
	offset, limit int) (branch.BranchList, int, error) {
	vers, err := repo.listByName(branchName)
	if err != nil {
		return nil, 0, err
	}

	length := len(vers)
	if length == 0 {
		return nil, 0, nil
	}

	if offset > length {
		return nil, 0, fmt.Errorf("invalid offset: %d", offset)
	}

	var list []string
	if length > offset+limit {
		length = offset + limit
	}
	for i := offset; i < length; i++ {
		list = append(list, vers[i])
	}
	return list, len(vers), nil
}

func (repo *OSTree) Snapshot(branchName, dstDir string) error {
	if !repo.Exist(branchName) {
		return fmt.Errorf("not found the branchName: %s", branchName)
	}
	_ = os.MkdirAll(filepath.Dir(dstDir), 0600)
	_, err := doAction([]string{"checkout", "--repo=" + repo.repoDir,
		branchName, dstDir})
	return err
}

func (repo *OSTree) Commit(branchName, subject, dataDir string) error {
	if !branch.IsValid(branchName) {
		return fmt.Errorf("invalid branch name: %s", branchName)
	}
	encodedText := base64.StdEncoding.EncodeToString([]byte(subject))
	_, err := doAction([]string{"commit", "--repo=" + repo.repoDir,
		"--branch=" + branchName, "--subject=" + encodedText, dataDir})
	return err
}

func (repo *OSTree) Diff(baseBranch, targetBranch, dstFile string) error {
	if len(baseBranch) == 0 || len(targetBranch) == 0 {
		return fmt.Errorf("invalid baseBranch(%q) or targetBranch(%q)",
			baseBranch, targetBranch)
	}

	data, err := doAction([]string{"diff", "--repo=" + repo.repoDir, baseBranch, targetBranch})
	if err != nil {
		return err
	}
	return ioutil.WriteFile(dstFile, data, 0600)
}

func (repo *OSTree) Cat(branchName, filepath, dstFile string) error {
	data, err := doAction([]string{"cat", "--repo=" + repo.repoDir, branchName,
		filepath})
	if err != nil {
		return err
	}
	return ioutil.WriteFile(dstFile, data, 0600)
}

func (repo *OSTree) Previous(targetBranch string) (string, error) {
	list, err := repo.listRefs()
	if err != nil {
		return "", err
	}

	length := len(list)
	for i, v := range list {
		if v != targetBranch {
			continue
		}
		if i+1 == length {
			return targetBranch, nil
		}
		return list[i+1], nil
	}
	return "", fmt.Errorf("not found the version: %q", targetBranch)
}

func (repo *OSTree) listByName(branchName string) ([]string, error) {
	refs, err := repo.listRefs()
	if err != nil {
		return nil, err
	}

	if len(branchName) == 0 {
		return refs, nil
	}

	for i, ref := range refs {
		if ref != branchName {
			continue
		}
		return refs[:i], nil
	}
	return refs, nil
}

func (repo *OSTree) listRefs() (branch.BranchList, error) {
	out, err := doAction([]string{"refs", "--repo=" + repo.repoDir})
	if err != nil {
		return nil, fmt.Errorf("%s: %s", err, string(out))
	}
	var refs branch.BranchList
	lines := strings.Split(string(out), "\n")
	for _, line := range lines {
		if len(line) == 0 {
			continue
		}
		refs = append(refs, line)
	}

	if len(refs) == 0 {
		return refs, nil
	}

	sort.Sort(refs)
	return refs, nil
}

func (repo *OSTree) Delete(branchName string) error {
	if !repo.Exist(branchName) {
		return fmt.Errorf("branch does not exist")
	}
	refs, err := repo.listRefs()
	if err != nil {
		return err
	}
	if refs[len(refs)-1] == branchName {
		return fmt.Errorf("the first version cannot be deleted")
	}
	out, err := doAction([]string{"log", "--repo=" + repo.repoDir, branchName})
	if err != nil {
		return err
	}
	lines := strings.Split(string(out), "\n")
	rows := strings.Fields(lines[0])
	if len(rows) < 2 {
		return fmt.Errorf("commit does not exist")
	}
	commit := strings.TrimSpace(rows[1])
	_, err = doAction([]string{"refs", "--repo=" + repo.repoDir, "--delete", branchName})
	if err != nil {
		return err
	}
	_, err = doAction([]string{"prune", "--repo=" + repo.repoDir, "--delete-commit=" + commit})
	if err != nil {
		return err
	}
	return nil
}

func (repo *OSTree) Subject(branchName string) (string, error) {
	out, err := doAction([]string{"log", "--repo=" + repo.repoDir, branchName})
	if err != nil {
		return "", err
	}
	lines := strings.Split(string(out), "\n\n")
	if len(lines) < 2 {
		return "", fmt.Errorf("commit does not exist")
	}
	encodedText, _ := base64.StdEncoding.DecodeString((strings.TrimSpace(lines[1])))
	return string(encodedText), nil
}

func (repo *OSTree) CommitTime(branchName string) (string, error) {
	out, err := doAction([]string{"log", "--repo=" + repo.repoDir, branchName})
	if err != nil {
		return "", err
	}
	lines := strings.Split(string(out), "\n")
	if len(lines) < 3 {
		return "", fmt.Errorf("commit does not exist")
	}

	for _, v := range lines {
		if strings.HasPrefix(v, "Date:") {
			rows := strings.Fields(v)
			if len(rows) < 3 {
				return "", nil
			}
			t, err := time.ParseInLocation("2006-01-02 15:04:05", rows[1]+" "+rows[2], time.Local)
			if err != nil {
				return "", nil
			}
			duration, _ := time.ParseDuration(util.GetTimeZoomDiff())
			format := t.Add(duration).Format("2006-01-02 15:04:05")
			return format, nil

		}
	}
	return "", nil
}

func doAction(args []string) ([]byte, error) {
	out, err := util.ExecCommandWithOut("ostree", args)
	if err != nil {
		return nil, fmt.Errorf("%s: %s", err, string(out))
	}
	return out, nil
}
