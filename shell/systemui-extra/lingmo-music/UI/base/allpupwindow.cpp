/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "allpupwindow.h"
#include "widgetstyle.h"
#include "UI/mainwidget.h"
#include "xatom-helper.h"
#include <lingmo-log4qt.h>

#define PT_12 12
#define PT_11 11

QRegExp gInvalidName("[\\\\/:\\*\\?\\\"&<>\\|]");/* 文件名或文件夹名中不能出现以下字符：\、/、:、*、?、"、&、<、>、|  */

AllPupWindow::AllPupWindow(QWidget *parent) : QDialog(parent)
{
    inpupdialog();
    dlgcolor();
}

void AllPupWindow::closeDialog()
{
    this->close();
    enterLineEdit->clear();
}

void AllPupWindow::slotTextChanged(QString text)
{
    KyInfo() << "text = " << text;
    if (!nameIsValid(text) && text != "") {
        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Could not contain characters: \ / : * ? \" & < > |"), QMessageBox::Ok);

        enterLineEdit->cursorBackward(true, 1);
        enterLineEdit->del();
    }

    enterLineEdit->setLabelNumber(15 - enterLineEdit->text().length());
    if (text.length() == 15) {
        tips->setText(tr("Reached upper character limit"));
        tips->setStyleSheet("QLabel{color:#F44E50;}");
    } else {
        tips->setText("");
    }
}

void AllPupWindow::inpupdialog()
{
    this->setWindowTitle(tr("Music Player"));
    this->setFixedSize(376, 222);
//    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

//    MotifWmHints hints;
//    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
//    hints.functions = MWM_FUNC_ALL;
//    hints.decorations = MWM_DECOR_BORDER;
//    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    this->setWindowModality(Qt::ApplicationModal); //弹出自定义对话框时主界面不可操作
//    pupDialog->setModal(true);
    testLayout = new QVBoxLayout();
    this->setLayout(testLayout);
    titleLayout = new QHBoxLayout();
    titleLab = new QLabel(this);
    closeBtn = new QToolButton(this);
    closeBtn->setFixedSize(30,30);
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setAutoRaise(true);
    closeBtn->hide();

//    titleLab->setText("提示信息");
    titleLab->setText(tr("Prompt information"));
    titleLab->setFixedHeight(28);

    titleLayout->addWidget(titleLab,Qt::AlignLeft);
    titleLayout->addWidget(closeBtn,0,Qt::AlignRight);

    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);

    enterLineEdit = new LabEdit;
    enterLineEdit->setFixedSize(312, 40);
    enterLineEdit->setMaxLength(15);

    tips = new QLabel(this);
    tips->setFixedHeight(28);


    confirmBtn = new QPushButton(this);

    cancelBtn = new QPushButton(this);

    btnLayout = new QHBoxLayout();

    stackWid = new QStackedWidget();
    stackWid->addWidget(enterLineEdit);

    stackWid->setFixedSize(312,40);
//    confirmBtn->setText("确认");
    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setFixedSize(105,36);
    confirmBtn->setProperty("isImportant", true);
//    cancelBtn->setText("取消");
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setFixedSize(105,36);

    btnLayout->addWidget(cancelBtn, 0, Qt::AlignRight);
    btnLayout->addWidget(confirmBtn, Qt::AlignRight);
    btnLayout->setSpacing(20);
    btnLayout->setMargin(0);

    enterLineEdit->setPlaceholderText("请输入歌单标题：");
    enterLineEdit->setPlaceholderText(tr("Please input playlist name:"));

    testLayout->setMargin(0);
    testLayout->setSpacing(0);
    testLayout->addLayout(titleLayout);
    testLayout->addSpacing(16);
    testLayout->addWidget(stackWid);
    testLayout->addSpacing(2);
    testLayout->addWidget(tips);
    testLayout->addSpacing(14);
    testLayout->addLayout(btnLayout);
    testLayout->setContentsMargins(32, 16, 32, 24);

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

