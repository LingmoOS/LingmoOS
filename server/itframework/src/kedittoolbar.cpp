/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kedittoolbar.h"
#include "debug.h"
#include "kedittoolbar_p.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QPushButton>
#include <QShowEvent>
#include <QStandardPaths>
#include <QToolButton>

#include <KIconDialog>
#include <KListWidgetSearchLine>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSeparator>

#include "kactioncollection.h"
#include "ktoolbar.h"
#include "kxmlguifactory.h"

#include "../kxmlgui_version.h"
#include "ktoolbarhelper_p.h"

// static const char *const s_XmlTypeToString[] = { "Shell", "Part", "Local", "Merged" };

typedef QList<QDomElement> ToolBarList;

namespace KDEPrivate
{
/**
 * Return a list of toolbar elements given a toplevel element
 */
static ToolBarList findToolBars(const QDomElement &start)
{
    ToolBarList list;

    for (QDomElement elem = start; !elem.isNull(); elem = elem.nextSiblingElement()) {
        if (elem.tagName() == QLatin1String("ToolBar")) {
            if (elem.attribute(QStringLiteral("noEdit")) != QLatin1String("true")) {
                list.append(elem);
            }
        } else {
            if (elem.tagName() != QLatin1String("MenuBar")) { // there are no toolbars inside the menubar :)
                list += findToolBars(elem.firstChildElement()); // recursive
            }
        }
    }

    return list;
}

class XmlData
{
public:
    enum XmlType { Shell = 0, Part, Local, Merged };

    explicit XmlData(XmlType xmlType, const QString &xmlFile, KActionCollection *collection)
        : m_isModified(false)
        , m_xmlFile(xmlFile)
        , m_type(xmlType)
        , m_actionCollection(collection)
    {
    }
    void dump() const
    {
#if 0
        qDebug(240) << "XmlData" << this << "type" << s_XmlTypeToString[m_type] << "xmlFile:" << m_xmlFile;
        foreach (const QDomElement &element, m_barList) {
            qDebug(240) << "    ToolBar:" << toolBarText(element);
        }
        if (m_actionCollection) {
            qDebug(240) << "    " << m_actionCollection->actions().count() << "actions in the collection.";
        } else {
            qDebug(240) << "    no action collection.";
        }
#endif
    }
    QString xmlFile() const
    {
        return m_xmlFile;
    }
    XmlType type() const
    {
        return m_type;
    }
    KActionCollection *actionCollection() const
    {
        return m_actionCollection;
    }
    void setDomDocument(const QDomDocument &domDoc)
    {
        m_document = domDoc.cloneNode().toDocument();
        m_barList = findToolBars(m_document.documentElement());
    }
    // Return reference, for e.g. actionPropertiesElement() to modify the document
    QDomDocument &domDocument()
    {
        return m_document;
    }
    const QDomDocument &domDocument() const
    {
        return m_document;
    }

    /**
     * Return the text (user-visible name) of a given toolbar
     */
    QString toolBarText(const QDomElement &it) const;

    bool m_isModified = false;
    ToolBarList &barList()
    {
        return m_barList;
    }
    const ToolBarList &barList() const
    {
        return m_barList;
    }

private:
    ToolBarList m_barList;
    QString m_xmlFile;
    QDomDocument m_document;
    XmlType m_type;
    KActionCollection *m_actionCollection;
};

QString XmlData::toolBarText(const QDomElement &it) const
{
    QString name = KToolbarHelper::i18nToolBarName(it);

    // the name of the toolbar might depend on whether or not
    // it is in kparts
    if ((m_type == XmlData::Shell) || (m_type == XmlData::Part)) {
        QString doc_name(m_document.documentElement().attribute(QStringLiteral("name")));
        name += QLatin1String(" <") + doc_name + QLatin1Char('>');
    }
    return name;
}

typedef QList<XmlData> XmlDataList;

class ToolBarItem : public QListWidgetItem
{
public:
    ToolBarItem(QListWidget *parent, const QString &tag = QString(), const QString &name = QString(), const QString &statusText = QString())
        : QListWidgetItem(parent)
        , m_internalTag(tag)
        , m_internalName(name)
        , m_statusText(statusText)
        , m_isSeparator(false)
        , m_isSpacer(false)
        , m_isTextAlongsideIconHidden(false)
    {
        // Drop between items, not onto items
        setFlags((flags() | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
    }

    void setInternalTag(const QString &tag)
    {
        m_internalTag = tag;
    }
    void setInternalName(const QString &name)
    {
        m_internalName = name;
    }
    void setStatusText(const QString &text)
    {
        m_statusText = text;
    }
    void setSeparator(bool sep)
    {
        m_isSeparator = sep;
    }
    void setSpacer(bool spacer)
    {
        m_isSpacer = spacer;
    }
    void setTextAlongsideIconHidden(bool hidden)
    {
        m_isTextAlongsideIconHidden = hidden;
    }
    QString internalTag() const
    {
        return m_internalTag;
    }
    QString internalName() const
    {
        return m_internalName;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    bool isSeparator() const
    {
        return m_isSeparator;
    }
    bool isSpacer() const
    {
        return m_isSpacer;
    }
    bool isTextAlongsideIconHidden() const
    {
        return m_isTextAlongsideIconHidden;
    }

    int index() const
    {
        return listWidget()->row(const_cast<ToolBarItem *>(this));
    }

private:
    QString m_internalTag;
    QString m_internalName;
    QString m_statusText;
    bool m_isSeparator;
    bool m_isSpacer;
    bool m_isTextAlongsideIconHidden;
};

static QDataStream &operator<<(QDataStream &s, const ToolBarItem &item)
{
    s << item.internalTag();
    s << item.internalName();
    s << item.statusText();
    s << item.isSeparator();
    s << item.isSpacer();
    s << item.isTextAlongsideIconHidden();
    return s;
}
static QDataStream &operator>>(QDataStream &s, ToolBarItem &item)
{
    QString internalTag;
    s >> internalTag;
    item.setInternalTag(internalTag);
    QString internalName;
    s >> internalName;
    item.setInternalName(internalName);
    QString statusText;
    s >> statusText;
    item.setStatusText(statusText);
    bool sep;
    s >> sep;
    item.setSeparator(sep);
    bool spacer;
    s >> spacer;
    item.setSpacer(spacer);
    bool hidden;
    s >> hidden;
    item.setTextAlongsideIconHidden(hidden);
    return s;
}

////

ToolBarListWidget::ToolBarListWidget(QWidget *parent)
    : QListWidget(parent)
    , m_activeList(true)
{
    setDragDropMode(QAbstractItemView::DragDrop); // no internal moves
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QMimeData *ToolBarListWidget::mimeData(const QList<QListWidgetItem *> items) const
#else
QMimeData *ToolBarListWidget::mimeData(const QList<QListWidgetItem *> &items) const
#endif
{
    if (items.isEmpty()) {
        return nullptr;
    }
    QMimeData *mimedata = new QMimeData();

    QByteArray data;
    {
        QDataStream stream(&data, QIODevice::WriteOnly);
        // we only support single selection
        ToolBarItem *item = static_cast<ToolBarItem *>(items.first());
        stream << *item;
    }

    mimedata->setData(QStringLiteral("application/x-kde-action-list"), data);
    mimedata->setData(QStringLiteral("application/x-kde-source-treewidget"), m_activeList ? "active" : "inactive");

    return mimedata;
}

bool ToolBarListWidget::dropMimeData(int index, const QMimeData *mimeData, Qt::DropAction action)
{
    Q_UNUSED(action)
    const QByteArray data = mimeData->data(QStringLiteral("application/x-kde-action-list"));
    if (data.isEmpty()) {
        return false;
    }
    QDataStream stream(data);
    const bool sourceIsActiveList = mimeData->data(QStringLiteral("application/x-kde-source-treewidget")) == "active";
    ToolBarItem *item = new ToolBarItem(this); // needs parent, use this temporarily
    stream >> *item;
    Q_EMIT dropped(this, index, item, sourceIsActiveList);
    return true;
}

ToolBarItem *ToolBarListWidget::currentItem() const
{
    return static_cast<ToolBarItem *>(QListWidget::currentItem());
}

IconTextEditDialog::IconTextEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Change Text"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QGridLayout *grid = new QGridLayout;
    grid->setContentsMargins(0, 0, 0, 0);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setClearButtonEnabled(true);
    QLabel *label = new QLabel(i18n("Icon te&xt:"), this);
    label->setBuddy(m_lineEdit);
    grid->addWidget(label, 0, 0);
    grid->addWidget(m_lineEdit, 0, 1);

    m_cbHidden = new QCheckBox(i18nc("@option:check", "&Hide text when toolbar shows text alongside icons"), this);
    grid->addWidget(m_cbHidden, 1, 1);

    layout->addLayout(grid);

    m_buttonBox = new QDialogButtonBox(this);
    m_buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttonBox);

    connect(m_lineEdit, &QLineEdit::textChanged, this, &IconTextEditDialog::slotTextChanged);

    m_lineEdit->setFocus();
    setFixedHeight(sizeHint().height());
}

void IconTextEditDialog::setIconText(const QString &text)
{
    m_lineEdit->setText(text);
}

QString IconTextEditDialog::iconText() const
{
    return m_lineEdit->text().trimmed();
}

void IconTextEditDialog::setTextAlongsideIconHidden(bool hidden)
{
    m_cbHidden->setChecked(hidden);
}

bool IconTextEditDialog::textAlongsideIconHidden() const
{
    return m_cbHidden->isChecked();
}

void IconTextEditDialog::slotTextChanged(const QString &text)
{
    // Do not allow empty icon text
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.trimmed().isEmpty());
}

class KEditToolBarWidgetPrivate
{
public:
    /**
     *
     * @param collection In the old-style constructor, this is the collection passed
     * to the KEditToolBar constructor.
     * In the xmlguifactory-based constructor, we let KXMLGUIClient create a dummy one,
     * but it probably isn't used.
     */
    KEditToolBarWidgetPrivate(KEditToolBarWidget *widget, const QString &cName, KActionCollection *collection)
        : m_collection(collection)
        , m_widget(widget)
        , m_factory(nullptr)
        , m_componentName(cName)
        , m_helpArea(nullptr)
        , m_isPart(false)
        , m_loadedOnce(false)
    {
        // We want items with an icon to align with items without icon
        // So we use an empty QPixmap for that
        const int iconSize = widget->style()->pixelMetric(QStyle::PM_SmallIconSize);
        m_emptyIcon = QPixmap(iconSize, iconSize);
        m_emptyIcon.fill(Qt::transparent);
    }
    ~KEditToolBarWidgetPrivate()
    {
    }

