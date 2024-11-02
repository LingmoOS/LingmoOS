/********************************************************************************
** Form generated from reading UI file 'about-dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUT_2D_DIALOG_H
#define UI_ABOUT_2D_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_3;
    QTextEdit *briefTextedit;
    QSpacerItem *verticalSpacer_6;
    QLabel *openlinkLabel;
    QSpacerItem *verticalSpacer_5;
    QHBoxLayout *horizontalLayout_4;
    QVBoxLayout *verticalLayout_6;
    QLabel *logoLabel;
    QLabel *titleLabel;
    QPushButton *closeBtn;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *iconLabel;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_4;
    QVBoxLayout *verticalLayout_4;
    QVBoxLayout *verticalLayout;
    QLabel *nameLabel;
    QLabel *versionLabel;
    QSpacerItem *horizontalSpacer_5;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(420, 492);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AboutDialog->sizePolicy().hasHeightForWidth());
        AboutDialog->setSizePolicy(sizePolicy);
        AboutDialog->setMinimumSize(QSize(420, 324));
        AboutDialog->setMaximumSize(QSize(420, 560));
        gridLayout = new QGridLayout(AboutDialog);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(32, -1, 32, -1);
        briefTextedit = new QTextEdit(AboutDialog);
        briefTextedit->setObjectName(QString::fromUtf8("briefTextedit"));
        sizePolicy.setHeightForWidth(briefTextedit->sizePolicy().hasHeightForWidth());
        briefTextedit->setSizePolicy(sizePolicy);
        briefTextedit->setMinimumSize(QSize(0, 0));
        briefTextedit->setMaximumSize(QSize(16777215, 16777215));
        briefTextedit->setFocusPolicy(Qt::NoFocus);
        briefTextedit->setFrameShape(QFrame::NoFrame);

        verticalLayout_3->addWidget(briefTextedit);

        verticalSpacer_6 = new QSpacerItem(20, 24, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_6);

        openlinkLabel = new QLabel(AboutDialog);
        openlinkLabel->setObjectName(QString::fromUtf8("openlinkLabel"));

        verticalLayout_3->addWidget(openlinkLabel);

        verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_5);


        gridLayout->addLayout(verticalLayout_3, 3, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(8);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(4, 4, 4, 4);
        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(4, 4, -1, -1);
        logoLabel = new QLabel(AboutDialog);
        logoLabel->setObjectName(QString::fromUtf8("logoLabel"));
        logoLabel->setMinimumSize(QSize(24, 24));
        logoLabel->setMaximumSize(QSize(24, 24));

        verticalLayout_6->addWidget(logoLabel);


        horizontalLayout_4->addLayout(verticalLayout_6);

        titleLabel = new QLabel(AboutDialog);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setMinimumSize(QSize(0, 28));
        titleLabel->setMaximumSize(QSize(16777215, 24));

        horizontalLayout_4->addWidget(titleLabel);

        closeBtn = new QPushButton(AboutDialog);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));
        closeBtn->setMinimumSize(QSize(30, 30));
        closeBtn->setMaximumSize(QSize(30, 30));
        closeBtn->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_4->addWidget(closeBtn);


        gridLayout->addLayout(horizontalLayout_4, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer_2 = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(-1, 38, -1, -1);
        iconLabel = new QLabel(AboutDialog);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        iconLabel->setMinimumSize(QSize(96, 96));
        iconLabel->setMaximumSize(QSize(96, 96));
        iconLabel->setStyleSheet(QString::fromUtf8(""));
        iconLabel->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);

        verticalLayout_2->addWidget(iconLabel);


        horizontalLayout->addLayout(verticalLayout_2);

        horizontalSpacer_3 = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(-1, 0, -1, 0);
        horizontalSpacer_4 = new QSpacerItem(88, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(-1, 16, -1, 12);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(12);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        nameLabel = new QLabel(AboutDialog);
        nameLabel->setObjectName(QString::fromUtf8("nameLabel"));
        nameLabel->setMinimumSize(QSize(0, 28));
        nameLabel->setMaximumSize(QSize(16777215, 28));
        QFont font;
        font.setPointSize(11);
        nameLabel->setFont(font);
        nameLabel->setLayoutDirection(Qt::LeftToRight);
        nameLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(nameLabel);

        versionLabel = new QLabel(AboutDialog);
        versionLabel->setObjectName(QString::fromUtf8("versionLabel"));
        versionLabel->setMinimumSize(QSize(0, 24));
        versionLabel->setMaximumSize(QSize(16777215, 24));
        versionLabel->setFont(font);
        versionLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(versionLabel);


        verticalLayout_4->addLayout(verticalLayout);


        horizontalLayout_3->addLayout(verticalLayout_4);

        horizontalSpacer_5 = new QSpacerItem(88, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_5);


        gridLayout->addLayout(horizontalLayout_3, 2, 0, 1, 1);


        retranslateUi(AboutDialog);

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "Dialog", nullptr));
        briefTextedit->setHtml(QApplication::translate("AboutDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Noto Sans CJK SC'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:11pt;\"><br /></p></body></html>", nullptr));
        openlinkLabel->setText(QApplication::translate("AboutDialog", "TextLabel", nullptr));
        logoLabel->setText(QString());
        titleLabel->setText(QString());
        closeBtn->setText(QString());
        iconLabel->setText(QString());
        nameLabel->setText(QString());
        versionLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_2D_DIALOG_H
