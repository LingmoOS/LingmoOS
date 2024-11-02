#include "custompartitionframe.h"
#include <QGridLayout>
#include <QCheckBox>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QListView>
#include <QScrollArea>
#include <QHBoxLayout>
#include "partman/libparted_unit.h"
#include "./delegate/custom_partition_delegate.h"
#include "./delegate/customize_device_model.h"
#include "partman/partition_server.h"
#include "../PluginService/ui_unit/xrandrobject.h"

#include "../PluginService/ui_unit/xrandrobject.h"
namespace KInstaller {
using namespace KServer;
const int tableWidth = KServer::GetScreenRect().width() * 0.6 ;
CustomPartitionFrame::CustomPartitionFrame(CustomPartitiondelegate* customDelegate, QWidget *parent) : QWidget(parent),
    m_delegate(customDelegate),
    m_createPartFrame(nullptr),
    m_modifyPartFrame(nullptr),
    m_bootstr(""),
    layout1Width(0)
{
//    m_modifyPartFrame->hide();
//    m_createPartFrame->hide();
    initUI();
    addStyleSheet();
    initAllConnect();
    translateStr();
    this->setFocusPolicy(Qt::StrongFocus);
    //this->grabKeyboard();
}

void CustomPartitionFrame::initUI()
{
    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gridLayout);
    gridLayout->setColumnStretch(0, 2);
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 2);

    QHBoxLayout* hlayout = new QHBoxLayout;
    m_bootlabel = new QLabel;
    hlayout->addWidget(m_bootlabel);

    m_bootCombox = new QComboBox;
    hlayout->addWidget(m_bootCombox);
    m_bootCombox->setView(new QListView());
    m_bootCombox->setObjectName("bootCombox");
    m_bootCombox->installEventFilter(this);
    hlayout->addStretch();
    m_BCToolTip = new QLabel(this);
    m_BCToolTip->setFixedSize(500, 40);
    m_BCToolTip->setVisible(false);

    m_revertBtn = new QPushButton;
    m_revertBtn->setObjectName("revertBtn");
    hlayout->addWidget(m_revertBtn);
    m_revertBtn->setFlat(true);
    m_revertBtn->setIcon(QIcon(":/res/pic/revert.svg"));
    hlayout->addSpacerItem(new QSpacerItem(12, 10, QSizePolicy::Fixed, QSizePolicy::Preferred));

    addTableWidget();
    addPartProgress();

    gridLayout->addWidget(tableViewWidget, 3, 1, 1, 1);
    gridLayout->addLayout(hlayout, 4, 1, 1, 1);
//    gridLayout->setRowStretch(5, 1);
}
void CustomPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/CustomPartitionFrame.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void CustomPartitionFrame::initAllConnect()
{
    connect(m_delegate, &CustomPartitiondelegate::deviceRefreshed, this, &CustomPartitionFrame::repaintDevice);
    connect(m_bootCombox, &QComboBox::currentTextChanged, this, &CustomPartitionFrame::slotSelectBootCombox);
    connect(m_revertBtn, &QPushButton::clicked, this, &CustomPartitionFrame::revertPartition);
}

//添加tabelwidget控件
void CustomPartitionFrame::addTableWidget()
{
    tableViewWidget = new QWidget;
    tableViewWidget->setObjectName("tableLayoutWidget");

    pScroll = new QScrollArea();
    pScroll->setObjectName("pScroll");
    pScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pScroll->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    pScroll->setFrameShape(QFrame::NoFrame);
    pScroll->setStyleSheet("QScrollArea{background-color:transparent;");
    pScroll->viewport()->setStyleSheet("background-color:transparent;");
    psWidget = new QWidget();
    psWidget->setObjectName("psWidget");
    pScroll->setWidget(psWidget);
    pScroll->setWidgetResizable(true);
    psWidget->adjustSize();
    pScroll->adjustSize();
//    psWidget->setMaximumHeight(500);

    QVBoxLayout* vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addWidget(pScroll);
    vlayout->addStretch();
    tableViewWidget->setLayout(vlayout);
    vtlayout = new QVBoxLayout;
    vtlayout->setSpacing(10);
}

void CustomPartitionFrame::updateScrollAreaWidget()
{
    if(psWidget->height() > 500) {
        int width = psWidget->width();
        int height = psWidget->height();
        psWidget->setMinimumSize(tableWidth, height);
        tableViewWidget->setMinimumSize(tableWidth - 5, height);
//        m_colorProgressFrame->setMinimumSize(tableWidth, height);
//        pScroll->setMinimumSize(width, height);
    }
}

