/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SelectMakeModel.h"
#include "ui_SelectMakeModel.h"

#include "KCupsRequest.h"
#include "NoSelectionRectDelegate.h"
#include "kcupslib_log.h"

#include <QItemSelection>
#include <QLineEdit>
#include <QStandardItemModel>

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusReply>

#include <KLocalizedString>
#include <KMessageBox>

// Marshall the MyStructure data into a D-Bus argument
QDBusArgument &operator<<(QDBusArgument &argument, const DriverMatch &driverMatch)
{
    argument.beginStructure();
    argument << driverMatch.ppd << driverMatch.match;
    argument.endStructure();
    return argument;
}

// Retrieve the MyStructure data from the D-Bus argument
const QDBusArgument &operator>>(const QDBusArgument &argument, DriverMatch &driverMatch)
{
    argument.beginStructure();
    argument >> driverMatch.ppd >> driverMatch.match;
    argument.endStructure();
    return argument;
}

SelectMakeModel::SelectMakeModel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SelectMakeModel)
{
    ui->setupUi(this);

    // Configure the error message widget
    ui->messageWidget->setWordWrap(true);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->hide();

    m_sourceModel = new PPDModel(this);

    ui->makeView->setModel(m_sourceModel);
    ui->makeView->setItemDelegate(new NoSelectionRectDelegate(this));
    // Updates the PPD view to the selected Make
    connect(ui->makeView->selectionModel(), &QItemSelectionModel::currentChanged, ui->ppdsLV, &QListView::setRootIndex);

    ui->ppdsLV->setModel(m_sourceModel);
    ui->ppdsLV->setItemDelegate(new NoSelectionRectDelegate(this));
    connect(m_sourceModel, &PPDModel::dataChanged, this, &SelectMakeModel::checkChanged);

    // Clear the PPD view selection, so the Next/Finish button gets disabled
    connect(ui->makeView->selectionModel(), &QItemSelectionModel::currentChanged, ui->ppdsLV->selectionModel(), &QItemSelectionModel::clearSelection);

    // Make sure we update the Next/Finish button if a PPD is selected
    connect(ui->ppdsLV->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectMakeModel::checkChanged);

    // When the radio button changes the signal must be emitted
    connect(ui->ppdFileRB, &QRadioButton::toggled, this, &SelectMakeModel::checkChanged);
    connect(ui->ppdFilePathUrl, &KUrlRequester::textChanged, this, &SelectMakeModel::checkChanged);

    qDBusRegisterMetaType<DriverMatch>();
    qDBusRegisterMetaType<DriverMatchList>();
}

SelectMakeModel::~SelectMakeModel()
{
    delete ui;
}

void SelectMakeModel::setDeviceInfo(const QString &deviceId, const QString &make, const QString &makeAndModel, const QString &deviceUri)
{
    qCDebug(LIBKCUPS) << "===================================" << deviceId << makeAndModel << deviceUri;
    m_gotBestDrivers = false;
    m_hasRecommended = false;
    m_make = make;
    m_makeAndModel = makeAndModel;

    // Get the best drivers
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("/org/fedoraproject/Config/Printing"),
                                             QLatin1String("org.fedoraproject.Config.Printing"),
                                             QLatin1String("GetBestDrivers"));
    message << deviceId;
    message << makeAndModel;
    message << deviceUri;
    QDBusConnection::sessionBus().callWithCallback(message,
                                                   this,
                                                   SLOT(getBestDriversFinished(QDBusMessage)),
                                                   SLOT(getBestDriversFailed(QDBusError, QDBusMessage)));

    if (!m_ppdRequest) {
        m_ppdRequest = new KCupsRequest;
        connect(m_ppdRequest, &KCupsRequest::finished, this, &SelectMakeModel::ppdsLoaded);
        m_ppdRequest->getPPDS();
    }
}

void SelectMakeModel::setMakeModel(const QString &make, const QString &makeAndModel)
{
    ui->radioButtonSelectDriver->setText(i18n("Choose the driver for %1", makeAndModel));
    if (!m_ppdRequest) {
        // We won't try to get the best driver
        // we should be we need more info and testing
        // TODO
        m_gotBestDrivers = true;
        m_hasRecommended = false;
        m_make = make;
        m_makeAndModel = makeAndModel;

        m_ppdRequest = new KCupsRequest;
        connect(m_ppdRequest, &KCupsRequest::finished, this, &SelectMakeModel::ppdsLoaded);
        m_ppdRequest->getPPDS();
    } else {
        // TODO test this
        setModelData();
    }
}

void SelectMakeModel::ppdsLoaded(KCupsRequest *request)
{
    if (request->hasError()) {
        qCWarning(LIBKCUPS) << "Failed to get PPDs" << request->errorMsg();
        ui->messageWidget->setText(i18n("Failed to get a list of drivers: '%1'", request->errorMsg()));
        ui->messageWidget->animatedShow();

        // Force the changed signal to be sent
        checkChanged();

    } else {
        m_ppds = request->ppds();

        // Try to show the PPDs
        setModelData();
    }
    m_ppdRequest = nullptr;
    request->deleteLater();
}