    // private slots
    void slotToolBarSelected(int index);

    void slotInactiveSelectionChanged();
    void slotActiveSelectionChanged();

    void slotInsertButton();
    void slotRemoveButton();
    void slotUpButton();
    void slotDownButton();

    void selectActiveItem(const QString &);

    void slotChangeIcon();
    void slotChangeIconText();

    void slotDropped(ToolBarListWidget *list, int index, ToolBarItem *item, bool sourceIsActiveList);

    void setupLayout();

    void initOldStyle(const QString &file, bool global, const QString &defaultToolbar);
    void initFromFactory(KXMLGUIFactory *factory, const QString &defaultToolbar);
    void loadToolBarCombo(const QString &defaultToolbar);
    void loadActions(const QDomElement &elem);

    QString xmlFile(const QString &xml_file) const
    {
        return xml_file.isEmpty() ? m_componentName + QLatin1String("ui.rc") : xml_file;
    }

    /**
     * Load in the specified XML file and dump the raw xml
     */
    QString loadXMLFile(const QString &_xml_file)
    {
        QString raw_xml;
        QString xml_file = xmlFile(_xml_file);
        // qCDebug(DEBUG_KXMLGUI) << "loadXMLFile xml_file=" << xml_file;

        if (!QDir::isRelativePath(xml_file)) {
            raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
        } else {
            raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_componentName);
        }

        return raw_xml;
    }

    /**
     * Look for a given item in the current toolbar
     */
    QDomElement findElementForToolBarItem(const ToolBarItem *item) const
    {
        // qDebug(240) << "looking for name=" << item->internalName() << "and tag=" << item->internalTag();
        for (QDomNode n = m_currentToolBarElem.firstChild(); !n.isNull(); n = n.nextSibling()) {
            QDomElement elem = n.toElement();
            if ((elem.attribute(QStringLiteral("name")) == item->internalName()) && (elem.tagName() == item->internalTag())) {
                return elem;
            }
        }
        // qDebug(240) << "no item found in the DOM with name=" << item->internalName() << "and tag=" << item->internalTag();
        return QDomElement();
    }

    void insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend = false);
    void removeActive(ToolBarItem *item);
    void moveActive(ToolBarItem *item, ToolBarItem *before);
    void updateLocal(QDomElement &elem);

#ifndef NDEBUG
    void dump() const
    {
        for (const auto &xmlFile : m_xmlFiles) {
            xmlFile.dump();
        }
    }
#endif

    QComboBox *m_toolbarCombo;

    QToolButton *m_upAction;
    QToolButton *m_removeAction;
    QToolButton *m_insertAction;
    QToolButton *m_downAction;

    // QValueList<QAction*> m_actionList;
    KActionCollection *m_collection;
    KEditToolBarWidget *const m_widget;
    KXMLGUIFactory *m_factory;
    QString m_componentName;

    QPixmap m_emptyIcon;

    XmlData *m_currentXmlData;
    QDomElement m_currentToolBarElem;

    QString m_xmlFile;
    QString m_globalFile;
    QString m_rcFile;
    QDomDocument m_localDoc;

    ToolBarList m_barList;
    ToolBarListWidget *m_inactiveList;
    ToolBarListWidget *m_activeList;

