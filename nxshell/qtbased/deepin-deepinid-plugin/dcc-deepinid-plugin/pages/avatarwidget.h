// Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QLabel>

class AvatarWidget : public QLabel
{
    Q_OBJECT
public:
    explicit AvatarWidget(QWidget *parent = nullptr);
    explicit AvatarWidget(const QString &avatar, QWidget *parent = nullptr);

    const QString avatarPath() const;
    void setAvatarPath(const QString &avatar);

    void clearAvatar();

protected:
    void paintEvent(QPaintEvent *e);

private:
    QPixmap m_avatar;
    QString m_avatarPath;
};
