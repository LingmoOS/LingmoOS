/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QBitmap>
#include <QPalette>
#include <QPainter>
#include <QTextList>
//#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QImageReader>

#include <KWindowSystem>
// Necessary for closeApplication()
#include <NETWM>

#include "widget.h"
#include "ui_widget.h"
#include "editPage.h"
#include "ui_editPage.h"
#include "windowmanage.hpp"
#include "information_collector.h"
#include <lingmosdk/diagnosetest/libkydatacollect.h>


EditPage::EditPage(Widget *page, int noteId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Edit_page),
    m_editColor(SelectColor::LINGMO_BLUE),
    m_noteId(noteId),
    m_lastBlockList(0),
    m_notebook(page),
    m_setSizePage(new SetFontSize(m_notebook)),
    m_isFullscreen(false),
    m_isInsImg(false),
    m_windowId("")
{
    ui->setupUi(this);
    initSetup();
    slotsSetup();
    installEventFilter(this);

    setMinimumSize(390, 364);
    resize(minimumSize());

    handleBtnSlot(SelectColor::LINGMO_BLUE);
    handleTabletMode(LingmoUISettingsMonitor::instance().tabletMode());
}

EditPage::~EditPage()
{
    delete ui;
    m_lstUrls.clear();
}

void EditPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRect(this->rect());
    p.fillPath(rectPath, palette().color(QPalette::Base));
}

bool EditPage::eventFilter(QObject *obj,QEvent *event)
{
    if (obj == this) {
        if (event->type() == QEvent::WindowActivate) {
            m_noteHead->hide();
            m_noteHeadMenu->show();
        }
        if (event->type() == QEvent::WindowDeactivate) {
            m_noteHead->show();
            m_noteHeadMenu->hide();
        }
    }
    return false;
}

void EditPage::initSetup()
{
    // 标题
    this->setWindowTitle(tr("Notes"));


#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    // 高分屏适配
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    kabase::WindowManage::getWindowId(&m_windowId);
    kabase::WindowManage::removeHeader(this);
    // 配置按钮
    initColor();
    // 获取字体按钮状态
    ui->fontTwinButtonGroup->getFontSizeBtn()->setButtonSize("11");
    fontChanged(ui->textEdit->font());

    m_noteHead = new noteHead(this);
    m_noteHeadMenu = new noteHeadMenu(this);
    m_colorPage = new SelectColor(m_noteHeadMenu->ui->pushButtonPalette);

    ui->verticalLayout_3->addWidget(m_noteHead);
    ui->verticalLayout_3->addWidget(m_noteHeadMenu);
    m_noteHeadMenu->hide();

    ui->textEdit->setFrameShape(QFrame::NoFrame);
    ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 右键菜单
    ui->textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textEdit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(textRightMenu(QPoint)));

    // 设置字体大小
    m_setSizePage = new SetFontSize(ui->fontTwinButtonGroup->getFontSizeBtn());
    m_setSizePage->hide();
    // 设置字体颜色
    m_setColorFontPage = new SetFontColor(ui->fontTwinButtonGroup->getFontColorBtn());
    m_setColorFontPage->hide();
}