    XmlDataList m_xmlFiles;

    QLabel *m_comboLabel;
    KSeparator *m_comboSeparator;
    QLabel *m_helpArea;
    QPushButton *m_changeIcon;
    QPushButton *m_changeIconText;
    bool m_isPart : 1;
    bool m_loadedOnce : 1;
};

}

using namespace KDEPrivate;

class KEditToolBarPrivate
{
public:
    KEditToolBarPrivate(KEditToolBar *qq)
        : q(qq)
    {
    }

    void init();

    void slotButtonClicked(QAbstractButton *button);
    void acceptOK(bool);
    void enableApply(bool);
    void okClicked();
    void applyClicked();
    void defaultClicked();

    KEditToolBar *const q;
    bool m_accept = false;
    // Save parameters for recreating widget after resetting toolbar
    bool m_global = false;
    KActionCollection *m_collection = nullptr;
    QString m_file;
    QString m_defaultToolBar;
    KXMLGUIFactory *m_factory = nullptr;
    KEditToolBarWidget *m_widget = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};

Q_GLOBAL_STATIC(QString, s_defaultToolBarName)

KEditToolBar::KEditToolBar(KActionCollection *collection, QWidget *parent)
    : QDialog(parent)
    , d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(collection, this);
    d->init();
    d->m_collection = collection;
}

KEditToolBar::KEditToolBar(KXMLGUIFactory *factory, QWidget *parent)
    : QDialog(parent)
    , d(new KEditToolBarPrivate(this))
{
    d->m_widget = new KEditToolBarWidget(this);
    d->init();
    d->m_factory = factory;
}

void KEditToolBarPrivate::init()
{
    m_accept = false;
    m_factory = nullptr;

    q->setDefaultToolBar(QString());

    q->setWindowTitle(i18nc("@title:window", "Configure Toolbars"));
    q->setModal(false);

    m_layout = new QVBoxLayout(q);
    m_layout->addWidget(m_widget);

    m_buttonBox = new QDialogButtonBox(q);
    m_buttonBox->setStandardButtons(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::apply());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(m_buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    q->connect(m_buttonBox, &QDialogButtonBox::clicked, q, [this](QAbstractButton *button) {
        slotButtonClicked(button);
    });
    QObject::connect(m_buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);
    m_layout->addWidget(m_buttonBox);

    q->connect(m_widget, &KEditToolBarWidget::enableOk, q, [this](bool state) {
        acceptOK(state);
        enableApply(state);
    });
    enableApply(false);

    q->setMinimumSize(q->sizeHint());
}

void KEditToolBar::setResourceFile(const QString &file, bool global)
{
    d->m_file = file;
    d->m_global = global;
    d->m_widget->load(d->m_file, d->m_global, d->m_defaultToolBar);
}

KEditToolBar::~KEditToolBar()
{
    s_defaultToolBarName()->clear();
}

void KEditToolBar::setDefaultToolBar(const QString &toolBarName)
{
    if (toolBarName.isEmpty()) {
        d->m_defaultToolBar = *s_defaultToolBarName();
    } else {
        d->m_defaultToolBar = toolBarName;
    }
}

void KEditToolBarPrivate::acceptOK(bool b)
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(b);
    m_accept = b;
}

void KEditToolBarPrivate::enableApply(bool b)
{
    m_buttonBox->button(QDialogButtonBox::Apply)->setEnabled(b);
}

void KEditToolBarPrivate::defaultClicked()
{
    if (KMessageBox::warningContinueCancel(
            q,
            i18n("Do you really want to reset all toolbars of this application to their default? The changes will be applied immediately."),
            i18n("Reset Toolbars"),
            KGuiItem(i18n("Reset")))
        != KMessageBox::Continue) {
        return;
    }

    KEditToolBarWidget *oldWidget = m_widget;
    m_widget = nullptr;
    m_accept = false;

    if (m_factory) {
        const auto clients = m_factory->clients();
        for (KXMLGUIClient *client : clients) {
            const QString file = client->localXMLFile();
            if (file.isEmpty()) {
                continue;
            }
            // qDebug(240) << "Deleting local xml file" << file;
            // << "for client" << client << typeid(*client).name();
            if (QFile::exists(file)) {
                if (!QFile::remove(file)) {
                    qCWarning(DEBUG_KXMLGUI) << "Could not delete" << file;
                }
            }
        }

        // Reload the xml files in all clients, now that the local files are gone
        oldWidget->rebuildKXMLGUIClients();

        m_widget = new KEditToolBarWidget(q);
        m_widget->load(m_factory, m_defaultToolBar);
    } else {
        int slash = m_file.lastIndexOf(QLatin1Char('/')) + 1;
        if (slash) {
            m_file.remove(0, slash);
        }
        const QString xml_file = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kxmlgui5/")
            + QCoreApplication::instance()->applicationName() + QLatin1Char('/') + m_file;

        if (QFile::exists(xml_file)) {
            if (!QFile::remove(xml_file)) {
                qCWarning(DEBUG_KXMLGUI) << "Could not delete " << xml_file;
            }
        }

        m_widget = new KEditToolBarWidget(m_collection, q);
        q->setResourceFile(m_file, m_global);
    }

    // Copy the geometry to minimize UI flicker
    m_widget->setGeometry(oldWidget->geometry());
    delete oldWidget;
    m_layout->insertWidget(0, m_widget);

    q->connect(m_widget, &KEditToolBarWidget::enableOk, q, [this](bool state) {
        acceptOK(state);
        enableApply(state);
    });
    enableApply(false);

    Q_EMIT q->newToolBarConfig();
#if KXMLGUI_BUILD_DEPRECATED_SINCE(4, 0)
    Q_EMIT q->newToolbarConfig(); // compat
#endif
}

void KEditToolBarPrivate::slotButtonClicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton type = m_buttonBox->standardButton(button);

    switch (type) {
    case QDialogButtonBox::Ok:
        okClicked();
        break;
    case QDialogButtonBox::Apply:
        applyClicked();
        break;
    case QDialogButtonBox::RestoreDefaults:
        defaultClicked();
        break;
    default:
        break;
    }
}

void KEditToolBarPrivate::okClicked()
{
    if (!m_accept) {
        q->reject();
        return;
    }

    // Do not rebuild GUI and emit the "newToolBarConfig" signal again here if the "Apply"
    // button was already pressed and no further changes were made.
    if (m_buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
        m_widget->save();
        Q_EMIT q->newToolBarConfig();
#if KXMLGUI_BUILD_DEPRECATED_SINCE(4, 0)
        Q_EMIT q->newToolbarConfig(); // compat
#endif
    }
    q->accept();
}

void KEditToolBarPrivate::applyClicked()
{
    (void)m_widget->save();
    enableApply(false);
    Q_EMIT q->newToolBarConfig();
#if KXMLGUI_BUILD_DEPRECATED_SINCE(4, 0)
    Q_EMIT q->newToolbarConfig(); // compat
#endif
}

