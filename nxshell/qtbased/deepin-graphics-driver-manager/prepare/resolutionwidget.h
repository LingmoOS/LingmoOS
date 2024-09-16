// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOLUTIONWIDGET_H_
#define RESOLUTIONWIDGET_H_

#include "resolution.h"
#include "graphicsdriverproxy.h"


#include <QFrame>
#include <QDebug>
#include <QProcess>
#include <QDir>
#include <QLineEdit>
#include <QTimer>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
class QLabel;
class ResolutionWidget: public QFrame
{
    Q_OBJECT

public:
    explicit ResolutionWidget(ComDeepinDaemonGraphicsDriverInterface *graphicsDriver, const Resolution &resolution,  QWidget *parent = nullptr);
    ~ResolutionWidget() override;
    virtual void initUI();
    virtual void setChecked(bool checked);
    virtual void prepareInstall();
    virtual Resolution resolution() const { return m_resolution; }
    virtual bool canUpdate();

signals:
    void clicked() const;
    void preInstallProgress(int);

private Q_SLOTS:
    virtual void onThemeChanged(Dtk::Gui::DGuiApplicationHelper::ColorType type);

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;

protected:
    ComDeepinDaemonGraphicsDriverInterface *m_graphicsDriver;
    Resolution m_resolution;
    bool m_checked = false;
    bool m_using = false;
    QLabel *m_checkedBtn = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_version = nullptr;
    QLabel *m_description = nullptr;
    QTimer *m_timer = nullptr;

};

#endif
