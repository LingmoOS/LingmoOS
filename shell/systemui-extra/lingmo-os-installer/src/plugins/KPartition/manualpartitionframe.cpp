#include "manualpartitionframe.h"
#include <QGridLayout>
#include <QCheckBox>
#include <QFile>
#include <QDebug>
#include <QTextStream>

ManualPartitionFrame::ManualPartitionFrame(QWidget *parent) : QWidget(parent)
{
    addUI();
    addStyleSheet();
    initAllConnect();

}

void ManualPartitionFrame::addUI()
{
    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gridLayout);
    gridLayout->setColumnStretch(0, 2);
    gridLayout->setColumnStretch(1, 4);
    gridLayout->setColumnStretch(2, 2);
    addPartProgress();
    addTableWidget();
}
void ManualPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/ManualPartitionFrame.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void ManualPartitionFrame::initAllConnect()
{

}
//添加tabelwidget控件
void ManualPartitionFrame::addTableWidget()
{

    //测试硬盘分区数据
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"/dev/sda1"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"ext4"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"/boot"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL,new QCheckBox(),NULL));
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"9999MB"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"未知"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,""));//构造表格的单元格项

    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"/dev/sda1"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"ext4"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"/boot"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::WIDGET,"",NULL,new QCheckBox(),NULL));
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"9999MB"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,"未知"));//构造表格的单元格项
    items.append(new CBaseTableItem(CBaseTableItem::TEXT,""));//构造表格的单元格项


    QStringList header;
    //设备,类型,挂载点,格式化,大小,已用,已装系统
    header << tr("device") << tr("type") << tr("Mount point") << tr("format")
            << tr("size") << tr("used") << tr("Installed system")  ;
    tableWidget = new CBaseTableWidget(3, 7, items, true, this);//构建表格
    //隐藏tablewidget行标题
    tableWidget->setHeaderVisible(true);
    tableWidget->verticalHeader()->hide();
    //添加列标题
    tableWidget->setHeaderLabels(header, true);
    tableWidget->setMinimumSize(QSize(600,200));
    tableWidget->setColWidthRowHeight(85, 30);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //x先自适应宽度
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);     //然后设置要根据内容使用宽度的列
    //将隐藏listwidget控件显示,显示tablewidget控件
    gridLayout->addWidget(tableWidget, 2, 1, 1, 1);
    gridLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 3, 1, 1, 1);

}
void ManualPartitionFrame::addPartProgress()
{
    //QWidget *widPro = new QWidget();
    progressLbale = new ProgressLabel(this);
    progressLbale->progressBar->setObjectName(QStringLiteral("progressBar"));
    progressLbale->progressBar->setMinimumHeight(35);
    progressLbale->progressBar->setMaximumHeight(35);
    progressLbale->progressBar->setMaximum(100);

    progressLbale->progressBar->setTextVisible(false);

//    progressLbale->insetStructLabel(QPixmap(":/resources/pic/l1.png").scaled(QSize(5,5), Qt::KeepAspectRatioByExpanding), tr("idle"));
//    progressLbale->insetStructLabel(QPixmap(":/resources/pic/l1.png").scaled(QSize(5,5), Qt::KeepAspectRatioByExpanding), tr("idle"));
//    progressLbale->insetStructLabel(QPixmap(":/resources/pic/l1.png").scaled(QSize(5,5), Qt::KeepAspectRatioByExpanding), tr("idle"));

    //进度条显示要求,需要显示的数据加上显示的颜色一起搭配,同时还需要设置最大值
    progressLbale->colorList.append("rgba(255, 255, 0, 255)");
    progressLbale->progressValue.append("50");
    progressLbale->progressBar->setValue(100);
    partValueChanged();
    //添加进度条
    gridLayout->addWidget(progressLbale->widget, 0, 1, 1, 1);
    gridLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 1, 1, 1);
}
void ManualPartitionFrame::partValueChanged()
{
    progressLbale->progressBar->setStyleSheet(QString(""
        "    QProgressBar {"
        "    border: 0px solid grey;"
        "   height: 20px;"
        "    border-radius: 5px;"
        ""
        "}"

        "QProgressBar::chunk {"
            "    background-color: "
            "%1;"
        "}").arg(progressLbale->calcValue(progressLbale->progressBar->maximum())));
}


