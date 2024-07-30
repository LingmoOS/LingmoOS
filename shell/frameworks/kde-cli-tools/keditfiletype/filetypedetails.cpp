/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000, 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

// Own
#include "filetypedetails.h"

// Qt
#include <QButtonGroup>
#include <QCheckBox>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMimeDatabase>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <KIconButton>
#include <KIconDialog>
#include <KLineEdit>
#include <KLocalizedString>
#include <KSharedConfig>

// Local
#include "kservicelistwidget.h"
#include "typeslistitem.h"

FileTypeDetails::FileTypeDetails(QWidget *parent)
    : QWidget(parent)
    , m_mimeTypeData(nullptr)
    , m_item(nullptr)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);

    m_mimeTypeLabel = new QLabel(this);
    m_mimeTypeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    topLayout->addWidget(m_mimeTypeLabel, 0, Qt::AlignCenter);

    m_tabWidget = new QTabWidget(this);
    topLayout->addWidget(m_tabWidget);

    QString wtstr;
    // First tab - General
    QWidget *firstWidget = new QWidget(m_tabWidget);
    QVBoxLayout *firstLayout = new QVBoxLayout(firstWidget);

    QHBoxLayout *hBox = new QHBoxLayout();
    firstLayout->addLayout(hBox);

    iconButton = new KIconButton(firstWidget);
    iconButton->setIconType(KIconLoader::Desktop, KIconLoader::MimeType);
    connect(iconButton, &KIconButton::iconChanged, this, &FileTypeDetails::updateIcon);
    iconButton->setWhatsThis(
        i18n("This button displays the icon associated"
             " with the selected file type. Click on it to choose a different icon."));
    iconButton->setFixedSize(70, 70);
    iconLabel = nullptr;
    hBox->addWidget(iconButton);

    QGroupBox *gb = new QGroupBox(i18n("Filename Patterns"), firstWidget);
    hBox->addWidget(gb);

    hBox = new QHBoxLayout(gb);

    extensionLB = new QListWidget(gb);
    connect(extensionLB, &QListWidget::itemSelectionChanged, this, &FileTypeDetails::enableExtButtons);
    hBox->addWidget(extensionLB);

    extensionLB->setFixedHeight(extensionLB->minimumSizeHint().height());

    extensionLB->setWhatsThis(
        i18n("This box contains a list of patterns that can be"
             " used to identify files of the selected type. For example, the pattern *.txt is"
             " associated with the file type 'text/plain'; all files ending in '.txt' are recognized"
             " as plain text files."));

    QVBoxLayout *vbox = new QVBoxLayout();
    hBox->addLayout(vbox);

    addExtButton = new QPushButton(i18n("Add..."), gb);
    addExtButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    addExtButton->setEnabled(false);
    connect(addExtButton, &QAbstractButton::clicked, this, &FileTypeDetails::addExtension);
    vbox->addWidget(addExtButton);
    addExtButton->setWhatsThis(i18n("Add a new pattern for the selected file type."));

    removeExtButton = new QPushButton(i18n("Remove"), gb);
    removeExtButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    removeExtButton->setEnabled(false);
    connect(removeExtButton, &QAbstractButton::clicked, this, &FileTypeDetails::removeExtension);
    vbox->addWidget(removeExtButton);
    removeExtButton->setWhatsThis(i18n("Remove the selected filename pattern."));

    vbox->addStretch(1);

    gb->setFixedHeight(gb->minimumSizeHint().height());

    description = new KLineEdit(firstWidget);
    description->setClearButtonEnabled(true);
    connect(description, &QLineEdit::textChanged, this, &FileTypeDetails::updateDescription);

    QHBoxLayout *descriptionBox = new QHBoxLayout;
    descriptionBox->addWidget(new QLabel(i18n("Description:"), firstWidget));
    descriptionBox->addWidget(description);
    firstLayout->addLayout(descriptionBox);

    wtstr = i18n(
        "You can enter a short description for files of the selected"
        " file type (e.g. 'HTML Page'). This description will be used by applications"
        " like Konqueror to display directory content.");
    description->setWhatsThis(wtstr);

    serviceListWidget = new KServiceListWidget(KServiceListWidget::SERVICELIST_APPLICATIONS, firstWidget);
    connect(serviceListWidget, &KServiceListWidget::changed, this, &FileTypeDetails::changed);
    connect(serviceListWidget, &KServiceListWidget::multiApply, this, &FileTypeDetails::multiApply);
    firstLayout->addWidget(serviceListWidget, 5);

    // Second tab - Embedding
    QWidget *secondWidget = new QWidget(m_tabWidget);
    QVBoxLayout *secondLayout = new QVBoxLayout(secondWidget);

    m_autoEmbedBox = new QGroupBox(i18n("Left Click Action in Konqueror"), secondWidget);
    secondLayout->addWidget(m_autoEmbedBox);

    m_autoEmbedBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QRadioButton *embViewerRadio = new QRadioButton(i18n("Show file in embedded viewer"));
    QRadioButton *sepViewerRadio = new QRadioButton(i18n("Show file in separate viewer"));
    m_rbGroupSettings = new QRadioButton(QStringLiteral("Use settings for '%1' group"));

    m_chkAskSave = new QCheckBox(i18n("Ask whether to save to disk instead (only for Konqueror browser)"));
    connect(m_chkAskSave, &QAbstractButton::toggled, this, &FileTypeDetails::slotAskSaveToggled);

    m_autoEmbedGroup = new QButtonGroup(m_autoEmbedBox);
    m_autoEmbedGroup->addButton(embViewerRadio, 0);
    m_autoEmbedGroup->addButton(sepViewerRadio, 1);
    m_autoEmbedGroup->addButton(m_rbGroupSettings, 2);
    connect(m_autoEmbedGroup, &QButtonGroup::idClicked, this, &FileTypeDetails::slotAutoEmbedClicked);

    vbox = new QVBoxLayout(m_autoEmbedBox);
    vbox->addWidget(embViewerRadio);
    vbox->addWidget(sepViewerRadio);
    vbox->addWidget(m_rbGroupSettings);
    vbox->addWidget(m_chkAskSave);

    m_autoEmbedBox->setWhatsThis(
        i18n("Here you can configure what the Konqueror file manager"
             " will do when you click on a file of this type. Konqueror can either display the file in"
             " an embedded viewer, or start up a separate application. If set to 'Use settings for G group',"
             " the file manager will behave according to the settings of the group G to which this type belongs;"
             " for instance, 'image' if the current file type is image/png. Dolphin"
             " always shows files in a separate viewer."));

    embedServiceListWidget = new KServiceListWidget(KServiceListWidget::SERVICELIST_SERVICES, secondWidget);
    //  embedServiceListWidget->setMinimumHeight( serviceListWidget->sizeHint().height() );
    connect(embedServiceListWidget, &KServiceListWidget::changed, this, &FileTypeDetails::changed);
    connect(embedServiceListWidget, &KServiceListWidget::multiApply, this, &FileTypeDetails::multiApply);
    secondLayout->addWidget(embedServiceListWidget);

    m_tabWidget->addTab(firstWidget, i18n("&General"));
    m_tabWidget->addTab(secondWidget, i18n("&Embedding"));
}

