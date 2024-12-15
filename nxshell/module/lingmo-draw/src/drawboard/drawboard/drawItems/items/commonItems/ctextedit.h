// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CTEXTEDIT_H
#define CTEXTEDIT_H
#include <globaldefine.h>
#include <QTextEdit>

#include <QMenu>
#include <QGraphicsView>

#include <QInputMethodEvent>
#include <QTextLayout>

//#ifdef USE_DTK
//DWIDGET_USE_NAMESPACE
//#endif

class TextItem;

class DRAWLIB_EXPORT TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit TextEdit(TextItem *item, QWidget *parent = nullptr);
    ~TextEdit() override;

    /**
     * @brief currentFormat 当前文字的格式
     */
    QTextCharFormat currentFormat(bool considerSelection = true);

    /**
     * @brief setCurrentFormat 设置当前文字的格式
     */
    void setCurrentFormat(const QTextCharFormat &format, bool merge = false);

    /**
     * @brief currentColor 当前文字的颜色
     */
    QColor   currentColor();

    /**
     * @brief setCurrentColor 设置当前文字的格式
     */
    void setCurrentColor(const QColor &color);

    /**
     * @brief currentColor 当前文字的字体
     */
    QFont    currentFont(bool considerSelection = true);

    /**
     * @brief setCurrentColor 设置当前文字的格式
     */
    void setCurrentFont(const QFont &ft);

    /**
     * @brief currentColor 当前文字字体的字号(pt)
     */
    int  currentFontSize();

    /**
     * @brief setCurrentColor 设置当前文字字体的字号(pt)
     */
    void setCurrentFontSize(const int sz);

    /**
     * @brief currentFontFamily 当前文字字体的字体族
     */
    QString  currentFontFamily();

    /**
     * @brief setCurrentFontFamily 设置当前文字字体的字体族
     */
    void setCurrentFontFamily(const QString &family);

    /**
     * @brief currentFontStyle 当前文字字体的样式
     */
    QString  currentFontStyle();

    /**
     * @brief setCurrentFontStyle 设置当前文字字体的样式
     */
    void setCurrentFontStyle(const QString &style);


    /* 刷新背景色 (使用场景:因为不再随主题的变化而变化就要固定背景色)*/
    void updateBgColorTo(const QColor c = QColor(255, 255, 255), bool laterDo = false);

    /**
    * @brief toWeight 将style字符串转成int的字体粗细
    */
    static int  toWeight(const QString &styleName);

    /**
    * @brief toStyle 将字重int值转成style字符串
    */
    static QString toStyle(const int &weight);

    /**
    * @brief firstPosFormat 当前光标位置0的字格式
    */
    QTextCharFormat firstPosFormat()const;

    /**
    * @brief setDefaultFormat 设置默认字格式
    */
    void setDefaultFormat(const QTextCharFormat &format);

    /**
    * @brief setMenu 设置右键菜单
    */
    void setMenu();
    /**
    * @brief clearSelectState 取消文本选中状态
    */
    void clearSelectState();

public slots:
    /**
    * @brief onTextChanged 当doc中内容变化时进行响应(主要是实现自动调整大小)
    */
    void onTextChanged();

    /**
    * @brief onCursorPositionChanged 当光标位置变化时响应(刷新属性界面展示)
    */
    void onCursorPositionChanged();

    /**
    * @brief markCursorDataDirty 标识光标标记的文本为脏数据, 重新进行文本布局
    *       主要调用 QTextDocument 提供的 markContentsDirty() 函数
    */
    void markCursorDataDirty();

    /**
    * @brief onSelectionChanged 选中区域变化时响应(刷新属性界面展示)
    */
    void onSelectionChanged();

    /**
    * @brief onCurrentCharFormatChanged 格式发生变化
    */
    void onCurrentCharFormatChanged(const QTextCharFormat &format);

    /**
    * @brief updatePropertyWidget 刷新属性界面展示
    */
    void updatePropertyWidget();

    /**
    * @brief applyDefaultToFirstFormat 设置默认的字格式为当前光标位置0的字格式
    */
    void applyDefaultToFirstFormat();

protected:
    void insertFromMimeData(const QMimeData *source) override;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void inputMethodEvent(QInputMethodEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    QVector<QTextLayout::FormatRange> getCharFormats(int posBegin, int posEnd);

    void updateSelectionFormat();

private:
    PRIVATECLASS(TextEdit)
};
Q_DECLARE_METATYPE(QTextCharFormat)  //声明变量
#endif // CTEXTEDIT_H
