// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessibledefine.h"

#include "AuthDialog.h"

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QToolButton>
#include <QFrame>
#include <QMenu>

#include <DVerticalLine>
#include <DWindowMinButton>
#include <DWindowCloseButton>
#include <DWindowMaxButton>
#include <DIconButton>
#include <DPasswordEdit>
#include <DLabel>
#include <DSuggestButton>
#include <DLineEdit>

DWIDGET_USE_NAMESPACE

SET_WIDGET_ACCESSIBLE(AuthDialog, QAccessible::Dialog, "authdialog")
SET_FORM_ACCESSIBLE(QWidget, "widget")
SET_LABEL_ACCESSIBLE(QLabel, "label")
SET_WIDGET_ACCESSIBLE(QComboBox, QAccessible::ComboBox, "combobox")
SET_BUTTON_ACCESSIBLE(QPushButton, "btn")
SET_EDITABLE_ACCESSIBLE(QLineEdit, "lineedit")
SET_WIDGET_ACCESSIBLE(QCheckBox, QAccessible::CheckBox, "checkbox")
SET_BUTTON_ACCESSIBLE(QToolButton, "toolbutton")
SET_FORM_ACCESSIBLE(QFrame, "frame")
SET_WIDGET_ACCESSIBLE(QMenu, QAccessible::PopupMenu, "menu")

SET_FORM_ACCESSIBLE(DVerticalLine, "VerticalLine")
SET_BUTTON_ACCESSIBLE(DWindowMinButton, "WindowMinButton")
SET_BUTTON_ACCESSIBLE(DWindowCloseButton, "WindowCloseButton")
SET_BUTTON_ACCESSIBLE(DWindowMaxButton, "WindowMaxButton")
SET_BUTTON_ACCESSIBLE(DIconButton, "IconButton")
SET_EDITABLE_ACCESSIBLE(DPasswordEdit, "PasswordEdit")
SET_LABEL_ACCESSIBLE(DLabel, "imagebutton")
SET_BUTTON_ACCESSIBLE(DSuggestButton, "showpassword")
SET_EDITABLE_ACCESSIBLE(DLineEdit, "LineEdit")

QAccessibleInterface *accessibleFactory(const QString &classname, QObject *object)
{
    QAccessibleInterface *interface = nullptr;
    USE_ACCESSIBLE(classname, AuthDialog)
    ELSE_USE_ACCESSIBLE(classname, QWidget)
    ELSE_USE_ACCESSIBLE(classname, QLabel)
    ELSE_USE_ACCESSIBLE(classname, QComboBox)
    ELSE_USE_ACCESSIBLE(classname, QPushButton)
    ELSE_USE_ACCESSIBLE(classname, QLineEdit)
    ELSE_USE_ACCESSIBLE(classname, QCheckBox)
    ELSE_USE_ACCESSIBLE(classname, QToolButton)
    ELSE_USE_ACCESSIBLE(classname, QFrame)
    ELSE_USE_ACCESSIBLE(classname, QMenu)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DVerticalLine)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DWindowMinButton)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DWindowCloseButton)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DWindowMaxButton)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DIconButton)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DPasswordEdit)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DLabel)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DSuggestButton)
    ELSE_USE_ACCESSIBLE(QString(classname).replace("Dtk::Widget::", ""), DLineEdit)

    return interface;
}
