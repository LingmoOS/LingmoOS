// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef ENABLE_ACCESSIBILITY
#ifndef DESKTOP_ACCESSIBLE_OBJECT_LIST_H
#define DESKTOP_ACCESSIBLE_OBJECT_LIST_H

#include "accessiblefunctions.h"

#include <QDialog>
#include <QPushButton>
#include <QToolButton>
#include <QListWidget>
#include <DTitlebar>
#include <QFrame>
#include <QLabel>
#include <DMainWindow>
#include <QAction>
#include <QComboBox>
#include <QFontComboBox>
#include <QObject>
#include "application.h"


#ifdef USE_DTK
DWIDGET_USE_NAMESPACE
#endif
// 添加accessible
SET_FORM_ACCESSIBLE(QFrame, m_w->objectName())
SET_COMBOX_ACCESSIBLE(QComboBox, m_w->objectName())
SET_COMBOX_ACCESSIBLE(QFontComboBox, m_w->objectName())
SET_FORM_ACCESSIBLE(QWidget, m_w->objectName())
SET_LABEL_ACCESSIBLE(QLabel, m_w->objectName())
//SET_FORM_ACCESSIBLE(QDialog, m_w->objectName())
SET_BUTTON_ACCESSIBLE(QPushButton, m_w->objectName())
SET_BUTTON_ACCESSIBLE(QToolButton, m_w->objectName())
SET_SLIDER_ACCESSIBLE(DMainWindow, m_w->objectName())
SET_SLIDER_ACCESSIBLE(QListWidget, m_w->objectName())
SET_FORM_ACCESSIBLE(DTitlebar, m_w->objectName())
//SET_FORM_ACCESSIBLE(QMenu, m_w->objectName())

#include <QDebug>
QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, QFrame);
    USE_ACCESSIBLE(classname, QWidget);
    USE_ACCESSIBLE(classname, QLabel);
    //USE_ACCESSIBLE(classname, QDialog);
    USE_ACCESSIBLE(classname, QPushButton);
    USE_ACCESSIBLE(classname, QToolButton);
    USE_ACCESSIBLE(classname, DMainWindow);
    USE_ACCESSIBLE(classname, QListWidget);
    USE_ACCESSIBLE(classname, DTitlebar);
    //USE_ACCESSIBLE(classname, QMenu);
    USE_ACCESSIBLE(classname, QFontComboBox);
    USE_ACCESSIBLE(classname, QComboBox);
    return interface;
}

#endif // DESKTOP_ACCESSIBLE_OBJECT_LIST_H
#endif
