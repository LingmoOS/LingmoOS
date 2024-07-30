/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003, 2004, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcmoduleloader.h"
#include "kcmoduledata.h"
#include "kcmoduleqml_p.h"
#include "kquickconfigmoduleloader.h"
#include <kcmutils_debug.h>

#include <QJsonArray>
#include <QLabel>
#include <QLibrary>
#include <QVBoxLayout>

#include <KAboutData>
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <memory>
#include <qqmlengine.h>

#include "qml/kquickconfigmodule.h"

using namespace KCModuleLoader;

/***************************************************************/
/**
 * When something goes wrong in loading the module, this one
 * jumps in as a "dummy" module.
 */
class KCMError : public KCModule
{
    Q_OBJECT
public:
    KCMError(const QString &msg, const QString &details, QWidget *parent)
        : KCModule(parent, KPluginMetaData())
    {
        QString realDetails = details.trimmed();
        if (realDetails.isNull()) {
            realDetails = i18n(
                "<qt><p>Possible reasons:<ul><li>An error occurred during your last "
                "system upgrade, leaving an orphaned control module behind</li><li>You have old third party "
                "modules lying around.</li></ul></p><p>Check these points carefully and try to remove "
                "the module mentioned in the error message. If this fails, consider contacting "
                "your distributor or packager.</p></qt>");
        }

        QVBoxLayout *topLayout = new QVBoxLayout(widget());
        QLabel *lab = new QLabel(msg, widget());
        {
            // Similar to Kirigami.Heading: Primary, level 3
            QFont font = lab->font();
            font.setPointSizeF(font.pointSizeF() * 1.15);
            font.setBold(true);
            lab->setFont(font);
        }
        lab->setWordWrap(true);
        lab->setTextInteractionFlags(lab->textInteractionFlags() | Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        topLayout->addWidget(lab);

        lab = new QLabel(realDetails, widget());
        lab->setWordWrap(true);
        lab->setTextInteractionFlags(lab->textInteractionFlags() | Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        topLayout->addWidget(lab);
    }
};

KCModule *KCModuleLoader::loadModule(const KPluginMetaData &metaData, QWidget *parent, const QVariantList &args, const std::shared_ptr<QQmlEngine> &eng)
{
    if (!KAuthorized::authorizeControlModule(metaData.pluginId())) {
        return new KCMError(i18n("The module %1 is disabled.", metaData.pluginId()), i18n("The module has been disabled by the system administrator."), parent);
    }

    const auto qmlKcm = KQuickConfigModuleLoader::loadModule(metaData, parent, args, eng).plugin;
    if (qmlKcm) {
        if (!qmlKcm->mainUi()) {
            return new KCMError(i18n("Error loading QML file."), qmlKcm->errorString(), parent);
        }
        qCDebug(KCMUTILS_LOG) << "loaded KCM" << metaData.fileName();
        return new KCModuleQml(qmlKcm, parent);
    }

    const QVariantList pluginArgs = QVariantList(args) << metaData.rawData().value(QLatin1String("X-KDE-KCM-Args")).toArray().toVariantList();
    const auto kcmoduleResult = KPluginFactory::instantiatePlugin<KCModule>(metaData, parent, pluginArgs);

    if (kcmoduleResult) {
        qCDebug(KCMUTILS_LOG) << "loaded KCM" << metaData.fileName();
        return kcmoduleResult.plugin;
    }

    return new KCMError(QString(), kcmoduleResult.errorString, parent);
}

#include "kcmoduleloader.moc"