void KEditToolBar::setGlobalDefaultToolBar(const char *toolbarName)
{
    *s_defaultToolBarName() = QString::fromLatin1(toolbarName);
}

KEditToolBarWidget::KEditToolBarWidget(KActionCollection *collection, QWidget *parent)
    : QWidget(parent)
    , d(new KEditToolBarWidgetPrivate(this, componentName(), collection))
{
    d->setupLayout();
}

KEditToolBarWidget::KEditToolBarWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KEditToolBarWidgetPrivate(this, componentName(), KXMLGUIClient::actionCollection() /*create new one*/))
{
    d->setupLayout();
}

KEditToolBarWidget::~KEditToolBarWidget()
{
    delete d;
}

void KEditToolBarWidget::load(const QString &file, bool global, const QString &defaultToolBar)
{
    d->initOldStyle(file, global, defaultToolBar);
}

void KEditToolBarWidget::load(KXMLGUIFactory *factory, const QString &defaultToolBar)
{
    d->initFromFactory(factory, defaultToolBar);
}

void KEditToolBarWidgetPrivate::initOldStyle(const QString &resourceFile, bool global, const QString &defaultToolBar)
{
    // TODO: make sure we can call this multiple times?
    if (m_loadedOnce) {
        return;
    }

    m_loadedOnce = true;
    // d->m_actionList = collection->actions();

    // handle the merging
    if (global) {
        m_widget->loadStandardsXmlFile(); // ui_standards.rc
    }
    const QString localXML = loadXMLFile(resourceFile);
    m_widget->setXML(localXML, global ? true /*merge*/ : false);

    // first, get all of the necessary info for our local xml
    XmlData local(XmlData::Local, xmlFile(resourceFile), m_collection);
    QDomDocument domDoc;
    domDoc.setContent(localXML);
    local.setDomDocument(domDoc);
    m_xmlFiles.append(local);

    // then, the merged one (ui_standards + local xml)
    XmlData merge(XmlData::Merged, QString(), m_collection);
    merge.setDomDocument(m_widget->domDocument());
    m_xmlFiles.append(merge);

#ifndef NDEBUG
    dump();
#endif

    // now load in our toolbar combo box
    loadToolBarCombo(defaultToolBar);
    m_widget->adjustSize();
    m_widget->setMinimumSize(m_widget->sizeHint());
}

void KEditToolBarWidgetPrivate::initFromFactory(KXMLGUIFactory *factory, const QString &defaultToolBar)
{
    // TODO: make sure we can call this multiple times?
    if (m_loadedOnce) {
        return;
    }

    m_loadedOnce = true;

    m_factory = factory;

    // add all of the client data
    bool first = true;
    const auto clients = factory->clients();
    for (KXMLGUIClient *client : clients) {
        if (client->xmlFile().isEmpty()) {
            continue;
        }

        XmlData::XmlType type = XmlData::Part;
        if (first) {
            type = XmlData::Shell;
            first = false;
            Q_ASSERT(!client->localXMLFile().isEmpty()); // where would we save changes??
        }

        XmlData data(type, client->localXMLFile(), client->actionCollection());
        QDomDocument domDoc = client->domDocument();
        data.setDomDocument(domDoc);
        m_xmlFiles.append(data);

        // d->m_actionList += client->actionCollection()->actions();
    }

#ifndef NDEBUG
    // d->dump();
#endif

    // now load in our toolbar combo box
    loadToolBarCombo(defaultToolBar);
    m_widget->adjustSize();
    m_widget->setMinimumSize(m_widget->sizeHint());

    m_widget->actionCollection()->addAssociatedWidget(m_widget);
    const auto widgetActions = m_widget->actionCollection()->actions();
    for (QAction *action : widgetActions) {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    }
}

void KEditToolBarWidget::save()
{
    // qDebug(240) << "KEditToolBarWidget::save";
    for (const auto &xmlFile : std::as_const(d->m_xmlFiles)) {
        // let's not save non-modified files
        if (!xmlFile.m_isModified) {
            continue;
        }

        // let's also skip (non-existent) merged files
        if (xmlFile.type() == XmlData::Merged) {
            continue;
        }

        // Add noMerge="1" to all the menus since we are saving the merged data
        QDomNodeList menuNodes = xmlFile.domDocument().elementsByTagName(QStringLiteral("Menu"));
        for (int i = 0; i < menuNodes.length(); ++i) {
            QDomNode menuNode = menuNodes.item(i);
            QDomElement menuElement = menuNode.toElement();
            if (menuElement.isNull()) {
                continue;
            }
            menuElement.setAttribute(QStringLiteral("noMerge"), QStringLiteral("1"));
        }

        // qCDebug(DEBUG_KXMLGUI) << (*it).domDocument().toString();

        // qDebug(240) << "Saving " << (*it).xmlFile();
        // if we got this far, we might as well just save it
        KXMLGUIFactory::saveConfigFile(xmlFile.domDocument(), xmlFile.xmlFile());
    }

    if (!d->m_factory) {
        return;
    }

    rebuildKXMLGUIClients();
}

void KEditToolBarWidget::rebuildKXMLGUIClients()
{
    if (!d->m_factory) {
        return;
    }

    const QList<KXMLGUIClient *> clients = d->m_factory->clients();
    // qDebug() << "factory: " << clients.count() << " clients";

    if (clients.isEmpty()) {
        return;
    }

    // remove the elements starting from the last going to the first
    for (auto it = clients.crbegin(); it != clients.crend(); ++it) {
        // qDebug() << "factory->removeClient " << client;
        d->m_factory->removeClient(*it);
    }

    KXMLGUIClient *firstClient = clients.first();

    // now, rebuild the gui from the first to the last
    // qDebug(240) << "rebuilding the gui";
    for (KXMLGUIClient *client : clients) {
        // qDebug(240) << "updating client " << client << " " << client->componentName() << "  xmlFile=" << client->xmlFile();
        QString file(client->xmlFile()); // before setting ui_standards!
        if (!file.isEmpty()) {
            // passing an empty stream forces the clients to reread the XML
            client->setXMLGUIBuildDocument(QDomDocument());

            // for the shell, merge in ui_standards.rc
            if (client == firstClient) { // same assumption as in the ctor: first==shell
                client->loadStandardsXmlFile();
            }

            // and this forces it to use the *new* XML file
            client->setXMLFile(file, client == firstClient /* merge if shell */);

            // [we can't use reloadXML, it doesn't load ui_standards.rc]
        }
    }

    // Now we can add the clients to the factory
    // We don't do it in the loop above because adding a part automatically
    // adds its plugins, so we must make sure the plugins were updated first.
    for (KXMLGUIClient *client : clients) {
        d->m_factory->addClient(client);
    }
}

