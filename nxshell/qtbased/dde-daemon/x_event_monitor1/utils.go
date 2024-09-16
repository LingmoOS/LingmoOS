// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package x_event_monitor

const (
	MotionFlag = int32(1) //当此标志为1, 则鼠标在册注区域中移动时,会实时发送鼠标位置信号
	ButtonFlag = int32(1 << 1)
	KeyFlag    = int32(1 << 2)
)

func hasMotionFlag(flag int32) bool {
	return flag&MotionFlag != 0
}

func hasKeyFlag(flag int32) bool {
	return flag&KeyFlag != 0
}

func hasButtonFlag(flag int32) bool {
	return flag&ButtonFlag != 0
}

func isInArea(x, y int32, area coordinateRange) bool {
	if (x >= area.X1 && x <= area.X2) &&
		(y >= area.Y1 && y <= area.Y2) {
		return true
	}

	return false
}

func isInIdList(md5Str string, list []string) bool {
	for _, v := range list {
		if md5Str == v {
			return true
		}
	}

	return false
}
