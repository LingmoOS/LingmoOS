// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"

	"github.com/adrg/xdg"
	"github.com/lingmoos/go-lib/xdg/basedir"
)

const (
	ActionTypeShortcut    = "shortcut"
	ActionTypeCommandline = "commandline"
	ActionTypeBuiltin     = "built-in"
)

var (
	configUserPath      = filepath.Join(basedir.GetUserConfigDir(), "lingmo/ocean-daemon/gesture.json")
	configSystemPath, _ = xdg.SearchDataFile("ocean-daemon/gesture.json")
)

const (
	gestureSchemaId         = "com.lingmo.ocean.gesture"
	gsKeyTouchPadEnabled    = "touch-pad-enabled"
	gsKeyTouchScreenEnabled = "touch-screen-enabled"

	configManagerId = "org.desktopspec.ConfigManager"
)

type ActionInfo struct {
	Type   string
	Action string
}

type EventInfo struct {
	Name      string
	Direction string
	Fingers   int32
}

type gestureInfo struct {
	Event  EventInfo
	Action ActionInfo
}
type gestureInfos []*gestureInfo

func (action ActionInfo) toString() string {
	return fmt.Sprintf("Type:%s, Action=%s", action.Type, action.Action)
}

func (evInfo EventInfo) toString() string {
	return fmt.Sprintf("Name=%s, Direction=%s, Fingers=%d", evInfo.Name, evInfo.Direction, evInfo.Fingers)
}

func (infos gestureInfos) Get(evInfo EventInfo) *gestureInfo {
	for _, info := range infos {
		if info.Event == evInfo {
			return info
		}
	}
	return nil
}

func (infos gestureInfos) Set(evInfo EventInfo, action ActionInfo) error {
	info := infos.Get(evInfo)
	if info == nil {
		return fmt.Errorf("not found gesture info for: %s, %s, %d", evInfo.Name, evInfo.Direction, evInfo.Fingers)
	}
	info.Action = action
	return nil
}

func newGestureInfosFromFile(filename string) (gestureInfos, error) {
	content, err := os.ReadFile(filepath.Clean(filename))
	if err != nil {
		return nil, err
	}

	if len(content) == 0 {
		return nil, fmt.Errorf("file '%s' is empty", filename)
	}

	var infos gestureInfos
	err = json.Unmarshal(content, &infos)
	if err != nil {
		return nil, err
	}
	return infos, nil
}
