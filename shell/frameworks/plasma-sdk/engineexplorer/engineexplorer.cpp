/*
 *   SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "engineexplorer.h"

#include <QApplication>
#include <QBitArray>
#include <QBitmap>
#include <QDialogButtonBox>
#include <QMenu>
#include <QStandardItemModel>
#include <QUrl>

#include <KLocalizedString>
#include <KPluginMetaData>
#include <KStandardAction>
#include <KStringHandler>
#include <QAction>
#include <QDateTime>

#include <Plasma5Support/PluginLoader>

Q_DECLARE_METATYPE(Plasma5Support::DataEngine::Data)

#include "modelviewer.h"
#include "serviceviewer.h"
#include "titlecombobox.h"

enum {
    ColumnDataSourceAndKey = 0,
    ColumnType = 1,
    ColumnValue = 2,

    ColumnCount,
};

EngineExplorer::EngineExplorer(QWidget *parent)
    : QDialog(parent)
    , m_engine(nullptr)
    , m_sourceCount(0)
    , m_requestingSource(false)
    , m_expandButton(new QPushButton(i18n("Expand All"), this))
    , m_collapseButton(new QPushButton(i18n("Collapse All"), this))
{
    setWindowTitle(i18n("Plasma Engine Explorer"));
    QWidget *mainWidget = new QWidget(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_expandButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_collapseButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Close);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mainWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setupUi(mainWidget);

    m_dataModel = new QStandardItemModel(this);
    const int size = m_title->style()->pixelMetric(QStyle::PM_LargeIconSize);
    m_title->setIconSize(QSize(size, size));
    m_title->setIcon(QIcon::fromTheme("plasma"));
    connect(m_engines, &QComboBox::textActivated, this, &EngineExplorer::showEngine);
    connect(m_sourceRequesterButton, SIGNAL(clicked(bool)), this, SLOT(requestSource()));
    connect(m_serviceRequesterButton, &QAbstractButton::clicked, this, &EngineExplorer::requestServiceForSource);
    m_data->setModel(m_dataModel);
    m_data->setWordWrap(true);

    m_searchLine->setTreeView(m_data);
    m_searchLine->setPlaceholderText(i18n("Search"));

    listEngines();
    m_engines->setFocus();

    connect(m_collapseButton, &QAbstractButton::clicked, m_data, &QTreeView::collapseAll);
    connect(m_expandButton, &QAbstractButton::clicked, m_data, &QTreeView::expandAll);
    enableButtons(false);

    addAction(KStandardAction::quit(qApp, SLOT(quit()), this));

    connect(m_data, &QWidget::customContextMenuRequested, this, &EngineExplorer::showDataContextMenu);
    m_data->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &EngineExplorer::cleanUp);
}

EngineExplorer::~EngineExplorer()
{
}

void EngineExplorer::cleanUp()
{
}

void EngineExplorer::setApp(const QString &app)
{
    m_app = app;

    if (m_engines->count() > 0) {
        listEngines();
    }
}

void EngineExplorer::setEngine(const QString &engine)
{
    // find the engine in the combo box
    const int index = m_engines->findText(engine);
    if (index != -1) {
        qDebug() << QString("Engine %1 found!").arg(engine);
        m_engines->setCurrentIndex(index);
        showEngine(engine);
    }
}

void EngineExplorer::setInterval(const int interval)
{
    m_updateInterval->setValue(interval);
}

void EngineExplorer::removeExtraRows(QStandardItem *parent, int preserve)
{
    if (parent->rowCount() > preserve) {
        parent->removeRows(preserve, parent->rowCount() - preserve);
    }
}

void EngineExplorer::dataUpdated(const QString &source, const Plasma5Support::DataEngine::Data &data)
{
    QList<QStandardItem *> items = m_dataModel->findItems(source, Qt::MatchExactly);

    if (items.isEmpty()) {
        return;
    }

    QStandardItem *parent = items.first();

    int rows = showData(parent, data);
    removeExtraRows(parent, rows);
}

void EngineExplorer::listEngines()
{
    m_engines->clear();
    QVector<KPluginMetaData> engines = Plasma5Support::PluginLoader::listDataEngineMetaData(m_app);
    std::sort(engines.begin(), engines.end(), [](auto lhs, auto rhs) {
        if (lhs.category() < rhs.category()) {
            return true;
        }
        if (lhs.category() == rhs.category()) {
            return lhs.name() < rhs.name();
        }
        return false;
    });

    for (const KPluginMetaData &engine : std::as_const(engines)) {
        m_engines->addItem(QIcon::fromTheme(engine.iconName()), engine.pluginId());
    }

    m_engines->setCurrentIndex(-1);
}

void EngineExplorer::showEngine(const QString &name)
{
    m_sourceRequester->setEnabled(false);
    m_sourceRequesterButton->setEnabled(false);
    m_serviceRequester->setEnabled(false);
    m_serviceRequesterButton->setEnabled(false);
    enableButtons(false);
    m_dataModel->clear();
    m_dataModel->setColumnCount(ColumnCount);
    QStringList headers{i18n("DataSource/Key"), i18n("Type"), i18n("Value")};
    m_dataModel->setHorizontalHeaderLabels(headers);
    m_data->setColumnWidth(ColumnDataSourceAndKey, 200);
    m_engine = nullptr;
    m_sourceCount = 0;

    m_engineName = name;
    if (m_engineName.isEmpty()) {
        updateTitle();
        return;
    }

    if (auto res = KPluginFactory::instantiatePlugin<Plasma5Support::DataEngine>(
            KPluginMetaData::findPluginById(QStringLiteral("plasma5support/dataengine"), m_engineName))) {
        m_engine = res.plugin;
    } else {
        m_engineName.clear();
        updateTitle();
        return;
    }

    // qDebug() << "showing engine " << m_engine->objectName();
    // qDebug() << "we have " << sources.count() << " data sources";
    connect(m_engine, &Plasma5Support::DataEngine::sourceAdded, this, &EngineExplorer::addSource);
    connect(m_engine, &Plasma5Support::DataEngine::sourceRemoved, this, &EngineExplorer::removeSource);
    const QStringList &sources = m_engine->sources();
    for (const QString &source : sources) {
        // qDebug() << "adding " << source;
        addSource(source);
    }

    m_sourceRequesterButton->setEnabled(true);
    m_updateInterval->setEnabled(true);
    m_sourceRequester->setEnabled(true);
    m_sourceRequester->setFocus();
    m_serviceRequester->setEnabled(true);
    m_serviceRequesterButton->setEnabled(true);
    updateTitle();
}

void EngineExplorer::addSource(const QString &source)
{
    // qDebug() << "adding" << source;
    QList<QStandardItem *> items = m_dataModel->findItems(source, Qt::MatchExactly);
    if (!items.isEmpty()) {
        // qDebug() << "er... already there?";
        return;
    }

    QStandardItem *parent = new QStandardItem(source);
    parent->setToolTip(source);
    m_dataModel->appendRow(parent);

    // qDebug() << "getting data for source " << source;
    if (!m_requestingSource || m_sourceRequester->text() != source) {
        // qDebug() << "connecting up now";
        m_engine->connectSource(source, this);
    }

    ++m_sourceCount;
    updateTitle();

    enableButtons(true);
}

void EngineExplorer::removeSource(const QString &source)
{
    const QList<QStandardItem *> items = m_dataModel->findItems(source, Qt::MatchExactly);

    if (items.count() < 1) {
        return;
    }

    for (QStandardItem *item : items) {
        m_dataModel->removeRow(item->row());
    }

    --m_sourceCount;
    m_engine->disconnectSource(source, this);
    updateTitle();
}

void EngineExplorer::requestSource()
{
    requestSource(m_sourceRequester->text());
}

void EngineExplorer::requestServiceForSource()
{
    ServiceViewer *viewer = new ServiceViewer(m_engine, m_serviceRequester->text());
    viewer->show();
}

void EngineExplorer::requestSource(const QString &source)
{
    if (!m_engine || source.isEmpty()) {
        return;
    }

    qDebug() << "request source" << source;
    m_requestingSource = true;
    m_engine->connectSource(source, this, (uint)m_updateInterval->value());
    m_requestingSource = false;
}

void EngineExplorer::showDataContextMenu(const QPoint &point)
{
    QModelIndex index = m_data->indexAt(point);
    if (index.isValid()) {
        while (index.parent().isValid()) {
            index = index.parent();
        }

        if (index.column() != 0) {
            index = m_dataModel->index(index.row(), 0);
        }

        const QString source = index.data().toString();
        QMenu menu;
        menu.addSection(source);
        QAction *service = menu.addAction(i18n("Get associated service"));
        QAction *model = menu.addAction(i18n("Get associated model"));
        QAction *update = menu.addAction(i18n("Update source now"));
        QAction *remove = menu.addAction(i18n("Remove source"));

        QAction *activated = menu.exec(m_data->viewport()->mapToGlobal(point));
        if (activated == service) {
            ServiceViewer *viewer = new ServiceViewer(m_engine, source);
            viewer->show();
        } else if (activated == model) {
            ModelViewer *viewer = new ModelViewer(m_engine, source);
            viewer->show();
        } else if (activated == update) {
            m_engine->connectSource(source, this);
            // Plasma5Support::DataEngine::Data data = m_engine->query(source);
        } else if (activated == remove) {
            removeSource(source);
        }
    }
}

QString EngineExplorer::convertToString(const QVariant &value)
{
    switch (value.type()) {
    case QVariant::BitArray: {
        return i18np("<1 bit>", "<%1 bits>", value.toBitArray().size());
    }
    case QVariant::Bitmap: {
        QBitmap bitmap = value.value<QBitmap>();
        return QString("<%1x%2px - %3bpp>").arg(bitmap.width()).arg(bitmap.height()).arg(bitmap.depth());
    }
    case QVariant::ByteArray: {
        // Return the array size if it is not displayable
        if (value.toString().isEmpty()) {
            return i18np("<1 byte>", "<%1 bytes>", value.toByteArray().size());
        } else {
            return value.toString();
        }
    }
    case QVariant::Image: {
        QImage image = value.value<QImage>();
        return QString("<%1x%2px - %3bpp>").arg(image.width()).arg(image.height()).arg(image.depth());
    }
    case QVariant::Line: {
        QLine line = value.toLine();
        return QString("<x1:%1, y1:%2, x2:%3, y2:%4>").arg(line.x1()).arg(line.y1()).arg(line.x2()).arg(line.y2());
    }
    case QVariant::LineF: {
        QLineF lineF = value.toLineF();
        return QString("<x1:%1, y1:%2, x2:%3, y2:%4>").arg(lineF.x1()).arg(lineF.y1()).arg(lineF.x2()).arg(lineF.y2());
    }
    case QVariant::Locale: {
        return QString("%1").arg(value.toLocale().name());
    }
    case QVariant::Map: {
        QVariantMap map = value.toMap();
        QString str = i18np("<1 item>", "<%1 items>", map.size());

        for (auto it = map.constBegin(); it != map.constEnd(); it++) {
            str += "\n" + it.key() + ": " + convertToString(it.value());
        }

        return str;
    }
    case QVariant::Pixmap: {
        QPixmap pixmap = value.value<QPixmap>();
        return QString("<%1x%2px - %3bpp>").arg(pixmap.width()).arg(pixmap.height()).arg(pixmap.depth());
    }
    case QVariant::Point: {
        QPoint point = value.toPoint();
        return QString("<x:%1, y:%2>").arg(point.x()).arg(point.y());
    }
    case QVariant::PointF: {
        QPointF pointF = value.toPointF();
        return QString("<x:%1, y:%2>").arg(pointF.x()).arg(pointF.y());
    }
    case QVariant::Rect: {
        QRect rect = value.toRect();
        return QString("<x:%1, y:%2, w:%3, h:%4>").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
    }
    case QVariant::RectF: {
        QRectF rectF = value.toRectF();
        return QString("<x:%1, y:%2, w:%3, h:%4>").arg(rectF.x()).arg(rectF.y()).arg(rectF.width()).arg(rectF.height());
    }
    case QVariant::RegularExpression: {
        return value.toRegularExpression().pattern();
    }
    case QVariant::Region: {
        QRect region = value.value<QRegion>().boundingRect();
        return QString("<x:%1, y:%2, w:%3, h:%4>").arg(region.x()).arg(region.y()).arg(region.width()).arg(region.height());
    }
    case QVariant::Size: {
        QSize size = value.toSize();
        return QString("<w:%1, h:%2>").arg(size.width()).arg(size.height());
    }
    case QVariant::SizeF: {
        QSizeF sizeF = value.toSizeF();
        return QString("<w:%1, h:%2>").arg(sizeF.width()).arg(sizeF.height());
    }
    case QVariant::Url: {
        return QString("%1").arg(value.toUrl().toString());
    }
    case QVariant::StringList: {
        return QString("%1").arg(value.toStringList().join(", "));
    }
    case QVariant::Date: {
        return QString("%1").arg(value.toDate().toString());
    }
    case QVariant::DateTime: {
        return QString("%1").arg(value.toDateTime().toString());
    }
    case QVariant::Time: {
        return QString("%1").arg(value.toTime().toString());
    }
    default: {
        if (QLatin1String(value.typeName()) == "QDateTime") {
            return QString("%1").arg(value.value<QDateTime>().toString());
        }

        Plasma5Support::DataEngine::Data data = value.value<Plasma5Support::DataEngine::Data>();
        if (!data.isEmpty()) {
            QStringList result;

            for (auto it = data.constBegin(); it != data.constEnd(); it++) {
                result << (it.key() + ": " + convertToString(it.value()));
            }

            return result.join("\n");
        } else if (value.canConvert<QString>()) {
            QString str = value.toString();
            if (str.isEmpty()) {
                return i18nc("The user did a query to a dataengine and it returned empty data", "<empty>");
            } else {
                return str;
            }
        }

        return i18nc("A the dataengine returned something that the humble view on the engineexplorer can't display, like a picture", "<not displayable>");
    }
    }
}

int EngineExplorer::showData(QStandardItem *parent, Plasma5Support::DataEngine::Data data)
{
    int rowCount = 0;
    for (auto it = data.constBegin(); it != data.constEnd(); it++) {
        showData(parent, rowCount++, it.key(), it.value());
    }
    return rowCount;
}

void EngineExplorer::showData(QStandardItem *parent, int row, const QString &key, const QVariant &value)
{
    static_assert(ColumnDataSourceAndKey == 0);
    // QTreeView only expands tree for children of column #zero.
    QStandardItem *current = new QStandardItem(key);
    current->setToolTip(key);
    parent->setChild(row, ColumnDataSourceAndKey, current);

    const char *typeName = value.typeName();
    int rowCount = 0;

    if (value.userType() == qMetaTypeId<QList<QVariantMap>>()) {
        // this case is a bit special, and has to be handled before generic QVariantList
        const QList<QVariantMap> list = value.value<QList<QVariantMap>>();
        rowCount = showContainerData(parent, current, row, typeName, list);
    } else if (value.canConvert<QVariantList>() && value.type() != QVariant::String && value.type() != QVariant::ByteArray) {
        const QVariantList list = value.toList();
        rowCount = showContainerData(parent, current, row, typeName, list);
    } else if (value.canConvert<QVariantMap>()) {
        const QVariantMap map = value.toMap();
        rowCount = showContainerData(parent, current, row, typeName, map);
    } else {
        parent->setChild(row, ColumnType, new QStandardItem(typeName));
        // clang-format off
        QStandardItem *item = value.canConvert<QIcon>()
            ? new QStandardItem(value.value<QIcon>(), "")
            : new QStandardItem(convertToString(value));
        // clang-format on
        item->setToolTip(item->text());
        parent->setChild(row, ColumnValue, item);
        // leave rowCount at value 0
    }
    removeExtraRows(current, rowCount);
}

int EngineExplorer::showContainerData(QStandardItem *parent, QStandardItem *current, int row, const char *typeName, const QList<QVariantMap> &list)
{
    QStandardItem *typeItem = new QStandardItem(typeName);
    typeItem->setToolTip(typeItem->text());
    parent->setChild(row, ColumnType, typeItem);
    parent->setChild(row, ColumnValue, new QStandardItem(ki18ncp("Length of the list", "<%1 item>", "<%1 items>").subs(list.length()).toString()));

    int rowCount = 0;
    for (const QVariantMap &map : list) {
        showData(current, rowCount, QString::number(rowCount), map);
        rowCount++;
    }
    return rowCount;
}

int EngineExplorer::showContainerData(QStandardItem *parent, QStandardItem *current, int row, const char *typeName, const QVariantList &list)
{
    QStandardItem *typeItem = new QStandardItem(typeName);
    typeItem->setToolTip(typeItem->text());
    parent->setChild(row, ColumnType, typeItem);
    parent->setChild(row, ColumnValue, new QStandardItem(ki18ncp("Length of the list", "<%1 item>", "<%1 items>").subs(list.length()).toString()));

    int rowCount = 0;
    for (const QVariant &var : list) {
        showData(current, rowCount, QString::number(rowCount), var);
        rowCount++;
    }
    return rowCount;
}

int EngineExplorer::showContainerData(QStandardItem *parent, QStandardItem *current, int row, const char *typeName, const QVariantMap &map)
{
    QStandardItem *typeItem = new QStandardItem(typeName);
    typeItem->setToolTip(typeItem->text());
    parent->setChild(row, ColumnType, typeItem);
    parent->setChild(row, ColumnValue, new QStandardItem(ki18ncp("Size of the map", "<%1 pair>", "<%1 pairs>").subs(map.size()).toString()));

    int rowCount = 0;
    for (auto it = map.constBegin(); it != map.constEnd(); it++) {
        showData(current, rowCount++, it.key(), it.value());
    }
    return rowCount;
}

void EngineExplorer::updateTitle()
{
    if (!m_engine || !m_engine->metadata().isValid()) {
        m_title->setIcon(QIcon::fromTheme("plasma"));
        m_title->setText(i18n("Plasma DataEngine Explorer"));
        return;
    }

    m_title->setText(ki18ncp("The name of the engine followed by the number of data sources", "%1 Engine - 1 data source", "%1 Engine - %2 data sources")
                         .subs(KStringHandler::capwords(m_engine->metadata().name()))
                         .subs(m_sourceCount)
                         .toString());

    if (m_engine->metadata().iconName().isEmpty()) {
        m_title->setIcon(QIcon::fromTheme("plasma"));
    } else {
        m_title->setIcon(QIcon::fromTheme(m_engine->metadata().iconName()));
    }
}

void EngineExplorer::enableButtons(bool enable)
{
    if (m_expandButton) {
        m_expandButton->setEnabled(enable);
    }

    if (m_collapseButton) {
        m_collapseButton->setEnabled(enable);
    }
}

#include "moc_engineexplorer.cpp"