void FileTypeDetails::updateRemoveButton()
{
    removeExtButton->setEnabled(extensionLB->count() > 0);
}

void FileTypeDetails::updateIcon(const QString &icon)
{
    if (!m_mimeTypeData) {
        return;
    }

    m_mimeTypeData->setUserSpecifiedIcon(icon);

    if (m_item) {
        m_item->setIcon(icon);
    }

    Q_EMIT changed(true);
}

void FileTypeDetails::updateDescription(const QString &desc)
{
    if (!m_mimeTypeData) {
        return;
    }

    m_mimeTypeData->setComment(desc);

    Q_EMIT changed(true);
}

void FileTypeDetails::addExtension()
{
    if (!m_mimeTypeData) {
        return;
    }

    bool ok;
    QString ext = QInputDialog::getText(this, i18n("Add New Extension"), i18n("Extension:"), QLineEdit::Normal, QStringLiteral("*."), &ok);
    if (ok) {
        extensionLB->addItem(ext);
        QStringList patt = m_mimeTypeData->patterns();
        patt += ext;
        m_mimeTypeData->setPatterns(patt);
        updateRemoveButton();
        Q_EMIT changed(true);
    }
}

void FileTypeDetails::removeExtension()
{
    if (extensionLB->currentRow() == -1) {
        return;
    }
    if (!m_mimeTypeData) {
        return;
    }
    QStringList patt = m_mimeTypeData->patterns();
    patt.removeAll(extensionLB->currentItem()->text());
    m_mimeTypeData->setPatterns(patt);
    delete extensionLB->takeItem(extensionLB->currentRow());
    updateRemoveButton();
    Q_EMIT changed(true);
}

void FileTypeDetails::slotAutoEmbedClicked(int button)
{
    if (!m_mimeTypeData || (button > 2)) {
        return;
    }

    m_mimeTypeData->setAutoEmbed((MimeTypeData::AutoEmbed)button);

    updateAskSave();

    Q_EMIT changed(true);
}

