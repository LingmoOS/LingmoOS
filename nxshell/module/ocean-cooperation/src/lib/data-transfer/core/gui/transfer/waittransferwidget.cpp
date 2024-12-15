#include "waittransferwidget.h"
#include "../type_defines.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QMovie>

#include <net/helper/transferhepler.h>

WaitTransferWidget::WaitTransferWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

WaitTransferWidget::~WaitTransferWidget()
{
}

void WaitTransferWidget::initUI()
{
    setStyleSheet(".WaitTransferWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(30);

    QLabel *titileLabel = new QLabel(tr("Waiting for transfer..."), this);
    titileLabel->setFixedHeight(50);
    StyleHelper::setAutoFont(titileLabel, 24, QFont::DemiBold);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel *tipLabel = new QLabel(tr("Please select the data to transfer on Windows"), this);
    tipLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    auto iconWidget = new MovieWidget("waiting", this);
    QHBoxLayout *iconLayout = new QHBoxLayout();
    iconLayout->addWidget(iconWidget, Qt::AlignCenter);

    ButtonLayout *buttonLayout = new ButtonLayout();
    buttonLayout->setCount(1);
    backButton = buttonLayout->getButton1();
    backButton->setText(tr("Cancel"));
#ifdef __linux__
    connect(backButton, &QPushButton::clicked, this, &WaitTransferWidget::cancel);
#endif

    IndexLabel *indelabel = new IndexLabel(2, this);
    indelabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indelabel, Qt::AlignCenter);

    mainLayout->addWidget(titileLabel);
    mainLayout->addWidget(tipLabel);
    mainLayout->addSpacing(65);
    mainLayout->addLayout(iconLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(indexLayout);
}

void WaitTransferWidget::nextPage()
{
    emit TransferHelper::instance()->changeWidget(PageName::transferringwidget);
}
void WaitTransferWidget::backPage()
{
#ifdef __linux__
    emit TransferHelper::instance()->changeWidget(PageName::connectwidget);
#endif
}

void WaitTransferWidget::themeChanged(int theme)
{
    //light
    if (theme == 1) {
        setStyleSheet(".WaitTransferWidget{background-color: white; border-radius: 10px;}");
    } else {
        //dark
        setStyleSheet(".WaitTransferWidget{background-color: rgb(37, 37, 37); border-radius: 10px;}");
    }
}

#ifdef __linux__
#    include <DDialog>
DWIDGET_USE_NAMESPACE
void WaitTransferWidget::cancel()
{
    QMainWindow *activeMainWindow = qobject_cast<QMainWindow *>(QApplication::activeWindow());
    DDialog dlg(activeMainWindow);
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(tr("Cancel"));
    dlg.addButton(tr("Close"), true, DDialog::ButtonWarning);

    dlg.setTitle(tr("This operation will clear the transmission progress, Do you want to continue."));
    dlg.setMessage(tr("This operation is not recoverable"));

    dlg.move(activeMainWindow->pos() + QPoint(185, 200));

    int code = dlg.exec();
    if (code == 1) {
        backPage();
        TransferHelper::instance()->disconnectRemote();
    }
}

#endif
