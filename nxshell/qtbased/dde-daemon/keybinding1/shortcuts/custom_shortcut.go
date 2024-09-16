// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"os"
	"path/filepath"
	"strings"

	"github.com/linuxdeepin/dde-daemon/keybinding1/util"
	wm "github.com/linuxdeepin/go-dbus-factory/session/com.deepin.wm"
	"github.com/linuxdeepin/go-lib/keyfile"
)

const (
	kfKeyName       = "Name"
	kfKeyKeystrokes = "Accels"
	kfKeyAction     = "Action"
)

type CustomShortcut struct {
	BaseShortcut
	manager *CustomShortcutManager
	Cmd     string `json:"Exec"`
	wm      wm.Wm
}

func (cs *CustomShortcut) Marshal() (string, error) {
	cs.mu.Lock()
	defer cs.mu.Unlock()
	return util.MarshalJSON(cs)
}

func (cs *CustomShortcut) SaveKeystrokes() error {
	section := cs.GetId()
	if _useWayland {
		ok, err := setShortForWayland(cs, cs.wm)
		if !ok {
			return err
		}
	}
	csm := cs.manager
	csm.kfile.SetStringList(section, kfKeyKeystrokes, cs.getKeystrokesStrv())
	return csm.Save()
}

// 经过 Reset 重置后， 自定义快捷键的 keystrokes 被设置为空，始终返回 false
// 是为了另外计算改变的自定义快捷键项目。
func (cs *CustomShortcut) ReloadKeystrokes() bool {
	cs.setKeystrokes(nil)
	return false
}

func (cs *CustomShortcut) ShouldEmitSignalChanged() bool {
	return true
}

func (cs *CustomShortcut) Save() error {
	section := cs.GetId()
	kfile := cs.manager.kfile
	kfile.SetString(section, kfKeyName, cs.Name)
	kfile.SetString(section, kfKeyAction, cs.Cmd)
	kfile.SetStringList(section, kfKeyKeystrokes, cs.getKeystrokesStrv())
	return cs.manager.Save()
}

func (cs *CustomShortcut) GetAction() *Action {
	_, err := os.Stat(cs.Cmd)
	if !os.IsNotExist(err) {
		if strings.HasSuffix(cs.Cmd, ".desktop") {
			return &Action{
				Type: ActionTypeDesktopFile,
				Arg:  cs.Cmd,
			}
		}
	}

	a := &Action{
		Type: ActionTypeExecCmd,
		Arg: &ActionExecCmdArg{
			Cmd: cs.Cmd,
		},
	}
	return a
}

func (cs *CustomShortcut) SetName(name string) {
	cs.Name = name
	if cs.manager.pinyinEnabled {
		cs.nameBlocksInit = false
		cs.initNameBlocks()
	}
}

type CustomShortcutManager struct {
	file          string
	kfile         *keyfile.KeyFile
	pinyinEnabled bool
}

func newCustomShort(id, name, cmd string, keystrokes []string, wm wm.Wm, csm *CustomShortcutManager) *CustomShortcut {
	return &CustomShortcut{
		BaseShortcut: BaseShortcut{
			Id:         id,
			Type:       ShortcutTypeCustom,
			Name:       name,
			Keystrokes: ParseKeystrokes(keystrokes),
		},
		manager: csm,
		wm:      wm,
		Cmd:     cmd,
	}
}

func NewCustomShortcutManager(file string) *CustomShortcutManager {
	kfile := keyfile.NewKeyFile()
	err := kfile.LoadFromFile(file)
	if err != nil && !os.IsNotExist(err) {
		logger.Warning(err)
	}

	m := &CustomShortcutManager{
		file:  file,
		kfile: kfile,
	}
	return m
}

func (csm *CustomShortcutManager) List() []Shortcut {
	kfile := csm.kfile
	sections := kfile.GetSections()
	ret := make([]Shortcut, 0, len(sections))
	for _, section := range sections {
		id := section
		name, _ := kfile.GetString(section, kfKeyName)
		cmd, _ := kfile.GetString(section, kfKeyAction)
		keystrokes, _ := kfile.GetStringList(section, kfKeyKeystrokes)

		shortcut := &CustomShortcut{
			BaseShortcut: BaseShortcut{
				Id:         id,
				Type:       ShortcutTypeCustom,
				Keystrokes: ParseKeystrokes(keystrokes),
				Name:       name,
			},
			manager: csm,
			Cmd:     cmd,
		}

		ret = append(ret, shortcut)
	}
	return ret
}

func (csm *CustomShortcutManager) Save() error {
	err := os.MkdirAll(filepath.Dir(csm.file), 0755)
	if err != nil {
		return err
	}
	return csm.kfile.SaveToFile(csm.file)
}

func (csm *CustomShortcutManager) Add(name, action string, keystrokes []*Keystroke, wm wm.Wm) (Shortcut, error) {
	id := name
	csm.kfile.SetString(id, kfKeyName, name)
	csm.kfile.SetString(id, kfKeyAction, action)

	keystrokesStrv := make([]string, 0, len(keystrokes))
	for _, ks := range keystrokes {
		keystrokesStrv = append(keystrokesStrv, ks.String())
	}
	csm.kfile.SetStringList(id, kfKeyKeystrokes, keystrokesStrv)

	shortcut := &CustomShortcut{
		BaseShortcut: BaseShortcut{
			Id:         id,
			Type:       ShortcutTypeCustom,
			Keystrokes: keystrokes,
			Name:       name,
		},
		manager: csm,
		Cmd:     action,
		wm:      wm,
	}
	return shortcut, csm.Save()
}

func (csm *CustomShortcutManager) Delete(id string) error {
	if _, err := csm.kfile.GetSection(id); err != nil {
		return err
	}

	csm.kfile.DeleteSection(id)
	return csm.Save()
}