//    connect(addSongListBtn, SIGNAL(clicked(bool)), this, SLOT(addSongList()));
    connect(enterLineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotTextChanged(QString)));
    connect(closeBtn,SIGNAL(clicked(bool)),this,SLOT(closeDialog()));
    connect(cancelBtn,SIGNAL(clicked(bool)),this,SLOT(closeDialog()));
    connect(enterLineEdit, &QLineEdit::textChanged, this, &AllPupWindow::checkInput);

    //限制应用内字体固定大小
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    confirmBtn->setFont(sizeFont);
//    cancelBtn->setFont(sizeFont);
//    titleLab->setFont(sizeFont);
//    enterLineEdit->setFont(sizeFont);

}
void AllPupWindow::checkInput()
{
    if (enterLineEdit->text().isEmpty()) {
        confirmBtn->setDisabled(true);
    } else {
        confirmBtn->setDisabled(false);
    }
}

void AllPupWindow::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_11;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    tips->setFont(font);
}

void AllPupWindow::dlgcolor()
{
    if(WidgetStyle::themeColor == 1 )
    {
//        cancelBtn->setStyleSheet("PushButton{border-radius:4px;border:1px solid #DDDFE7;color:#606265;}"
//                                 "QPushButton::hover{background:#4D4D4D;}");
//        cancelBtn->setStyleSheet("QPushButton{border-radius:4px;border:1px solid #DDDFE7;color:#606265;}"
//                                 "QPushButton::hover{background:#4D4D4D;}");

//        enterLineEdit->setStyleSheet("width:392px;height:32px;border-radius:4px;\
//                                     border: 1px solid #606265;color:#F9F9F9;background:#3D3D41;");

//        pupDialog->setStyleSheet("background-color:#3D3D41;");

//        confirmBtn->setStyleSheet("QPushButton{background:#3790FA;border-radius:4px;color:#FFFFFF;}"
//                                  "QPushButton::hover{background:#40A9FB;}"
//                                  "QPushButton::pressed{background:#296CD9;}"
//                                  );

//        titleLab->setStyleSheet("width:80px;height:14px;\
//                                font-weight: 600;\
//                                border:none;\
//                                color: #F9F9F9;\
//                                line-height:14px;");
    }
    else if(WidgetStyle::themeColor == 0)
    {
//        cancelBtn->setStyleSheet("QPushButton{border-radius:4px;border:1px solid #DDDFE7;color:#8F9399;}"
//                                 "QPushButton::hover{background:#F2F6FD;}");

//        enterLineEdit->setStyleSheet("width:392px;height:32px;border-radius:4px;border:1px solid #DDDFE7;");

//        pupDialog->setStyleSheet("background-color:#FFFFFF;");

//        confirmBtn->setStyleSheet("QPushButton{background:#3790FA;border-radius:4px;color:#FFFFFF;}"
//                                  "QPushButton::hover{background:#40A9FB;}"
//                                  "QPushButton::pressed{background:#296CD9;}"
//                                  );

//        titleLab->setStyleSheet("width:80px;height:14px;\
//                                font-weight: 600;\
//                                border:none;\
//                                color: #1B1B1B;\
//                                line-height:14px;");
    }
}

bool AllPupWindow::nameIsValid(QString textName)
{
    if (nullptr == textName) {
        return false;
    }

    return !textName.contains(gInvalidName);
}


MusicInfoDialog::MusicInfoDialog(QWidget *parent,musicDataStruct date)
       :QDialog(parent)
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setWindowTitle(tr("Music Player"));
    musicDate = date;
    initStyle();
}

MusicInfoDialog::~MusicInfoDialog()
{

}

//void MusicInfoDialog::setMusicDataStruct(musicDataStruct data)
//{
//    musicDate = data;