void CustomPartitionFrame::newTableViews(DeviceList devs)
{
    psWidget->setLayout(vtlayout);
    vtlayout->setContentsMargins(0, 0, 0, 0);
    int widgetH = 0;
    for(int i = 0; i < devs.length(); i++) {
        //将隐藏listwidget控件显示,显示tablewidget控件
        TableWidgetView* tableView = new TableWidgetView();
        tableView->setObjectName("TableWidgetView");
        tableView->getItems(devs.at(i));
        vtlayout->addWidget(tableView, Qt::AlignTop | Qt::AlignCenter);
        vtlayout->addStretch();
        m_tableViews.append(tableView);
        tableView->tableWidget->setFixedWidth(tableWidth - 5);

        int w = tableView->tableWidget->columnWidth(0) * 9;
        int h = tableView->tableWidget->rowHeight(0) * (tableView->tableWidget->rowCount() + 3);
        widgetH += h;
//        tableView->setFixedWidth(w);
        tableView->setMinimumHeight(h);
//        tableView->setMaximumHeight(h);

        qDebug() << "signalAdd" <<connect(tableView, &TableWidgetView::signalAdd, this, &CustomPartitionFrame::slotCreatePartition);
        qDebug() << "signalChange" <<connect(tableView, &TableWidgetView::signalChange, this, &CustomPartitionFrame::slotModifyPartition);
        qDebug() << "signalDelete" <<connect(tableView, &TableWidgetView::signalDelete, this, &CustomPartitionFrame::slotDeletePartition);
        qDebug() << "signalCreatorTable" <<connect(tableView, &TableWidgetView::signalCreateTable, [=](Device::Ptr dev){emit signalCreatePartTable(dev);});

        connect(tableView, &TableWidgetView::signalSelectTable, this, &CustomPartitionFrame::updateTableView);
        tableView->adjustSize();
    }
    vtlayout->addStretch();
    vtlayout->addSpacerItem(new QSpacerItem(10, 25, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tableViewWidget->adjustSize();
}

void CustomPartitionFrame::updateTableView(TableWidgetView* tablewidget)
{
    for(TableWidgetView* table : m_tableViews) {
        if(table != tablewidget){
            for(int i = 0; i < table->tableWidget->rowCount(); i++) {
                table->tableWidget->clearSelection();
//                table->updateTableShow();
            }
        }
    }
    m_colorProgressFrame->setDevice(tablewidget->m_dev);
}

void CustomPartitionFrame::addPartProgress()
{
    m_colorProgressFrame = new DiskPartitionColorProgress();
    gridLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    gridLayout->addWidget(m_colorProgressFrame, 1, 1, 1, 1);
    gridLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);
}

void CustomPartitionFrame::slotCreatePartition(Device::Ptr dev, Partition::Ptr part)
{
    qDebug() << Q_FUNC_INFO << dev << part->m_devPath;
    // slotCreatePartition m_blCreatorPartition=true;

    QString dev_path = part->m_devPath;
    if(!m_delegate->isPartitionTableMatchDe(dev_path)) {
        qDebug() << "isPartitionTableMatchDe is false;";
        return;
    }
    bool bl = true;
    if (m_delegate->canAddPrimary(part)) {
        bl = true;
    } else if(!m_delegate->canAddPrimary(part) && m_delegate->canAddLogical(part)) {
        bl = false;
    }else {
        qDebug() << GetPartTableNameByType(dev->m_partTableType);
        return;
    }
    m_createPartFrame = new CreatePartitionFrame(m_delegate,&devlist,m_pFrame);
//    m_createPartFrame->m_SCreenWidth=m_SCreenWidth;
//    m_createPartFrame->m_SCreenHeight=m_SCreenHeight;
    connect(m_createPartFrame, &CreatePartitionFrame::finished, [=]{repaintDevice();m_createPartFrame->close();/*this->raise();activateWindow();*/});
    m_createPartFrame->setNewPartition(dev, part, bl, m_SCreenWidth, m_SCreenHeight);
    m_createPartFrame->show();
}

void CustomPartitionFrame::slotDeletePartition(Device::Ptr dev, Partition::Ptr part)
{
    qDebug() << "slotDeletePartition" ;
    if(part) {
        m_delegate->deletePartition(part);
        m_delegate->refreshShow();
        repaintDevice();
    }
}

void CustomPartitionFrame::slotRestorePartition()
{
    qDebug() << "slotRestorePartitin" ;
}

void CustomPartitionFrame::slotModifyPartition(Device::Ptr dev, Partition::Ptr part)
{
    qDebug() << "slotModifyPartition" ;
    QString dev_path = part->m_devPath;
    if(!m_delegate->isPartitionTableMatchDe(dev_path)) {
        qDebug() << "isPartitionTableMatchDe is false;";
        return;
    } else {
        m_modifyPartFrame = new ModifyPartitionFrame(m_delegate,m_pFrame);
        m_modifyPartFrame->devlst=&devlist;//把硬盘列表传过去
//        m_modifyPartFrame->m_SCreenWidth=m_SCreenWidth;
//        m_modifyPartFrame->m_SCreenHeight=m_SCreenHeight;
        connect(m_modifyPartFrame, &ModifyPartitionFrame::finished, [=]{repaintDevice();m_modifyPartFrame->close(); /*this->raise();activateWindow();*/});
        m_modifyPartFrame->setModifyPartition(part, m_SCreenWidth, m_SCreenHeight);
        m_modifyPartFrame->show();
    }
}

void CustomPartitionFrame::repaintDevice()
{
    qDebug() << Q_FUNC_INFO ;
    devlist = m_delegate->virtualDevices();
    for (QLayoutItem* item = vtlayout->takeAt(0);
          item != nullptr;
          item = vtlayout->takeAt(0)) {
       delete item->widget();
       delete item;
       item = nullptr;
     }
    m_tableViews.clear();
    newTableViews(devlist);
    if(devlist.count() > 0) {
        m_colorProgressFrame->setDevice(devlist.at(0));
    }
    if(m_bootCombox->count() <= 0) {
        int i = 0;
        for(Device::Ptr dev : devlist) {
////            m_bootCombox->addItem(QString("%1 %2").arg(dev->m_path).arg(dev->m_model));
            QString strRealData(QString("%1 %2").arg(dev->m_path).arg(dev->m_model));
            QString strShow = m_bootCombox->view()->fontMetrics().elidedText(strRealData,Qt::ElideRight,300-6-2-9-10);
            m_bootCombox->addItem(strShow, strRealData);
            qDebug() << "Device Name Show:"<< strShow <<"Device Name:"<<strRealData;
        }
        m_bootCombox->setCurrentIndex(0);
    }
    updateScrollAreaWidget();

    update();
    qDebug() << Q_FUNC_INFO<< "signalFinishedLoadDisk1" ;
    emit signalFinishedLoadDisk();
}

void CustomPartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
        this->repaintDevice();
    } else {
        QWidget::changeEvent(event);
    }
}