void KEditToolBarWidgetPrivate::setupLayout()
{
    // the toolbar name combo
    m_comboLabel = new QLabel(i18n("&Toolbar:"), m_widget);
    m_toolbarCombo = new QComboBox(m_widget);
    m_comboLabel->setBuddy(m_toolbarCombo);
    m_comboSeparator = new KSeparator(m_widget);
    QObject::connect(m_toolbarCombo, qOverload<int>(&QComboBox::activated), m_widget, [this](int index) {
        slotToolBarSelected(index);
    });

    //  QPushButton *new_toolbar = new QPushButton(i18n("&New"), this);
    //  new_toolbar->setPixmap(BarIcon("document-new", KIconLoader::SizeSmall));
    //  new_toolbar->setEnabled(false); // disabled until implemented
    //  QPushButton *del_toolbar = new QPushButton(i18n("&Delete"), this);
    //  del_toolbar->setPixmap(BarIcon("edit-delete", KIconLoader::SizeSmall));
    //  del_toolbar->setEnabled(false); // disabled until implemented

    // our list of inactive actions
    QLabel *inactive_label = new QLabel(i18n("A&vailable actions:"), m_widget);
    m_inactiveList = new ToolBarListWidget(m_widget);
    m_inactiveList->setDragEnabled(true);
    m_inactiveList->setActiveList(false);
    m_inactiveList->setMinimumSize(180, 200);
    m_inactiveList->setDropIndicatorShown(false); // #165663
    inactive_label->setBuddy(m_inactiveList);
    QObject::connect(m_inactiveList, &QListWidget::itemSelectionChanged, m_widget, [this]() {
        slotInactiveSelectionChanged();
    });
    QObject::connect(m_inactiveList, &QListWidget::itemDoubleClicked, m_widget, [this]() {
        slotInsertButton();
    });
    QObject::connect(m_inactiveList,
                     &ToolBarListWidget::dropped,
                     m_widget,
                     [this](ToolBarListWidget *list, int index, ToolBarItem *item, bool sourceIsActiveList) {
                         slotDropped(list, index, item, sourceIsActiveList);
                     });

    KListWidgetSearchLine *inactiveListSearchLine = new KListWidgetSearchLine(m_widget, m_inactiveList);
    inactiveListSearchLine->setPlaceholderText(i18n("Filter"));

    // our list of active actions
    QLabel *active_label = new QLabel(i18n("Curr&ent actions:"), m_widget);
    m_activeList = new ToolBarListWidget(m_widget);
    m_activeList->setDragEnabled(true);
    m_activeList->setActiveList(true);
    // With Qt-4.1 only setting MiniumWidth results in a 0-width icon column ...
    m_activeList->setMinimumSize(m_inactiveList->minimumWidth(), 100);
    active_label->setBuddy(m_activeList);

    QObject::connect(m_activeList, &QListWidget::itemSelectionChanged, m_widget, [this]() {
        slotActiveSelectionChanged();
    });
    QObject::connect(m_activeList, &QListWidget::itemDoubleClicked, m_widget, [this]() {
        slotRemoveButton();
    });
    QObject::connect(m_activeList,
                     &ToolBarListWidget::dropped,
                     m_widget,
                     [this](ToolBarListWidget *list, int index, ToolBarItem *item, bool sourceIsActiveList) {
                         slotDropped(list, index, item, sourceIsActiveList);
                     });

    KListWidgetSearchLine *activeListSearchLine = new KListWidgetSearchLine(m_widget, m_activeList);
    activeListSearchLine->setPlaceholderText(i18n("Filter"));

    // "change icon" button
    m_changeIcon = new QPushButton(i18nc("@action:button", "Change &Icon..."), m_widget);
    m_changeIcon->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-icons")));
    m_changeIcon->setEnabled(m_activeList->currentItem());

    QObject::connect(m_changeIcon, &QPushButton::clicked, m_widget, [this]() {
        slotChangeIcon();
    });

    // "change icon text" button
    m_changeIconText = new QPushButton(i18nc("@action:button", "Change Te&xt..."), m_widget);
    m_changeIconText->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    m_changeIconText->setEnabled(m_activeList->currentItem() != nullptr);

    QObject::connect(m_changeIconText, &QPushButton::clicked, m_widget, [this]() {
        slotChangeIconText();
    });

    // The buttons in the middle

    m_upAction = new QToolButton(m_widget);
    m_upAction->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    m_upAction->setEnabled(false);
    m_upAction->setAutoRepeat(true);
    QObject::connect(m_upAction, &QToolButton::clicked, m_widget, [this]() {
        slotUpButton();
    });

    m_insertAction = new QToolButton(m_widget);
    m_insertAction->setIcon(QIcon::fromTheme(QApplication::isRightToLeft() ? QStringLiteral("go-previous") : QStringLiteral("go-next")));
    m_insertAction->setEnabled(false);
    QObject::connect(m_insertAction, &QToolButton::clicked, m_widget, [this]() {
        slotInsertButton();
    });

    m_removeAction = new QToolButton(m_widget);
    m_removeAction->setIcon(QIcon::fromTheme(QApplication::isRightToLeft() ? QStringLiteral("go-next") : QStringLiteral("go-previous")));
    m_removeAction->setEnabled(false);
    QObject::connect(m_removeAction, &QToolButton::clicked, m_widget, [this]() {
        slotRemoveButton();
    });

    m_downAction = new QToolButton(m_widget);
    m_downAction->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    m_downAction->setEnabled(false);
    m_downAction->setAutoRepeat(true);
    QObject::connect(m_downAction, &QToolButton::clicked, m_widget, [this]() {
        slotDownButton();
    });

    m_helpArea = new QLabel(m_widget);
    m_helpArea->setWordWrap(true);

    // now start with our layouts
    QVBoxLayout *top_layout = new QVBoxLayout(m_widget);
    top_layout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *name_layout = new QVBoxLayout();
    QHBoxLayout *list_layout = new QHBoxLayout();

    QVBoxLayout *inactive_layout = new QVBoxLayout();
    QVBoxLayout *active_layout = new QVBoxLayout();
    QHBoxLayout *changeIcon_layout = new QHBoxLayout();

    QGridLayout *button_layout = new QGridLayout();

    name_layout->addWidget(m_comboLabel);
    name_layout->addWidget(m_toolbarCombo);
    //  name_layout->addWidget(new_toolbar);
    //  name_layout->addWidget(del_toolbar);

    button_layout->setSpacing(0);
    button_layout->setRowStretch(0, 10);
    button_layout->addWidget(m_upAction, 1, 1);
    button_layout->addWidget(m_removeAction, 2, 0);
    button_layout->addWidget(m_insertAction, 2, 2);
    button_layout->addWidget(m_downAction, 3, 1);
    button_layout->setRowStretch(4, 10);

    inactive_layout->addWidget(inactive_label);
    inactive_layout->addWidget(inactiveListSearchLine);
    inactive_layout->addWidget(m_inactiveList, 1);

    active_layout->addWidget(active_label);
    active_layout->addWidget(activeListSearchLine);
    active_layout->addWidget(m_activeList, 1);
    active_layout->addLayout(changeIcon_layout);

    changeIcon_layout->addWidget(m_changeIcon);
    changeIcon_layout->addStretch(1);
    changeIcon_layout->addWidget(m_changeIconText);

    list_layout->addLayout(inactive_layout);
    list_layout->addLayout(button_layout);
    list_layout->addLayout(active_layout);

    top_layout->addLayout(name_layout);
    top_layout->addWidget(m_comboSeparator);
    top_layout->addLayout(list_layout, 10);
    top_layout->addWidget(m_helpArea);
    top_layout->addWidget(new KSeparator(m_widget));
}

