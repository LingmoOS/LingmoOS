// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"
#include "dfinstallnormalwindow.h"
#include "utils.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DVerticalLine>

#include <QButtonGroup>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE
#define LISTVIEW_LEFT_SPACING 10

/*************************************************************************
 <Function>      DFInstallErrorDialog
 <Description>   构造函数-构造安装验证框页面
 <Author>        null
 <Input>
    <param1>     parent                       Description:父对象
    <param2>     errorInstallFontFileList     Description:安装时重复字体列表
    <param3>     systemFontFileListPsname     Description:安装时系统字体Psname列表
    <param4>     systemFontFileListFamilyname Description:安装时系统字体Familyname列表
 <Return>        null                         Description:null
 <Note>          null
*************************************************************************/
DFInstallErrorDialog::DFInstallErrorDialog(QWidget *parent, const QStringList &errorInstallFontFileList)
    : DDialog(parent)
    , m_parent(qobject_cast<DFInstallNormalWindow *>(parent))
    , m_signalManager(SignalManager::instance())
    , m_errorInstallFiles(errorInstallFontFileList)
{
//    setWindowOpacity(0.3); //Debug
    initData();

    initUI();
    emit m_signalManager->popInstallErrorDialog();
    connect(m_signalManager, &SignalManager::updateInstallErrorListview, this, &DFInstallErrorDialog::addData);
    resetContinueInstallBtnStatus();

    connect(this, &DFInstallErrorDialog::buttonClicked, this, &DFInstallErrorDialog::onControlButtonClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &DFInstallErrorDialog::slotSizeModeChanged);
#endif
}

/*************************************************************************
 <Function>      ~DFInstallErrorDialog
 <Description>   析构函数-析构字体验证框页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallErrorDialog::~DFInstallErrorDialog()
{
    qDebug() << __func__ << "start" << endl;
    emit m_signalManager->hideInstallErrorDialog();
    m_errorInstallFiles.clear();
    m_installErrorFontModelList.clear();
//    m_NeedSelectFiles.clear();
    qDebug() << __func__ << "end" << endl;
}

/*************************************************************************
 <Function>      initData
 <Description>   构造时初始化字体信息列表
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::initData()
{
    DFontInfo fontInfo;
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    m_installErrorFontModelList.clear();


    for (auto &it : m_errorInstallFiles) {
        fontInfo = fontInfoManager->getFontInfo(it);

        DFInstallErrorItemModel itemModel;
        if (fontInfo.isError) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Broken file");

            m_installErrorFontModelList.push_back(itemModel);
        } else if (fontInfo.isInstalled && !m_parent->isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.bIsNormalUserFont = true;//SP3--安装验证页面，回车取消/选中--设置字体状态
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");
//            m_NeedSelectFiles.append(it);
            m_installErrorFontModelList.push_back(itemModel);
        } else if (m_parent->isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
//            m_SystemFontCount++;
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.bSystemFont = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "System Font");
            m_installErrorFontModelList.push_back(itemModel);
        } else {
//            qDebug() << "verifyFontFiles->" << it << " :new file";
        }
    }

}

/*************************************************************************
 <Function>      initUI
 <Description>   字体验证框主页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::initUI()
{
    setContentsMargins(0, 0, 0, 0);
    setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
    setWindowTitle(DApplication::translate("ExceptionWindow", "Font Verification"));

    this->setFixedSize(448, 302);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setContentsMargins(0, 0, 0, 0);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(m_mainLayout);
    addContent(m_mainFrame, Qt::AlignHCenter);

    DVerticalLine *verticalSplit = new DVerticalLine;
    verticalSplit->setFixedWidth(1);
    verticalSplit->setFixedHeight(28);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplit);
    QBrush splitBrush = pa.brush(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitBrush);
    verticalSplit->setPalette(pa);
    verticalSplit->setBackgroundRole(QPalette::Background);
    verticalSplit->setAutoFillBackground(true);

    m_installErrorListView = new DFInstallErrorListView(m_installErrorFontModelList, this);
    m_installErrorListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_installErrorListView->setFixedWidth(width() - LISTVIEW_LEFT_SPACING * 2);
    m_installErrorListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_installErrorListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_mainLayout->addWidget(m_installErrorListView);

    insertButton(0, DApplication::translate("ExceptionWindow", "Exit"), false, ButtonNormal);
    insertButton(1, DApplication::translate("ExceptionWindow", "Continue"), true, ButtonRecommend);

    connect(m_installErrorListView, SIGNAL(clickedErrorListItem(QModelIndex)), this,
            SLOT(onListItemClicked(QModelIndex)));
}

/*************************************************************************
 <Function>      getErrorFontCheckedCount
 <Description>   获取已勾选继续安装项个数
 <Author>
 <Input>         null
 <Return>        int             Description:选中项个数
 <Note>          null
*************************************************************************/
int DFInstallErrorDialog::getErrorFontCheckedCount()
{
    int checkedCount = 0;
    QStandardItemModel *sourceModel = m_installErrorListView->getErrorListSourceModel();
    for (int i = 0; i < sourceModel->rowCount(); i++) {
        QModelIndex index = sourceModel->index(i, 0);
        DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(
                                                sourceModel->data(index));
        if (itemModel.bChecked) {
            ++checkedCount;
        }
    }
    return checkedCount;
}

