/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QApplication>
#include <ktitlewidget.h>

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class KTitleWidgetTestWidget : public QWidget
{
public:
    KTitleWidgetTestWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        KTitleWidget *titleWidget = new KTitleWidget(this);
        titleWidget->setText(QStringLiteral("Title"));
        titleWidget->setIconSize(QSize(22, 22));
        titleWidget->setIcon(QIcon::fromTheme(QStringLiteral("kde")), KTitleWidget::ImageLeft);

        mainLayout->addWidget(titleWidget);

        KTitleWidget *errorTitle = new KTitleWidget(this);
        errorTitle->setText(QStringLiteral("Title"));
        errorTitle->setComment(QStringLiteral("Error Comment"), KTitleWidget::ErrorMessage);

        mainLayout->addWidget(errorTitle);

        KTitleWidget *checkboxTitleWidget = new KTitleWidget(this);

        QWidget *checkBoxTitleMainWidget = new QWidget(this);
        QVBoxLayout *titleLayout = new QVBoxLayout(checkBoxTitleMainWidget);
        titleLayout->setContentsMargins(6, 6, 6, 6);

        QCheckBox *checkBox = new QCheckBox(QStringLiteral("Text Checkbox"), checkBoxTitleMainWidget);
        titleLayout->addWidget(checkBox);
        checkboxTitleWidget->setWidget(checkBoxTitleMainWidget);

        mainLayout->addWidget(checkboxTitleWidget);

        QLabel *otherLabel = new QLabel(QStringLiteral("Some text..."), this);

        mainLayout->addWidget(otherLabel);

        mainLayout->addStretch();
    }
};

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("ktitlewidgettest"));
    QApplication app(argc, argv);

    KTitleWidgetTestWidget *mainWidget = new KTitleWidgetTestWidget;
    mainWidget->show();

    return app.exec();
}
