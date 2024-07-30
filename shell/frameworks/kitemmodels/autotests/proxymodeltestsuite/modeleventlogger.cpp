/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modeleventlogger.h"
#include "indexfinder.h"
#include "modeldumper.h"

#include "eventloggerregister.h"

#include <QDebug>
#include <QFile>
#include <QStringList>

#ifdef Grantlee_FOUND
#include "grantlee_paths.h"
#include <grantlee_core.h>

/**
  Don't escape the code generation output.

  'const QString &' should not become 'const QString &amp;'
*/
class NoEscapeOutputStream : public Grantlee::OutputStream
{
public:
    NoEscapeOutputStream()
        : Grantlee::OutputStream()
    {
    }

    NoEscapeOutputStream(QTextStream *stream)
        : OutputStream(stream)
    {
    }

    virtual QSharedPointer<Grantlee::OutputStream> clone() const
    {
        return QSharedPointer<Grantlee::OutputStream>(new NoEscapeOutputStream);
    }

    virtual QString escape(const QString &input) const
    {
        return input;
    }
};
#endif

class ModelWrapper : public QAbstractItemModel
{
public:
    ModelWrapper(QAbstractItemModel * /*model*/, QObject *parent = nullptr)
        : QAbstractItemModel(parent)
    {
    }

    QModelIndexList per() const
    {
        return persistentIndexList();
    }

    QModelIndex index(int /*row*/, int /*column*/, const QModelIndex & /*parent*/ = QModelIndex()) const override
    {
        return QModelIndex();
    }
    int rowCount(const QModelIndex & /*parent*/ = QModelIndex()) const override
    {
        return 0;
    }
    QModelIndex parent(const QModelIndex & /*child*/) const override
    {
        return QModelIndex();
    }
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override
    {
        return 0;
    }
    QVariant data(const QModelIndex & /*index*/, int /*role*/ = Qt::DisplayRole) const override
    {
        return QVariant();
    }
};

ModelEvent::ModelEvent(QObject *parent)
    : QObject(parent)
{
}

static const char *const sTypes[] = {"Init", "RowsInserted", "RowsRemoved", "DataChanged", "LayoutChanged", "ModelReset"};

QString ModelEvent::type() const
{
    return QLatin1String(*(sTypes + m_type));
}

// ModelEvent::Type ModelEvent::type() const
// {
//   return m_type;
// }

void ModelEvent::setType(ModelEvent::Type type)
{
    m_type = type;
}

int ModelEvent::start() const
{
    return m_start;
}

void ModelEvent::setStart(int start)
{
    m_start = start;
}

int ModelEvent::end() const
{
    return m_end;
}

void ModelEvent::setEnd(int end)
{
    m_end = end;
}

QString ModelEvent::rowAncestors() const
{
    QString result(QStringLiteral("QList<int>()"));

    for (const int row : std::as_const(m_rowAncestors)) {
        result.append(" << ");
        result.append(QString::number(row));
    }
    return result;
}

// QList< int > ModelEvent::rowAncestors() const
// {
//   return m_rowAncestors;
// }

void ModelEvent::setRowAncestors(QList<int> rowAncestors)
{
    m_rowAncestors = rowAncestors;
}

bool ModelEvent::hasInterpretString() const
{
    return !m_interpretString.isEmpty();
}

QString ModelEvent::interpretString() const
{
    return m_interpretString;
}

void ModelEvent::setInterpretString(const QString &interpretString)
{
    m_interpretString = interpretString;
}

ModelEventLogger::ModelEventLogger(QAbstractItemModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_modelDumper(new ModelDumper)
    , m_numLogs(0)
{
    connect(model, &QAbstractItemModel::dataChanged, this, &ModelEventLogger::dataChanged);
    connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &ModelEventLogger::layoutAboutToBeChanged);
    connect(model, &QAbstractItemModel::layoutChanged, this, &ModelEventLogger::layoutChanged);
    connect(model, &QAbstractItemModel::modelReset, this, &ModelEventLogger::modelReset);
    connect(model, &QAbstractItemModel::rowsInserted, this, &ModelEventLogger::rowsInserted);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &ModelEventLogger::rowsRemoved);

    ModelEvent *modelEvent = new ModelEvent(this);
    modelEvent->setType(ModelEvent::Init);
    modelEvent->setInterpretString(m_modelDumper->dumpModel(model));

    m_modelName = QString::fromLatin1(model->metaObject()->className()).toLower();

    m_initEvent = QVariant::fromValue(static_cast<QObject *>(modelEvent));

    EventLoggerRegister::instance()->registerLogger(this);
}

