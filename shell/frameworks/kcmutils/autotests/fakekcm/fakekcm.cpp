/*
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KPluginFactory>
#include <KPluginMetaData>
#include <kcmodule.h>

#include <QLabel>

class FakeKcm : public KCModule
{
public:
    FakeKcm(QObject *parent, const KPluginMetaData &data)
        : KCModule(parent, data)
    {
        auto label = new QLabel(widget());
        label->setText(QStringLiteral("Demo Text"));
        qWarning() << Q_FUNC_INFO;
    }
    void load() override
    {
        qWarning() << Q_FUNC_INFO;
        // The QObject hierarchy until we are at the KCMultiDialog
        auto p = parent()->parent()->parent()->parent();
        p->setProperty("loadcalled", p->property("loadcalled").toInt() + 1);
        setNeedsSave(false);
    }
};

K_PLUGIN_CLASS_WITH_JSON(FakeKcm, "fakekcm.json")

#include "fakekcm.moc"
