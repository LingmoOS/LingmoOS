/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Benjamin C Meyer <ben+kdelibs at meyerhome dot net>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2004 Michael Brade <brade@kde.org>
    SPDX-FileCopyrightText: 2021 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigdialog.h"

#include <KCoreConfigSkeleton>
#include <KLocalizedString>
#include <KPageWidgetModel>
#include <kconfigdialogmanager.h>
#include <khelpclient.h>

#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

#include <vector>

class KConfigDialogPrivate
{
public:
    KConfigDialogPrivate(const QString &name, KCoreConfigSkeleton *config, KConfigDialog *qq)
        : q(qq)
    {
        const QString dialogName = !name.isEmpty() ? name : QString::asprintf("SettingsDialog-%p", static_cast<void *>(q));

        q->setObjectName(dialogName);
        q->setWindowTitle(i18nc("@title:window", "Configure"));
        q->setFaceType(KPageDialog::List);
        s_openDialogs.push_back({dialogName, q});

        QDialogButtonBox *buttonBox = q->buttonBox();
        buttonBox->setStandardButtons(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel
                                      | QDialogButtonBox::Help);
        QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, q, &KConfigDialog::updateSettings);
        QObject::connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, &KConfigDialog::updateSettings);
        QObject::connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, [this]() {
            updateButtons();
        });
        QObject::connect(buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, q, &KConfigDialog::updateWidgets);
        QObject::connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, q, &KConfigDialog::updateWidgetsDefault);
        QObject::connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, q, [this]() {
            updateButtons();
        });
        QObject::connect(buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, q, &KConfigDialog::showHelp);

        QObject::connect(q, &KPageDialog::pageRemoved, q, &KConfigDialog::onPageRemoved);

        manager = new KConfigDialogManager(q, config);
        setupManagerConnections(manager);

        if (QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply)) {
            applyButton->setEnabled(false);
        };
    }

    KPageWidgetItem *addPageInternal(QWidget *page, const QString &itemName, const QString &pixmapName, const QString &header);

    void setupManagerConnections(KConfigDialogManager *manager);

    void updateApplyButton();
    void updateDefaultsButton();
    void updateButtons();
    void settingsChangedSlot();

    KConfigDialog *const q;
    QString mAnchor;
    QString mHelpApp;
    bool shown = false;
    KConfigDialogManager *manager = nullptr;

    struct WidgetManager {
        QWidget *widget = nullptr;
        KConfigDialogManager *manager = nullptr;
    };
    std::vector<WidgetManager> m_managerForPage;

    /**
     * The list of existing dialogs.
     */
    struct OpenDialogInfo {
        QString dialogName;
        KConfigDialog *dialog = nullptr;
    };
    static std::vector<OpenDialogInfo> s_openDialogs;
};

std::vector<KConfigDialogPrivate::OpenDialogInfo> KConfigDialogPrivate::s_openDialogs;

KConfigDialog::KConfigDialog(QWidget *parent, const QString &name, KCoreConfigSkeleton *config)
    : KPageDialog(parent)
    , d(new KConfigDialogPrivate(name, config, this))
{
}

KConfigDialog::~KConfigDialog()
{
    auto &openDlgs = KConfigDialogPrivate::s_openDialogs;
    const QString currentObjectName = objectName();
    auto it = std::find_if(openDlgs.cbegin(), openDlgs.cend(), [=](const KConfigDialogPrivate::OpenDialogInfo &info) {
        return currentObjectName == info.dialogName;
    });

    if (it != openDlgs.cend()) {
        openDlgs.erase(it);
    }
}

KPageWidgetItem *KConfigDialog::addPage(QWidget *page, const QString &itemName, const QString &pixmapName, const QString &header, bool manage)
{
    Q_ASSERT(page);
    if (!page) {
        return nullptr;
    }

    KPageWidgetItem *item = d->addPageInternal(page, itemName, pixmapName, header);
    if (manage) {
        d->manager->addWidget(page);
    }

    if (d->shown && manage) {
        // update the default button if the dialog is shown
        QPushButton *defaultButton = buttonBox()->button(QDialogButtonBox::RestoreDefaults);
        if (defaultButton) {
            bool is_default = defaultButton->isEnabled() && d->manager->isDefault();
            defaultButton->setEnabled(!is_default);
        }
    }
    return item;
}

KPageWidgetItem *KConfigDialog::addPage(QWidget *page, KCoreConfigSkeleton *config, const QString &itemName, const QString &pixmapName, const QString &header)
{
    Q_ASSERT(page);
    if (!page) {
        return nullptr;
    }

    KPageWidgetItem *item = d->addPageInternal(page, itemName, pixmapName, header);
    auto *manager = new KConfigDialogManager(page, config);
    d->m_managerForPage.push_back({page, manager});
    d->setupManagerConnections(manager);

    if (d->shown) {
        // update the default button if the dialog is shown
        QPushButton *defaultButton = buttonBox()->button(QDialogButtonBox::RestoreDefaults);
        if (defaultButton) {
            const bool is_default = defaultButton->isEnabled() && manager->isDefault();
            defaultButton->setEnabled(!is_default);
        }
    }
    return item;
}