void ModelEventLogger::writeLog()
{
#ifdef Grantlee_FOUND
    QString logFileName = QString("main.%1.%2.%3.cpp").arg(m_modelName).arg(reinterpret_cast<qint64>(this)).arg(m_numLogs++);
    qDebug() << "Writing to " << logFileName;
    QFile outputFile(logFileName);
    const bool logFileOpened = outputFile.open(QFile::WriteOnly | QFile::Text);
    Q_ASSERT(logFileOpened);

    Grantlee::Engine engine;
    Grantlee::FileSystemTemplateLoader::Ptr loader(new Grantlee::FileSystemTemplateLoader);
    loader->setTemplateDirs(QStringList() << ":/templates");
    engine.addTemplateLoader(loader);
    engine.setPluginPaths(QStringList() << GRANTLEE_PLUGIN_PATH);

    // Write out.
    Grantlee::Template t = engine.loadByName("main.cpp");
    if (!t->error()) {
        Grantlee::Context c;
        c.insert("initEvent", m_initEvent);
        c.insert("events", m_events);

        QTextStream textStream(&outputFile);
        NoEscapeOutputStream outputStream(&textStream);
        t->render(&outputStream, &c);
    }
    outputFile.close();

    if (t->error()) {
        qDebug() << t->errorString();
    }
#else
    qDebug() << "Grantlee not found. No log written.";
#endif
}

ModelEventLogger::~ModelEventLogger()
{
    writeLog();
    delete m_modelDumper;
    EventLoggerRegister::instance()->unregisterLogger(this);
}

void ModelEventLogger::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    ModelEvent *modelEvent = new ModelEvent(this);
    modelEvent->setType(ModelEvent::DataChanged);
    modelEvent->setRowAncestors(IndexFinder::indexToIndexFinder(topLeft.parent()).rows());
    modelEvent->setStart(topLeft.row());
    modelEvent->setEnd(bottomRight.row());

    m_events.append(QVariant::fromValue(static_cast<QObject *>(modelEvent)));
}

void ModelEventLogger::persistChildren(const QModelIndex & /*parent*/)
{
}

void ModelEventLogger::layoutAboutToBeChanged()
{
    m_oldPaths.clear();
    m_persistentIndexes.clear();
    const QModelIndexList list = static_cast<const ModelWrapper *>(m_model)->per();
    for (const QModelIndex &idx : list) {
        m_persistentIndexes.append(QPersistentModelIndex(idx));
        m_oldPaths.append(IndexFinder::indexToIndexFinder(idx).rows());
    }
}

void ModelEventLogger::layoutChanged()
{
    ModelEvent *modelEvent = new ModelEvent(this);
    modelEvent->setType(ModelEvent::LayoutChanged);
    modelEvent->setInterpretString(m_modelDumper->dumpModel(m_model));

    QList<PersistentChange *> changes;

    for (int i = 0; i < m_persistentIndexes.size(); ++i) {
        const QPersistentModelIndex pIdx = m_persistentIndexes.at(i);
        if (!pIdx.isValid()) {
            PersistentChange *change = new PersistentChange(this);
            change->newPath = QList<int>();
            change->oldPath = m_oldPaths.at(i);
            changes.append(change);
            continue;
        }
        const QList<int> rows = IndexFinder::indexToIndexFinder(pIdx).rows();
        if (m_oldPaths.at(i) == rows) {
            continue;
        }

        PersistentChange *change = new PersistentChange(this);
        change->newPath = rows;
        change->oldPath = m_oldPaths.at(i);
        changes.append(change);
    }

    modelEvent->setChanges(changes);

    m_events.append(QVariant::fromValue(static_cast<QObject *>(modelEvent)));
}

void ModelEventLogger::modelReset()
{
    ModelEvent *modelEvent = new ModelEvent(this);
    modelEvent->setType(ModelEvent::ModelReset);
    modelEvent->setInterpretString(m_modelDumper->dumpModel(m_model));

    m_events.append(QVariant::fromValue(static_cast<QObject *>(modelEvent)));
}

void ModelEventLogger::rowsInserted(const QModelIndex &parent, int start, int end)
{
    ModelEvent *modelEvent = new ModelEvent(this);
    modelEvent->setType(ModelEvent::RowsInserted);
    modelEvent->setRowAncestors(IndexFinder::indexToIndexFinder(parent).rows());
    modelEvent->setStart(start);
    QString s = m_modelDumper->dumpTree(m_model, parent, start, end);
    modelEvent->setInterpretString(s);

    m_events.append(QVariant::fromValue(static_cast<QObject *>(modelEvent)));
}

void ModelEventLogger::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    ModelEvent *modelEvent = new ModelEvent(this);
    modelEvent->setType(ModelEvent::RowsRemoved);
    modelEvent->setRowAncestors(IndexFinder::indexToIndexFinder(parent).rows());
    modelEvent->setStart(start);
    modelEvent->setEnd(end);

    m_events.append(QVariant::fromValue(static_cast<QObject *>(modelEvent)));
}

#include "moc_modeleventlogger.cpp"
