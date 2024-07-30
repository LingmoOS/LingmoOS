/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "qmlplugin.h"

#include "author.h"
#include "categoriesmodel.h"
#include "commentsmodel.h"
#include "quickengine.h"
#include "quickitemsmodel.h"
#include "quickquestionlistener.h"
#include "quicksettings.h"
#include "searchpresetmodel.h"

#include "provider.h"
#include "providersmodel.h"
#include "question.h"

#include <QQmlEngine>
#include <qqml.h>

void QmlPlugins::initializeEngine(QQmlEngine * /*engine*/, const char *)
{
}

void QmlPlugins::registerTypes(const char *uri)
{
    const char *coreUri{"org.kde.newstuff.core"};

    // Initial version
    qmlRegisterType<Engine>(uri, 1, 0, "Engine");
    qmlRegisterType<ItemsModel>(uri, 1, 0, "ItemsModel");

    // Version 1.62
    qmlRegisterType<KNewStuffQuick::Author>(uri, 1, 62, "Author");
    qmlRegisterType<KNewStuffQuick::CommentsModel>(uri, 1, 62, "CommentsModel");
    qmlRegisterUncreatableType<CategoriesModel>(
        uri,
        1,
        0,
        "CategoriesModel",
        QStringLiteral("This should only be created by the Engine, and provides the categories available in that engine"));
    qmlRegisterUncreatableMetaObject(KNSCore::Provider::staticMetaObject,
                                     coreUri,
                                     1,
                                     62,
                                     "Provider",
                                     QStringLiteral("Error: this only exists to forward enums"));
    qmlRegisterUncreatableMetaObject(KNSCore::Question::staticMetaObject,
                                     coreUri,
                                     1,
                                     62,
                                     "Question",
                                     QStringLiteral("Error: this only exists to forward enums"));
    qmlRegisterSingletonType<KNewStuffQuick::QuickQuestionListener>(uri,
                                                                    1,
                                                                    62,
                                                                    "QuickQuestionListener",
                                                                    [](QQmlEngine *engine, QJSEngine * /*scriptEngine*/) -> QObject * {
                                                                        engine->setObjectOwnership(KNewStuffQuick::QuickQuestionListener::instance(),
                                                                                                   QQmlEngine::CppOwnership);
                                                                        return KNewStuffQuick::QuickQuestionListener::instance();
                                                                    });
    qmlRegisterUncreatableMetaObject(KNSCore::Entry::staticMetaObject, uri, 1, 91, "Entry", QStringLiteral("Entries should only be created by the engine"));
    qmlRegisterUncreatableMetaObject(KNSCore::ErrorCode::staticMetaObject,
                                     uri,
                                     1,
                                     91,
                                     "ErrorCode",
                                     QStringLiteral("Only for access to the KNSCore::ErrorCode enum"));

    // Version 1.81
    qmlRegisterSingletonType<KNewStuffQuick::Settings>(uri, 1, 81, "Settings", [](QQmlEngine *engine, QJSEngine * /*scriptEngine*/) -> QObject * {
        engine->setObjectOwnership(KNewStuffQuick::Settings::instance(), QQmlEngine::CppOwnership);
        return KNewStuffQuick::Settings::instance();
    });
    // Version 1.83
    qmlRegisterUncreatableType<SearchPresetModel>(
        uri,
        1,
        83,
        "SearchPresetModel",
        QStringLiteral("This should only be created by the Engine, and provides the SearchPresets available in that engine"));

    // Version 1.85
    qmlRegisterType<KNSCore::ProvidersModel>(uri, 1, 85, "ProvidersModel");
}

#include "moc_qmlplugin.cpp"
