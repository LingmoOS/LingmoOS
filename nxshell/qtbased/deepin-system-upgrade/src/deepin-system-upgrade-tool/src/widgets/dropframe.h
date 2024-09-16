// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include "backgroundframe.h"

class DropFrame: public BackgroundFrame
{
    Q_OBJECT
public:
    explicit DropFrame(QWidget *parent = nullptr);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

Q_SIGNALS:
    void fileAboutAccept();
    void fileCancel();
    void fileDrop(const QString file);
};
