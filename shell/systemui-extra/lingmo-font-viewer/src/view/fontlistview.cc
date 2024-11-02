#include <QFileDialog>
#include <QDir>
#include <QScrollBar>
#include <QRegExp>
#include <QDebug>

/* 适配SDK */
#include "windowmanage.hpp"

#include "fontlistview.h"
#include "include/core.h"

FontListView::FontListView(QListView *parent) :
    QListView(parent)
{
    setWidgetUi();
    setSlotConnet();
}

FontListView::~FontListView()
{
}

void FontListView::setWidgetUi()
{

    this->setMinimumSize(588, 440);
    // 右键菜单
    m_funcMenu = new QMenu();
    m_addFontAction = m_funcMenu->addAction(tr("Add Font"));
    m_applyFontAction = m_funcMenu->addAction(tr("Apply Font"));
    m_removeFontAction = m_funcMenu->addAction(tr("Remove Font"));
    m_exportFontAction = m_funcMenu->addAction(tr("Export Font"));
    m_collectFontAction = m_funcMenu->addAction(tr("Cancel Collection"));
    m_checkFontAction = m_funcMenu->addAction(tr("Check Font"));

    m_fontModel = FontListModel::getInstance();    /* 获取model */
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        m_fontDelegateug = new FontListDelegateug(this);   /* 设置代理 */
        this->setItemDelegate(m_fontDelegateug);
    }
    else{
    m_fontDelegate = new FontListDelegate(this);   /* 设置代理 */
    this->setItemDelegate(m_fontDelegate);
    }
    m_fontFilter = new FontListFilter();
    connect(m_fontModel, &FontListModel::sigGetAllFontOver, this, &FontListView::setModelAfterGetAllData);

    this->setSpacing(0);
    this->setDragEnabled(false);
    this->setFrameShape(QListView::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    this->setAlternatingRowColors(true);   /* 奇偶行颜色不同 */
    return;
}

void FontListView::setSlotConnet()
{
    /* 收藏字体 */
    connect(this, &FontListView::clicked, this, &FontListView::slotClickCollect);
    /* 右键菜单 */
    connect(this, &FontListView::customContextMenuRequested, this, &FontListView::showContextMenu);
    /* 右键功能 */
    // connect(m_addFontAction, &QAction::triggered, this, &FontListView::slotAddFont);
    connect(m_addFontAction, &QAction::triggered, this, &FontListView::showFontTips);
    connect(m_applyFontAction, &QAction::triggered, this, &FontListView::slotApplyFont);
    connect(m_removeFontAction, &QAction::triggered, this, &FontListView::slotRemoveFont);
    connect(m_exportFontAction,  &QAction::triggered, this, &FontListView::slotExportFont);
    connect(m_collectFontAction,  &QAction::triggered, this, &FontListView::slotCollectFont);
    connect(m_checkFontAction, &QAction::triggered, this, &FontListView::slotCheckFont);
    connect(m_fontModel, &FontListModel::sigGetAllFontOver, this, [=](){
            m_firstOpen = false;
        });
    return;
}

void FontListView::showContextMenu(const QPoint &pos)
{
    if (m_fontInstallList.size() != 0) {
        changeItemInstallStyle(false); // 界面有安装字体被选中时，清空UI界面选中字体
    }

    QModelIndex selectIndex = this->indexAt(pos);
    if (!selectIndex.isValid()) {
        return ;
    }

    this->setCurrentIndex(selectIndex);
    if (!((this->selectionModel()->selectedIndexes()).empty())) {

        /* 将代理index转换为消息index */
        QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
        int fontFamily = fontIndex.data(GlobalSizeData::FontFamily).toInt();
        int fontCollect = fontIndex.data(GlobalSizeData::CollectState).toInt();

        m_addFontAction->setVisible(true);
        m_applyFontAction->setVisible(true);
        m_removeFontAction->setVisible(true);
        m_exportFontAction->setVisible(true);
        m_collectFontAction->setVisible(true);
        m_checkFontAction->setVisible(true);

        if (fontFamily == GlobalSizeData::FamilyType::SystemFont) {
            m_removeFontAction->setEnabled(false);   /* 系统文件不可被导出，卸载 */
            m_exportFontAction->setEnabled(false);
        } else {
            if (isControlApplyFont()) {
            
                m_removeFontAction->setEnabled(false);
            }else {
                m_removeFontAction->setEnabled(true);
                m_exportFontAction->setEnabled(true);
            }
            
        }

        if (fontCollect == GlobalSizeData::CollectType::NotCollected) {  /* 如果字体未被收藏 */
            m_collectFontAction->setText(tr("Collection"));

        } else if (fontCollect == GlobalSizeData::CollectType::IsCollected) { /* 如果字体被收藏 */
            m_collectFontAction->setText(tr("Cancel Collection"));
        }

        m_funcMenu->exec(QCursor::pos());
        this->selectionModel()->clear();
    }
    return;
}

