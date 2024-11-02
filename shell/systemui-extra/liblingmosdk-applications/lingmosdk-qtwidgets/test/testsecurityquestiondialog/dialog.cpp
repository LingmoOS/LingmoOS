#include "dialog.h"
#include <QDebug>

#include "ksecurityquestiondialog.h"
using namespace kdk;

Dialog::Dialog()
{
    KSecurityQuestionDialog *questionDialog =  new KSecurityQuestionDialog();
    questionDialog->setWindowIcon("lingmo-control-center");
    questionDialog->setWindowTitle("Security Question");
    questionDialog->setTitleText(tr("Answer the security questions and reset your password"));
    questionDialog->addSecurityQuestionItem(3, true);
    QStringList AA;
    AA << "111" << "222" << "333" << "444";
    questionDialog->initQustionCombox(AA);

    if (questionDialog->answerLabel(1) != nullptr) {
        questionDialog->answerLabel(1)->setText("answerLabel1");
        qDebug() << questionDialog->answerLabel(1)->text();
    }

    if (questionDialog->questionLabel(1) != nullptr) {
        questionDialog->questionLabel(1)->setText("test111");
        qDebug() << questionDialog->questionLabel(1)->text();
    }

    if (questionDialog->answerLineedit(1) != nullptr) {
        qDebug() << questionDialog->answerLineedit(1)->text();
    }

    if (questionDialog->questionCombox(1) != nullptr) {
        qDebug() << questionDialog->questionCombox(1)->currentText();
    }

    if (questionDialog->tipsLabel(1) != nullptr) {
        questionDialog->tipsLabel(1)->setText("Incorrect answer");
        qDebug() << questionDialog->tipsLabel(1)->text();
    }

    connect(questionDialog->cancelButton(), &KPushButton::clicked, this, [=](){
        questionDialog->close();
    });

    questionDialog->show();
}
