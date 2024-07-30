/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SURVEYEDITOR_H
#define KUSERFEEDBACK_CONSOLE_SURVEYEDITOR_H

#include <core/product.h>

#include <QWidget>

#include <memory>

namespace KUserFeedback {
namespace Console {

class RESTClient;
class SurveyModel;

namespace Ui
{
class SurveyEditor;
}

class SurveyEditor : public QWidget
{
    Q_OBJECT
public:
    explicit SurveyEditor(QWidget *parent = nullptr);
    ~SurveyEditor() override;

    void setRESTClient(RESTClient *client);
    void setProduct(const Product &product);

Q_SIGNALS:
    void logMessage(const QString &msg);

private:
    void createSurvey();
    void editSurvey();
    void deleteSurvey();
    void updateActions();

private:
    std::unique_ptr<Ui::SurveyEditor> ui;
    SurveyModel *m_surveyModel;
    RESTClient *m_restClient = nullptr;
    Product m_product;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SURVEYEDITOR_H
