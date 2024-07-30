/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <config-userfeedback.h>

#include "surveydialog.h"
#include "ui_surveydialog.h"


#include <common/surveytargetexpressionparser.h>

#include <QIcon>
#include <QPushButton>
#include <QUrl>
#include <QUuid>

using namespace KUserFeedback::Console;

SurveyDialog::SurveyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SurveyDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-question")));

    connect(ui->buttonBox->button(QDialogButtonBox::Discard), &QPushButton::clicked, this, &QDialog::reject);

    connect(ui->name, &QLineEdit::textChanged, this, &SurveyDialog::updateState);
    connect(ui->url, &QLineEdit::textChanged, this, &SurveyDialog::updateState);
    connect(ui->targetExpression, &QPlainTextEdit::textChanged, this, &SurveyDialog::updateState);
    updateState();
}

SurveyDialog::~SurveyDialog() = default;

Survey SurveyDialog::survey() const
{
    Survey s = m_survey;
    s.setName(ui->name->text());
    s.setUrl(QUrl(ui->url->text()));
    s.setTarget(ui->targetExpression->toPlainText());
    if (s.uuid().isNull())
        s.setUuid(QUuid::createUuid());
    return s;
}

void SurveyDialog::setSurvey(const Survey& survey)
{
    m_survey = survey;
    ui->name->setText(survey.name());
    ui->url->setText(survey.url().toString());
    ui->targetExpression->setPlainText(survey.target());
    updateState();
}

void SurveyDialog::updateState()
{
    bool valid = true;
    valid &= !ui->name->text().isEmpty();
    const auto url = QUrl(ui->url->text());
    valid &= url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty();

#ifdef HAVE_SURVEY_TARGET_EXPRESSIONS
    if (!ui->targetExpression->toPlainText().isEmpty()) {
        SurveyTargetExpressionParser p;
        valid &= p.parse(ui->targetExpression->toPlainText());
    }
#endif

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(valid);
}

#include "moc_surveydialog.cpp"
