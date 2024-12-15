// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSTEXTITEM_H
#define CGRAPHICSTEXTITEM_H
#include "cgraphicsrectitem.h"

#include <QTextDocument>
#include <QTextCharFormat>
#include <QWidget>
#include <QPainterPath>

class CTextEdit;
class CGraphicsProxyWidget;
class CGraphicsItemHighLight;

class CGraphicsTextItem : public CGraphicsRectItem
{
public:
    /**
     * @brief CGraphicsTextItem 构造函数
     */
    explicit CGraphicsTextItem(const QString &text = "", CGraphicsItem *parent = nullptr);

    /**
     * @brief CGraphicsItem 析构函数
     */
    ~CGraphicsTextItem() override;

    DrawAttribution::SAttrisList attributions() override;
    void  setAttributionVar(int attri, const QVariant &var, int phase) override;

    /**
     * @brief textEditor 文字编辑控件
     */
    CTextEdit *textEditor() const;

    /**
     * @brief type 文字图元的类型
     */
    int  type() const override;

    /**
     * @brief setRect 设置文字图元的大小
     */
    void setRect(const QRectF &rect) override;

    /**
     * @brief updateProxyItemPos 刷新代理图元的位置
     */
    void updateProxyItemPos();

    /**
     * @brief font 文字的字体
     */
    QFont font() const;

    /**
     * @brief setFont 设置文字的字体
     */
    void setFont(const QFont &font);

    /**
     * @brief setFontStyle 设置文字字体的样式
     */
    void setFontStyle(const QString &style);

    /**
     * @brief fontStyle 文字字体的样式
     */
    QString fontStyle();

    /**
     * @brief setFontSize 设置文字字体的大小(pt)
     */
    void setFontSize(int size);

    /**
     * @brief fontSize 文字字字体的大小(pt)
     */
    int fontSize();

    /**
     * @brief setFontFamily 设置文字字体的族名
     */
    void setFontFamily(const QString &family);

    /**
     * @brief fontFamily 文字字体的族名
     */
    QString fontFamily()const;

    /**
     * @brief setTextColor 设置文字的颜色
     */
    void setTextColor(const QColor &col);

    /**
     * @brief textColor 文字的颜色
     */
    QColor textColor() const;

    /**
     * @brief setCurrentFormat 设置字符格式
     */
    void setCurrentFormat(const QTextCharFormat &format, bool merge = false);

    /**
     * @brief updateTextFormat 从当前view的CDrawParamSigleton缓存中读取文字的信息进行格式刷新
     */
    void updateToDefaultTextFormat();

    /**
     * @brief loadGraphicsUnit 加载文字图元数据
     */
    void loadGraphicsUnit(const CGraphicsUnit &data) override;

    /**
     * @brief getGraphicsUnit 获取当前文字图元的数据
     */
    CGraphicsUnit getGraphicsUnit(EDataReason reson) const override;

    /**
     * @brief setTextState 设置当前文字的状态
     * @param state EReadOnly表示只读的状态,EInEdit表示编辑中的状态
     * @param selectAllText 切换状态后是否选中编辑框内文字
     */
    enum EState {EReadOnly, EInEdit};
    void setTextState(EState state, bool selectAllText = true);

    /**
     * @brief textState 当前文字状态(EReadOnly:0表示只读的状态,EInEdit:1表示编辑中的状态)
     */
    EState textState() const;

    /**
     * @brief isEditState 当前是否是编辑状态
     */
    bool   isEditState() const;

    /**
     * @brief toFocusEiditor 使编辑控件占有焦点
     */
    void   toFocusEiditor();

public:
    /**
     * @brief 右键餐单功能
     */
    void doCut();
    void doCopy();
    void doPaste();
    void doSelectAll();
    void setSelectTextBlockAlign(const Qt::Alignment &align);
    void doUndo();
    void doRedo();
    void doDelete();

    /**
     * @brief setAutoAdjustSize 设置是否根据文本的内容调整大小
     */
    void setAutoAdjustSize(bool b);

    /**
     * @brief isAutoAdjustSize 是否根据文本的内容调整大小(否则就是自动换行)
     */
    bool isAutoAdjustSize() const;

    /**
     * @brief getHighLightPath 获取高亮path
     * @return
     */
    QPainterPath getHighLightPath() override;

    /**
     * @brief currentCharFormat 获取到当前的字体格式
     */
    QTextCharFormat currentCharFormat();

    /**
     * @brief isSelectionEmpty 是否存在选中的文字
     */
//    bool isSelectionEmpty();

    /**
     * @brief beginPreview 如果某个改动是频繁连续的,在改动的过程中需要预览,那么在开始前可以调用该函数进行预览准备
     */
    void beginPreview();

    /**
     * @brief endPreview 结束预览
     * @param revert 是否加载回开始预览前的信息(还原)
     */
    void endPreview(bool revert = true);

    /**
     * @brief beginPreview 是否正在预览中
     */
    bool isPreview();

protected:

    /**
     * @brief changToEditState 切换到编辑状态
     */
    void changToEditState(bool selectAll = true);

    /**
     * @brief changToReadOnlyState 切换到编辑状态
     */
    void changToReadOnlyState(bool selectAll = true);

    /**
     * @brief mouseDoubleClickEvent 图元被双击时需要进入编辑状态
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief isPosPenetrable 是否在某个位置是可穿透的
     * @param posLocal 位置点(在自身坐标系)
     */
    bool isPosPenetrable(const QPointF &posLocal) override;

    /**
     * @brief operatingEnd  操作结束(scale后要设置标记为)
     */
    //void operatingBegin(CGraphItemEvent *event) override;

    void doScalBegin(CGraphItemScalEvent *event) override;

    /**
     * @brief itemChange 图元变更
     * @param change 变更属性
     * @param value 变更的值
     * @return
     */
    QVariant itemChange(GraphicsItemChange doChange, const QVariant &value) override;

    /**
     * @brief paint 图元的绘制
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;


    /**
     * @brief paintSelf 绘制自身的样貌
     */
    void paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option) override;

private:
    /**
     * @brief drawDocument  绘制出文章内容(仅当处于Readonly状态时才会被调用绘制,参见paint函数)
     */
    void drawDocument(QPainter *painter,
                      const QTextDocument *doc,
                      const QRectF &r = QRectF());

    /**
     * @brief initTextEditor 初始化编辑控件
     */
    void initTextEditor(const QString &text = "");

private:
    CTextEdit            *m_pTextEdit = nullptr;
    CGraphicsProxyWidget *m_pProxy    = nullptr;

    QFont  m_Font;
    QColor m_color;
    bool   _autoAdjustSize = true;
    bool   _isPreview      = false;
};

#endif // CGRAPHICSTEXTITEM_H
