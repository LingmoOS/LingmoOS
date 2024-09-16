// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsutility.h"

#include <dobject_p.h>

#ifdef BUILD_WITH_X11
#include "private/utility_x11_p.h"
#endif

DS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class UtilityPrivate : public DObjectPrivate
{
public:
    explicit UtilityPrivate(Utility *qq)
        : DObjectPrivate(qq)
    {

    }
    ~UtilityPrivate() override
    {

    }

    D_DECLARE_PUBLIC(Utility);
};

Utility *Utility::instance()
{
    static Utility* g_instance = nullptr;
    if (g_instance == nullptr) {
        auto platformName = QGuiApplication::platformName();
#ifdef BUILD_WITH_X11
        if (QStringLiteral("xcb") == platformName) {
            g_instance = new X11Utility();
        }
#endif
        if (g_instance == nullptr)
            g_instance = new Utility();
    }
    return g_instance;
}

QList<QWindow *> Utility::allChildrenWindows(QWindow *target)
{
    QList<QWindow *> ret;
    auto allWindows = qGuiApp->allWindows();
    while (!allWindows.isEmpty()) {
        auto window = allWindows.takeFirst();
        while (window) {
            if (window->transientParent() == target) {
                ret << window;
                break;
            }
            window = window->transientParent();
        }
    }
    return ret;
}

Utility::Utility(QObject *parent)
    : QObject(parent)
    , DObject(*new UtilityPrivate(this), this)
{

}

bool Utility::grabKeyboard(QWindow *target, bool grab)
{
    Q_UNUSED(target)
    Q_UNUSED(grab)
    return false;
}

bool Utility::grabMouse(QWindow *target, bool grab)
{
    Q_UNUSED(target)
    Q_UNUSED(grab)
    return false;
}

DS_END_NAMESPACE
