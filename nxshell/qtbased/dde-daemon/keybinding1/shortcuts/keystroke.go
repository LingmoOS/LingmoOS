// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package shortcuts

import (
	"bytes"
	"errors"
	"strconv"
	"strings"

	x "github.com/linuxdeepin/go-x11-client"
	"github.com/linuxdeepin/go-x11-client/util/keysyms"
)

// Keystroke
// field Mods ignore mod2(Num_Lock) and lock(Caps_Lock)
type Keystroke struct {
	Mods     Modifiers
	Keystr   string
	Keysym   x.Keysym
	Shortcut Shortcut

	isKeystrAboveTab bool
}

func (ks *Keystroke) DebugString() string {
	str := ks.String()
	if ks.Shortcut == nil {
		return "Keystroke{" + str + "}"
	} else {
		return "Keystroke{" + str + " own by " + ks.Shortcut.GetUid() + "}"
	}
}

func (a *Keystroke) Equal(keySymbols *keysyms.KeySymbols, b *Keystroke) bool {
	logger.Debug(a, " equal? ", b)
	if a.Mods != b.Mods {
		logger.Debug("Mods no equal, return false")
		return false
	}
	// ap1.Mods == ap2.Mods
	if a.Keystr == b.Keystr {
		logger.Debug("Key equal, return true")
		return true
	}

	// ap1.Key != ap2.Key
	codes1, err := keySymbols.StringToKeycodes(a.Keystr)
	if err != nil {
		return false
	}
	codes2, err := keySymbols.StringToKeycodes(b.Keystr)
	if err != nil {
		return false
	}

	keycodesEq := isKeycodesEqual(codes1, codes2)
	logger.Debug("keycodesEq:", keycodesEq)
	return keycodesEq
}

func isKeycodesEqual(list1, list2 []x.Keycode) bool {
	logger.Debug("isKeycodesEqual:", list1, list2)
	l1 := len(list1)
	l2 := len(list2)
	if l1 != l2 {
		return false
	}

	for i, v := range list1 {
		if v != list2[i] {
			return false
		}
	}

	return true
}

func (ks *Keystroke) MarshalJSON() ([]byte, error) {
	str := ks.String()
	quoted := strconv.Quote(str)
	return []byte(quoted), nil
}

func GetKeyFirstCode(keySymbols *keysyms.KeySymbols, str string) (x.Keycode, error) {
	codes, err := keySymbols.StringToKeycodes(str)
	if err != nil {
		return 0, err
	}
	var code x.Keycode
	for _, kc := range codes {
		if kc != 0 {
			code = kc
			break
		}
	}
	if code == 0 {
		return 0, errors.New("not found keycode")
	}
	//logger.Debugf("GetKeyFirstCode str %q codes: %v code: %d", str, codes, code)
	return code, nil
}

func (ks *Keystroke) ToKey(keySymbols *keysyms.KeySymbols) (Key, error) {
	code, err := GetKeyFirstCode(keySymbols, ks.Keystr)
	if err != nil {
		return Key{}, err
	}
	return Key{
		Mods: ks.Mods,
		Code: Keycode(code),
	}, nil
}

func (ks *Keystroke) ToKeyList(keySymbols *keysyms.KeySymbols) ([]Key, error) {
	codes, err := keySymbols.StringToKeycodes(ks.Keystr)
	if err != nil {
		return nil, err
	}
	var keyList []Key
	for _, code := range codes {
		if code == 0 {
			continue
		}
		if strings.HasPrefix(ks.Keystr, "XF86") && keySymbols.GetKeysym(code, 0) != ks.Keysym {
			// XF86 只取基本层级
			continue
		}
		keyList = append(keyList, Key{
			Mods: ks.Mods,
			Code: Keycode(code),
		})
	}
	return keyList, nil
}