//添加高级安装中的操作按钮,并存安装,覆盖安装和自定义安装
void ManualPartitionFrame::addCustomPartTable()
{
    createPartBtn = new QPushButton();
    deletePartBtn = new QPushButton();
    modifyPartBtn = new QPushButton();
    newPartTableBtn = new QPushButton();
    reventBtn = new QPushButton();

    //设置按钮可选
    createPartBtn->setCheckable(true);
    deletePartBtn->setCheckable(true);
    modifyPartBtn->setCheckable(true);
    newPartTableBtn->setCheckable(true);
    reventBtn->setCheckable(true);

    //设置按钮样式
    createPartBtn->setFlat(true);
    createPartBtn->setAutoFillBackground(true);
    deletePartBtn->setFlat(true);
    deletePartBtn->setAutoFillBackground(true);
    modifyPartBtn->setFlat(true);
    modifyPartBtn->setAutoFillBackground(true);
    newPartTableBtn->setFlat(true);
    newPartTableBtn->setAutoFillBackground(true);
    reventBtn->setFlat(true);
    reventBtn->setAutoFillBackground(true);

    //设置按钮的objectname
    createPartBtn->setObjectName("ChooseInstallbtn");
    deletePartBtn->setObjectName("ChooseInstallbtn");
    modifyPartBtn->setObjectName("ChooseInstallbtn");
    newPartTableBtn->setObjectName("ChooseInstallbtn");
    reventBtn->setObjectName("ChooseInstallbtn");

    //按钮消息连接
    connect(createPartBtn, SIGNAL(clicked()), this, SLOT(clickCreateBtn()));
    connect(deletePartBtn, SIGNAL(clicked()), this, SLOT(ClickDeleteBtn()));
    connect(modifyPartBtn, SIGNAL(clicked()), this, SLOT(clickModifyBtn()));
    connect(newPartTableBtn, SIGNAL(clicked()), this, SLOT(ClickNewTableBtn()));
    connect(reventBtn, SIGNAL(clicked()), this, SLOT(clickRevertBtn()));

    createPartBtn->setText(tr("Create"));//创建分区
    deletePartBtn->setText(tr("Delete"));//删除分区
    modifyPartBtn->setText(tr("Modify"));//修改分区
    newPartTableBtn->setText(tr("New partition table"));//新建分区表
    reventBtn->setText(tr("revert"));//还原

    QIcon icon;
    icon.addFile(":/resources/pic/add.png", QSize(12,12));
    createPartBtn->setIcon(icon);//创建分区
    icon.addFile(":/resources/pic/reduce.png", QSize(12,12));
    deletePartBtn->setIcon(icon);//删除分区
    icon.addFile(":/resources/pic/modify.png", QSize(12,12));
    modifyPartBtn->setIcon(icon);//修改分区


    //添加按钮之间的分割线
    QHBoxLayout *hBtnLayout = new QHBoxLayout;
    installLine1 = new QFrame;
    installLine2 = new QFrame;
    installLine3 = new QFrame;

    installLine1->setObjectName(QStringLiteral("installLine1"));
    installLine2->setObjectName(QStringLiteral("installLine2"));
    installLine3->setObjectName(QStringLiteral("installLine2"));

    //设置按钮线形式
    installLine1->setFrameShape(QFrame::VLine);
    installLine2->setFrameShape(QFrame::VLine);
    installLine3->setFrameShape(QFrame::VLine);

    installLine1->setFrameShadow(QFrame::Sunken);
    installLine2->setFrameShadow(QFrame::Sunken);
    installLine3->setFrameShadow(QFrame::Sunken);

    //设置线的样式
    installLine1->setMaximumWidth(1);
    installLine1->setMaximumHeight(15);
    installLine2->setMaximumWidth(1);
    installLine2->setMaximumHeight(15);
    installLine3->setMaximumWidth(1);
    installLine3->setMaximumHeight(15);

    //添加按钮和分割线到布局
    hBtnLayout->setSpacing(5);
    hBtnLayout->addWidget(createPartBtn);
    hBtnLayout->addWidget(installLine1);
    hBtnLayout->addWidget(deletePartBtn);
    hBtnLayout->addWidget(installLine2);
    hBtnLayout->addWidget(modifyPartBtn);
    hBtnLayout->addStretch();
 //  hBtnLayout->addSpacing(100);

    QHBoxLayout *hBtnLayout1 = new QHBoxLayout;\
    hBtnLayout1->setSpacing(5);
    hBtnLayout1->addStretch();
   // hBtnLayout1->addItem(new QSpacerItem(50, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hBtnLayout1->addWidget(newPartTableBtn);
    hBtnLayout1->addWidget(installLine3);
    hBtnLayout1->addWidget(reventBtn);

    //添加布局到middlewidget主页显示
    gridLayout->addLayout(hBtnLayout, 7, 1, 1, 1);
    gridLayout->addLayout(hBtnLayout1, 7, 2, 1, 1);
}

void ManualPartitionFrame::slotCreatePartitionTable()
{

}
void ManualPartitionFrame::slotCreatePartition()
{

}
void ManualPartitionFrame::slotDeletePartition()
{

}
void ManualPartitionFrame::slotRestorePartitinTable()
{

}