void EditPage::slotsSetup()
{
    connect(m_noteHeadMenu->ui->pushButtonExit, &QPushButton::clicked, this, [=](){
        if (ui->textEdit->document()->isEmpty()) {
            emit isEmptyNote(m_noteId);
        }
        this->close();
    });
    connect(m_noteHeadMenu->m_menuActionDel, &QAction::triggered, this, [=](){
        this->close();
        requestDel(m_noteId);
    });
    connect(ui->textEdit, &QTextEdit::textChanged, this, &EditPage::textChangedSlot);
    // 调色板菜单
    connect(m_noteHeadMenu->ui->pushButtonPalette, &QPushButton::clicked, this, [=](){
        m_colorPage->show();
    });
    // 调色按钮

    connect(m_colorPage, &CustomColorPanel::currentColorChanged, this, [=](const QColor &color) {
        m_colorPage->hide();
        handleBtnSlot(color);
    });

    // 字体颜色大小
    connect(m_setSizePage->ui->listWidget, &QListWidget::itemClicked, this,
            &EditPage::setFontSizeSlot);
    connect(m_setColorFontPage, &CustomColorPanel::currentColorChanged, this, [=](const QColor &color) {
        m_setColorFontPage->hide();
        setFontColorSlot(color);
    });
    // 文本格式
    connect(ui->fontPropertyWidget->boldBtn(), &QPushButton::clicked, this, &EditPage::setBoldSlot);
    connect(ui->fontPropertyWidget->italicBtn(), &QPushButton::clicked, this, &EditPage::setItalicSlot);
    connect(ui->fontPropertyWidget->underlineBtn(), &QPushButton::clicked, this, &EditPage::setUnderlineSlot);
    connect(ui->fontPropertyWidget->strikeOutBtn(), &QPushButton::clicked, this, &EditPage::setStrikeOutSlot);
    connect(ui->fontPropertyWidget->unorderedBtn(), &QPushButton::clicked, this, &EditPage::setUnorderedListSlot);
    connect(ui->fontPropertyWidget->orderedBtn(), &QPushButton::clicked, this, &EditPage::setOrderedListSlot);
    connect(ui->fontPropertyWidget->insertBtn(), &QPushButton::clicked, this, &EditPage::insertpicture);
    connect(ui->textEdit, &QTextEdit::cursorPositionChanged, this,
            &EditPage::cursorPositionChangedSlot);
    connect(ui->textEdit, &QTextEdit::currentCharFormatChanged, this,
            &EditPage::currentCharFormatChangedSlot);

    connect(ui->fontTwinButtonGroup->getFontSizeBtn(), &QPushButton::clicked, this, [=](){
        m_setSizePage->show();
    });
    connect(ui->fontTwinButtonGroup->getFontColorBtn(), &QPushButton::clicked, this, [=](){
        m_setColorFontPage->show();
    });
    connect(m_noteHeadMenu, &noteHeadMenu::requestUpdateMenuIcon, this, [=](){
        KWindowInfo info(this->winId(), NET::WMState);
        bool b = info.state() & NET::KeepAbove;

        if(b){
            m_noteHeadMenu->m_topAction->setIcon(QPixmap(":/image/1x/select.png"));
        }
        else{
            m_noteHeadMenu->m_topAction->setIcon(QPixmap(""));
        }
    });


    connect(m_noteHeadMenu, &noteHeadMenu::requestUpdateMenuIcon, this, [=](){
        KWindowInfo info(this->winId(), NET::WMState);
        bool b = info.state() & NET::KeepAbove;

        if(b){
            m_noteHeadMenu->m_topAction->setIcon(QPixmap(":/image/1x/select.png"));
        }
        else{
            m_noteHeadMenu->m_topAction->setIcon(QPixmap(""));
        }
    });

    connect(m_noteHeadMenu, &noteHeadMenu::requestTopMost, this, [=](){
        InformationCollector::getInstance().addPoint(InformationCollector::UiTop);
        kabase::WindowManage::keepWindowAbove(m_windowId);
     });

    connect(&LingmoUISettingsMonitor::instance(), &LingmoUISettingsMonitor::tabletModeUpdate,
            this, &EditPage::handleTabletMode);
}

void EditPage::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if (!cursor.hasSelection()) {
        // cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    ui->textEdit->mergeCurrentCharFormat(format);
    ui->textEdit->setFocus(Qt::TabFocusReason);
}

