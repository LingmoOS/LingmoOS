// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfinstallerrorlistview.h"
#include "globaldef.h"
#include "utils.h"

#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DCheckBox>
#include <DFontSizeManager>

#include <QPainter>
#include <QStandardItemModel>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

#define FTM_ERROR_ITEM_FONTNAME_LEFT 39
#define FTM_PARAM_OF_WIDTH 350

/*************************************************************************
 <Function>      DFInstallErrorListDelegate
 <Description>   代理用于绘制字体验证框中的字体列表页面
 <Author>
 <Input>
    <param1>     parent          Description:父对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallErrorListDelegate::DFInstallErrorListDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(qobject_cast<DFInstallErrorListView *>(parent))
{
}

/*************************************************************************
 <Function>      drawCheckBox
 <Description>   代理绘制勾选按钮函数
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param2>     itemModel       Description:需要绘制的ItemModel对象
    <param3>     bgRect          Description:绘制区域
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::drawCheckBox(QPainter *painter, DFInstallErrorItemModel itemModel, QRect bgRect) const
{
    QStyleOptionButton checkBoxOption;
    bool checked = itemModel.bChecked;
    checkBoxOption.state |= QStyle::State_Enabled;
    //根据值判断是否选中
    if (checked) {
        checkBoxOption.state |= QStyle::State_On;
    } else {
        checkBoxOption.state |= QStyle::State_Off;
    }

    int checkBoxSize = 16;
    DCheckBox checkBox;
    QRect checkboxRect = QRect(bgRect.left() + 10, bgRect.top() + (bgRect.height() - checkBoxSize) / 2, checkBoxSize, checkBoxSize);
    checkBoxOption.rect = checkboxRect;
    DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox,
                                         &checkBoxOption,
                                         painter,
                                         &checkBox);
}

/*************************************************************************
 <Function>      drawCheckBoxIcon
 <Description>   代理绘制勾选按钮图标函数
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param3>     bgRect          Description:绘制区域
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::drawCheckBoxIcon(QPainter *painter, QRect bgRect) const
{
    int checkBoxSize = 30;
    QRect checkboxRect = QRect(bgRect.left() + 5, bgRect.top() + (bgRect.height() - checkBoxSize) / 2, checkBoxSize, checkBoxSize);

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        QImage checkBox("://checkbox_unchecked_light.svg");
        painter->drawImage(checkboxRect, checkBox);
    } else {
        QImage checkBox("://checkbox_unchecked_dark.svg");
        painter->drawImage(checkboxRect, checkBox);
    }
}

/*************************************************************************
 <Function>      drawFontName
 <Description>   代理绘制字体名
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param1>     option          Description:QStyleOptionViewItem对象
    <param1>     itemModel       Description:需要绘制的ItemModel对象
    <param3>     bgRect          Description:绘制区域
    <param3>     bSelectable     Description:是否可勾选
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::drawFontName(QPainter *painter, const QStyleOptionViewItem &option,
                                              DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable) const
{
    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    painter->setFont(nameFont);

    QString strFontFileName = itemModel.strFontFileName;
    QString strStatus = itemModel.strFontInstallStatus;

    int checkBoxSize = 20;
    QRect checkboxRect = QRect(bgRect.left() + 10, bgRect.top() + 14 + 2, checkBoxSize - 4, checkBoxSize - 4);
    int statusLabelMaxWidth = 160;
    int fontNameLeft = FTM_ERROR_ITEM_FONTNAME_LEFT;

    QFontMetrics fontMetric(nameFont);
    int m_StatusWidth = fontMetric.width(strStatus);
    if (m_StatusWidth > statusLabelMaxWidth)
        m_StatusWidth = statusLabelMaxWidth;
    int m_NameWidth = fontMetric.width(strFontFileName);

    int fontFileNameRectHeight = 30;
    QRect fontFileNameRect = QRect(bgRect.left() + fontNameLeft,
                                   bgRect.top() + (bgRect.height() - fontFileNameRectHeight) / 2,
                                   bgRect.width() - fontNameLeft - m_StatusWidth,
                                   fontFileNameRectHeight);

    QString elidedFontFileNameText;

    if (m_NameWidth  + m_StatusWidth < 360) {
        elidedFontFileNameText = strFontFileName;
    } else {
        elidedFontFileNameText = lengthAutoFeed(painter, strFontFileName, m_StatusWidth);
    }

//    if (option.state & QStyle::State_Selected) {
//        QColor penColor = option.palette.color(DPalette::Text);
//        if (bSelectable == false) {
//            penColor.setAlphaF(0.4);
//        }
//        painter->setPen(QPen(penColor));
//        painter->drawText(fontFileNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedFontFileNameText);
//    } else {
    QColor penColor = option.palette.color(DPalette::Text);
    if (bSelectable == false) {
        penColor.setAlphaF(0.4);
    }
    painter->setPen(QPen(penColor));
    painter->drawText(fontFileNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedFontFileNameText);
//    }
}

/*************************************************************************
 <Function>      drawFontStyle
 <Description>   代理绘制字体样式
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param1>     option          Description:QStyleOptionViewItem对象
    <param1>     itemModel       Description:需要绘制的ItemModel对象
    <param3>     bgRect          Description:绘制区域
    <param3>     bSelectable     Description:是否可勾选
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::drawFontStyle(QPainter *painter, const QStyleOptionViewItem &option,
                                               DFInstallErrorItemModel itemModel, QRect bgRect, bool bSelectable) const
{
    Q_UNUSED(option);
    Q_UNUSED(bSelectable);
    int statusLabelMaxWidth = 160;
    QColor penColor;
    QRect installStatusRect = QRect(bgRect.left() + (bgRect.width() - statusLabelMaxWidth) - 10,
                                    bgRect.top(),
                                    statusLabelMaxWidth,
                                    bgRect.height());
    QString strStatus = itemModel.strFontInstallStatus;


    QFont installStatusFont = painter->font();
    installStatusFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));
    painter->setFont(installStatusFont);


    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));
    painter->setFont(nameFont);

    QFontMetrics fontMetric(nameFont);
    QFontMetrics stateFontMetric(installStatusFont);
    QString elidedStatusText = fontMetric.elidedText(strStatus,
                                                     Qt::ElideRight,
                                                     installStatusRect.width());

    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        penColor = QColor("#FF6D6D");
    } else {
        penColor = QColor("#FF4E4E");
    }


    painter->setPen(QPen(penColor));
    painter->drawText(installStatusRect, Qt::AlignRight | Qt::AlignVCenter, elidedStatusText);
    QFont::cleanup();
}

/*************************************************************************
 <Function>      drawSelectStatus
 <Description>   代理绘制勾选状态
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param1>     option          Description:QStyleOptionViewItem对象
    <param3>     bgRect          Description:绘制区域
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::drawSelectStatus(QPainter *painter, const QStyleOptionViewItem &option, QRect bgRect) const
{
    QPainterPath path;
    const int radius = 8;

    setPaintPath(bgRect, path, 0, 0, radius);

    if (option.state & QStyle::State_Selected) {
//        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);//dtk库接口不稳定，更换palette获取方式
// 如果是因为tab获取到的焦点，绘制tab选中的效果
        if (m_parentView->getIsTabFocus() == true) {
            paintTabFocusBackground(painter, option, bgRect);
        } else {
            DPalette pa = DApplicationHelper::instance()->applicationPalette();
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.2);
            painter->fillPath(path, QBrush(fillColor));
        }
    }
}

/*************************************************************************
 <Function>      paintTabFocusBackground
 <Description>   代理绘制tab聚焦状态背景颜色
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param1>     option          Description:QStyleOptionViewItem对象
    <param3>     bgRect          Description:绘制区域
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    const int radius = 8;

    //Highlight绘制区域的路径
    QPainterPath path;
    setPaintPath(bgRect, path, 3, 0, radius);

    //窗口色绘制区域的路径
    QPainterPath path2;
    setPaintPath(bgRect, path2, 5, 2, 6);

    //选中色绘制区域的路径
    QPainterPath path3;
    setPaintPath(bgRect, path3, 6, 3, 6);

    DPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                              ? DPalette::Normal : DPalette::Disabled;
    if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = DPalette::Inactive;
    }

    //通过绘制三个大小不一的区域，实现设计图中的tab选中的效果
    QColor fillColor = option.palette.color(cg, DPalette::Highlight);
    painter->setBrush(QBrush(fillColor));
    painter->fillPath(path, painter->brush());

    QColor fillColor2 = option.palette.color(cg, DPalette::Window);
    painter->setBrush(QBrush(fillColor2));
    painter->fillPath(path2, painter->brush());

    DStyleHelper styleHelper;
    DPalette pa = DApplicationHelper::instance()->applicationPalette();

    QColor fillColor3 = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
    fillColor3.setAlphaF(0.2);
    painter->setBrush(QBrush(fillColor3));
    painter->fillPath(path3, painter->brush());
}

/*************************************************************************
 <Function>      lengthAutoFeed
 <Description>   listview中字体名长度自适应处理
 <Author>        null
 <Input>
    <param1>     painter         Description:画家对象
    <param2>     sourceStr       Description:字体名字符串
    <param3>     m_StatusWidth   Description:名称区域宽度
 <Return>        QString         Description:处理过的字符串
 <Note>          null
*************************************************************************/
QString DFInstallErrorListDelegate::lengthAutoFeed(QPainter *painter, QString sourceStr, int m_StatusWidth) const
{
    //ut000442 listview中字体名长度自适应处理

    QFont nameFont = painter->font();
    QFontMetrics fontMetric(nameFont);

    QString m_TargetStr = sourceStr.left(1);
    QString m_Suffix("...");
    m_Suffix.append(sourceStr.right(5));
    m_TargetStr.append(m_Suffix);

    int m_TargetStrWidth = fontMetric.width(m_TargetStr);

    int m_index = 1;

    while (m_TargetStrWidth + m_StatusWidth < FTM_PARAM_OF_WIDTH) {

//      每次插入一个字符，直到长度超过最大范围
        m_TargetStr.insert(m_index, sourceStr.at(m_index));
        m_TargetStrWidth = fontMetric.width(m_TargetStr);
        m_index++;
    }

    QFont::cleanup();
//    m_TargetStr.append(m_Suffix);
//    qDebug() << m_TargetStr;
    return m_TargetStr;

}


