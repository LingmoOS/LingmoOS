// Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VideoBOXBUTTON_H
#define VideoBOXBUTTON_H
#pragma once

#include <QScopedPointer>
#include <QMap>
#include <QVariant>

#include <DPushButton>
#include <DButtonBox>

DWIDGET_USE_NAMESPACE

class VideoBoxButton : public DButtonBoxButton
{
    struct VideoPicPathInfo {
        QString normalPicPath;
        QString hoverPicPath;
        QString pressPicPath;
        QString checkedPicPath;
    };

    Q_OBJECT
public:
    explicit VideoBoxButton(const QString &text, QWidget *parent = Q_NULLPTR);

    VideoBoxButton(const QString &text, const QString &normalPic, const QString &hoverPic,
                   const QString &pressPic, const QString &checkedPic = QString(), QWidget *parent = nullptr);

//    cppcheck修改
//    void setPropertyPic(QString propertyName, const QVariant &value, const QString &normalPic, const QString &hoverPic,
//                        const QString &pressPic, const QString &checkedPic = QString());
//    void setPropertyPic(const QString &normalPic, const QString &hoverPic,
//                        const QString &pressPic, const QString &checkedPic = QString());

    void setTransparent(bool flag);
    void setAutoChecked(bool flag);

protected:
    //void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    char                                               status                  = 0;
    bool                                               autoChecked             = false;
    VideoPicPathInfo                                   defaultPicPath;
    bool                                               transparent             = true;
    QPair<QString, QMap<QVariant, VideoPicPathInfo> >  propertyPicPaths;
};


#endif // VideoBOXBUTTON_H








