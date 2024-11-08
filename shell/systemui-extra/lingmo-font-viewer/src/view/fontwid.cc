#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QDebug>
#include <QProcess>
#include <QPainterPath>

#include "fontwid.h"
#include "popupinstallfail.h"

/* 适配SDK */
#include "windowmanage.hpp"

FontWidget::FontWidget(QWidget *parent)
    : QWidget(parent)
{
    setWidgetUi();
    initLayout();
    setSlotConnet();
}

FontWidget::~FontWidget()
{
}

void FontWidget::setWidgetUi()
{
    this->setAcceptDrops(true);   /* 接受拖拽事件 */

    // 堆栈窗口
    m_stackedWid = new QStackedWidget(this);
    m_blankPage = new BlankPage(this);
    m_searchBlankPage = new BlankPage(this);

    m_fontView = new FontListView();

    m_titleBar = new TitleBar(this);

    m_previewArea = new BasePreviewArea();

    m_vLayout = new QVBoxLayout();
    m_hLayout = new QHBoxLayout();

    m_stackedWid->addWidget(m_blankPage);
    m_stackedWid->addWidget(m_fontView);
    m_stackedWid->addWidget(m_searchBlankPage);
    
    m_stackedWid->setCurrentIndex(1);
    
    return;
}

void FontWidget::initLayout()
{
    this->setMinimumSize(620 , 560);
    this->setBackgroundRole(QPalette::Base);

    m_hLayout->addSpacing(10);
    m_hLayout->addWidget(m_stackedWid);
    m_hLayout->addSpacing(10);
    m_hLayout->setSpacing(0);
    m_hLayout->setMargin(0);

    m_vLayout->addWidget(m_titleBar);
    m_vLayout->addLayout(m_hLayout);
    m_vLayout->addWidget(m_previewArea);
    m_vLayout->setSpacing(0);
    m_vLayout->setMargin(0);

    this->setLayout(m_vLayout);
    return;
}

void FontWidget::setSlotConnet()
{
    /* 预览区域 内容/字号 改变 */
    connect(m_previewArea, &BasePreviewArea::sigFontSize, m_fontView, &FontListView::slotGetFontSize);
    connect(m_previewArea, &BasePreviewArea::sigPreviewValue, m_fontView, &FontListView::slotUpdatePreView);

    /* 标题栏 搜索/添加字体 */
    connect(m_titleBar, &TitleBar::sigTitleSearch, m_fontView, &FontListView::slotFilterFontByReg);
    // connect(m_titleBar, &TitleBar::sigAddFont, m_fontView, &FontListView::slotAddFont);
    connect(m_titleBar, &TitleBar::sigAddFont, m_fontView, &FontListView::showFontTips);

    /* 主窗体改变 */
    connect(this, &FontWidget::sigWindowState, m_titleBar, &TitleBar::slotWindowState);

    /* 应用字体 */
    connect(m_fontView, &FontListView::sigChangeFont, m_previewArea, &BasePreviewArea::slotChangFont);
    connect(m_fontView, &FontListView::sigBlankPage, this, &FontWidget::slotShowBlankPage);
    connect(m_fontView, &FontListView::sigSearchFontBlankPage, this, &FontWidget::slotShowSearchBlankPage);
    connect(m_fontView, &FontListView::sigFontPage, this, &FontWidget::slotShowFontPage);
    return;
}

bool FontWidget::isFontFile(QString path)
{
    QString command = QString("file -i ") + '"' + path + '"';

    QProcess process;
    process.start(command);

    if (!process.waitForFinished()) {
        return false;
    }

    QString ret = process.readAll();
    
    if (ret.contains("font/sfnt") || ret.contains("application/vnd.ms-opentype")) {
        return true;
    }

    return false;
}

bool FontWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (m_titleBar->eventFilter(watched, event)) {
        return QObject::eventFilter(watched, event);
    }
    return QObject::eventFilter(watched, event);
}

void FontWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    QPainterPath rectPath;
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    return;
}

void FontWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
    return;
}

void FontWidget::dropEvent(QDropEvent *event)
{
    qDebug() << "拖拽安装：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    const QMimeData *mimeData = event->mimeData();

    if(!mimeData->hasUrls())
    {
        return;
    }

    QList<QUrl> urlList = mimeData->urls();
    int num = 0;
    QStringList installFontList;

    for (int i = 0; i < urlList.size(); i++) {
        QString dropFileName = urlList.at(i).toLocalFile();

        installFontList.insert(num, dropFileName);
    }
    // m_fontView->checkFontFile(installFontList);
    m_fontView->addFontByOtherWay(installFontList);

    return;
}

void FontWidget::slotChangeFilterType(FontType type)
{
    qDebug() << "fontType : " << type;
    m_fontView->getFilterType(type);
    m_titleBar->SearchFocusOut();
    m_previewArea->previewFocusOut();
    return;
}

void FontWidget::slotWindowState(QString state)
{
    emit sigWindowState(state);
    return;
}

void FontWidget::useFontFileOpen(QString fontFile)
{
    qDebug() << "使用字体文件打开：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;

    QStringList installFontList;
    installFontList = fontFile.split("/n/n");
    QStringList fontList;
    for (int i = 0; i < installFontList.size(); i++) {
        if (getFontFile(installFontList[i])) {
            fontList << installFontList[i];
        }
    }
    if (fontList.size() == 0) {
        return;
    }
    m_fontView->clickFontFileInstall(fontList);

    return;
}

bool FontWidget::getFontFile(QString fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }
    QFileInfo fileInfo(fileName);

    if (fileInfo.exists()) {
        if (fileInfo.isFile()) {
            return true;
        } 
    }
    return false;
}

void FontWidget::cilckFontFile(QString fontFile)
{
    qDebug() << "点击字体文件安装  fontFile ==" << fontFile <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;

    /* 添加 安装字体 SDK的打点 */
    // GlobalSizeData::getInstance()->addFontSDKPoint();

    QStringList installFontList;
    installFontList = fontFile.split("/n/n");
    QStringList fontList;
    for (int i = 0; i < installFontList.size(); i++) {
        if (getFontFile(installFontList[i])) {
            fontList << installFontList[i];
        }
    }
    if (fontList.size() == 0) {
        return;
    }
    
    // m_fontView->checkFontFile(installFontList);
    m_fontView->clickFontFileInstall(fontList);
    
    return;
}

void FontWidget::slotShowBlankPage()
{
    m_stackedWid->setCurrentIndex(0);
    m_blankPage->changeBlankText();
    return;
}

void FontWidget::slotShowFontPage()
{
    m_stackedWid->setCurrentIndex(1);
    return;
}

void FontWidget::slotShowSearchBlankPage()
{
    m_stackedWid->setCurrentIndex(2);
    m_searchBlankPage->changeSearchText();
    return;
}
void FontWidget::keyPressEvent(QKeyEvent *event){
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        m_titleBar->m_searchArea->setFocus();
    }
    return QWidget::keyPressEvent(event);
}
