// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CICONBUTTON_H
#define CICONBUTTON_H

#include <globaldefine.h>
#include <QWidget>
#include <QMap>

#ifdef USE_DTK

#define private protected
#include <DIconButton>
#undef private

#define ICONBUTTON DIconButton
DWIDGET_USE_NAMESPACE
#else
#include <QPushButton>
#define ICONBUTTON QPushButton
#endif

class CIconButton : public ICONBUTTON
{
    Q_OBJECT
public:
    enum EIconButtonSattus {
        Normal,
        Hover,
        Press,
        Active
    };
public:
    explicit CIconButton(const QMap<int, QMap<EIconButtonSattus, QString>> &pictureMap,
                         const QSize &size, QWidget *parent = nullptr,
                         bool isCheckLock = true);
    void setChecked(bool);
    bool isChecked() const;

    void setTheme(int currentTheme);
    void setIconMode();
    void setCenterColor(QColor c);

signals:
    void buttonClick();
    void mouseRelease();
public slots:

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateImage();

private:
    bool m_isHover;
    bool m_isChecked;
    bool m_isPressed;
    bool m_isCheckLock; //是否只允许点击选中 不允许点击取消选中
    EIconButtonSattus m_currentStatus;
    EIconButtonSattus m_tmpStatus;
    int m_currentTheme;
    int m_iconMode = false;//是否属于icon模式的图标
    QIcon m_normalIcon;//normal状态下的icon图标切图

    QMap<int, QMap<EIconButtonSattus, QString> > m_pictureMap;
    QColor          m_centerColor;

};

#endif // CICONBUTTON_H
