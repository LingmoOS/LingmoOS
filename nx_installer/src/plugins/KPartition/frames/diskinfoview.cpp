#include "diskinfoview.h"
#include <QMouseEvent>
#include <QTextStream>
#include <QEvent>
#include <QPainter>
#include "../PluginService/ui_unit/xrandrobject.h"
#include "QDebug"
using namespace KServer;

DiskInfoView::DiskInfoView(QWidget *parent) : QWidget(parent)
  ,checkable(true)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0,0,0,0));
    setPalette(pal);
    this->setObjectName(QStringLiteral("DiskInfoView"));
    this->installEventFilter(this);
    this->setAttribute(Qt::WA_Hover,true);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(1);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("widget"));

    widget->installEventFilter(this);
    widget->setAttribute(Qt::WA_Hover,true);

    layoutWidget = new QWidget(widget);

    layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
    verticalLayout = new QVBoxLayout(layoutWidget);

    verticalLayout->setAlignment(Qt::AlignCenter);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    diskType = new QLabel(layoutWidget);
    diskType->setObjectName(QStringLiteral("diskType"));
    diskType->setAlignment(Qt::AlignCenter);

    verticalLayout->addWidget(diskType, Qt::AlignCenter);

    diskPath = new QLabel(layoutWidget);
    diskPath->setObjectName(QStringLiteral("devPath"));
    diskPath->setAlignment(Qt::AlignCenter);
    verticalLayout->addWidget(diskPath, Qt::AlignCenter);

    diskName = new QLabel(layoutWidget);
    diskName->setObjectName(QStringLiteral("diskName"));
    diskName->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    diskName->setWordWrap(true);
    verticalLayout->addWidget(diskName, Qt::AlignHCenter | Qt::AlignTop);
//    verticalLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));

    progressBar = new QProgressBar(layoutWidget);
    progressBar->setObjectName(QStringLiteral("progressBar"));
    progressBar->setValue(24);
    progressBar->setTextVisible(false);
    progressBar->setAlignment(Qt::AlignHCenter);
//    progressBar->setMaximumHeight(5);

    verticalLayout->addWidget(progressBar, Qt::AlignHCenter);

    diskSize = new QLabel(layoutWidget);
    diskSize->setObjectName(QStringLiteral("diskSize"));
//    diskSize->setText("label_4");
    diskSize->setAlignment(Qt::AlignCenter);
    verticalLayout->addWidget(diskSize, Qt::AlignCenter);
//    label_4->setStyleSheet(QStringLiteral("font-size:14px;color:white;background-color:transparent;"));

    hardDiskPng = new QLabel(this);
    hardDiskPng->setObjectName(QStringLiteral("hardDiskPng"));
    hardDiskPng->setPixmap(QPixmap(":/res/pic/drive-harddisk-system.png").scaled(QSize(84, 84)));

    checkpic = new QLabel(widget);
    checkpic->setAttribute(Qt::WA_TranslucentBackground, true);
    checkpic->setObjectName("checkpic");
    checkpic->setAlignment(Qt::AlignRight);
    checkpic->setPixmap(QPixmap(":/res/pic/checkpic.svg"));
    checkpic->setVisible(false);


   // this->adjustSize();

    this->resize(200, 280);

    dataSel=new QCheckBox(this);
    dataSel->setObjectName("dataSel");
    dataSel->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);
#ifdef DATASEL


   // contex->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    QSize m=dataSel->size();
     dataSel->move((this->size().width()-m.width())/2,this->size().height()-(2*m.height()));
    dataSel->show();
    dataSel->setChecked(false);
     dataSel->setFocus();
#else
    dataSel->hide();
    dataSel->setDisabled(true);
    dataSel->setVisible(false);

#endif
    addStyleSheet();
    initAllConnect();

translateStr();

}
void DiskInfoView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    /* 0x20为透明层颜色，可自定义设置为0x0到0xff */
    painter.fillRect(this->rect(), QColor(0, 0, 0, 0));
}
void DiskInfoView::initAllConnect()
{

    connect(this, &DiskInfoView::signalClicked, this, &DiskInfoView::slotFrameClicked);

    #ifdef DATASEL
    connect(dataSel,&QCheckBox::clicked,this,&DiskInfoView::clickDataSel);
    #endif
}


void DiskInfoView::slotFrameClicked()
{
    this->setWindowOpacity(1);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

void DiskInfoView::clickDataSel()
{
    emit signalClickView(mIndex);
}
bool DiskInfoView::eventFilter(QObject *object, QEvent *event)
{
    if(object->objectName() == "DiskInfoView") {
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseevent = (QMouseEvent*)event;
            if(mouseevent->button() == Qt::LeftButton) {
               //如果选择为数据盘则点击无效
                 if(dataSel->isChecked()==false)
                    emit signalClicked(devPath);
                return true;
            }
        }
        if(event->type() == QEvent::HoverMove) {
            widget->setStyleSheet(" #widget:hover{background-color: rgba(255, 255, 255, 0.3);}");
               return true;
           }
               if(event->type() == QEvent::HoverLeave) {

       //            widget->setStyleSheet(QLatin1String("background-color: rgba(0, 0, 0, 0.15);"));
                   return true;
               }


    }
   return QWidget::eventFilter(object,event);

}
void DiskInfoView::addStyleSheet()
{
    QFile file(":/res/qss/DiskInfoView.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = "";
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void DiskInfoView::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

QSize DiskInfoView::getFramesize()
{
    return this->size();
}

void DiskInfoView::translateStr()
{
    #ifdef DATASEL
  dataSel->setText(tr("as data disk"));
    #endif
}

void DiskInfoView::setWidgetDisabled()
{
    this->setEnabled(false);
    widget->setDisabled(true);
    progressBar->setDisabled(true);
}

void DiskInfoView::setProgressStyle()
{
    progressBar->setStyleSheet("QProgressBar#progressBar::chunk {"
                               "max-width:140px;min-width:140px;max-height:6px;min-height:6px;border:none;border-radius:3px;background-color:rgba(251, 145, 64, 0.4);}");
}