void EditPage::fontChanged(const QFont &f)
{
    qDebug() << "font Changed" << f.pointSize() << f.bold();
    m_setSizePage->ui->listWidget->setCurrentRow(f.pointSize() - 10);
    ui->fontTwinButtonGroup->getFontSizeBtn()->setButtonSize(QString::number(f.pointSize()));
    if(f.pointSize() < 10 )
    {
        ui->fontTwinButtonGroup->getFontSizeBtn()->setButtonSize(QString::number(10));
    }
    else
    {
        ui->fontTwinButtonGroup->getFontSizeBtn()->setButtonSize(QString::number(f.pointSize()));
    }
    ui->fontPropertyWidget->boldBtn()->setChecked(f.bold());
    ui->fontPropertyWidget->italicBtn()->setChecked(f.italic());
    ui->fontPropertyWidget->underlineBtn()->setChecked(f.underline());
    ui->fontPropertyWidget->strikeOutBtn()->setChecked(f.strikeOut());

    if (ui->textEdit->textCursor().currentList()) {
        QTextListFormat lfmt = ui->textEdit->textCursor().currentList()->format();
        if (lfmt.style() == QTextListFormat::ListDisc) {
            ui->fontPropertyWidget->unorderedBtn()->setChecked(true);
            ui->fontPropertyWidget->orderedBtn()->setChecked(false);
        } else if (lfmt.style() == QTextListFormat::ListDecimal) {
            ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
            ui->fontPropertyWidget->orderedBtn()->setChecked(true);
        } else {
            ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
            ui->fontPropertyWidget->orderedBtn()->setChecked(false);
        }
    } else {
        ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
        ui->fontPropertyWidget->orderedBtn()->setChecked(false);
    }
}

void EditPage::fontColorChanged(const QColor &c)
{
    ui->fontTwinButtonGroup->getFontColorBtn()->setButtonColor(c);
    m_setColorFontPage->setSelectColor(c);
}

void EditPage::list(bool checked, QTextListFormat::Style style)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.beginEditBlock();
    if (!checked) {
        qDebug() << "unchecked";
        QTextBlockFormat obfmt = cursor.blockFormat();
        QTextBlockFormat bfmt;
        bfmt.setIndent(obfmt.indent());
        cursor.setBlockFormat(bfmt);
//        QTextDocument *document = ui->textEdit->document();
//        document->setIndentWidth(0);
    } else {
        qDebug() << "checked";
        QTextCharFormat charFormat = cursor.charFormat();
        qDebug() << charFormat.fontPointSize() << cursor.selectedText() ;
        qDebug() << cursor.anchor() << cursor.position();
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        }
        listFmt.setStyle(style);
//        QTextDocument *document = ui->textEdit->document();
//        document->setIndentWidth(15);
        cursor.createList(listFmt);
    }
    cursor.endEditBlock();
}


/**************** Slots *******************/

void EditPage::cursorPositionChangedSlot()
{
    qDebug() << "cursorPositionChangedSlot";
    QTextList *l = ui->textEdit->textCursor().currentList();
    if (m_lastBlockList && (l == m_lastBlockList || (l != 0 && m_lastBlockList != 0
                                                     && l->format().style()
                                                     == m_lastBlockList->format().style()))) {
        return;
    }
    m_lastBlockList = l;
    if (l) {
        QTextListFormat lfmt = l->format();
        if (lfmt.style() == QTextListFormat::ListDisc) {
            ui->fontPropertyWidget->unorderedBtn()->setChecked(true);
            ui->fontPropertyWidget->orderedBtn()->setChecked(false);
        } else if (lfmt.style() == QTextListFormat::ListDecimal) {
            ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
            ui->fontPropertyWidget->orderedBtn()->setChecked(true);
        } else {
            ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
            ui->fontPropertyWidget->orderedBtn()->setChecked(false);
        }
    } else {
        ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
        ui->fontPropertyWidget->orderedBtn()->setChecked(false);
    }
}

void EditPage::currentCharFormatChangedSlot(const QTextCharFormat &format)
{
    qDebug() << "currentCharFormatChangedSlot";
    fontChanged(format.font());
    // fontColorChanged((format.foreground().isOpaque()) ? format.foreground().color() : QColor());
    fontColorChanged(format.foreground().color());
}

