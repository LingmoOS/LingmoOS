/*
    SPDX-FileCopyrightText: 2011 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundsonk@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "persondetailsview.h"

#include "kpeople_widgets_debug.h"

#include "abstractfieldwidgetfactory.h"
#include "emaildetailswidget.h"
#include "global.h"

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QVBoxLayout>

#include "ui_person-details-presentation.h"

namespace KPeople
{
class PersonDetailsViewPrivate
{
public:
    PersonData *m_person;
    Ui::PersonDetailsPresentation *m_personDetailsPresentation;
    QWidget *m_mainWidget;
    QList<AbstractFieldWidgetFactory *> m_plugins;
};
}

using namespace KPeople;

class CoreFieldsPlugin : public AbstractFieldWidgetFactory
{
public:
    CoreFieldsPlugin(const QString &field);
    ~CoreFieldsPlugin() override;
    QString label() const override;
    int sortWeight() const override;
    QWidget *createDetailsWidget(const PersonData &person, QWidget *parent) const override;

private:
    QString m_field;
};

CoreFieldsPlugin::CoreFieldsPlugin(const QString &field)
    : m_field(field)
{
}

CoreFieldsPlugin::~CoreFieldsPlugin()
{
}

QString CoreFieldsPlugin::label() const
{
#ifdef __GNUC__
#warning fixme, should be made user-visible somehow
#endif
    return m_field;
}

int CoreFieldsPlugin::sortWeight() const
{
    return 1;
}

QWidget *CoreFieldsPlugin::createDetailsWidget(const PersonData &person, QWidget *parent) const
{
    //  we have a plugin specific for e-mails.
    if (m_field == QLatin1String("email")) {
        return nullptr;
    }

    QString text = person.contactCustomProperty(m_field).toString();
    if (text.isEmpty()) {
        return nullptr;
    }
    return new QLabel(text, parent);
}

PersonDetailsView::PersonDetailsView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new PersonDetailsViewPrivate())
{
    Q_D(PersonDetailsView);

    auto *layout = new QVBoxLayout(this);
    d->m_mainWidget = new QWidget(this);
    d->m_person = nullptr;

    QWidget *details = new QWidget();
    d->m_personDetailsPresentation = new Ui::PersonDetailsPresentation();
    d->m_personDetailsPresentation->setupUi(details);
    layout->addWidget(details);
    layout->addWidget(d->m_mainWidget);
    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    // create plugins
#ifdef __GNUC__
#warning figure out a way to list properties
#endif
    const QStringList fields{QStringLiteral("name"), QStringLiteral("all-email")};
    for (const QString &field : fields) {
        d->m_plugins << new CoreFieldsPlugin(field);
    }

    d->m_plugins << new EmailFieldsPlugin();

    // load every KPeopleWidgets Plugin
    const QList<KPluginMetaData> personPluginList = KPluginMetaData::findPlugins(QStringLiteral("kpeople/widgets"));

    for (const KPluginMetaData &data : personPluginList) {
        auto fieldWidgetFactoryResult = KPluginFactory::instantiatePlugin<AbstractFieldWidgetFactory>(data);
        Q_ASSERT_X(fieldWidgetFactoryResult, Q_FUNC_INFO, qPrintable(fieldWidgetFactoryResult.errorText));
        d->m_plugins << fieldWidgetFactoryResult.plugin;
    }

    // TODO Sort plugins
}

PersonDetailsView::~PersonDetailsView()
{
    delete d_ptr;
}

void PersonDetailsView::setPerson(PersonData *person)
{
    Q_D(PersonDetailsView);
    if (d->m_person) {
        disconnect(d->m_person, SIGNAL(dataChanged()), this, SLOT(reload()));
    }

    d->m_person = person;
    connect(d->m_person, SIGNAL(dataChanged()), this, SLOT(reload()));
    reload();
}

// void PersonDetailsView::setPersonsModel(PersonsModel *model)
// {
//     Q_D(PersonDetailsView);
//     Q_FOREACH (AbstractPersonDetailsWidget *detailsWidget, d->m_detailWidgets) {
//         detailsWidget->setPersonsModel(model);
//     }
// }

void PersonDetailsView::reload()
{
    Q_D(PersonDetailsView);

    // replace the entire main widget
    int layoutIndex = layout()->indexOf(d->m_mainWidget);
    layout()->takeAt(layoutIndex);
    d->m_mainWidget->deleteLater();
    d->m_mainWidget = new QWidget(this);
    dynamic_cast<QVBoxLayout *>(layout())->insertWidget(layoutIndex, d->m_mainWidget);

    QFormLayout *layout = new QFormLayout(d->m_mainWidget);
    layout->setSpacing(4);

    // update header information
    // FIXME - possibly split this out into a new class with a nice setPerson method

    QPixmap avatar = d->m_person->photo();

    d->m_personDetailsPresentation->avatarPixmapLabel->setPixmap(avatar.scaled(96, 96, Qt::KeepAspectRatio)); // FIXME
    d->m_personDetailsPresentation->presencePixmapLabel->setPixmap(QIcon::fromTheme(d->m_person->presenceIconName()).pixmap(32, 32)); // FIXME
    d->m_personDetailsPresentation->nameLabel->setText(d->m_person->name());

    for (AbstractFieldWidgetFactory *widgetFactory : std::as_const(d->m_plugins)) {
        const QString label = widgetFactory->label() + QLatin1Char(':');
        QWidget *widget = widgetFactory->createDetailsWidget(d->m_person->personUri(), this);

        if (widget) {
            QFont font = widget->font();
            font.setBold(true);
            widget->setFont(font);
            QLabel *widgetLabel = new QLabel(label, this);
            layout->addRow(widgetLabel, widget);
        }
    }
}

#include "moc_persondetailsview.cpp"
