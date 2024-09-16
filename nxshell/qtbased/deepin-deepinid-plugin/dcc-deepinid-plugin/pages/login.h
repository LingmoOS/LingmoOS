// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QLabel>
#include <QSvgRenderer>

class QVBoxLayout;

class PicLabel:public QLabel
{
    Q_OBJECT
public:
    explicit PicLabel(QWidget *parent = nullptr);

    void SetImage(const QString &imgurl);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    QSize        m_defSize;
    QSvgRenderer *m_svgrender;
};

class LoginPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginPage(QWidget *parent = nullptr);

private:
    /**
     * @brief getAgreementHttp  返回协议 0 为使用协议 1为 隐私协议
     * @return
     */
    QStringList getAgreementHttp();

public Q_SLOTS:
    void onLinkActivated(const QString link);

Q_SIGNALS:
    void requestLoginUser() const;

private:
    QVBoxLayout *m_mainLayout;
};