void EditPage::textChangedSlot()
{
    qDebug() << "emit textchange" << "note id" << m_noteId << this->m_id;
    emit texthasChanged(m_noteId, this->m_id);
}

//textedit右键菜单
void EditPage::textRightMenu(QPoint)
{
    MyMenu *m_rightMenu = new MyMenu;

    //定义action
    QAction *undoact = new QAction(tr("undo"));
    QAction *redoact = new QAction(tr("redo"));
    QAction *cutact = new QAction(tr("cut"));
    QAction *copyact = new QAction(tr("copy"));
    QAction *pasteact = new QAction(tr("paste"));
    QAction *textfornew = new QAction(tr("copy to newpage"));

    undoact->setIcon(QIcon::fromTheme("edit-undo"));
    redoact->setIcon(QIcon::fromTheme("edit-redo"));
    cutact->setIcon(QIcon::fromTheme("edit-cut"));
    copyact->setIcon(QIcon::fromTheme("edit-copy"));
    pasteact->setIcon(QIcon::fromTheme("edit-paste"));
//    textfornew->setIcon(QIcon::fromTheme("add-files-to-archive"));
    textfornew->setIcon(QIcon(":/image/1x/copy_to_new_note.png"));
    //定义action end

    //action使能与非使能
    connect(ui->textEdit->document(), SIGNAL(undoAvailable(bool)),
            undoact, SLOT(setEnabled(bool)));
    connect(ui->textEdit->document(), SIGNAL(redoAvailable(bool)),
            redoact, SLOT(setEnabled(bool)));
    undoact->setEnabled(ui->textEdit->document()->isUndoAvailable());
    redoact->setEnabled(ui->textEdit->document()->isRedoAvailable());

    cutact->setEnabled(!ui->textEdit->textCursor().selectedText().isEmpty());
    copyact->setEnabled(!ui->textEdit->textCursor().selectedText().isEmpty());
    pasteact->setEnabled(!QApplication::clipboard()->text().isEmpty());
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=](){
        pasteact->setEnabled(!QApplication::clipboard()->text().isEmpty());
    });

    textfornew->setEnabled(!ui->textEdit->textCursor().selectedText().isEmpty());
    //action使能与非使能 end

    //action绑定槽函数
    connect(undoact, SIGNAL(triggered(bool)), ui->textEdit, SLOT(undo()));
    connect(redoact, SIGNAL(triggered(bool)), ui->textEdit, SLOT(redo()));

    connect(cutact, SIGNAL(triggered()), ui->textEdit, SLOT(cut()));
    connect(copyact, SIGNAL(triggered()), ui->textEdit, SLOT(copy()));
    connect(pasteact, SIGNAL(triggered(bool)), ui->textEdit, SLOT(paste()));

    connect(textfornew, SIGNAL(triggered()), this, SLOT(textForNewEditpageSlot()));
    //action绑定槽函数 end

    //action添加到menu
    m_rightMenu->addAction(undoact);
    m_rightMenu->addAction(redoact);
    m_rightMenu->addSeparator();         // 添加分割线

    m_rightMenu->addAction(cutact);
    m_rightMenu->addAction(copyact);
    m_rightMenu->addAction(pasteact);
    // m_rightMenu->addSeparator();         // 添加分割线

    // m_rightMenu->addAction(textfornew);
    //action添加到menu end

    m_rightMenu->move (cursor().pos());
    m_rightMenu->show();

}

//拷贝选中内容到新便签页
void EditPage::textForNewEditpageSlot()
{
    if(!ui->textEdit->textCursor().selectedText().isEmpty())
    {
        ui->textEdit->copy();
        emit textForNewEditpage();
    }
}