func splitKeystroke(str string) ([]string, error) {
	if str == "" {
		return nil, nil
	}

	var keys []string
	reader := strings.NewReader(str)
	for {
		ch, err := reader.ReadByte()
		if err != nil {
			// eof
			break
		}

		switch ch {
		case '<':
			// read byte is not '>' , fill buf key
			// read byte is '>' push key.String() to keys
			var key bytes.Buffer
		Loop:
			for {
				ch, err := reader.ReadByte()
				if err != nil {
					// eof
					return nil, errors.New("> not found")
				}
				switch ch {
				case '>':
					break Loop
				case '<':
					return nil, errors.New("unexpect < found")
				default:
					key.WriteByte(ch)
				}
			}
			if key.Len() > 0 {
				keys = append(keys, key.String())
			} else {
				return nil, errors.New("empty modifier found")
			}
		default:
			err := reader.UnreadByte()
			if err != nil {
				logger.Warning("UnreadByte Failed:", err)
			}
			var key bytes.Buffer
			// read rest bytes
			for {
				ch, err := reader.ReadByte()
				if err != nil {
					break
				}
				switch ch {
				case '<', '>':
					return nil, errors.New("unexpect < or > found")
				default:
					key.WriteByte(ch)
				}
			}
			keys = append(keys, key.String())
		}
	}
	return keys, nil
}

// <Super>L mods (mod4) key L
// <Super>% mods (mod4, shift) key %
// <Control><Alt>T mods (control,mod1) key T
// <Control><shift>T mods(control,shift) key T
// <Control>> mods(control) key >
// <Control>< invalid
// Super< invalid
// <Super> mods() key Super
// Print mods() key Print
// <Control>Print mods(Control) key Print
// check Keystroke.Keystr valid later
func ParseKeystroke(keystroke string) (*Keystroke, error) {
	parts, err := splitKeystroke(keystroke)
	if err != nil {
		return nil, err
	}
	if len(parts) == 0 {
		return nil, errors.New("keystroke is empty")
	}

	str := parts[len(parts)-1]
	if str == "Space" {
		str = "space"
	}
	// check key valid
	var sym x.Keysym
	var isKeystrAboveTab bool
	if str == "Above_Tab" {
		isKeystrAboveTab = true
	} else {
		var ok bool
		sym, ok = keysyms.StringToKeysym(str)
		if !ok {
			return nil, errors.New("bad key " + str)
		}
	}

	var mods Modifiers
	for _, part := range parts[:len(parts)-1] {
		switch strings.ToLower(part) {
		case "shift":
			mods |= keysyms.ModMaskShift
		case "control":
			mods |= keysyms.ModMaskControl
		case "alt":
			mods |= keysyms.ModMaskAlt
		case "super":
			mods |= keysyms.ModMaskSuper
		default:
			return nil, errors.New("unknown mod " + part)
		}
	}

	return &Keystroke{
		Mods:             mods,
		Keystr:           str,
		Keysym:           sym,
		isKeystrAboveTab: isKeystrAboveTab,
	}, nil
}

func ParseKeystrokes(keystrokes []string) []*Keystroke {
	result := make([]*Keystroke, 0, len(keystrokes))
	for _, keystroke := range keystrokes {
		parsed, err := ParseKeystroke(keystroke)
		if err == nil {
			result = append(result, parsed)
		}
		// TODO else warning
	}
	return result
}

func (ks *Keystroke) String() string {
	var keys []string
	mods := ks.Mods
	if mods&keysyms.ModMaskShift > 0 {
		keys = append(keys, "<Shift>")
	}
	if mods&keysyms.ModMaskControl > 0 {
		keys = append(keys, "<Control>")
	}
	if mods&keysyms.ModMaskAlt > 0 {
		keys = append(keys, "<Alt>")
	}
	if mods&keysyms.ModMaskSuper > 0 {
		keys = append(keys, "<Super>")
	}

	keys = append(keys, ks.Keystr)
	return strings.Join(keys, "")
}

