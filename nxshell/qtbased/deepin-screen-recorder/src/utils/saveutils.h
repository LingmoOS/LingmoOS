// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAVEUTILS_H
#define SAVEUTILS_H

#include <QObject>

enum SaveAction : unsigned int {
    SaveToClipboard,
    SaveToDesktop,
    SaveToImage,
    SaveToSpecificDir,
    AutoSave, // 自定义保存目录
};
Q_DECLARE_METATYPE(SaveAction);
#endif // SAVEUTILS_H