void FileTypeDetails::updateAskSave()
{
    if (!m_mimeTypeData) {
        return;
    }
    QMimeDatabase db;

    MimeTypeData::AutoEmbed autoEmbed = m_mimeTypeData->autoEmbed();
    if (m_mimeTypeData->isMeta() && autoEmbed == MimeTypeData::UseGroupSetting) {
        // Resolve by looking at group (we could cache groups somewhere to avoid the re-parsing?)
        autoEmbed = MimeTypeData(m_mimeTypeData->majorType()).autoEmbed();
    }

    const QString mimeType = m_mimeTypeData->name();

    QString dontAskAgainName;
    if (autoEmbed == MimeTypeData::Yes) { // Embedded
        dontAskAgainName = QStringLiteral("askEmbedOrSave") + mimeType;
    } else {
        dontAskAgainName = QStringLiteral("askSave") + mimeType;
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("filetypesrc"), KConfig::NoGlobals);
    // default value
    bool ask = config->group("Notification Messages").readEntry(dontAskAgainName, QString()).isEmpty();
    // per-mimetype override if there's one
    m_mimeTypeData->getAskSave(ask);

    bool neverAsk = false;

    if (autoEmbed == MimeTypeData::Yes) {
        const QMimeType mime = db.mimeTypeForName(mimeType);
        if (mime.isValid()) {
            // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
            // NOTE: Keep this function in sync with
            // kparts/src/browseropenorsavequestion.cpp BrowserOpenOrSaveQuestionPrivate::autoEmbedMimeType

            // Don't ask for:
            // - html (even new tabs would ask, due to about:blank!)
            // - dirs obviously (though not common over HTTP :),
            // - images (reasoning: no need to save, most of the time, because fast to see)
            // e.g. postscript is different, because takes longer to read, so
            // it's more likely that the user might want to save it.
            // - multipart/* ("server push", see kmultipart)
            // clang-format off
            if (mime.inherits(QStringLiteral("text/html"))
                || mime.inherits(QStringLiteral("application/xml"))
                || mime.inherits(QStringLiteral("inode/directory"))
                || mimeType.startsWith(QLatin1String("image"))
                || mime.inherits(QStringLiteral("multipart/x-mixed-replace"))
                || mime.inherits(QStringLiteral("multipart/replace"))) {
                neverAsk = true;
            }
            // clang-format on
        }
    }

    m_chkAskSave->blockSignals(true);
    m_chkAskSave->setChecked(ask && !neverAsk);
    m_chkAskSave->setEnabled(!neverAsk);
    m_chkAskSave->blockSignals(false);
}

void FileTypeDetails::slotAskSaveToggled(bool askSave)
{
    if (!m_mimeTypeData) {
        return;
    }

    m_mimeTypeData->setAskSave(askSave);
    Q_EMIT changed(true);
}

void FileTypeDetails::setMimeTypeData(MimeTypeData *mimeTypeData, TypesListItem *item)
{
    m_mimeTypeData = mimeTypeData;
    m_item = item; // can be 0
    Q_ASSERT(mimeTypeData);
    m_mimeTypeLabel->setText(i18n("File type %1", mimeTypeData->name()));
    if (iconButton) {
        iconButton->setIcon(mimeTypeData->icon());
        iconButton->setToolTip(mimeTypeData->icon());
    } else {
        iconLabel->setPixmap(QIcon::fromTheme(mimeTypeData->icon()).pixmap(KIconLoader::SizeLarge));
    }
    description->setText(mimeTypeData->comment());
    m_rbGroupSettings->setText(i18n("Use settings for '%1' group", mimeTypeData->majorType()));
    extensionLB->clear();
    addExtButton->setEnabled(true);
    removeExtButton->setEnabled(false);

    serviceListWidget->setMimeTypeData(mimeTypeData);
    embedServiceListWidget->setMimeTypeData(mimeTypeData);
    m_autoEmbedGroup->button(mimeTypeData->autoEmbed())->setChecked(true);
    m_rbGroupSettings->setEnabled(mimeTypeData->canUseGroupSetting());

    extensionLB->addItems(mimeTypeData->patterns());

    updateAskSave();
}

void FileTypeDetails::enableExtButtons()
{
    removeExtButton->setEnabled(true);
}

void FileTypeDetails::refresh()
{
    if (!m_mimeTypeData) {
        return;
    }

    // Called when ksycoca has been updated -> refresh data, then widgets
    m_mimeTypeData->refresh();
    setMimeTypeData(m_mimeTypeData, m_item);
}

void FileTypeDetails::allowMultiApply(bool allow)
{
    serviceListWidget->allowMultiApply(allow);
    embedServiceListWidget->allowMultiApply(allow);
}

#include "moc_filetypedetails.cpp"
