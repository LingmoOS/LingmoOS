// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"errors"
	"fmt"
	"strconv"
	"sync"

	"github.com/linuxdeepin/dde-daemon/keybinding1/util"
	"github.com/linuxdeepin/go-lib/pinyin_search"
)

type BaseShortcut struct {
	mu             sync.Mutex
	Id             string
	Type           int32
	Keystrokes     []*Keystroke `json:"Accels"`
	Name           string
	nameBlocksInit bool
	nameBlocks     pinyin_search.Blocks
}

func (sb *BaseShortcut) String() string {
	sb.mu.Lock()
	str := fmt.Sprintf("Shortcut{id=%s type=%d name=%q keystrokes=%v}", sb.Id, sb.Type, sb.Name, sb.Keystrokes)
	sb.mu.Unlock()
	return str
}

func (sb *BaseShortcut) Marshal() (string, error) {
	sb.mu.Lock()
	defer sb.mu.Unlock()
	return util.MarshalJSON(sb)
}

func (sb *BaseShortcut) GetId() string {
	return sb.Id
}

func idType2Uid(id string, type0 int32) string {
	return strconv.Itoa(int(type0)) + id
}

func (sb *BaseShortcut) GetUid() string {
	return idType2Uid(sb.Id, sb.Type)
}

func (sb *BaseShortcut) GetKeystrokes() []*Keystroke {
	sb.mu.Lock()
	defer sb.mu.Unlock()
	return sb.Keystrokes
}

func (sb *BaseShortcut) getKeystrokesStrv() []string {
	keystrokes := sb.GetKeystrokes()
	strv := make([]string, len(keystrokes))
	for i, k := range keystrokes {
		strv[i] = k.String()
	}
	return strv
}

func (sb *BaseShortcut) setKeystrokes(val []*Keystroke) {
	sb.mu.Lock()
	sb.Keystrokes = val
	sb.mu.Unlock()
}

func (sb *BaseShortcut) GetType() int32 {
	return sb.Type
}

func (sb *BaseShortcut) GetName() string {
	return sb.Name
}

func (sb *BaseShortcut) GetAction() *Action {
	return ActionNoOp
}

func (sb *BaseShortcut) GetKeystrokesModifiable() bool {
	return sb.Type != ShortcutTypeFake
}

func (sb *BaseShortcut) ShouldEmitSignalChanged() bool {
	return false
}

func (sb *BaseShortcut) GetNameBlocks() pinyin_search.Blocks {
	if sb.nameBlocksInit {
		return sb.nameBlocks
	}
	sb.initNameBlocks()
	return sb.nameBlocks
}

func (sb *BaseShortcut) initNameBlocks() {
	name := sb.GetName()
	sb.nameBlocks = pinyin_search.Split(name)
	sb.nameBlocksInit = true
}

const (
	ShortcutTypeSystem int32 = iota
	ShortcutTypeCustom
	ShortcutTypeMedia
	ShortcutTypeWM
	ShortcutTypeFake
)

type Shortcut interface {
	GetId() string
	GetUid() string
	GetType() int32

	GetName() string
	GetNameBlocks() pinyin_search.Blocks

	GetKeystrokesModifiable() bool
	getKeystrokesStrv() []string
	GetKeystrokes() []*Keystroke
	setKeystrokes([]*Keystroke)
	SaveKeystrokes() error
	ReloadKeystrokes() bool

	GetAction() *Action
	Marshal() (string, error)
	ShouldEmitSignalChanged() bool
}

// errors:
var ErrOpNotSupported = errors.New("operation is not supported")
var ErrTypeAssertionFail = errors.New("type assertion failed")
var ErrNilAction = errors.New("action is nil")
var ErrInvalidActionType = errors.New("invalid action type")

type FakeShortcut struct {
	BaseShortcut
	action *Action
}

func NewFakeShortcut(action *Action) *FakeShortcut {
	return &FakeShortcut{
		BaseShortcut: BaseShortcut{
			Type: ShortcutTypeFake,
		},
		action: action,
	}
}

func (s *FakeShortcut) GetAction() *Action {
	return s.action
}

func (s *FakeShortcut) SaveKeystrokes() error {
	return ErrOpNotSupported
}

func (s *FakeShortcut) ReloadKeystrokes() bool {
	return false
}