void FontListView::removeFont(QString path)
{
    /* 循环找到在model中的索引 */
    for (int i = 0; i < this->m_fontModel->rowCount(); i++) {
        QModelIndex index = this->m_fontModel->index(i, 0);
        QString fontPath = index.data(GlobalSizeData::FontPath).toString();

        if(fontPath == path){
            this->m_fontModel->removeFont(fontPath, index.row());
            break;
        }
    }
    m_popupRemove->close();
    return;
}

int FontListView::getFontSize()
{
    return m_fontSize;
}

QString FontListView::getPreviewValue()
{
    return m_previewValue;
}

void FontListView::getFilterType(FontType type)
{
    m_filterType = type;
    if (m_fontSearch != "") {
        this->slotFilterFontByReg(m_fontSearch);
    } else {
        this->slotFilterFontByReg("");
    }
    return;
}

void FontListView::dragEnterAddFont(QStringList fontList)
{
    /* 添加 安装字体 SDK的打点 */
    GlobalSizeData::getInstance()->addFontSDKPoint();
    
    checkFontFile(fontList);

    this->update();

    return;
}

void FontListView::updateDelegate()
{
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        delete m_fontDelegateug;
        m_fontDelegateug = new FontListDelegateug(this);   /* 设置代理 */
        this->setItemDelegate(m_fontDelegateug);

    }else{
    delete m_fontDelegate;
    m_fontDelegate = new FontListDelegate(this);
    this->setItemDelegate(m_fontDelegate);
    }
    return;
}

void FontListView::slotFilterFontByReg(QString fontSearch)
{
    m_fontSearch = fontSearch;
    m_fontFilter->setFilterRegExp(QRegExp(fontSearch, Qt::CaseInsensitive, QRegExp::FixedString));

    if (m_fontFilter->rowCount() == 0) {
        if (m_fontSearch == "") {
            emit sigBlankPage();
        } else {
            emit sigSearchFontBlankPage();
        }
    } else {
        emit sigFontPage();
    }

    return;
}

void FontListView::slotAddFont()
{
    qDebug() << "安装字体" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    /* 添加 安装字体 SDK的打点 */
    GlobalSizeData::getInstance()->addFontSDKPoint();

    QFileDialog *fileDialog = new QFileDialog();

    fileDialog->setWindowTitle(tr("Add Fonts"));
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setNameFilter(tr("font(*.ttf *.otf)"));

    if (fileDialog->exec() == QFileDialog::Accepted) {
        QStringList selectFileList = fileDialog->selectedFiles();

        checkFontFile(selectFileList);
    }

    if (fileDialog != nullptr) {
        delete fileDialog;
        fileDialog = nullptr;
    }

    this->update();

    return;
}

void FontListView::slotApplyFont()
{
    /* 添加 应用字体 SDK的打点 */
    GlobalSizeData::getInstance()->applyFontSDKPoint();

    // 将过滤index转换为model的index
    QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
    QString fontName = fontIndex.data(GlobalSizeData::FontName).toString();   /* 获取字体Path */
    QString fontPath = fontIndex.data(GlobalSizeData::FontPath).toString();
    Core::getInstance()->applyFont(fontName, fontPath);

    this->update();

    emit sigChangeFont(fontName);

    return;
}

void FontListView::slotRemoveFont()
{
    /* 添加 卸载字体 SDK的打点 */
    GlobalSizeData::getInstance()->removeFontSDKPoint();

    QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
    QString deleFontPath = fontIndex.data(GlobalSizeData::FontPath).toString();   /* 获取字体Path */

    m_popupRemove = new PopupRemove(deleFontPath,this);
    m_popupRemove->show();

    connect(m_popupRemove, &PopupRemove::sigSureClick, this, &FontListView::removeFont);
    return;
}