// 加粗
void EditPage::setBoldSlot()
{
    InformationCollector::getInstance().addPoint(InformationCollector::Bold);
    qDebug() << "setBoldSlot";
    QTextCharFormat fmt;
    fmt.setFontWeight(ui->fontPropertyWidget->boldBtn()->isCheckable() ? QFont::Bold : QFont::Normal);

    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.charFormat().fontWeight() == QFont::Bold) {
        qDebug() << "current cursor charFormat QFont::Bold";
        fmt.setFontWeight(QFont::Normal);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        qDebug() << "current cursor charFormat QFont::Normal";
        fmt.setFontWeight(QFont::Bold);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    }
}

// 斜体
void EditPage::setItalicSlot()
{
    InformationCollector::getInstance().addPoint(InformationCollector::Italics);
    qDebug()<<"-------setItalicSlot------------";
    QTextCharFormat fmt;
    fmt.setFontItalic(ui->fontPropertyWidget->italicBtn()->isCheckable());// ? QFont::StyleItalic : QFont::Normal);
    // mergeFormatOnWordOrSelection(fmt);

    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.charFormat().fontItalic()) { // return boolProperty(FontItalic)
        fmt.setFontItalic(QFont::StyleNormal);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        fmt.setFontItalic(QFont::StyleItalic);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    }
}

// 划线
void EditPage::setUnderlineSlot()
{
    InformationCollector::getInstance().addPoint(InformationCollector::Underline);
    qDebug() << "setUnderlineSlot";
    QTextCharFormat fmt;
    fmt.setFontUnderline(ui->fontPropertyWidget->underlineBtn()->isCheckable());// ? QFont::UnderlineResolved : QFont::Normal );
    // mergeFormatOnWordOrSelection(fmt);

    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.charFormat().fontUnderline()) { //
        fmt.setFontUnderline(QFont::StyleNormal);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        ui->textEdit->mergeCurrentCharFormat(fmt);
    }
}

// 除线
void EditPage::setStrikeOutSlot()
{
    InformationCollector::getInstance().addPoint(InformationCollector::Deleteline);
    qDebug() << "setStrikeOutSlot";
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(ui->fontPropertyWidget->strikeOutBtn()->isCheckable());// ? QFont::StrikeOutResolved : QFont::Normal );
    // mergeFormatOnWordOrSelection(fmt);

    QTextCursor cursor = ui->textEdit->textCursor();
    if (cursor.charFormat().fontStrikeOut()) {
        fmt.setFontStrikeOut(QFont::StyleNormal);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        ui->textEdit->mergeCurrentCharFormat(fmt);
    }
}

// 无序列表
void EditPage::setUnorderedListSlot(bool checked)
{
    KTrackData *unordered_node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    // 传入事件描述点击“无序列表”，所在界面“主界面”，上传事件
    kdk_dia_upload_default(unordered_node,"set_unordered_List_click_event","mainWindow");
    kdk_dia_data_free(unordered_node);
    qDebug() << "setUnorderedListSlot" << checked;
    if (checked) {
        InformationCollector::getInstance().addPoint(InformationCollector::UnorderList);
        ui->fontPropertyWidget->orderedBtn()->setChecked(false);
        qDebug() << "show list set false" << checked;
        qDebug() << "text_edit_page->ui->fontPropertyWidget->orderedBtn() is checked ? :" << ui->fontPropertyWidget->orderedBtn()->isChecked();
    }
    list(checked, QTextListFormat::ListDisc);
    qDebug() << "text_edit_page->ui->fontPropertyWidget->orderedBtn() is checked ? :" << ui->fontPropertyWidget->orderedBtn()->isChecked();
}

// 有序列表
void EditPage::setOrderedListSlot(bool checked)
{
    KTrackData *ordered_node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    // 传入事件描述点击“有序列表”，所在界面“主界面”，上传事件
    kdk_dia_upload_default(ordered_node, "set_ordered_List_click_event", "mainWindow");
    kdk_dia_data_free(ordered_node);
    qDebug() << "show num list";
    if (checked) {
        InformationCollector::getInstance().addPoint(InformationCollector::OrderList);
        ui->fontPropertyWidget->unorderedBtn()->setChecked(false);
    }
    list(checked, QTextListFormat::ListDecimal);
}

