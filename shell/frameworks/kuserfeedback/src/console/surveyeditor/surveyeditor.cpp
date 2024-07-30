/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "surveyeditor.h"
#include "ui_surveyeditor.h"

#include "surveydialog.h"

#include <model/surveymodel.h>
#include <rest/restapi.h>

#include <QMessageBox>
#include <QNetworkReply>
#include <QUuid>

using namespace KUserFeedback::Console;

SurveyEditor::SurveyEditor(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::SurveyEditor),
    m_surveyModel(new SurveyModel(this))
{
    ui->setupUi(this);
    ui->surveyView->setModel(m_surveyModel);
    ui->surveyView->addActions({ ui->actionAddSurvey, ui->actionEditSurvey, ui->actionDeleteSurvey });
    connect(ui->surveyView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SurveyEditor::updateActions);
    connect(ui->surveyView, &QAbstractItemView::doubleClicked, this, &SurveyEditor::editSurvey);
    connect(m_surveyModel, &QAbstractItemModel::modelReset, this, &SurveyEditor::updateActions);

    connect(ui->actionAddSurvey, &QAction::triggered, this, &SurveyEditor::createSurvey);
    connect(ui->actionEditSurvey, &QAction::triggered, this, &SurveyEditor::editSurvey);
    connect(ui->actionDeleteSurvey, &QAction::triggered, this, &SurveyEditor::deleteSurvey);

    addActions({ ui->actionAddSurvey, ui->actionEditSurvey, ui->actionDeleteSurvey });
    updateActions();
}

SurveyEditor::~SurveyEditor() = default;

void SurveyEditor::setRESTClient(RESTClient* client)
{
    m_restClient = client;
    m_surveyModel->setRESTClient(client);
}

void SurveyEditor::setProduct(const Product& product)
{
    m_product = product;
    m_surveyModel->setProduct(product);
    updateActions();
}

void SurveyEditor::createSurvey()
{
    if (!m_product.isValid())
        return;
    SurveyDialog dlg(this);
    if (!dlg.exec())
        return;
    auto reply = RESTApi::createSurvey(m_restClient, m_product, dlg.survey());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            Q_EMIT logMessage(QString::fromUtf8(reply->readAll()));
        }
        m_surveyModel->reload();
        reply->deleteLater();
    });
}

void SurveyEditor::editSurvey()
{
    if (!m_product.isValid())
        return;
    const auto selection = ui->surveyView->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;
    const auto survey = selection.first().data(SurveyModel::SurveyRole).value<Survey>();

    SurveyDialog dlg;
    dlg.setSurvey(survey);
    if (dlg.exec() != QDialog::Accepted)
        return;

    auto reply = RESTApi::updateSurvey(m_restClient, dlg.survey());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError)
            return;
        Q_EMIT logMessage(QString::fromUtf8(reply->readAll()));
        m_surveyModel->reload();
        reply->deleteLater();
    });
}

void SurveyEditor::deleteSurvey()
{
    if (!m_product.isValid())
        return;
    const auto selection = ui->surveyView->selectionModel()->selectedRows();
    if (selection.isEmpty())
        return;

    const auto r = QMessageBox::critical(this, tr("Delete Survey"),
        tr("Do you really want to delete the selected survey?"),
        QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);
    if (r != QMessageBox::Discard)
        return;

    const auto survey = selection.first().data(SurveyModel::SurveyRole).value<Survey>();
    if (survey.uuid().isNull())
        return;
    auto reply = RESTApi::deleteSurvey(m_restClient, survey);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError)
            return;
        Q_EMIT logMessage(QString::fromUtf8(reply->readAll()));
        m_surveyModel->reload();
        reply->deleteLater();
    });
}

void SurveyEditor::updateActions()
{
    ui->actionAddSurvey->setEnabled(m_product.isValid());
    const auto hasSelection = !ui->surveyView->selectionModel()->selectedRows().isEmpty();
    ui->actionEditSurvey->setEnabled(hasSelection);
    ui->actionDeleteSurvey->setEnabled(hasSelection);
}

#include "moc_surveyeditor.cpp"