/**
*  @brief  得到需要绘制区域的路径
*  @param[in]  bgRect listview一项的区域范围
*  @param[in]  path 需要绘制区域的路径
*  @param[in]  xDifference x方向需要变化的数值
*  @param[in]  yDifference y方向需要变化的数值
*  @param[in]  radius  圆弧半径
*/
void DFInstallErrorListDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
{
    QPoint path_bottomRight(bgRect.bottomRight().x() - xDifference, bgRect.bottomRight().y() - yDifference);
    QPoint path_topRight(bgRect.topRight().x() - xDifference, bgRect.topRight().y() + yDifference);
    QPoint path_topLeft(bgRect.topLeft().x() + xDifference, bgRect.topLeft().y() + yDifference);
    QPoint path_bottomLeft(bgRect.bottomLeft().x() + xDifference, bgRect.bottomLeft().y() - yDifference);
    path.moveTo(path_bottomRight - QPoint(0, 10));
    path.lineTo(path_topRight + QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_topRight - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(path_topLeft + QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_topLeft), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(path_bottomLeft - QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_bottomLeft - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(path_bottomRight - QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_bottomRight - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
}

/*************************************************************************
 <Function>      paint
 <Description>   代理绘制字体列表入口
 <Author>
 <Input>
    <param1>     painter         Description:画家对象
    <param1>     option          Description:QStyleOptionViewItem对象
    <param1>     index           Description:当前绘制对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    if (index.isValid()) {

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varErrorItem = index.data(Qt::DisplayRole);
        DFInstallErrorItemModel itemModel = varErrorItem.value<DFInstallErrorItemModel>();
        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        QRect rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        QRect bgRect = QRect(rect.left(), rect.top(), rect.width(), rect.height());

        if (itemModel.bSelectable) {
            drawSelectStatus(painter, option, bgRect);
            drawCheckBox(painter, itemModel, bgRect);
            drawFontName(painter, option, itemModel, bgRect);
            drawFontStyle(painter, option, itemModel, bgRect);
        } else {
            drawCheckBoxIcon(painter, bgRect);
            drawFontName(painter, option, itemModel, bgRect, false);
            drawFontStyle(painter, option, itemModel, bgRect, false);
        }
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
    QFont::cleanup();
}

/*************************************************************************
 <Function>      sizeHint
 <Description>   获得当前QModelIndex对象size
 <Author>
 <Input>
    <param1>     option          Description:画家对象
    <param2>     index           Description:当前绘制对象
 <Return>        QSize           Description:返回QModelIndex对象size
 <Note>          null
*************************************************************************/
QSize DFInstallErrorListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(index)
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        return QSize(option.rect.width(), 38);
    }
#endif
    return QSize(option.rect.width(), 48);
}

/*************************************************************************
 <Function>      DFInstallErrorListView
 <Description>   构造函数-构造字体列表视图实例对象
 <Author>
 <Input>
    <param1>     installErrorFontModelList Description:异常字体列表信息
    <param2>     parent                    Description:父对象
 <Return>        null                      Description:null
 <Note>          null
*************************************************************************/
DFInstallErrorListView::DFInstallErrorListView(const QList<DFInstallErrorItemModel> &installErrorFontModelList,
                                               QWidget *parent)
    : DListView(parent)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_errorListSourceModel(nullptr)
    , m_installErrorFontModelList(installErrorFontModelList)
{
//    QWidget *topSpaceWidget = new QWidget;
//    topSpaceWidget->setFixedSize(this->width(), 70 - FTM_TITLE_FIXED_HEIGHT);
//    this->addHeaderWidget(topSpaceWidget);

    setAutoScroll(true);
    setMouseTracking(true);

    initErrorListData();
    initDelegate();
    initSelectedItem();
    installEventFilter(this);
}

/*************************************************************************
 <Function>      ~DFInstallErrorListView
 <Description>   析构函数-析构字体列表视图实例对象
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallErrorListView::~DFInstallErrorListView()
{
    initModel(false);
    m_installErrorFontModelList.clear();
}

/*************************************************************************
 <Function>      initErrorListData
 <Description>   初始化异常字体列表信息
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::initErrorListData()
{
    initModel();
    DFontInfo fontInfo;
    for (int i = 0; i < m_installErrorFontModelList.size(); i++) {

        QStandardItem *item = new QStandardItem;
        DFInstallErrorItemModel itemModel = m_installErrorFontModelList.at(i);
        item->setData(QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_errorListSourceModel->appendRow(item);

        fontInfo = m_fontInfoManager->getFontInfo(itemModel.strFontFilePath);
        m_errorFontlist.append(fontInfo.familyName + fontInfo.styleName);
    }

    this->setModel(m_errorListSourceModel);

//    //设置默认选中第一行
//    QModelIndex firstRowModelIndex = m_errorListSourceModel->index(0, 0);

//    DFInstallErrorItemModel itemModel =
//        qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(firstRowModelIndex));

//    if (itemModel.bSelectable) {
//        selectionModel()->select(firstRowModelIndex, QItemSelectionModel::Select);
//        setCurrentIndex(firstRowModelIndex);
//    }
}

/*************************************************************************
 <Function>      initDelegate
 <Description>   初始化绘图代理-实例化代理对象
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::initDelegate()
{
    m_errorListItemDelegate = new DFInstallErrorListDelegate(this);
    this->setItemDelegate(m_errorListItemDelegate);
}

/*************************************************************************
 <Function>      addErrorListData
 <Description>   新增异常字体信息至列表
 <Author>
 <Input>
    <param1>     installErrorFontModelList Description:新增的异常字体列表信息
 <Return>        null                      Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::addErrorListData(const QList<DFInstallErrorItemModel> &installErrorFontModelList)
{
    initModel();

    QList<DFInstallErrorItemModel> m_newErrorFontModel;
    DFontInfo fontInfo;

    for (const DFInstallErrorItemModel &it : installErrorFontModelList) {
        fontInfo = m_fontInfoManager->getFontInfo(it.strFontFilePath);
        QString str = QString("%1%2").arg(fontInfo.familyName).arg(fontInfo.styleName);
        if (!m_errorFontlist.contains(str)) {
            m_newErrorFontModel.append(it);
            m_errorFontlist.append(str);
        }
    }
    m_installErrorFontModelList.append(m_newErrorFontModel);

//    QSet<QVariant> m_installErrorFontModelSet;

//    foreach (auto it, m_installErrorFontModelList) {
//        m_installErrorFontModelSet.insert(QVariant::fromValue(it));
//    }

//    qDebug() << m_installErrorFontModelSet.count() << endl;


    for (int i = 0; i < m_installErrorFontModelList.size(); i++) {
        QStandardItem *item = new QStandardItem;
        DFInstallErrorItemModel itemModel = m_installErrorFontModelList.at(i);
        item->setData(QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_errorListSourceModel->appendRow(item);
    }

    this->setModel(m_errorListSourceModel);
}

/*************************************************************************
 <Function>      checkScrollToIndex
 <Description>   检查是否需要滚动至指定QModelIndex
 <Author>
 <Input>
    <param1>     addHalfInstalledFiles Description:中途新增异常字体列表信息
    <param2>     oldHalfInstalledFiles Description:初始化时异常字体列表信息
    <param3>     errorFileList         Description:异常字体列表信息
 <Return>        null                  Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::checkScrollToIndex(QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles, QStringList &errorFileList)
{
    if (addHalfInstalledFiles.count() > 0) {
        scrollToIndex(addHalfInstalledFiles.first());
    } else if (addHalfInstalledFiles.count() == 0 && oldHalfInstalledFiles.count() != 0) {
        scrollToIndex(oldHalfInstalledFiles.first());
    } else if (addHalfInstalledFiles.count() == 0  && oldHalfInstalledFiles.count() == 0 && errorFileList.count() > 0) {
        scrollToIndex(errorFileList.first());
    } else {
        QModelIndex modelIndex = m_errorListSourceModel->index(beforeSelectRow, 0);
        scrollTo(modelIndex);
    }
}

/*************************************************************************
 <Function>      scrollToIndex
 <Description>   滚动至指定QModelIndex
 <Author>
 <Input>
    <param1>     filePath        Description:文件路径参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::scrollToIndex(QString &filePath)
{
    DFontInfo fontinfo = m_fontInfoManager->getFontInfo(filePath);
    QString file;
    if (!fontinfo.psname.compare("")) {
        file = fontinfo.familyName + fontinfo.styleName;
    } else {
        file = fontinfo.psname + fontinfo.styleName;
    }

    for (int i = 0; i < m_errorListSourceModel->rowCount(); i++) {
        QModelIndex modelIndex = m_errorListSourceModel->index(i, 0);
        QVariant varModel = m_errorListSourceModel->data(modelIndex, Qt::DisplayRole);
        DFInstallErrorItemModel itemData = varModel.value<DFInstallErrorItemModel>();
//            qDebug() << itemData.strFontFilePath << "!!!!!!!!!!!!" << endl;
        DFontInfo info =  m_fontInfoManager->getFontInfo(itemData.strFontFilePath);
        QString fileName;
        if (!info.psname.compare("")) {
            fileName = info.familyName + info.styleName;
        } else {
            fileName = info.psname + info.styleName;
        }
        if (file == fileName) {
            scrollTo(modelIndex);
        }
    }
}

/*************************************************************************
 <Function>      setSelectStatus
 <Description>   设置选中状态
 <Author>
 <Input>
    <param1>     HalfInstalledFiles Description:中途新增异常字体列表
    <param2>     beforeSelectFiles  Description:初始化时异常字体列表
 <Return>        null               Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::setSelectStatus(QStringList &HalfInstalledFiles, QModelIndexList &beforeSelectFiles)
{
    QStringList str;
    for (auto &it : HalfInstalledFiles) {
        DFontInfo fontinfo = m_fontInfoManager->getFontInfo(it);
        QString file;
        if (!fontinfo.psname.compare("")) {
            file = fontinfo.familyName + fontinfo.styleName;
        } else {
            file = fontinfo.psname + fontinfo.styleName;
        }
        str << file;
    }



    bool selected = false;

    for (int i = 0; i < m_errorListSourceModel->rowCount(); i++) {
        QModelIndex modelIndex = m_errorListSourceModel->index(i, 0);
        QVariant varModel = m_errorListSourceModel->data(modelIndex, Qt::DisplayRole);
        DFInstallErrorItemModel itemData = varModel.value<DFInstallErrorItemModel>();
        DFontInfo info =  m_fontInfoManager->getFontInfo(itemData.strFontFilePath);
        QString fileName;
        if (!info.psname.compare("")) {
            fileName = info.familyName + info.styleName;
        } else {
            fileName = info.psname + info.styleName;
        }
        if (str.contains(fileName)) {
            if (!itemData.bChecked && itemData.bSelectable) {
                itemData.bChecked = true;
                updateErrorFontModelList(i, itemData);
                m_errorListSourceModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }

            if (itemData.bSelectable) {
                selected = true;
                selectionModel()->select(modelIndex, QItemSelectionModel::Select);
            }
        }
    }

    if (selected == false) {
//        this->selectionModel()->select(beforeSelectFiles)
        for (auto &it : beforeSelectFiles) {
//            selectionModel()->select(it, QItemSelectionModel::Select);
            QModelIndex modelIndex = m_errorListSourceModel->index(it.row(), 0);
            selectionModel()->select(modelIndex, QItemSelectionModel::Select);
        }
    }
}

/*************************************************************************
 <Function>      updateErrorFontModelList
 <Description>   更新DFInstallErrorItemModel信息
 <Author>
 <Input>
    <param1>     m_currentItemModel Description:当前需要更新的ItemModel
    <param2>     index              Description:当前需要更新的位置索引
    <param3>     null            Description:
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::updateErrorFontModelList(int index, DFInstallErrorItemModel m_currentItemModel)
{
    m_installErrorFontModelList.replace(index, m_currentItemModel);
}

/*************************************************************************
 <Function>      initModel
 <Description>   初始化ItemModel对象
 <Author>
 <Input>
    <param1>     newOne          Description:是否新建
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::initModel(bool newOne)
{
    if (m_errorListSourceModel != nullptr) {
        int rowCnt = m_errorListSourceModel->rowCount();
        m_errorListSourceModel->removeRows(0, rowCnt);
        for (int i = rowCnt; i >= 0; i--) {
            QStandardItem *item = m_errorListSourceModel->takeItem(i);
            if (item)
                delete item;
        }
        m_errorListSourceModel->clear();
        delete m_errorListSourceModel;
        m_errorListSourceModel = nullptr;
    }

    if (newOne)
        m_errorListSourceModel = new QStandardItemModel(this);
}

/*************************************************************************
 <Function>      getErrorListSourceModel
 <Description>   获取当前QStandardItemModel实例对象
 <Author>
 <Input>         null
 <Return>        m_errorListSourceModel Description:当前QStandardItemModel实例对象
 <Note>          null
*************************************************************************/
QStandardItemModel *DFInstallErrorListView::getErrorListSourceModel()
{
    return m_errorListSourceModel;
}

/*************************************************************************
 <Function>      getIsTabFocus
 <Description>   获取当前是否为tab聚焦状态
 <Author>
 <Input>         null
 <Return>        m_IsTabFocus Description:是否为tab聚焦状态标志位
 <Note>          null
*************************************************************************/
bool DFInstallErrorListView::getIsTabFocus() const
{
    return m_IsTabFocus;
}

/*************************************************************************
 <Function>      mousePressEvent
 <Description>   重写鼠标press事件
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint point = event->pos();
        QModelIndex modelIndex = indexAt(point);
        DFInstallErrorItemModel itemModel =
            qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(modelIndex));
        if (itemModel.bIsNormalUserFont && getIconRect(visualRect(modelIndex)).contains(point))
            emit clickedErrorListItem(modelIndex);
    }

    DListView::mousePressEvent(event);
}

/*************************************************************************
 <Function>      setSelection
 <Description>   重写设置选中函数
 <Author>
 <Input>
    <param1>     rect            Description:选中区域
    <param2>     command         Description:选中命令参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);
}

/*************************************************************************
 <Function>      selectNextIndex
 <Description>   判断下个选中
 <Author>
 <Input>
    <param1>     nextIndex       Description:下个选中项索引
 <Return>        bool            Description:返回下个选中结果
 <Note>          null
*************************************************************************/
bool DFInstallErrorListView::selectNextIndex(int nextIndex)
{
    if (nextIndex == currentIndex().row() && selectedIndexes().count() != 0)//循环到当前选中时，结束
        return true;

    QModelIndex nextModelIndex = m_errorListSourceModel->index(nextIndex, 0);
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(getErrorListSourceModel()->data(nextModelIndex));
    if (itemModel.bIsNormalUserFont) {
        if (selectedIndexes().count() == 0)//没有选项则设置并滚动到第一个可选项
            scrollTo(nextModelIndex);
        setCurrentIndex(nextModelIndex);
        return true;
    } else {
        return false;
    }
//    return false;
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   重写键盘press事件-安装验证页面，listview上下键自动跳过异常字体
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::keyPressEvent(QKeyEvent *event)
{
    bool isEventResponsed = false;
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
            if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
                isEventResponsed = true;
            }
        } else if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
            DListView::keyPressEvent(event);
            isEventResponsed = true;
        } else {
            bool noneSelection = false;
            int cnt = 0;
            if (event->key() == Qt::Key_Down) {//循环判断是否可选,如果没有可选则不改变选项
                for (int i = currentIndex().row() + 1; i <= this->count(); i++) {
                    //没有选项则从头开始检查是否可选
                    if (selectedIndexes().count() == 0 && cnt == 0) {
                        i = 0;
                        noneSelection = true;
                    }
                    cnt++;
                    if (selectNextIndex(i)) {
                        return;
                    }
                    if (i == this->count() - 1 && noneSelection == true)
                        return;
                    if (i == (this->count() - 1) || i == this->count())
                        i = -1;
                }
                isEventResponsed = true;
            } else {//循环判断是否可选,如果没有可选则不改变选项
                for (int i = currentIndex().row() - 1; i >= -1; i--) {
                    //没有选项则从底部开始检查是否可选
                    if (selectedIndexes().count() == 0 && cnt == 0) {
                        i = count() - 1;
                        noneSelection = true;
                    }
                    cnt++;
                    if (selectNextIndex(i))
                        return;
                    if (i == 0 && noneSelection == true)
                        return;
                    if (i == 0 || i == -1)
                        i = this->count();
                }
                isEventResponsed = true;
            }
        }
    } else if (event->key() == Qt::Key_Home && event->modifiers() == Qt::NoModifier) {
        responseToHomeAndEnd(true);
        isEventResponsed = true;
    } else if (event->key() == Qt::Key_End && event->modifiers() == Qt::NoModifier) {
        responseToHomeAndEnd(false);
        isEventResponsed = true;
    } else if (event->key() == Qt::Key_PageUp && event->modifiers() == Qt::NoModifier) {
        responseToPageUpAndPageDown(true);
        isEventResponsed = true;
    } else if (event->key() == Qt::Key_PageDown && event->modifiers() == Qt::NoModifier) {
        responseToPageUpAndPageDown(false);
        isEventResponsed = true;
    }
    //如果事件已被接收或事件为空格按键，不做响应，直接退出，如果以后要实现空格功能，在这里添加即可
    if (isEventResponsed || event->key() == Qt::Key_Space)
        return;
    DListView::keyPressEvent(event);
}
/*************************************************************************
 <Function>      initSelectedItem
 <Description>   初始选中第一个可选项
 <Author>        UT000539
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::initSelectedItem()
{
    if (selectionModel()->selectedIndexes().count() == 0) {
        for (int i = 0; i < this->count(); i++) {
            if (selectNextIndex(i))
                break;
        }
    }
}

/*************************************************************************
 <Function>      getIconRect
 <Description>   获取图标范围
 <Author>        UT000539
 <Input>
    <param1>     rect            Description:当前modelindex范围
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QRect DFInstallErrorListView::getIconRect(const QRect &rect)
{
    int checkBoxSize = 20 + 10;
    return QRect(rect.left() + 15, rect.top() + 10, checkBoxSize, checkBoxSize);
}

/*************************************************************************
 <Function>      responseToHomeAndEnd
 <Description>   响应home和end快捷按键-fix bug 43109-便于父对象调用，整理成函数
 <Author>        UT000539
 <Input>
    <param1>     isHomeKeyPressed Description:区分home还是end
    <param2>     null             Description:
    <param3>     null             Description:
 <Return>        null             Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::responseToHomeAndEnd(bool isHomeKeyPressed)
{
    QModelIndex index;
    if (isHomeKeyPressed) {
        index = m_errorListSourceModel->index(0, 0);
        scrollToTop();
    } else {
        index = m_errorListSourceModel->index(count() - 1, 0);
        scrollToBottom();
    }
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(index));
    clearSelection();
    if (itemModel.bIsNormalUserFont) {
        selectionModel()->select(index, QItemSelectionModel::Select);
        setCurrentIndex(index);
    }
}

/*************************************************************************
 <Function>      responseToPageUpAndPageDown
 <Description>   响应PageUp和PageDown快捷按键-便于父对象调用，整理成函数
 <Author>
 <Input>
    <param1>     isPageUpPressed Description:区分是PageUp还是PageDown
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorListView::responseToPageUpAndPageDown(bool isPageUpPressed)
{
    if (isPageUpPressed) {
        QModelIndex firstVisibleItem = indexAt(QPoint(3, 3));
        if (firstVisibleItem.isValid()) {
            scrollTo(firstVisibleItem, QAbstractItemView::PositionAtBottom);
        }
    } else {
        QRect visibleRect = geometry();
        QModelIndex lastVisibleItem = indexAt(QPoint(3, visibleRect.height() - 3));
        if (lastVisibleItem.isValid()) {
            scrollTo(lastVisibleItem, QAbstractItemView::PositionAtTop);
        }
    }
}

/*******************************************************************************
 1. @函数:    ifNeedScrollTo(QModelIndex idx)
 2. @作者:    UT000539 宁玉闯
 3. @参数:    idx:当前选中的QModelIndex
 4. @说明:    不可见则滚动到modelindex
*******************************************************************************/
void DFInstallErrorListView::ifNeedScrollTo(QModelIndex idx)
{
    if (!viewport()->visibleRegion().contains(visualRect(idx).center()))
        scrollTo(idx);
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   重写事件过滤器
 <Author>
 <Input>
    <param1>     obj             Description:事件接收对象
    <param2>     event           Description:事件对象
 <Return>        bool            Description:返回事件处理结果,true：已处理；false：未处理
 <Note>          null
*************************************************************************/
bool DFInstallErrorListView::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    //失去焦点时重置各个标志位
    if (event->type() == QEvent::FocusOut) {
        QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);

        //需要排除因为窗口切换导致errorlistview失去焦点的情况，这种情况不需要清空标志位
        if (focusEvent->reason() != Qt::ActiveWindowFocusReason) {
            m_IsTabFocus = false;
        }

    }

    if (event->type() == QEvent::FocusIn) {
        QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);

        //需要排除因为窗口切换导致errorlistview获取到焦点的情况
        if (focusEvent->reason() != Qt::ActiveWindowFocusReason) {
            //不是因为窗口切换获取到焦点时，判断获取焦点的方式，如果不是通过鼠标点击或者安装过程后设置的焦点，就
            //判断为通过tab获取到的焦点。
            m_IsTabFocus = true;
        }

        //没有选中项时，切换到异常字体列表时，默认选中第一个
        initSelectedItem();
        /*else {
                    QModelIndex selectIndex = selectionModel()->selectedIndexes().first();
                    selectNextIndex(selectIndex.row());
                    scrollTo(selectIndex);
                }*/
    }

    return false;
}