void KEditToolBarWidgetPrivate::loadToolBarCombo(const QString &defaultToolBar)
{
    const QLatin1String attrName("name");
    // just in case, we clear our combo
    m_toolbarCombo->clear();

    int defaultToolBarId = -1;
    int count = 0;
    // load in all of the toolbar names into this combo box
    for (const auto &xmlFile : std::as_const(m_xmlFiles)) {
        // skip the merged one in favor of the local one,
        // so that we can change icons
        // This also makes the app-defined named for "mainToolBar" appear rather than the ui_standards-defined name.
        if (xmlFile.type() == XmlData::Merged) {
            continue;
        }

        // each xml file may have any number of toolbars
        for (const auto &bar : std::as_const(xmlFile.barList())) {
            const QString text = xmlFile.toolBarText(bar);
            m_toolbarCombo->addItem(text);
            const QString name = bar.attribute(attrName);
            if (defaultToolBarId == -1 && name == defaultToolBar) {
                defaultToolBarId = count;
            }
            count++;
        }
    }
    const bool showCombo = (count > 1);
    m_comboLabel->setVisible(showCombo);
    m_comboSeparator->setVisible(showCombo);
    m_toolbarCombo->setVisible(showCombo);
    if (defaultToolBarId == -1) {
        defaultToolBarId = 0;
    }
    // we want to the specified item selected and its actions loaded
    m_toolbarCombo->setCurrentIndex(defaultToolBarId);
    slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBarWidgetPrivate::loadActions(const QDomElement &elem)
{
    const QLatin1String tagSeparator("Separator");
    const QLatin1String tagSpacer("Spacer");
    const QLatin1String tagMerge("Merge");
    const QLatin1String tagActionList("ActionList");
    const QLatin1String tagAction("Action");
    const QLatin1String attrName("name");

    const QString separatorstring = i18n("--- separator ---");
    const QString spacerstring = i18n("--- expanding spacer ---");

    int sep_num = 0;
    QString sep_name(QStringLiteral("separator_%1"));
    int spacer_num = 0;
    QString spacer_name(QStringLiteral("spacer_%1"));

    // clear our lists
    m_inactiveList->clear();
    m_activeList->clear();
    m_insertAction->setEnabled(false);
    m_removeAction->setEnabled(false);
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);

    // We'll use this action collection
    KActionCollection *actionCollection = m_currentXmlData->actionCollection();

    // store the names of our active actions
    QSet<QString> active_list;

    // Filtering message requested by translators (scripting).
    KLocalizedString nameFilter = ki18nc("@item:intable Action name in toolbar editor", "%1");

    // see if our current action is in this toolbar
    QDomNode n = elem.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        QDomElement it = n.toElement();
        if (it.isNull()) {
            continue;
        }
        if (it.tagName() == tagSeparator) {
            ToolBarItem *act = new ToolBarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), QString());
            act->setSeparator(true);
            act->setText(separatorstring);
            it.setAttribute(attrName, act->internalName());
            continue;
        }
        if (it.tagName() == tagSpacer) {
            ToolBarItem *act = new ToolBarItem(m_activeList, tagSpacer, spacer_name.arg(spacer_num++), QString());
            act->setSpacer(true);
            act->setText(spacerstring);
            it.setAttribute(attrName, act->internalName());
            continue;
        }

        if (it.tagName() == tagMerge) {
            // Merge can be named or not - use the name if there is one
            QString name = it.attribute(attrName);
            ToolBarItem *act =
                new ToolBarItem(m_activeList, tagMerge, name, i18n("This element will be replaced with all the elements of an embedded component."));
            if (name.isEmpty()) {
                act->setText(i18n("<Merge>"));
            } else {
                act->setText(i18n("<Merge %1>", name));
            }
            continue;
        }

        if (it.tagName() == tagActionList) {
            ToolBarItem *act =
                new ToolBarItem(m_activeList,
                                tagActionList,
                                it.attribute(attrName),
                                i18n("This is a dynamic list of actions. You can move it, but if you remove it you will not be able to re-add it."));
            act->setText(i18n("ActionList: %1", it.attribute(attrName)));
            continue;
        }

        // iterate through this client's actions
        // This used to iterate through _all_ actions, but we don't support
        // putting any action into any client...
        const auto actions = actionCollection->actions();
        for (QAction *action : actions) {
            // do we have a match?
            if (it.attribute(attrName) == action->objectName()) {
                // we have a match!
                ToolBarItem *act = new ToolBarItem(m_activeList, it.tagName(), action->objectName(), action->toolTip());
                act->setText(nameFilter.subs(KLocalizedString::removeAcceleratorMarker(action->iconText())).toString());
                act->setIcon(!action->icon().isNull() ? action->icon() : m_emptyIcon);
                act->setTextAlongsideIconHidden(action->priority() < QAction::NormalPriority);

                active_list.insert(action->objectName());
                break;
            }
        }
    }

    // go through the rest of the collection
    const auto actions = actionCollection->actions();
    for (QAction *action : actions) {
        // skip our active ones
        if (active_list.contains(action->objectName())) {
            continue;
        }

        ToolBarItem *act = new ToolBarItem(m_inactiveList, tagAction, action->objectName(), action->toolTip());
        act->setText(nameFilter.subs(KLocalizedString::removeAcceleratorMarker(action->text())).toString());
        act->setIcon(!action->icon().isNull() ? action->icon() : m_emptyIcon);
    }

    m_inactiveList->sortItems(Qt::AscendingOrder);

    // finally, add default separators and spacers to the inactive list
    ToolBarItem *sep = new ToolBarItem(nullptr, tagSeparator, sep_name.arg(sep_num++), QString());
    sep->setSeparator(true);
    sep->setText(separatorstring);
    m_inactiveList->insertItem(0, sep);

    ToolBarItem *spacer = new ToolBarItem(nullptr, tagSpacer, spacer_name.arg(spacer_num++), QString());
    spacer->setSpacer(true);
    spacer->setText(spacerstring);
    m_inactiveList->insertItem(1, spacer);
}

KActionCollection *KEditToolBarWidget::actionCollection() const
{
    return d->m_collection;
}

