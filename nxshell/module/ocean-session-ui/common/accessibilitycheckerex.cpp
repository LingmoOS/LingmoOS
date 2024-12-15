// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessibilitycheckerex.h"

/**
 * @brief addIgnoreName 添加需要忽略的控件的objectName或accessibleName，一般用于设置property无法解决的情况
 * @param name 只能是objectName或accessibleName
 */
void AccessibilityCheckerEx::addIgnoreName(const QString &name)
{
    Q_ASSERT(!name.isEmpty());

    if (!m_nameList.contains(name)) {
        m_nameList.append(name);
    }
}

/**
 * @brief addIgnoreClasses 添加需要忽略的控件的类名，一般用于addIgnoreName和设置property无法解决的情况
 * @param name 只能是objectName或accessibleName
 */
void AccessibilityCheckerEx::addIgnoreClasses(const QStringList &classes)
{
    for (const auto &className : classes) {
        if (!className.isEmpty() && !m_classes.contains(className)) {
            m_classes.append(className);
        }
    }
}


/**
 * @brief isIgnore 忽略objectName和accessibleName在m_nameList中的对象,
 * 推荐方式（按优先级排序）
 * 1.设置控件的属性：setPropert("CheckAccessibleName",false)。
 * 2.addIgnoreName
 * 3.addIgnoreClasses
 * @param role
 * @param w
 * @return true:不检查此widget，false:检查
 */
bool AccessibilityCheckerEx::isIgnore(Role role, const QWidget *w)
{
    if (!w || "VLine" == w->objectName())
        return true;

    QVariant property = w->property("CheckAccessibleName");
    if (!property.isNull() && !property.toBool())
        return true;

    if (m_nameList.contains(w->objectName()) || m_nameList.contains(w->accessibleName()))
        return true;

    if (m_classes.contains(w->metaObject()->className()))
        return true;

    return DAccessibilityChecker::isIgnore(role, w);
}
