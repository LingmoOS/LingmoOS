/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <kuserfeedback_version.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helpcontroller.h"
#include "connectdialog.h"

#include <jobs/handshakejob.h>
#include <jobs/productexportjob.h>
#include <jobs/productimportjob.h>
#include <model/productmodel.h>

#include <rest/restapi.h>
#include <rest/restclient.h>
#include <rest/serverinfo.h>

#include <provider/widgets/feedbackconfigdialog.h>
#include <provider/widgets/notificationpopup.h>
#include <provider/core/applicationversionsource.h>
#include <provider/core/platforminfosource.h>
#include <provider/core/propertyratiosource.h>
#include <provider/core/provider.h>
#include <provider/core/qtversionsource.h>
#include <provider/core/startcountsource.h>
#include <provider/core/usagetimesource.h>

#include <QActionGroup>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScopedValueRollback>
#include <QSettings>
#include <QTimer>
#include <QUrl>

using namespace KUserFeedback::Console;

MainWindow::MainWindow() :
    ui(new Ui::MainWindow),
    m_restClient(new RESTClient(this)),
    m_productModel(new ProductModel(this)),
    m_feedbackProvider(new KUserFeedback::Provider(this))
{
    ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("search")));

    addView(ui->surveyEditor, ui->menuSurvey);
    addView(ui->schemaEdit, ui->menuSchema);
    addView(ui->analyticsView, ui->menuAnalytics);

    ui->productListView->setModel(m_productModel);
    ui->productListView->addActions({ ui->actionAddProduct, ui->actionDeleteProduct });

    connect(m_restClient, &RESTClient::errorMessage, this, &MainWindow::logError);
    m_productModel->setRESTClient(m_restClient);

    ui->actionViewAnalytics->setData(QVariant::fromValue(ui->analyticsView));
    ui->actionViewSurveys->setData(QVariant::fromValue(ui->surveyEditor));
    ui->actionViewSchema->setData(QVariant::fromValue(ui->schemaEdit));
    auto viewGroup = new QActionGroup(this);
    viewGroup->setExclusive(true);
    viewGroup->addAction(ui->actionViewAnalytics);
    viewGroup->addAction(ui->actionViewSurveys);
    viewGroup->addAction(ui->actionViewSchema);
    connect(viewGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        auto view = action->data().value<QWidget*>();
        if (ui->viewStack->currentWidget() == ui->schemaEdit && ui->schemaEdit->isDirty() && view != ui->schemaEdit) {
            const auto r = QMessageBox::critical(this, tr("Unsaved Schema Changes"),
                tr("You have unsaved changes in the schema editor. Do you really want to close it and discard your changes?"),
                QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
            if (r != QMessageBox::Discard) {
                ui->actionViewSchema->setChecked(true);
                return;
            }
        }
        ui->viewStack->setCurrentWidget(view);
    });
    ui->actionViewAnalytics->setChecked(true);

    connect(ui->actionConnectToServer, &QAction::triggered, this, [this]() {
        QSettings settings;
        auto info = ServerInfo::load(settings.value(QStringLiteral("LastServerInfo")).toString());
        ConnectDialog dlg(this);
        dlg.addRecentServerInfos(ServerInfo::allServerInfoNames());
        dlg.setServerInfo(info);
        if (dlg.exec()) {
            info = dlg.serverInfo();
            info.save();
            settings.setValue(QStringLiteral("LastServerInfo"), info.name());
            connectToServer(info);
        }
    });

    connect(ui->actionAddProduct, &QAction::triggered, this, &MainWindow::createProduct);
    connect(ui->actionDeleteProduct, &QAction::triggered, this, &MainWindow::deleteProduct);
    connect(ui->actionImportProduct, &QAction::triggered, this, &MainWindow::importProduct);
    connect(ui->actionExportProduct, &QAction::triggered, this, &MainWindow::exportProduct);

    connect(ui->schemaEdit, &SchemaEditor::productChanged, m_productModel, &ProductModel::reload);

    ui->actionQuit->setShortcut(QKeySequence::Quit);
    connect(ui->actionQuit, &QAction::triggered, this, &QMainWindow::close);
    ui->menuWindow->addAction(ui->productsDock->toggleViewAction());
    ui->menuWindow->addAction(ui->logDock->toggleViewAction());

    ui->actionUserManual->setEnabled(HelpController::isAvailable());
    ui->actionUserManual->setShortcut(QKeySequence::HelpContents);
    connect(ui->actionUserManual, &QAction::triggered, this, []() {
        HelpController::openContents();
    });
    ui->actionContribute->setVisible(m_feedbackProvider->isEnabled());
    connect(ui->actionContribute, &QAction::triggered, this, [this]() {
        FeedbackConfigDialog dlg(this);
        dlg.setFeedbackProvider(m_feedbackProvider);
        dlg.exec();
    });
    connect(ui->actionAbout, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("About User Feedback Console"), tr(
            "Version: %1\n"
            "License: MIT\n"
            "Copyright ⓒ 2017 Volker Krause <vkrause@kde.org>"
        ).arg(QStringLiteral(KUSERFEEDBACK_VERSION_STRING)));
    });

    connect(ui->productListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection&, const QItemSelection &deselected) {
        static bool recursionGuard = false;
        if (recursionGuard)
            return;
        if (ui->viewStack->currentWidget() == ui->schemaEdit && ui->schemaEdit->isDirty()) {
            const auto r = QMessageBox::critical(this, tr("Unsaved Schema Changes"),
                tr("You have unsaved changes in the schema editor, do you really want to open another product and discard your changes?"),
                QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
            if (r != QMessageBox::Discard) {
                QScopedValueRollback<bool> guard(recursionGuard, true);
                ui->productListView->selectionModel()->select(deselected, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
                return;
            }
        }
        productSelected();
    });
    connect(m_productModel, &QAbstractItemModel::dataChanged, this, &MainWindow::productSelected);

    connect(ui->viewStack, &QStackedWidget::currentChanged, this, &MainWindow::updateActions);
    connect(ui->viewStack, &QStackedWidget::currentChanged, this, [viewGroup](int index) {
        viewGroup->actions().at(index)->setChecked(true);
    });
    updateActions();

    QSettings settings;
    settings.beginGroup(QStringLiteral("MainWindow"));
    restoreGeometry(settings.value(QStringLiteral("Geometry")).toByteArray());
    restoreState(settings.value(QStringLiteral("State")).toByteArray());
    ui->viewStack->setCurrentIndex(settings.value(QStringLiteral("CurrentView"), 0).toInt());

    QTimer::singleShot(0, ui->actionConnectToServer, &QAction::trigger);

    m_feedbackProvider->setFeedbackServer(QUrl(QStringLiteral("https://telemetry.kde.org")));
    m_feedbackProvider->setSubmissionInterval(1);
    auto viewModeSource = new KUserFeedback::PropertyRatioSource(ui->viewStack, "currentIndex", QStringLiteral("viewRatio"));
    viewModeSource->setDescription(tr("Usage ratio of the analytics view, survey editor and schema editor."));
    viewModeSource->addValueMapping(0, QStringLiteral("analytics"));
    viewModeSource->addValueMapping(1, QStringLiteral("surveyEditor"));
    viewModeSource->addValueMapping(2, QStringLiteral("schemaEditor"));
    viewModeSource->setTelemetryMode(Provider::DetailedUsageStatistics);
    m_feedbackProvider->addDataSource(viewModeSource);
    m_feedbackProvider->addDataSource(new ApplicationVersionSource);
    m_feedbackProvider->addDataSource(new PlatformInfoSource);
    m_feedbackProvider->addDataSource(new QtVersionSource);
    m_feedbackProvider->addDataSource(new StartCountSource);
    m_feedbackProvider->addDataSource(new UsageTimeSource);

    m_feedbackProvider->setEncouragementDelay(60);
    m_feedbackProvider->setEncouragementInterval(5);
    m_feedbackProvider->setApplicationStartsUntilEncouragement(5);
    m_feedbackProvider->setApplicationUsageTimeUntilEncouragement(600); // 10 mins

    auto notifyPopup = new KUserFeedback::NotificationPopup(this);
    notifyPopup->setFeedbackProvider(m_feedbackProvider);
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("MainWindow"));
    settings.setValue(QStringLiteral("State"), saveState());
    settings.setValue(QStringLiteral("Geometry"), saveGeometry());
    settings.setValue(QStringLiteral("CurrentView"), ui->viewStack->currentIndex());
}