void KEditToolBarWidgetPrivate::slotToolBarSelected(int index)
{
    // We need to find the XmlData and toolbar element for this index
    // To do that, we do the same iteration as the one which filled in the combobox.

    int toolbarNumber = 0;
    for (auto &xmlFile : m_xmlFiles) {
        // skip the merged one in favor of the local one,
        // so that we can change icons
        if (xmlFile.type() == XmlData::Merged) {
            continue;
        }

        // each xml file may have any number of toolbars
        const auto &barList = xmlFile.barList();
        for (const auto &bar : barList) {
            // is this our toolbar?
            if (toolbarNumber == index) {
                // save our current settings
                m_currentXmlData = &xmlFile;
                m_currentToolBarElem = bar;

                // qCDebug(DEBUG_KXMLGUI) << "found toolbar" << m_currentXmlData->toolBarText(*it) << "m_currentXmlData set to";
                m_currentXmlData->dump();

                // If this is a Merged xmldata, clicking the "change icon" button would assert...
                Q_ASSERT(m_currentXmlData->type() != XmlData::Merged);

                // load in our values
                loadActions(m_currentToolBarElem);

                if (xmlFile.type() == XmlData::Part || xmlFile.type() == XmlData::Shell) {
                    m_widget->setDOMDocument(xmlFile.domDocument());
                }
                return;
            }
            ++toolbarNumber;
        }
    }
}

void KEditToolBarWidgetPrivate::slotInactiveSelectionChanged()
{
    if (!m_inactiveList->selectedItems().isEmpty()) {
        m_insertAction->setEnabled(true);
        QString statusText = static_cast<ToolBarItem *>(m_inactiveList->selectedItems().first())->statusText();
        m_helpArea->setText(i18nc("@label Action tooltip in toolbar editor, below the action list", "%1", statusText));
    } else {
        m_insertAction->setEnabled(false);
        m_helpArea->setText(QString());
    }
}

void KEditToolBarWidgetPrivate::slotActiveSelectionChanged()
{
    ToolBarItem *toolitem = nullptr;
    if (!m_activeList->selectedItems().isEmpty()) {
        toolitem = static_cast<ToolBarItem *>(m_activeList->selectedItems().first());
    }

    m_removeAction->setEnabled(toolitem);

    m_changeIcon->setEnabled(toolitem && toolitem->internalTag() == QLatin1String("Action"));

    m_changeIconText->setEnabled(toolitem && toolitem->internalTag() == QLatin1String("Action"));

    if (toolitem) {
        m_upAction->setEnabled(toolitem->index() != 0);
        m_downAction->setEnabled(toolitem->index() != toolitem->listWidget()->count() - 1);

        QString statusText = toolitem->statusText();
        m_helpArea->setText(i18nc("@label Action tooltip in toolbar editor, below the action list", "%1", statusText));
    } else {
        m_upAction->setEnabled(false);
        m_downAction->setEnabled(false);
        m_helpArea->setText(QString());
    }
}

void KEditToolBarWidgetPrivate::slotInsertButton()
{
    QString internalName = static_cast<ToolBarItem *>(m_inactiveList->currentItem())->internalName();

    insertActive(m_inactiveList->currentItem(), m_activeList->currentItem(), false);
    // we're modified, so let this change
    Q_EMIT m_widget->enableOk(true);

    slotToolBarSelected(m_toolbarCombo->currentIndex());

    selectActiveItem(internalName);
}

void KEditToolBarWidgetPrivate::selectActiveItem(const QString &internalName)
{
    int activeItemCount = m_activeList->count();
    for (int i = 0; i < activeItemCount; i++) {
        ToolBarItem *item = static_cast<ToolBarItem *>(m_activeList->item(i));
        if (item->internalName() == internalName) {
            m_activeList->setCurrentItem(item);
            break;
        }
    }
}

