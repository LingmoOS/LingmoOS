/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SURVEYDIALOG_H
#define KUSERFEEDBACK_CONSOLE_SURVEYDIALOG_H

#include <core/survey.h>

#include <QDialog>

#include <memory>

class QUrl;

namespace KUserFeedback {
namespace Console {

namespace Ui
{
class SurveyDialog;
}

class SurveyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SurveyDialog(QWidget *parent = nullptr);
    ~SurveyDialog() override;

    Survey survey() const;
    void setSurvey(const Survey &survey);

private:
    void updateState();

    Survey m_survey;
    std::unique_ptr<Ui::SurveyDialog> ui;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SURVEYDIALOG_H
