/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 */

#include "ernie-bot.h"
#include "message-model.h"
#include "session-manager.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QFile>

class ErnieBotPrivate
{
public:
    LingmoUIMenu::MetadataMap metadata;
    MessageModel *model {nullptr};
    SessionManager *sessionManager {nullptr};
};

ErnieBot::ErnieBot(QObject *parent) : WidgetExtension(parent), d(new ErnieBotPrivate)
{
    QString translationFile("/usr/share/lingmo-menu/extensions/translations/ernie-bot_" + QLocale::system().name() + ".qm");
    if (QFile::exists(translationFile)) {
        auto translator = new QTranslator(this);
        translator->load(translationFile);
        QCoreApplication::installTranslator(translator);
    }

    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Id, "ernie-bot");
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Name, tr("AI Assistant"));
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Tooltip, tr("AI Assistant"));
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Version, "1.0.0");
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Description, "ernie-bot");
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Main, "qrc:///ernie-bot/content/ui/Main.qml");
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Type, LingmoUIMenu::WidgetMetadata::Widget);
    d->metadata.insert(LingmoUIMenu::WidgetMetadata::Flag, LingmoUIMenu::WidgetMetadata::OnlySmallScreen);
    
    qRegisterMetaType<MessageModel*>("MessageModel*");

    d->sessionManager = new SessionManager(this);
    d->model = new MessageModel(this);

    connect(d->sessionManager, &SessionManager::sessionAdded, this, [this] (int sessionId) {
        d->model->setSession(d->sessionManager->getSession(sessionId));
    });

    connect(d->sessionManager, &SessionManager::sessionRemoved, this, [this] (int sessionId) {
        d->model->removeSession(sessionId);
    });
}

int ErnieBot::index() const
{
    return 100;
}

LingmoUIMenu::MetadataMap ErnieBot::metadata() const
{
    return d->metadata;
}

QVariantMap ErnieBot::data()
{
    QVariantMap data;
    data.insert("messageModel", QVariant::fromValue(d->model));
    data.insert("sessionManager", QVariant::fromValue(d->sessionManager));
    return data;
}

void ErnieBot::receive(const QVariantMap &data)
{
    d->model->talk(data.value("message").toString());
}

ErnieBotPlugin::~ErnieBotPlugin()
{

}

QString ErnieBotPlugin::id()
{
    return "ernie-bot";
}

LingmoUIMenu::WidgetExtension *ErnieBotPlugin::createWidgetExtension()
{
    return new ErnieBot;
}

LingmoUIMenu::ContextMenuExtension *ErnieBotPlugin::createContextMenuExtension()
{
    return nullptr;
}
