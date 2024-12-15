// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

typedef _mode_info struct {
	int32 width;
	int32 height;
	int32 refresh_rate;
	int32 flags; // available values: [current, preferred]
}ModeInfo;

typedef _output_info struct {
	char *model;
	char *manufacturer;
	char *uuid;

	int32 enabled;
	int32 x;
	int32 y;
	int32 width;
	int32 height;
	int32 refresh_rate;
	int32 transform;
	int32 phys_width;
	int32 phys_height;

	double scale;

	ModeInfo **modes;
} OutputInfo;