// 字号
void EditPage::setFontSizeSlot()
{
    qDebug() << "setFontSizeSlot";
    int num = m_setSizePage->ui->listWidget->currentRow();
    ui->fontTwinButtonGroup->getFontSizeBtn()->setButtonSize(QString::number(num+10));
    m_setSizePage->close();
    update();
    QTextCharFormat fmt;
    InformationCollector::getInstance().addMessage(QString("set font size to %1.").arg(num+10));
    fmt.setFontPointSize(num+10);
    mergeFormatOnWordOrSelection(fmt);
}

// 调色板
void EditPage::initColor()
{

}

void EditPage::setFontColorSlot(const QColor &color)
{
    QTextCharFormat fmt;
    if (color != CustomColorPanel::BLACK) {
        InformationCollector::getInstance().addMessage(QString("set font color to %1.").arg(color.name()));
        fmt.setForeground(color);
        ui->textEdit->mergeCurrentCharFormat(fmt);
    } else {
        if (LingmoUISettingsMonitor::instance().styleStatus() == LingmoUISettingsMonitor::StyleStatus::LIGHT) {
            fmt.setForeground(QColor("#000000"));
        } else {
            fmt.setForeground(QColor("#FFFFFF"));
        }

        // fmt.clearForeground();
        ui->textEdit->mergeCurrentCharFormat(fmt);
    }
}

void EditPage::handleBtnSlot(const QColor &color)
{
    m_editColor = color;
    emit colorhasChanged(m_editColor, m_noteId);
    m_noteHead->colorWidget = color;
    m_noteHeadMenu->colorWidget = color;
    update();
}

void EditPage::showFullScreenSlot()
{
    if (!m_isFullscreen) {
        this->showFullScreen();
        m_isFullscreen = true;
    } else {
        this->showNormal();
        m_isFullscreen = false;
    }
}

void EditPage::setWindowKeepAbove()
{
    KWindowSystem::setState(this->winId(), NET::KeepAbove);
}

void EditPage::setWindowStatusClear()
{
    KWindowSystem::clearState(this->winId(), NET::KeepAbove);
}

void EditPage::dropImage(const QImage& image, const QString& format) {
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.toLocal8Bit().data());
    buffer.close();
    QByteArray base64 = bytes.toBase64();
    QByteArray base64l;
    for (int i=0; i<base64.size(); i++)
    {
        base64l.append(base64[i]);
        if (i%80 == 0)
        {
            base64l.append("\n");
        }
    }

    if(image.isNull())
    {
        return ;
    }

    QTextCursor cursor = ui->textEdit->textCursor();

    if(cursor.atStart())
    {
        m_isInsImg = true;
    }

    QTextImageFormat imageFormat;
    int imgWidth = image.width();
    int imgHeight = image.height();
    int texteditWidth = ui->textEdit->width();
    int texteditHeight = ui->textEdit->height() - m_noteHeadMenu->height() + m_noteHead->height();
    double ratio;
    if(imgWidth > texteditWidth || imgHeight > texteditHeight){
        ratio = qMin(texteditWidth/(imgWidth*1.0),texteditHeight/(imgHeight*1.0));
    }else{
        ratio = 1.0;
    }
    imageFormat.setWidth  ( imgWidth*ratio );
    imageFormat.setHeight ( imgHeight*ratio );
    imageFormat.setName   ( QString("data:image/%1;base64,%2")
                                .arg(QString("%1.%2").arg(rand()).arg(format))
                                .arg(base64l.data())
                                );
    cursor.insertImage    ( imageFormat );
}