/*************************************************************************
 <Function>      getErrorFontSelectableCount
 <Description>   获取可选项个数
 <Author>
 <Input>         null
 <Return>        int             Description:选中项个数
 <Note>          null
*************************************************************************/
int DFInstallErrorDialog::getErrorFontSelectableCount()
{
    int checkedCount = 0;
    QStandardItemModel *sourceModel = m_installErrorListView->getErrorListSourceModel();
    for (int i = 0; i < sourceModel->rowCount(); i++) {
        QModelIndex index = sourceModel->index(i, 0);
        DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(
                                                sourceModel->data(index));
        if (itemModel.bSelectable) {
            ++checkedCount;
        }
    }
    return checkedCount;
}

/*************************************************************************
 <Function>      resetContinueInstallBtnStatus
 <Description>   刷新继续按钮的状态-选中数量大于1时，继续按钮可用
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::resetContinueInstallBtnStatus()
{
    //所有字体都未勾选时，禁止点击"继续安装"
    if (0 == getErrorFontCheckedCount()) {
        if (m_errorInstallFiles.count() > 0) {
            getButton(1)->setToolTip(DApplication::translate("ExceptionWindow", "No fonts to be installed"));
        }
        getButton(1)->setDisabled(true);
        getButton(1)->setFocusPolicy(Qt::NoFocus);
    } else {
        getButton(1)->setEnabled(true);
        getButton(1)->setFocusPolicy(Qt::TabFocus);
    }

    if (getErrorFontSelectableCount() > 0) {
        m_installErrorListView->setFocusPolicy(Qt::TabFocus);
    } else {
        m_installErrorListView->setFocusPolicy(Qt::NoFocus);
    }
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   重写键盘press事件
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->close();
    } else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        //SP3--安装验证页面，回车取消/选中
        if (m_installErrorListView->hasFocus()) {
            if (m_installErrorListView->selectionModel()->selectedIndexes().count() == 1) {
                onListItemClicked(m_installErrorListView->selectionModel()->selectedIndexes().first());
                return;
            } else if (m_installErrorListView->selectionModel()->selectedIndexes().count() > 1) {
                onListItemsClicked(m_installErrorListView->selectionModel()->selectedIndexes());
            }
        } else {
            //默认字体列表无焦点情况下，回车执行“继续”或“取消”按钮
            if (getButton(1)->isEnabled()) {
                emit getButton(1)->click();
            } else {
                emit getButton(0)->click();
            }
        }
    } else if (event->key() == Qt::Key_Home && event->modifiers() == Qt::NoModifier) {
        m_installErrorListView->responseToHomeAndEnd(true);
    } else if (event->key() == Qt::Key_End && event->modifiers() == Qt::NoModifier) {
        m_installErrorListView->responseToHomeAndEnd(false);
    } else if (event->key() == Qt::Key_PageUp && event->modifiers() == Qt::NoModifier) {
        m_installErrorListView->responseToPageUpAndPageDown(true);
    } else if (event->key() == Qt::Key_PageDown && event->modifiers() == Qt::NoModifier) {
        m_installErrorListView->responseToPageUpAndPageDown(false);
    }
    QWidget::keyPressEvent(event);
}

/*************************************************************************
 <Function>      closeEvent
 <Description>   重写关闭事件-发送取消继续安装信号
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    //设置菜单滚动可用
    emit m_signalManager->setSpliteWidgetScrollEnable(false);

    //关闭窗口时发送取消安装信号
    emit onCancelInstall();

    DDialog::closeEvent(event);
}

/*************************************************************************
 <Function>      onListItemClicked
 <Description>   勾选按钮点击或回车选中事件
 <Author>
 <Input>
    <param1>     index           Description:当前选中项
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::onListItemClicked(const QModelIndex &index)
{
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(index));
    //SP3--安装验证页面，回车取消/选中(539)--正常字体可操作
    if (itemModel.bIsNormalUserFont) {
        itemModel.bChecked = !itemModel.bChecked;
        m_installErrorListView->ifNeedScrollTo(index);
        m_installErrorListView->getErrorListSourceModel()->setData(index, QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_installErrorListView->updateErrorFontModelList(index.row(), itemModel);
        resetContinueInstallBtnStatus();
    }
}

/*************************************************************************/
/*                                                                       */
/* <Function>    onListItemsClicked                                      */
/*                                                                       */
/* <Description> 用于处理字体验证框中选择多个字体后使用快捷键改变选中字体的选择状态 */
/*                                                                       */
/* <para>       所有选中项的index构成的indexlist                            */
/*                                                                       */
/* <Return>     无返回值                                                  */
void DFInstallErrorDialog::onListItemsClicked(const QModelIndexList &indexList)
{
    QModelIndex firstIndex;
    for (auto &it : indexList) {
        if (!firstIndex.isValid() || firstIndex.row() > it.row())
            firstIndex = it;
    }

    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(firstIndex));

    for (auto &it : indexList) {
        qDebug() << it.row() << "++++++++++++++++++++++++++++++" << endl;
        DFInstallErrorItemModel itemModel2 =
            qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(it));
        if (itemModel2.bIsNormalUserFont) {
            itemModel2.bChecked = !itemModel.bChecked;
            //m_installErrorListView->ifNeedScrollTo(it);
            m_installErrorListView->getErrorListSourceModel()->setData(it, QVariant::fromValue(itemModel2), Qt::DisplayRole);
            m_installErrorListView->updateErrorFontModelList(it.row(), itemModel2);
        }

    }
    resetContinueInstallBtnStatus();
}