template <typename T>
void MainWindow::addView(T *view, QMenu *menu)
{
    for (auto action : view->actions())
        menu->addAction(action);

    view->setRESTClient(m_restClient);
    connect(view, &T::logMessage, this, &MainWindow::logMessage);
}

void MainWindow::connectToServer(const ServerInfo& info)
{
    m_restClient->setServerInfo(info);
    auto job = new HandshakeJob(m_restClient, this);
    connect(job, &Job::destroyed, this, &MainWindow::updateActions);
    connect(job, &Job::error, this, [this](const QString &msg) {
        logError(msg);
        QMessageBox::critical(this, tr("Connection Failure"), tr("Failed to connect to server: %1").arg(msg));
    });
    connect(job, &Job::info, this, &MainWindow::logMessage);
}

void MainWindow::createProduct()
{
    const auto name = QInputDialog::getText(this, tr("Add New Product"), tr("Product Identifier:"));
    if (name.isEmpty())
        return;
    Product product;
    product.setName(name);

    auto reply = RESTApi::createProduct(m_restClient, product);
    connect(reply, &QNetworkReply::finished, this, [this, reply, name]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            logMessage(QString::fromUtf8(reply->readAll()));
            m_productModel->reload();
        }
    });
}

void MainWindow::deleteProduct()
{
    auto sel = ui->productListView->selectionModel()->selectedRows();
    if (sel.isEmpty())
        return;
    const auto product = sel.first().data(ProductModel::ProductRole).value<Product>();
    const auto mb = QMessageBox::critical(this, tr("Delete Product"),
                        tr("Do you really want to delete product %1 with all its data?").arg(product.name()),
                        QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
    if (mb != QMessageBox::Discard)
        return;

    auto reply = RESTApi::deleteProduct(m_restClient, product);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            logMessage(QString::fromUtf8(reply->readAll()));
        }
        m_productModel->reload();
    });
}

