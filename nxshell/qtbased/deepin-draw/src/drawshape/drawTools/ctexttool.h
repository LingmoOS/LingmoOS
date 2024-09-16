// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CTEXTTOOL_H
#define CTEXTTOOL_H
#include "idrawtool.h"

class CGraphicsTextItem;
class CTextTool : public IDrawTool
{
    Q_OBJECT
public:
    CTextTool();

    ~CTextTool() override;

    DrawAttribution::SAttrisList attributions() override;

    void registerAttributionWidgets() override;

    QCursor cursor() const override;

protected:

    /**
     * @brief toolButton 定义工具的激活按钮
     */
    QAbstractButton *initToolButton() override;

    /**
     * @brief toolFinish 工具执行的结束
     * @param event 事件
     * @param scene 场景
     */
    void toolCreatItemFinish(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief toolFinish 创建图元
     * @param event 事件
     * @param scene 场景
     */
    CGraphicsItem *creatItem(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief minMoveUpdateDistance　工具创造业务图元指针
     */

    int minMoveUpdateDistance() override;

    /**
     * @brief isPressEventHandledByQt　鼠标点下是否要交给qt框架处理
     */
    bool isPressEventHandledByQt(CDrawToolEvent *event, ITERecordInfo *pInfo) override;

    /**
     * @brief autoSupUndoForCreatItem 是否支持在创建生成图元后自动添加到undo栈(默认是true)
     */
    bool autoSupUndoForCreatItem() override {return false;}

    bool rasterItemToLayer(CDrawToolEvent *event, IDrawTool::ITERecordInfo *pInfo) override;

    bool eventFilter(QObject *object, QEvent *e) override;

private:
    void initFontFamilyWidget(QComboBox *fontHeavy);
    void initFontWeightWidget();
    void reInitFontWeightComboxItems(const QString &family, QComboBox *fontHeavy);
    void initFontFontSizeWidget();

    bool isTextEnableUndoThisTime(bool considerRecorderEmpty = true);

    Q_SLOT void transferFocusBack();

    Q_SLOT void onSizeChanged(int fontSz, bool backFocus = true);

    void resetItemsFontFamily();
    void cachedItemsFontFamily();
    void restoreItemsFontFamily();


private:
    QComboBox *m_fontComBox = nullptr;
    QComboBox *m_fontHeavy = nullptr;
    QComboBox *m_fontSize = nullptr;

    bool      _activePackup = false;
    bool      _fontViewShowOut = false;
    QString   _cachedFontWeightStyle;

    QStringList supWeightStyleList;

    struct Font {
        QString fontFamily;
        QString FontWeight;
    };

    QMap<CGraphicsTextItem *, Font> _cachedFontFamily;


    int        _currenFontSize = 14;
    QList<int> _defaultFontSizeSet;

};

class QComboxMenuDelegate: public QAbstractItemDelegate
{
    Q_OBJECT
public:
    QComboxMenuDelegate(QObject *parent, QComboBox *cmb)
        : QAbstractItemDelegate(parent), mCombo(cmb), pressedIndex(-1)
    {}

    void dontShowCheckState(bool b) {showCheckStates = !b;}

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        if (!showCheckStates)
            opt.checked = false;
        painter->fillRect(option.rect, opt.palette.window());
        mCombo->style()->drawControl(QStyle::CE_MenuItem, &opt, painter, mCombo);
    }
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override
    {
        QStyleOptionMenuItem opt = getStyleOption(option, index);
        return mCombo->style()->sizeFromContents(
                   QStyle::CT_MenuItem, &opt, option.rect.size(), mCombo);
    }
//    bool editorEvent(QEvent *event, QAbstractItemModel *model,
//                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QStyleOptionMenuItem getStyleOption(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const;
    QComboBox *mCombo;
    int pressedIndex;
    bool showCheckStates = false;
};


#endif // CTEXTTOOL_H