void FontListView::slotExportFont()
{
    /* 添加 导出字体 SDK的打点 */
    GlobalSizeData::getInstance()->exportFontSDKPoint();

    QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
    QString fontPath = fontIndex.data(GlobalSizeData::FontPath).toString();   /* 获取字体Path */

    QString path = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this,tr("Export Fonts"), QString(getenv("HOME"))));
    if(!path.isEmpty()) {
        Core::getInstance()->copyFont(fontPath, path);
    }

    return;
}

void FontListView::slotCollectFont()
{
    QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
    QString fontPath = fontIndex.data(GlobalSizeData::FontPath).toString();   /* 获取字体path */
    m_fontModel->collectFont(fontPath);
    if (m_fontFilter->rowCount() == 0) {
        if (m_fontSearch == "") {
            emit sigBlankPage();
        } else {
            emit sigSearchFontBlankPage();
        }
    } else {
        emit sigFontPage();
    }
    return;
}

void FontListView::slotCheckFont()
{
    /* 添加 查看字体信息 SDK的打点 */
    GlobalSizeData::getInstance()->fontInfoSDKPoint();

    QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
    QString fontPath = fontIndex.data(GlobalSizeData::FontPath).toString();   /* 获取字体path */
    m_popupFontInfo = new PopupFontInfo(fontPath,this);
    m_popupFontInfo->show();
    return;
}

void FontListView::slotGetFontSize(int size)
{
    /* 添加 更改预览字号 SDK的打点 */
    GlobalSizeData::getInstance()->changeFontSizeSDKPoint();

    m_fontModel->previewChangeSize(size);

    m_fontSize = size;

    updateDelegate();

    return;
}

void FontListView::slotUpdatePreView(QString value)
{
    /* 添加 更改预览信息 SDK的打点 */
    GlobalSizeData::getInstance()->changecopywritingSDKPoint();
    
    this->setUpdatesEnabled(false);
    if (value == "") {
        m_previewValue = tr("Build the core strength of Chinese operating system");
    } else {
        m_previewValue = value;
    }
    this->setUpdatesEnabled(true);
    this->update();

    return;
}

void FontListView::slotClickCollect()
{
    QPoint pressPoint = this->mapFromGlobal(QCursor::pos());
     QPoint statePoint;
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        statePoint.setX(FontListDelegateug::m_collectPoint.x());
        statePoint.setY(FontListDelegateug::m_collectPoint.y());
    }else{
        statePoint.setX(FontListDelegate::m_collectPoint.x());
        statePoint.setY(FontListDelegate::m_collectPoint.y());
    }

    statePoint.setX(FontListDelegate::m_collectPoint.x());
    statePoint.setY(FontListDelegate::m_collectPoint.y());

    QPoint diffPoint = QPoint(pressPoint.x() - statePoint.x(), pressPoint.y() - statePoint.y());

    // 点击区域范围
    if (diffPoint.x() >= 0 && diffPoint.x() < 15 &&
        diffPoint.y() >= 0 && diffPoint.y() < 15) {
        slotCollectFont();
    }

    return;
}

bool FontListView::isControlApplyFont()
{
    QModelIndex fontIndex = m_fontFilter->mapToSource(this->currentIndex());
    QString fontName = fontIndex.data(GlobalSizeData::FontName).toString();   

    QString font = Core::getInstance()->ControlUsingFont();
    qDebug() << font << fontName;
    if (font == fontName) {
        return true;
    }
    return false;
}

void FontListView::clickFontFileInstall(QStringList fontList)
{
    addFontByOtherWay(fontList);
    
    // connect(m_fontModel, &FontListModel::sigGetAllFontOver,this, &FontListView::slotClickFontFile);
    
    return;
}

void FontListView::clickFontFileInstallAfterOpen(QStringList fontList)
{
    m_fontList = fontList;
    slotClickFontFile();
    return;
}

void FontListView::slotClickFontFile()
{
    if (m_fontList.size() != 0) {
        dragEnterAddFont(m_fontList);
    }
    m_fontList.clear();
    return;
}