func (ks *Keystroke) searchString() string {
	var strs []string
	mods := ks.Mods
	if mods&keysyms.ModMaskShift > 0 {
		strs = append(strs, "shift")
	}
	if mods&keysyms.ModMaskControl > 0 {
		strs = append(strs, "ctrl")
	}
	if mods&keysyms.ModMaskAlt > 0 {
		strs = append(strs, "alt")
	}
	if mods&keysyms.ModMaskSuper > 0 {
		strs = append(strs, "super")
	}

	visibleChar, ok := keysyms.KeysymVisibleCharMap[ks.Keysym]
	if ok {
		if visibleChar == keysyms.XK_space {
			strs = append(strs, "space")
		} else {
			strs = append(strs, strings.ToLower(string(visibleChar)))
		}
	} else {
		keyStr := fixSpecialKeyStr(ks.Keystr)
		strs = append(strs, strings.ToLower(keyStr))
	}

	return strings.Join(strs, "")
}

func fixSpecialKeyStr(keyStr string) string {
	// 处理特殊键值，搜索时，去除按键值的前后缀，例如Super_L,搜索super即可
	res := strings.TrimSuffix(keyStr, "_L")
	res = strings.TrimSuffix(res, "_R")
	res = strings.TrimPrefix(res, "KP_")
	return res
}

func isGoodNoMods(str string, sym x.Keysym) bool {
	// single key
	if keysyms.IsFunctionKey(sym) || keysyms.IsMiscFunctionKey(sym) {
		return true
	}

	switch sym {
	case keysyms.XK_BackSpace,
		keysyms.XK_Delete,
		keysyms.XK_Super_L,
		keysyms.XK_Super_R,
		keysyms.XK_Pause:
		return true
	}

	return strings.HasPrefix(str, "XF86")
}

func isGoodModShift(str string, sym x.Keysym) bool {
	// shift + ?
	if keysyms.IsFunctionKey(sym) || keysyms.IsMiscFunctionKey(sym) || keysyms.IsCursorKey(sym) {
		return true
	}

	switch sym {
	case keysyms.XK_BackSpace,
		keysyms.XK_space,
		keysyms.XK_Delete,
		keysyms.XK_Sys_Req,
		keysyms.XK_Escape,
		keysyms.XK_Tab:
		return true
	}

	return strings.HasPrefix(str, "XF86")
}

func (ks *Keystroke) IsGood() bool {
	if ks.Mods == 0 {
		return isGoodNoMods(ks.Keystr, ks.Keysym)
	}
	// else ks.Mod > 0
	if keysyms.IsModifierKey(ks.Keysym) {
		return false
	}
	if ks.Mods == keysyms.ModMaskShift {
		return isGoodModShift(ks.Keystr, ks.Keysym)
	}

	return true
}

func (ks *Keystroke) fix() *Keystroke {
	logger.Debug("before fix", ks)
	var key string
	switch ks.Keystr {
	case "KP_Prior":
		key = "KP_Page_Up"
	case "KP_Next":
		key = "KP_Page_Down"
	case "Prior":
		key = "Page_Up"
	case "Next":
		key = "Page_Down"
	case "ISO_Left_Tab":
		key = "Tab"
	default:
		key = ks.Keystr
	}

	sym, _ := keysyms.StringToKeysym(key)
	_, upperSym := keysyms.ConvertCase(sym)
	if sym != upperSym {
		key, _ = keysyms.KeysymToString(upperSym)
	}

	if ks.Mods > 0 && ks.Mods&^keysyms.ModMaskSuper == 0 {
		// ks is <Super>Super_L or <Super>Super_R
		if ks.Keysym == keysyms.XK_Super_L || ks.Keysym == keysyms.XK_Super_R {
			// clear modifiers
			ks.Mods = 0
		}
	}
	ks.Keystr = key
	ks.Keysym = upperSym
	logger.Debug("after fix", ks)
	return ks
}