/*参数1 正常筛选后筛选出的错误字体  参数2 字体验证框弹出过程中安装的所有字体
  参数3 新添加到字体验证框中的字体  参数4 之前添加到字体验证框中的字体   */
void DFInstallErrorDialog::addData(QStringList &errorFileList, QStringList &halfInstalledFilelist,
                                   QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles)
{
    DFontInfo fontInfo;
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    QList<DFInstallErrorItemModel> m_updateInstallErrorFontModelList;
    QStringList m_halfInstalledFilelist;

    m_installErrorFontModelList.clear();

    for (auto &it : errorFileList) {
        fontInfo = fontInfoManager->getFontInfo(it);
        qDebug() << "getfontinfo success!!!!!!!!!!1" << endl;
        DFInstallErrorItemModel itemModel;
        if (fontInfo.isError) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Broken file");

            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else if (fontInfo.isInstalled && !m_parent->isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.bIsNormalUserFont = true;//SP3--安装验证页面，回车取消/选中(539)--设置字体状态
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");

            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else if (m_parent->isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
//            m_SystemFontCount++;
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.bSystemFont = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "System Font");
            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else {
//            qDebug() << "verifyFontFiles->" << it << " :new file";
        }
    }

    for (auto &it : halfInstalledFilelist) {
        QFileInfo fileInfo(it);
        DFInstallErrorItemModel itemModel;
        itemModel.bSelectable = true;
        //默认勾选已安装字体
        itemModel.bChecked = true;
        itemModel.strFontFileName = fileInfo.fileName();
        itemModel.strFontFilePath = fileInfo.filePath();
        itemModel.bIsNormalUserFont = true;
        itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");
        m_updateInstallErrorFontModelList.push_back(itemModel);
    }

//    m_installErrorListView->getErrorListSourceModel();
    QModelIndexList list = m_installErrorListView->selectionModel()->selectedIndexes();
    m_installErrorListView->addErrorListData(m_updateInstallErrorFontModelList);
    m_installErrorListView->checkScrollToIndex(addHalfInstalledFiles, oldHalfInstalledFiles, errorFileList);

    addHalfInstalledFiles.append(oldHalfInstalledFiles);
    addHalfInstalledFiles.append(errorFileList);

    m_installErrorListView->setSelectStatus(addHalfInstalledFiles, list);

//    m_installErrorListView->setFocus();

    resetContinueInstallBtnStatus();

}

/*************************************************************************
 <Function>      onControlButtonClicked
 <Description>   按钮点击事件
 <Author>
 <Input>
    <param1>     btnIndex        Description:按钮索引
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::onControlButtonClicked(int btnIndex)
{
    if (0 == btnIndex) {
        //退出安装
        this->close();
    } else {
        //继续安装
        QStringList continueInstallFontFileList;

        //根据用户勾选情况添加到继续安装列表中
        for (int i = 0; i < m_installErrorListView->model()->rowCount(); i++) {
            DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->model()->data(m_installErrorListView->model()->index(i, 0)));
            if (itemModel.bChecked && !itemModel.bSystemFont)
                continueInstallFontFileList.push_back(itemModel.strFontFilePath);
        }

        emit onContinueInstall(continueInstallFontFileList);
        this->reject();
    }
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void DFInstallErrorDialog::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    Utils::clearImgCache();
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        this->setFixedSize(448, 259);
        setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(25, 25)));
    } else {
        this->setFixedSize(448, 302);
        setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
    }
}
#endif