void FontListView::setModelAfterGetAllData()
{
    m_fontFilter->setSourceModel(m_fontModel);     /* 将model放到过滤其中 */
    m_fontFilter->setDynamicSortFilter(true);
    m_fontFilter->sort(0);
    m_fontFilter->setFilterKeyColumn(0);
    this->setModel(m_fontFilter);
    return;
}

void FontListView::checkFontFile(QStringList list)
{
    qDebug() << "判断文件状态：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    int installNum = 0;
    int uninstallNum = 0;
    QStringList uninstallFont;
    uninstallFont.clear();

    uninstallFont = m_fontModel->addFont(list);
    uninstallNum = uninstallFont.size();

    if (uninstallNum == 0) {
        installNum = list.size();

        m_popupInstallSuccess = new PopupInstallSuccess(installNum , this);
        m_popupInstallSuccess->show();
    } else {
        installNum = list.size() - uninstallNum;
        m_popupInstallFail = new PopupInstallFail(uninstallFont, this);
        m_popupInstallFail->show();
        if (installNum != 0) {
            connect(m_popupInstallFail, &PopupInstallFail::signalWidgetClose, [=](){
                m_popupInstallSuccess = new PopupInstallSuccess(installNum , this);
                m_popupInstallSuccess->show();
                delete m_popupInstallFail;
                m_popupInstallFail = nullptr;
            });
        } else {
            connect(m_popupInstallFail, &PopupInstallFail::signalWidgetClose, [=](){
                delete m_popupInstallFail;
                m_popupInstallFail = nullptr;
            });
        }
    }
    if (m_fontInstallList.size() != 0) {
        changeItemInstallStyle(false);
    }
    connect(m_fontModel, &FontListModel::sigInstallFontOver,this, &FontListView::chooseFontInstallItem);
    return;
}

void FontListView::chooseFontInstallItem()
{
    m_fontInstallList.clear();
    m_fontInstallList = m_fontModel->getInstallSuccessList();
    changeItemInstallStyle(true);
    connect(this, &FontListView::clicked, [=](){
        if (m_fontInstallList.size() != 0) {
            changeItemInstallStyle(false);
        }
    });
    return;
}

void FontListView::changeItemInstallStyle(bool style)
{
    for (int i = 0; i < m_fontInstallList.size(); i++) {
        if ( m_fontModel->getItemByPath(m_fontInstallList[i]) == nullptr) {
            continue;
        }
        m_fontModel->getItemByPath(m_fontInstallList[i])->setData(style, GlobalSizeData::FontInstall);
    }
    if (style == false) {
        m_fontInstallList.clear();
    }
    return;
}

void FontListView::showFontTips()
{
    if (Core::getInstance()->getSettingFontTips()) {
        m_popupTips = new PopupTips(this);
        m_popupTips->show();
        m_popupTips->disconnect();
        connect(m_popupTips, &PopupTips::signalWidgetClose, this, &FontListView::slotAddFont);
    } else {
        slotAddFont();
    }
    return;
}

void FontListView::addFontByOtherWay(QStringList list)
{
    for (int i = 0; i < list.size(); i++) {
        m_fontList<<list[i];
    }
    if (m_firstOpen) {
        connect(m_fontModel, &FontListModel::sigGetAllFontOver, this, [=](){
            m_firstOpen = false;
            showTipsByClickFontFile();
        });
    } else {
        showTipsByClickFontFile();
    }
    
    return;
}

void FontListView::showTipsByClickFontFile()
{
    if (m_popupBox == nullptr) {
        m_popupBox = new PopupMessage(this);
    }
    m_popupBox->show();
    m_popupBox->disconnect();
    connect(m_popupBox, &PopupMessage::signalClickYes, [=](){
    
        if (Core::getInstance()->getSettingFontTips()) {
            if (m_popupTips == nullptr) {
                m_popupTips = new PopupTips(this);
            }
            m_popupTips->show();
            m_popupTips->disconnect();
            connect(m_popupTips, &PopupTips::signalWidgetClose, [=](){
                slotClickFontFile();
            });
        } else {
                slotClickFontFile();
        }
        if (m_popupBox != nullptr) {
            delete m_popupBox;
            m_popupBox = nullptr;
        } 
    });
    connect(m_popupBox, &PopupMessage::signalClickNo, [=](){
        m_fontList.clear();
        if (m_popupBox != nullptr) {
            delete m_popupBox;
            m_popupBox = nullptr;
        } 
    });
    return;
}
