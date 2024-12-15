// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBILITYCHECKEREX_H
#define ACCESSIBILITYCHECKEREX_H

#include <QObject>
#include <QWidget>

#include <DAccessibilityChecker>

DWIDGET_USE_NAMESPACE

class AccessibilityCheckerEx : public DAccessibilityChecker
{
    Q_OBJECT
public:
    void addIgnoreName(const QString &name);
    void addIgnoreClasses(const QStringList &classes);

protected:
    virtual bool isIgnore(Role role, const QWidget *w) override ;

private:
    QStringList m_nameList;
    QStringList m_classes;
};

#endif // ACCESSIBILITYCHECKEREX_H