void SelectMakeModel::checkChanged()
{
    qCDebug(LIBKCUPS);
    if (isFileSelected()) {
        Q_EMIT changed(!selectedPPDFileName().isNull());
    } else {
        // enable or disable the job action buttons if something is selected
        Q_EMIT changed(!selectedPPDName().isNull());

        selectFirstMake();
    }
}

QString SelectMakeModel::selectedPPDName() const
{
    QItemSelection ppdSelection = ui->ppdsLV->selectionModel()->selection();
    if (!isFileSelected() && !ppdSelection.indexes().isEmpty()) {
        QModelIndex index = ppdSelection.indexes().first();
        return index.data(PPDModel::PPDName).toString();
    }
    return QString();
}

QString SelectMakeModel::selectedPPDMakeAndModel() const
{
    QItemSelection ppdSelection = ui->ppdsLV->selectionModel()->selection();
    if (!isFileSelected() && !ppdSelection.indexes().isEmpty()) {
        QModelIndex index = ppdSelection.indexes().first();
        return index.data(PPDModel::PPDMakeAndModel).toString();
    }
    return QString();
}

QString SelectMakeModel::selectedPPDFileName() const
{
    if (isFileSelected()) {
        QFileInfo file = QFileInfo(ui->ppdFilePathUrl->url().toLocalFile());
        qCDebug(LIBKCUPS) << ui->ppdFilePathUrl->url().toLocalFile() << file.isFile() << file.filePath();
        if (file.isFile()) {
            return file.filePath();
        }
    }
    return QString();
}

bool SelectMakeModel::isFileSelected() const
{
    qCDebug(LIBKCUPS) << ui->ppdFileRB->isChecked();
    return ui->ppdFileRB->isChecked();
}

void SelectMakeModel::getBestDriversFinished(const QDBusMessage &message)
{
    if (message.type() == QDBusMessage::ReplyMessage && message.arguments().size() == 1) {
        QDBusArgument arg = message.arguments().first().value<QDBusArgument>();
        const DriverMatchList driverMatchList = qdbus_cast<DriverMatchList>(arg);
        m_driverMatchList = driverMatchList;
        m_hasRecommended = !m_driverMatchList.isEmpty();
        for (const DriverMatch &driverMatch : driverMatchList) {
            qCDebug(LIBKCUPS) << driverMatch.ppd << driverMatch.match;
        }
    } else {
        qCWarning(LIBKCUPS) << "Unexpected message" << message;
    }
    m_gotBestDrivers = true;
    setModelData();
}

void SelectMakeModel::getBestDriversFailed(const QDBusError &error, const QDBusMessage &message)
{
    qCWarning(LIBKCUPS) << "Failed to get best drivers" << error << message;

    // Show the PPDs anyway
    m_gotBestDrivers = true;
    ui->messageWidget->setText(i18n("Failed to search for a recommended driver: '%1'", error.message()));
    ui->messageWidget->animatedShow();
    setModelData();
}

void SelectMakeModel::setModelData()
{
    if (!m_ppds.isEmpty() && m_gotBestDrivers) {
        m_sourceModel->setPPDs(m_ppds, m_driverMatchList);

        // Pre-select the first Recommended PPD
        if (m_hasRecommended) {
            selectRecommendedPPD();
        } else if (!m_ppds.isEmpty() && !m_make.isEmpty()) {
            selectMakeModelPPD();
        }

        // Force changed signal to be emitted
        checkChanged();
    }
}

void SelectMakeModel::selectFirstMake()
{
    QItemSelection selection;
    selection = ui->makeView->selectionModel()->selection();
    // Make sure the first make is selected
    if (selection.indexes().isEmpty() && m_sourceModel->rowCount() > 0) {
        ui->makeView->selectionModel()->setCurrentIndex(m_sourceModel->index(0, 0), QItemSelectionModel::SelectCurrent);
    }
}

void SelectMakeModel::selectMakeModelPPD()
{
    const QList<QStandardItem *> makes = m_sourceModel->findItems(m_make);
    for (QStandardItem *make : makes) {
        // Check if the item is in this make
        for (int i = 0; i < make->rowCount(); i++) {
            if (make->child(i)->data(PPDModel::PPDMakeAndModel).toString() == m_makeAndModel) {
                ui->makeView->selectionModel()->setCurrentIndex(make->index(), QItemSelectionModel::SelectCurrent);
                ui->ppdsLV->selectionModel()->setCurrentIndex(make->child(i)->index(), QItemSelectionModel::SelectCurrent);
                return;
            }
        }
    }

    // the exact PPD wasn't found try to select just the make
    if (!makes.isEmpty()) {
        ui->makeView->selectionModel()->setCurrentIndex(makes.first()->index(), QItemSelectionModel::SelectCurrent);
    }
}

void SelectMakeModel::selectRecommendedPPD()
{
    // Force the first make to be selected
    selectFirstMake();

    QItemSelection ppdSelection = ui->ppdsLV->selectionModel()->selection();
    if (ppdSelection.indexes().isEmpty()) {
        QItemSelection makeSelection = ui->makeView->selectionModel()->selection();
        QModelIndex parent = makeSelection.indexes().first();
        if (parent.isValid()) {
            ui->ppdsLV->selectionModel()->setCurrentIndex(m_sourceModel->index(0, 0, parent), QItemSelectionModel::SelectCurrent);
        }
    }
}

#include "moc_SelectMakeModel.cpp"