void CustomPartitionFrame::translateStr()
{
    m_bootlabel->setText(tr("Device for boot loader path:"));//引导加载程序安装设备：
    m_revertBtn->setText(tr("Revert"));//还原
}

void CustomPartitionFrame::slotSelectBootCombox(QString bootstr)
{
    QString currentData = m_bootCombox->currentData().toString();//获取完整名称
    m_delegate->setBootLoaderPath(currentData);
    m_bootstr = currentData;

    //设置悬浮提示
    if(currentData != m_bootCombox->currentText()){
        m_BCToolTip->setText(currentData);
    }else{
        m_BCToolTip->clear();
    }
}

void CustomPartitionFrame::revertPartition()
{
    m_delegate->resetOperations();
    repaintDevice();
}

void CustomPartitionFrame::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << Q_FUNC_INFO << "keyPressEvent";
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        qDebug()<< "Custom partition back to main partition";
        emit backspacepressed();
    }
    else if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
        ;
    else
    {
        QWidget::keyPressEvent(event);
    }
}

bool CustomPartitionFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "bootCombox"){
        if(event->type() == QEvent::Enter){
            if(!m_BCToolTip->text().isEmpty()){
                m_BCToolTip->move(m_bootCombox->x()+22, m_bootCombox->y()-m_bootCombox->height());
                m_BCToolTip->setVisible(true);
            }
        } else if(event->type() == QEvent::Leave){
            if(!m_BCToolTip->text().isEmpty()){
                m_BCToolTip->setVisible(false);
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

//void CustomPartitionFrame::setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight)
//{
//    m_pFrame = pFrame;
//    m_SCreenWidth = SCreenWidth;
//    m_SCreenHeight= SCreenHeight;
//}

}