void MainWindow::importProduct()
{
    const auto fileName = QFileDialog::getExistingDirectory(this, tr("Import Product"));
    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if (!fi.exists() || !fi.isDir() || !fi.isReadable()) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Could not open file."));
        return;
    }

    auto job = new ProductImportJob(fileName, m_restClient, this);
    connect(job, &Job::error, this, [this](const auto &msg) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Import error: %1").arg(msg));
    });
    connect(job, &Job::finished, this, [this]() {
        logMessage(tr("Product imported successfully."));
        m_productModel->reload();
    });
}

void MainWindow::exportProduct()
{
    if (!selectedProduct().isValid())
        return;

    const auto fileName = QFileDialog::getExistingDirectory(this, tr("Export Product"));
    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if (!fi.exists() || !fi.isDir() || !fi.isWritable()) {
        QMessageBox::critical(this, tr("Import Failed"), tr("Could not open file."));
        return;
    }

    auto job = new ProductExportJob(selectedProduct(), fileName, m_restClient, this);
    connect(job, &Job::error, this, [this](const auto &msg) {
        QMessageBox::critical(this, tr("Export Failed"), tr("Export error: %1").arg(msg));
    });
    connect(job, &Job::finished, this, [this]() {
        logMessage(tr("Product exported successfully."));
    });
}

void MainWindow::productSelected()
{
    const auto product = selectedProduct();
    ui->surveyEditor->setProduct(product);
    ui->schemaEdit->setProduct(product);
    ui->analyticsView->setProduct(product);
    updateActions();
}

void MainWindow::logMessage(const QString& msg)
{
    ui->logWidget->appendPlainText(msg);
}

void MainWindow::logError(const QString& msg)
{
    ui->logWidget->appendHtml( QStringLiteral("<font color=\"red\">") + msg + QStringLiteral("</font>"));
}

Product MainWindow::selectedProduct() const
{
    const auto selection = ui->productListView->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return {};
    const auto idx = selection.first();
    return idx.data(ProductModel::ProductRole).value<Product>();
}

void MainWindow::updateActions()
{
    // product action state
    ui->actionAddProduct->setEnabled(m_restClient->isConnected());
    ui->actionDeleteProduct->setEnabled(selectedProduct().isValid());
    ui->actionImportProduct->setEnabled(m_restClient->isConnected());
    ui->actionExportProduct->setEnabled(selectedProduct().isValid());

    // deactivate menus of the inactive views
    ui->menuAnalytics->setEnabled(ui->viewStack->currentWidget() == ui->analyticsView);
    ui->menuSurvey->setEnabled(ui->viewStack->currentWidget() == ui->surveyEditor);
    ui->menuSchema->setEnabled(ui->viewStack->currentWidget() == ui->schemaEdit);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (ui->schemaEdit->isDirty()) {
        const auto r = QMessageBox::critical(this, tr("Unsaved Changes"),
            tr("There are unsaved changes in the schema editor. Do you want to discard them and close the application?"),
            QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
        if (r != QMessageBox::Discard) {
            event->ignore();
            return;
        }
    }
    QMainWindow::closeEvent(event);
}

#include "moc_mainwindow.cpp"