void EditPage::dropGifImage(const QUrl &url, const QString &filename)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    //图片缩放处理
    QImage image = QImageReader(filename).read();
    int imgWidth = image.width();
    int imgHeight = image.height();
    int texteditWidth = ui->textEdit->width();
    int texteditHeight = ui->textEdit->height() - m_noteHeadMenu->height() + m_noteHead->height();
    double ratio;
    if(imgWidth > texteditWidth || imgHeight > texteditHeight){
        ratio = qMin(texteditWidth/(imgWidth*1.0),texteditHeight/(imgHeight*1.0));
    }else{
        ratio = 1.0;
    }

    cursor.insertHtml("<img src='" + url.toString() + "' width='" +QString::number(imgWidth*ratio)+"' height='"+QString::number(imgHeight*ratio)+"'>");
    if(m_lstUrls.contains(url)){
        return;
    }else{
        m_lstUrls.append(url);
    }

    QMovie* movie = new QMovie;
    movie->setFileName(filename);
    movie->setCacheMode(QMovie::CacheNone);
    m_hashUrls.insert(movie,url);

    connect(movie,SIGNAL(frameChanged(int)),this,SLOT(subAnimate(int)));
    movie->start();
}

void EditPage::subAnimate(int)
{
    QMovie* movie = qobject_cast<QMovie*>(sender());
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.document()->addResource(QTextDocument::ImageResource,m_hashUrls.value(movie),movie->currentPixmap());
    ui->textEdit->setLineWrapColumnOrWidth(ui->textEdit->lineWrapColumnOrWidth());
}


void EditPage::insertpicture()
{
    KTrackData *insert_node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    // 传入事件描述点击“插入图片”，所在界面“主界面”，上传事件
    kdk_dia_upload_default(insert_node,"insert_picture_click_event","mainWindow");
    kdk_dia_data_free(insert_node);
    InformationCollector::getInstance().addPoint(InformationCollector::InsertPicture);
        QSettings s;
        QString attdir = s.value("general/filedialog-path").toString();
        QString file = QFileDialog::getOpenFileName(this,
                                        tr("Select an image"),
                                        attdir,
                                        tr("JPEG (*.jpg);; GIF (*.gif);; PNG (*.png);; BMP (*.bmp);; All (*)"));

        QString format = QFileInfo(file).suffix().toUpper().toLocal8Bit().data();
        if(format.compare("GIF")==0){
            QUrl giftext = QUrl::fromUserInput(file,QDir::currentPath(),QUrl::AssumeLocalFile);
            dropGifImage(giftext,file);
        }else{
            QImage image = QImageReader(file).read();
            dropImage(image, format);
        }
}

void EditPage::setHints()
{
    kabase::WindowManage::removeHeader(this);
}

void EditPage::defaultTextColorSlot()
{
    QPalette pal;
    m_editColor = pal.text().color();
    emit colorhasChanged(m_editColor, m_noteId);
    m_noteHead->colorWidget = m_editColor;
    m_noteHeadMenu->colorWidget = m_editColor;
    update();
}

void EditPage::enableMaximum()
{
    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    setWindowState(Qt::WindowMaximized);
    if (isHidden() == false) {
        hide();
        show();
    }
}

void EditPage::disableMaximum()
{
    setMaximumSize(this->minimumSize());
    setWindowState(Qt::WindowNoState);
    if (isHidden() == false) {
        hide();
        show();
    }
}

void EditPage::handleTabletMode(LingmoUISettingsMonitor::TabletStatus status)
{
    switch (status) {
    case LingmoUISettingsMonitor::TabletStatus::PCMode:
        handlePCMode();
        break;
    case LingmoUISettingsMonitor::TabletStatus::PADHorizontalMode:
    case LingmoUISettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}

void EditPage::handlePADMode()
{
    enableMaximum();
}

void EditPage::handlePCMode()
{
//    disableMaximum();
}

void EditPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

}

void EditPage::keyPressEvent(QKeyEvent *event)
{
    if((event->modifiers() == Qt::ShiftModifier) && (event->key() == Qt::Key_F10)){
        if(!isActiveWindow()){
            //不作處理
        }else{
            textRightMenu(cursor().pos());
        }
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}
