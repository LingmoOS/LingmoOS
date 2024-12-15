// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSIBILITYCHECKEREX_H
#define ACCESSIBILITYCHECKEREX_H

//需要把QObject放在QT_DEBUG前面，否则不生效
#include <QObject>
#if defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)

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
#endif //defined(DSS_CHECK_ACCESSIBILITY) && defined(QT_DEBUG)

#endif // ACCESSIBILITYCHECKEREX_H