KPageWidgetItem *KConfigDialogPrivate::addPageInternal(QWidget *page, const QString &itemName, const QString &pixmapName, const QString &header)
{
    QWidget *frame = new QWidget(q);
    QVBoxLayout *boxLayout = new QVBoxLayout(frame);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->setContentsMargins(0, 0, 0, 0);

    QScrollArea *scroll = new QScrollArea(q);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidget(page);
    scroll->setWidgetResizable(true);
    scroll->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    if (page->minimumSizeHint().height() > scroll->sizeHint().height() - 2) {
        if (page->sizeHint().width() < scroll->sizeHint().width() + 2) {
            // QScrollArea is planning only a vertical scroll bar,
            // try to avoid the horizontal one by reserving space for the vertical one.
            // Currently KPageViewPrivate::_k_modelChanged() queries the minimumSizeHint().
            // We can only set the minimumSize(), so this approach relies on QStackedWidget size calculation.
            scroll->setMinimumWidth(scroll->sizeHint().width() + qBound(0, scroll->verticalScrollBar()->sizeHint().width(), 200) + 4);
        }
    }

    boxLayout->addWidget(scroll);
    KPageWidgetItem *item = new KPageWidgetItem(frame, itemName);
    item->setHeader(header);
    if (!pixmapName.isEmpty()) {
        item->setIcon(QIcon::fromTheme(pixmapName));
    }

    q->KPageDialog::addPage(item);
    return item;
}

void KConfigDialogPrivate::setupManagerConnections(KConfigDialogManager *manager)
{
    q->connect(manager, qOverload<>(&KConfigDialogManager::settingsChanged), q, [this]() {
        settingsChangedSlot();
    });
    q->connect(manager, &KConfigDialogManager::widgetModified, q, [this]() {
        updateButtons();
    });

    QDialogButtonBox *buttonBox = q->buttonBox();
    q->connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, manager, &KConfigDialogManager::updateSettings);
    q->connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, manager, &KConfigDialogManager::updateSettings);
    q->connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, manager, &KConfigDialogManager::updateWidgets);
    q->connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, manager, &KConfigDialogManager::updateWidgetsDefault);
}

void KConfigDialogPrivate::updateApplyButton()
{
    QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
    if (!applyButton) {
        return;
    }

    const bool hasManagerChanged = std::any_of(m_managerForPage.cbegin(), m_managerForPage.cend(), [](const WidgetManager &widgetManager) {
        return widgetManager.manager->hasChanged();
    });

    applyButton->setEnabled(manager->hasChanged() || q->hasChanged() || hasManagerChanged);
}

void KConfigDialogPrivate::updateDefaultsButton()
{
    QPushButton *restoreDefaultsButton = q->buttonBox()->button(QDialogButtonBox::RestoreDefaults);
    if (!restoreDefaultsButton) {
        return;
    }

    const bool isManagerDefaulted = std::all_of(m_managerForPage.cbegin(), m_managerForPage.cend(), [](const WidgetManager &widgetManager) {
        return widgetManager.manager->isDefault();
    });

    restoreDefaultsButton->setDisabled(manager->isDefault() && q->isDefault() && isManagerDefaulted);
}

void KConfigDialog::onPageRemoved(KPageWidgetItem *item)
{
    auto it = std::find_if(d->m_managerForPage.cbegin(), d->m_managerForPage.cend(), [item](const KConfigDialogPrivate::WidgetManager &wm) {
        return item->widget()->isAncestorOf(wm.widget);
    });

    if (it != d->m_managerForPage.cend()) { // There is a manager for this page, so remove it
        delete it->manager;
        d->m_managerForPage.erase(it);
        d->updateButtons();
    }
}

KConfigDialog *KConfigDialog::exists(const QString &name)
{
    auto &openDlgs = KConfigDialogPrivate::s_openDialogs;
    auto it = std::find_if(openDlgs.cbegin(), openDlgs.cend(), [name](const KConfigDialogPrivate::OpenDialogInfo &info) {
        return name == info.dialogName;
    });

    return it != openDlgs.cend() ? it->dialog : nullptr;
}

bool KConfigDialog::showDialog(const QString &name)
{
    KConfigDialog *dialog = exists(name);
    if (dialog) {
        dialog->show();
    }
    return (dialog != nullptr);
}

void KConfigDialogPrivate::updateButtons()
{
    static bool only_once = false;
    if (only_once) {
        return;
    }
    only_once = true;

    updateApplyButton();
    updateDefaultsButton();

    Q_EMIT q->widgetModified();
    only_once = false;
}

void KConfigDialogPrivate::settingsChangedSlot()
{
    // Update the buttons
    updateButtons();
    Q_EMIT q->settingsChanged(q->objectName());
}

void KConfigDialog::showEvent(QShowEvent *e)
{
    if (!d->shown) {
        updateWidgets();
        d->manager->updateWidgets();
        for (auto [widget, manager] : d->m_managerForPage) {
            manager->updateWidgets();
        }

        d->updateApplyButton();
        d->updateDefaultsButton();

        d->shown = true;
    }
    KPageDialog::showEvent(e);
}

void KConfigDialog::updateSettings()
{
}

void KConfigDialog::updateWidgets()
{
}

void KConfigDialog::updateWidgetsDefault()
{
}

bool KConfigDialog::hasChanged()
{
    return false;
}

bool KConfigDialog::isDefault()
{
    return true;
}

void KConfigDialog::updateButtons()
{
    d->updateButtons();
}

void KConfigDialog::settingsChangedSlot()
{
    d->settingsChangedSlot();
}

void KConfigDialog::setHelp(const QString &anchor, const QString &appname)
{
    d->mAnchor = anchor;
    d->mHelpApp = appname;
}

void KConfigDialog::showHelp()
{
    KHelpClient::invokeHelp(d->mAnchor, d->mHelpApp);
}

#include "moc_kconfigdialog.cpp"
