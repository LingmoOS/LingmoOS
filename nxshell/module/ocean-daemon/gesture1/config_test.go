// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package gesture1

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

var (
	configPath = "testdata/gesture"
)

// 查找手势信息
func findGestureInfo(evInfo EventInfo, infos gestureInfos) bool {
	for _, info := range infos {
		if info.Event == evInfo {
			return true
		}
	}
	return false
}

// 测试: 从文件读取手势信息
func Test_newGestureInfosFromFile(t *testing.T) {
	infos, err := newGestureInfosFromFile(configPath)
	assert.NoError(t, err)

	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "up", Fingers: 3}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "down", Fingers: 3}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "left", Fingers: 3}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "right", Fingers: 3}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "up", Fingers: 4}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "down", Fingers: 4}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "left", Fingers: 4}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "right", Fingers: 4}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "up", Fingers: 5}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "down", Fingers: 5}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "left", Fingers: 5}, infos))
	assert.True(t, findGestureInfo(EventInfo{Name: "swipe", Direction: "right", Fingers: 5}, infos))
}

// 测试：Get接口
func Test_Get(t *testing.T) {
	infos, err := newGestureInfosFromFile(configPath)
	assert.NoError(t, err)

	// for touch long press
	infos = append(infos, &gestureInfo{
		Event: EventInfo{
			Name:      "touch right button",
			Direction: "down",
			Fingers:   0,
		},
		Action: ActionInfo{
			Type:   ActionTypeCommandline,
			Action: "xdotool mousedown 3",
		},
	})
	infos = append(infos, &gestureInfo{
		Event: EventInfo{
			Name:      "touch right button",
			Direction: "up",
			Fingers:   0,
		},
		Action: ActionInfo{
			Type:   ActionTypeCommandline,
			Action: "xdotool mouseup 3",
		},
	})

	assert.NoError(t, err)
	assert.NotNil(t, infos.Get(EventInfo{Name: "touch right button", Direction: "down", Fingers: 0}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "touch right button", Direction: "up", Fingers: 0}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "up", Fingers: 3}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "down", Fingers: 3}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "left", Fingers: 3}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "right", Fingers: 3}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "up", Fingers: 4}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "down", Fingers: 4}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "left", Fingers: 4}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "right", Fingers: 4}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "up", Fingers: 5}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "down", Fingers: 5}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "left", Fingers: 5}))
	assert.NotNil(t, infos.Get(EventInfo{Name: "swipe", Direction: "right", Fingers: 5}))
}

// 测试：Set接口
func Test_Set(t *testing.T) {
	infos, err := newGestureInfosFromFile(configPath)
	assert.NoError(t, err)

	action1 := ActionInfo{
		Type:   "shortcut",
		Action: "ctrl+minus",
	}
	action2 := ActionInfo{
		Type:   "shortcut",
		Action: "ctrl+find",
	}
	assert.NotNil(t, infos.Set(EventInfo{Name: "pinch", Direction: "in", Fingers: 2}, action1))
	assert.NotNil(t, infos.Set(EventInfo{Name: "pinch", Direction: "out", Fingers: 2}, action1))
	assert.Nil(t, infos.Set(EventInfo{Name: "swipe", Direction: "up", Fingers: 3}, action2))
	assert.Nil(t, infos.Set(EventInfo{Name: "swipe", Direction: "down", Fingers: 3}, action2))
}