//    if(musicDate.title != "")
//    {
//        songNameLabel->setText(tr("Song Name : ") + musicDate.title);
//        singerLabel  ->setText(tr("Singer : ") + musicDate.singer);
//        albumLabel   ->setText(tr("Album : ") + musicDate.album);
//        fileTypeLabel->setText(tr("File Type : ") + musicDate.filetype);
//        fileSizeLable->setText(tr("File Size : ") + musicDate.size);
//        fileTimeLabel->setText(tr("File Time : ") + musicDate.time);
//        filePathLabel->setText(tr("File Path : ") + musicDate.filepath);
//}
//}


QString MusicInfoDialog::feed(QString str,int maxWidth)
{

        QString strText = str;
        int AutoIndex =1;
        QFont font ;
        font.setPointSize(13);
        QFontMetrics fm(font);
        qDebug()<<mainWig->width();
        if(!strText.isEmpty()){
            for(int i =1;i<strText.size()+1;i++){
                if(fm.width(strText.left(i))>(maxWidth +80)*AutoIndex){
                    qDebug()<<"111";
                    strText.insert(i - 1,"\n");
                    AutoIndex++;
                }
            }
        }
        return strText;
}
void MusicInfoDialog::initStyle()
{
    this->setFixedSize(376,405);
    mainVLayout = new QVBoxLayout(this);
    songNameLabel = new QLabel(this);
    singerLabel = new QLabel(this);
    albumLabel = new QLabel(this);
    filePathLabel = new QLabel(this);
    fileTypeLabel = new QLabel(this);
    fileTimeLabel = new QLabel(this);
    fileSizeLable = new QLabel(this);
    musicInfoLabel = new QLabel(this);
    musicInfoLabel->setText(tr("  Music Information"));
    musicInfoLabel->setFixedHeight(24);

    QFont font;
    font.setPointSize(13);
    songNameLabel->setFixedHeight(28);
    singerLabel->setFixedHeight(28);
    albumLabel->setFixedHeight(28);
    fileTypeLabel->setFixedHeight(28);
    fileSizeLable->setFixedHeight(28);
    fileTimeLabel->setFixedHeight(28);
    filePathLabel->setFixedHeight(28);
    filePathLabel->setWordWrap(true);
    songNameLabel->setText(tr("Song Name : "));
    singerLabel->setText(tr("Singer : "));
    albumLabel->setText(tr("Album : "));
    fileTypeLabel->setText(tr("File Type : "));
    fileSizeLable->setText(tr("File Size : "));
    fileTimeLabel->setText(tr("File Time : "));
    filePathLabel->setText(tr("File Path : "));
    filePathLabel->setAlignment(Qt::AlignTop);
//    filePathLabel->setToolTip(musicDate.filepath);
    //filePathLabel->setWordWrap(true); 这句很重要
   // filePathLabel->setAlignment(Qt::AlignLeft);

    buttonLayout = new QHBoxLayout();
    cancelBtn = new QPushButton(this);
    confirmBtn = new QPushButton(this);
    btnWidget = new QWidget(this);
    mainFormLayout = new QFormLayout(this);
    //main = new QWidget(this);
   // mainFormLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    mainWig = new QWidget(this);
    mainWig->setLayout(mainFormLayout);
    //mainFormLayout->addWidget(songNameLabel,0,0);
    songNameLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
//    mainFormLayout->setAlignment(main,Qt::AlignTop);
    mainFormLayout->setWidget(0,QFormLayout::LabelRole,songNameLabel);
    //mainFormLayout->setSpacing( 5);
    QLabel *name = new QLabel(this);
    name->setWordWrap(true);
    QString m_name = feed(musicDate.title,mainWig->width());
    qDebug()<<"musicDate,title="<<musicDate.title;
    name->setText(m_name);
    name->setToolTip(musicDate.title);
    name->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    MyLabel *singer = new MyLabel(musicDate.singer);
    singer->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    MyLabel *album = new MyLabel(musicDate.album);
    album->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    MyLabel *filetype = new MyLabel(musicDate.filetype);
    filetype->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    MyLabel *size = new MyLabel(musicDate.size);
    size->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    MyLabel *time = new MyLabel(musicDate.time);
    time->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    mainFormLayout->setWidget(0,QFormLayout::FieldRole,name);
    mainFormLayout->setWidget(1,QFormLayout::LabelRole,singerLabel);
    mainFormLayout->setWidget(1,QFormLayout::FieldRole,singer);
    mainFormLayout->setWidget(2,QFormLayout::LabelRole,albumLabel);
    mainFormLayout->setWidget(2,QFormLayout::FieldRole,album);
    mainFormLayout->setWidget(3,QFormLayout::LabelRole,fileTypeLabel);
    mainFormLayout->setWidget(3,QFormLayout::FieldRole,filetype);
    mainFormLayout->setWidget(4,QFormLayout::LabelRole,fileSizeLable);
    mainFormLayout->setWidget(4,QFormLayout::FieldRole,size);
    mainFormLayout->setWidget(5,QFormLayout::LabelRole,fileTimeLabel);
    mainFormLayout->setWidget(5,QFormLayout::FieldRole,time);
    mainFormLayout->setWidget(6,QFormLayout::LabelRole,filePathLabel);
    QLabel *pathLabel = new QLabel(this);
    //pathLabel->setAlignment(Qt::AlignTop);
   // QString clean = musicDate.filepath.replace("/","/3");
    QString m_path = feed(musicDate.filepath,mainWig->width());

    pathLabel->setText(m_path);
    pathLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    //pathLabel->setAlignment();
    qDebug()<<"路径是"<<m_path;
    pathLabel->setToolTip(musicDate.filepath);
    pathLabel->setWordWrap(true);
    mainFormLayout->setWidget(6,QFormLayout::FieldRole,pathLabel);
    mainFormLayout->setSpacing(3);


    connect(cancelBtn,&QPushButton::clicked,this,&MusicInfoDialog::close);
    connect(confirmBtn,&QPushButton::clicked,this,&MusicInfoDialog::close);



    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setFixedSize(105,36);
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setFixedSize(105,36);
    confirmBtn->setProperty("isImportant", true);

    btnWidget->setLayout(buttonLayout);

    buttonLayout->setAlignment(Qt::AlignRight);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(confirmBtn);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(16);

    //QWidget *containerWidget = mainWig;
    qDebug()<<mainWig->height()<<this->height();

    mainVLayout->setAlignment(Qt::AlignTop);
    mainVLayout->addWidget(musicInfoLabel);
    mainVLayout->addSpacing(5);

    QFontMetrics m(font);
    qDebug()<<"宽度是"<<m.width(musicDate.filepath);
    if(m.width(musicDate.filepath) >= 360){ //360的宽度是正好不用加上滚动条的宽度，加上判断滚动条是否存在
        QScrollArea *filePathScrollArea = new QScrollArea(this);
        filePathScrollArea->setWidgetResizable(true);
        filePathScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        filePathScrollArea->setWidget(mainWig);
        filePathScrollArea->setFrameShape(QFrame::NoFrame);
        filePathScrollArea->setFrameShadow(QFrame::Plain);
        filePathScrollArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        mainVLayout->addWidget(filePathScrollArea);
    }else{
        mainVLayout->addWidget(mainWig);
    }

    mainVLayout->addSpacing(30);
    mainVLayout->addWidget(btnWidget);
    //mainVLayout->addWidget(filePathScrollArea);

    mainVLayout->setSpacing(6);
    mainVLayout->setContentsMargins(20,20,20,15); // 为了和主题qmessagebox间距一致，而不参照UI图间距
    this->setLayout(mainVLayout);
}

void MusicInfoDialog::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    musicInfoLabel->setFont(font);
}
