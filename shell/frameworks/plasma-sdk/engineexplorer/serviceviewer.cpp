/*
 *   SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "serviceviewer.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KStringHandler>
#include <QDebug>
#include <QDialogButtonBox>

#include <Plasma5Support/DataEngine>
#include <Plasma5Support/Service>
#include <Plasma5Support/ServiceJob>

#include "engineexplorer.h"

ServiceViewer::ServiceViewer(Plasma5Support::DataEngine *engine, const QString &source, QWidget *parent)
    : QDialog(parent)
    , m_engine(engine)
    , m_service(nullptr)
    , m_source(source)
    , m_operationCount(0)
    , m_operationButton(new QPushButton(i18n("Start Operation"), this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(m_operationButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Close);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(mainWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);

    setupUi(mainWidget);
    m_operationStatus->hide();

    connect(m_operationButton, &QAbstractButton::clicked, this, &ServiceViewer::startOperation);
    m_operationButton->setEnabled(false);

    connect(m_operations, &KComboBox::currentIndexChanged, this, [this](int index) {
        operationSelected(m_operations->itemText(index));
    });

    QString engineName = i18nc("Plasma engine with unknown name", "Unknown");
    QString serviceName = i18nc("Plasma service with unknown name", "Unknown");

    if (m_engine) {
        if (m_engine->metadata().isValid()) {
            engineName = KStringHandler::capwords(m_engine->metadata().name());
        }
        qDebug() << "Requesting service for source" << m_source;
        m_service = m_engine->serviceForSource(m_source);

        if (m_service != nullptr) {
            serviceName = m_service->name();
            updateOperations();
            connect(m_service, SIGNAL(operationsChanged()), this, SLOT(updateOperations()));
            connect(m_engine, &QObject::destroyed, this, &ServiceViewer::engineDestroyed);
        } else {
            KMessageBox::error(this, i18n("No valid service was returned. Verify that a service is available for this source."));
            close();
        }
    }

    setWindowTitle(i18nc("%1 is a Plasma service name", "%1 Service Explorer", serviceName));

    QString title = i18nc("Source: name of the data, Service: writes data instead of fetching",
                          "Engine: <b>%1</b>; Source: <b>%2</b>; Service: <b>%3</b>",
                          engineName,
                          m_source,
                          serviceName);
    m_title->setText(title);
    m_operations->setFocus();
}

ServiceViewer::~ServiceViewer()
{
    delete m_service;
    m_engine = nullptr;
}

void ServiceViewer::updateOperations()
{
    if (!m_engine) {
        return;
    }

    bool enable = false;

    m_operations->clear();
    m_operationDescription->clear();

    if (m_service) {
        const QStringList operations = m_service->operationNames();

        if (!operations.isEmpty()) {
            enable = true;

            for (const QString &operation : operations) {
                m_operations->addItem(operation);
            }
        }
    }

    m_operations->setEnabled(enable);
    m_operationsLabel->setEnabled(enable);
    m_operationDescription->setEnabled(enable);
}

void ServiceViewer::startOperation()
{
    if (!m_service) {
        return;
    }

    QString operation = m_operations->currentText();
    QVariantMap desc = m_service->operationDescription(operation);
    for (int i = 0; i < m_operationDescription->rowCount(); ++i) {
        QTableWidgetItem *item = m_operationDescription->item(i, 1);
        QString value = item->text();

        if (value.isEmpty()) {
            continue;
        }

        item = m_operationDescription->item(i, 0);
        QString key = item->text();
        desc[key] = value;
    }

    updateJobCount(1);
    Plasma5Support::ServiceJob *job = m_service->startOperationCall(desc);
    connect(job, &KJob::finished, this, &ServiceViewer::operationResult);
}

void ServiceViewer::operationSelected(const QString &operation)
{
    if (!m_service) {
        return;
    }

    m_operationButton->setEnabled(true);
    QStringList headers;
    headers << i18n("Key") << i18n("Value");
    m_operationDescription->setHorizontalHeaderLabels(headers);

    QVariantMap desc = m_service->operationDescription(operation);
    int i = 0;
    const QStringList keys = desc.keys();
    m_operationDescription->setRowCount(keys.count());
    for (const QString &key : keys) {
        QTableWidgetItem *item = new QTableWidgetItem(key);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        m_operationDescription->setItem(i, 0, item);

        item = new QTableWidgetItem(desc[key].toString());
        m_operationDescription->setItem(i, 1, item);

        ++i;
    }
}

void ServiceViewer::operationResult(KJob *j)
{
    if (!m_service) {
        return;
    }

    Plasma5Support::ServiceJob *job = qobject_cast<Plasma5Support::ServiceJob *>(j);
    if (!job) {
        return;
    }

    updateJobCount(-1);

    if (job->error()) {
        KMessageBox::information(this,
                                 i18n("<b>'%1'</b> operation with destination <b>'%2'</b> failed. "
                                      "<p>The error was: <b>'%3: %4'</b></p>",
                                      job->operationName(),
                                      job->destination(),
                                      job->error(),
                                      job->errorString()),
                                 i18n("Operation Result"));
    } else {
        QString result = EngineExplorer::convertToString(job->result());
        if (result.isEmpty()) {
            result = i18n("No response from job.");
        }

        KMessageBox::information(this,
                                 i18n("<b>'%1'</b> operation with destination <b>'%2'</b> returned successfully. "
                                      "<p>The result was: <b>'%3'</b></p>",
                                      job->operationName(),
                                      job->destination(),
                                      result),
                                 i18n("Operation Result"));
    }

    qDebug() << "operation results are in!";
}

void ServiceViewer::engineDestroyed()
{
    m_service = nullptr;
    m_engine = nullptr;
    hide();
    deleteLater();
}

void ServiceViewer::updateJobCount(int numberOfJobs)
{
    m_operationCount += numberOfJobs;

    if (m_operationCount < 1) {
        m_operationCount = 0;
        m_operationStatus->hide();
    } else {
        m_operationStatus->setText(i18np("One active operation…", "%1 operations active…", m_operationCount));
        m_operationStatus->show();
    }
}

#include "moc_serviceviewer.cpp"