void KEditToolBarWidgetPrivate::slotRemoveButton()
{
    removeActive(m_activeList->currentItem());

    slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBarWidgetPrivate::insertActive(ToolBarItem *item, ToolBarItem *before, bool prepend)
{
    if (!item) {
        return;
    }

    QDomElement new_item;
    // let's handle the separator and spacer specially
    if (item->isSeparator()) {
        new_item = m_widget->domDocument().createElement(QStringLiteral("Separator"));
    } else if (item->isSpacer()) {
        new_item = m_widget->domDocument().createElement(QStringLiteral("Spacer"));
    } else {
        new_item = m_widget->domDocument().createElement(QStringLiteral("Action"));
    }

    new_item.setAttribute(QStringLiteral("name"), item->internalName());

    Q_ASSERT(!m_currentToolBarElem.isNull());

    if (before) {
        // we have the item in the active list which is before the new
        // item.. so let's try our best to add our new item right after it
        QDomElement elem = findElementForToolBarItem(before);
        Q_ASSERT(!elem.isNull());
        m_currentToolBarElem.insertAfter(new_item, elem);
    } else {
        // simply put it at the beginning or the end of the list.
        if (prepend) {
            m_currentToolBarElem.insertBefore(new_item, m_currentToolBarElem.firstChild());
        } else {
            m_currentToolBarElem.appendChild(new_item);
        }
    }

    // and set this container as a noMerge
    m_currentToolBarElem.setAttribute(QStringLiteral("noMerge"), QStringLiteral("1"));

    // update the local doc
    updateLocal(m_currentToolBarElem);
}

void KEditToolBarWidgetPrivate::removeActive(ToolBarItem *item)
{
    if (!item) {
        return;
    }

    // we're modified, so let this change
    Q_EMIT m_widget->enableOk(true);

    // now iterate through to find the child to nuke
    QDomElement elem = findElementForToolBarItem(item);
    if (!elem.isNull()) {
        // nuke myself!
        m_currentToolBarElem.removeChild(elem);

        // and set this container as a noMerge
        m_currentToolBarElem.setAttribute(QStringLiteral("noMerge"), QStringLiteral("1"));

        // update the local doc
        updateLocal(m_currentToolBarElem);
    }
}

void KEditToolBarWidgetPrivate::slotUpButton()
{
    ToolBarItem *item = m_activeList->currentItem();

    if (!item) {
        Q_ASSERT(false);
        return;
    }

    int row = item->listWidget()->row(item) - 1;
    // make sure we're not the top item already
    if (row < 0) {
        Q_ASSERT(false);
        return;
    }

    // we're modified, so let this change
    Q_EMIT m_widget->enableOk(true);

    moveActive(item, static_cast<ToolBarItem *>(item->listWidget()->item(row - 1)));
}

void KEditToolBarWidgetPrivate::moveActive(ToolBarItem *item, ToolBarItem *before)
{
    QDomElement e = findElementForToolBarItem(item);

    if (e.isNull()) {
        return;
    }

    // remove item
    m_activeList->takeItem(m_activeList->row(item));

    // put it where it's supposed to go
    m_activeList->insertItem(m_activeList->row(before) + 1, item);

    // make it selected again
    m_activeList->setCurrentItem(item);

    // and do the real move in the DOM
    if (!before) {
        m_currentToolBarElem.insertBefore(e, m_currentToolBarElem.firstChild());
    } else {
        m_currentToolBarElem.insertAfter(e, findElementForToolBarItem(before));
    }

    // and set this container as a noMerge
    m_currentToolBarElem.setAttribute(QStringLiteral("noMerge"), QStringLiteral("1"));

    // update the local doc
    updateLocal(m_currentToolBarElem);
}

void KEditToolBarWidgetPrivate::slotDownButton()
{
    ToolBarItem *item = m_activeList->currentItem();

    if (!item) {
        Q_ASSERT(false);
        return;
    }

    // make sure we're not the bottom item already
    int newRow = item->listWidget()->row(item) + 1;
    if (newRow >= item->listWidget()->count()) {
        Q_ASSERT(false);
        return;
    }

    // we're modified, so let this change
    Q_EMIT m_widget->enableOk(true);

    moveActive(item, static_cast<ToolBarItem *>(item->listWidget()->item(newRow)));
}

void KEditToolBarWidgetPrivate::updateLocal(QDomElement &elem)
{
    for (auto &xmlFile : m_xmlFiles) {
        if (xmlFile.type() == XmlData::Merged) {
            continue;
        }

        if (xmlFile.type() == XmlData::Shell || xmlFile.type() == XmlData::Part) {
            if (m_currentXmlData->xmlFile() == xmlFile.xmlFile()) {
                xmlFile.m_isModified = true;
                return;
            }

            continue;
        }

        xmlFile.m_isModified = true;
        const QLatin1String attrName("name");
        for (const auto &bar : std::as_const(xmlFile.barList())) {
            const QString name(bar.attribute(attrName));
            const QString tag(bar.tagName());
            if ((tag != elem.tagName()) || (name != elem.attribute(attrName))) {
                continue;
            }

            QDomElement toolbar = xmlFile.domDocument().documentElement().toElement();
            toolbar.replaceChild(elem, bar);
            return;
        }

        // just append it
        QDomElement toolbar = xmlFile.domDocument().documentElement().toElement();
        Q_ASSERT(!toolbar.isNull());
        toolbar.appendChild(elem);
    }
}

void KEditToolBarWidgetPrivate::slotChangeIcon()
{
    m_currentXmlData->dump();
    Q_ASSERT(m_currentXmlData->type() != XmlData::Merged);

    QString icon = KIconDialog::getIcon(KIconLoader::Toolbar,
                                        KIconLoader::Action,
                                        false,
                                        0,
                                        false, // all defaults
                                        m_widget,
                                        i18n("Change Icon"));

    if (icon.isEmpty()) {
        return;
    }

    ToolBarItem *item = m_activeList->currentItem();
    // qCDebug(DEBUG_KXMLGUI) << item;
    if (item) {
        item->setIcon(QIcon::fromTheme(icon));

        m_currentXmlData->m_isModified = true;

        // Get hold of ActionProperties tag
        QDomElement elem = KXMLGUIFactory::actionPropertiesElement(m_currentXmlData->domDocument());
        // Find or create an element for this action
        QDomElement act_elem = KXMLGUIFactory::findActionByName(elem, item->internalName(), true /*create*/);
        Q_ASSERT(!act_elem.isNull());
        act_elem.setAttribute(QStringLiteral("icon"), icon);

        // we're modified, so let this change
        Q_EMIT m_widget->enableOk(true);
    }
}

void KEditToolBarWidgetPrivate::slotChangeIconText()
{
    m_currentXmlData->dump();
    ToolBarItem *item = m_activeList->currentItem();

    if (item) {
        QString iconText = item->text();
        bool hidden = item->isTextAlongsideIconHidden();

        IconTextEditDialog dialog(m_widget);
        dialog.setIconText(iconText);
        dialog.setTextAlongsideIconHidden(hidden);

        bool ok = dialog.exec() == QDialog::Accepted;
        iconText = dialog.iconText();
        hidden = dialog.textAlongsideIconHidden();

        bool hiddenChanged = hidden != item->isTextAlongsideIconHidden();
        bool iconTextChanged = iconText != item->text();

        if (!ok || (!hiddenChanged && !iconTextChanged)) {
            return;
        }

        item->setText(iconText);
        item->setTextAlongsideIconHidden(hidden);

        Q_ASSERT(m_currentXmlData->type() != XmlData::Merged);

        m_currentXmlData->m_isModified = true;

        // Get hold of ActionProperties tag
        QDomElement elem = KXMLGUIFactory::actionPropertiesElement(m_currentXmlData->domDocument());
        // Find or create an element for this action
        QDomElement act_elem = KXMLGUIFactory::findActionByName(elem, item->internalName(), true /*create*/);
        Q_ASSERT(!act_elem.isNull());
        if (iconTextChanged) {
            act_elem.setAttribute(QStringLiteral("iconText"), iconText);
        }
        if (hiddenChanged) {
            act_elem.setAttribute(QStringLiteral("priority"), hidden ? QAction::LowPriority : QAction::NormalPriority);
        }

        // we're modified, so let this change
        Q_EMIT m_widget->enableOk(true);
    }
}

void KEditToolBarWidgetPrivate::slotDropped(ToolBarListWidget *list, int index, ToolBarItem *item, bool sourceIsActiveList)
{
    // qCDebug(DEBUG_KXMLGUI) << "slotDropped list=" << (list==m_activeList?"activeList":"inactiveList")
    //         << "index=" << index << "sourceIsActiveList=" << sourceIsActiveList;
    if (list == m_activeList) {
        ToolBarItem *after = index > 0 ? static_cast<ToolBarItem *>(list->item(index - 1)) : nullptr;
        // qCDebug(DEBUG_KXMLGUI) << "after" << after->text() << after->internalTag();
        if (sourceIsActiveList) {
            // has been dragged within the active list (moved).
            moveActive(item, after);
        } else {
            // dragged from the inactive list to the active list
            insertActive(item, after, true);
        }
    } else if (list == m_inactiveList) {
        // has been dragged to the inactive list -> remove from the active list.
        removeActive(item);
    }

    delete item; // not needed anymore. must be deleted before slotToolBarSelected clears the lists

    // we're modified, so let this change
    Q_EMIT m_widget->enableOk(true);

    slotToolBarSelected(m_toolbarCombo->currentIndex());
}

void KEditToolBar::showEvent(QShowEvent *event)
{
    if (!event->spontaneous()) {
        // The dialog has been shown, enable toolbar editing
        if (d->m_factory) {
            // call the xmlgui-factory version
            d->m_widget->load(d->m_factory, d->m_defaultToolBar);
        } else {
            // call the action collection version
            d->m_widget->load(d->m_file, d->m_global, d->m_defaultToolBar);
        }

        KToolBar::setToolBarsEditable(true);
    }
    QDialog::showEvent(event);
}

void KEditToolBar::hideEvent(QHideEvent *event)
{
    // The dialog has been hidden, disable toolbar editing
    KToolBar::setToolBarsEditable(false);

    QDialog::hideEvent(event);
}

#include "moc_kedittoolbar.cpp"
#include "moc_kedittoolbar_p.cpp"
