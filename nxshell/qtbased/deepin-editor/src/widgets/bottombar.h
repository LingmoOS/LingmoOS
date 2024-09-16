// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include <QWidget>
#include <QLabel>
#include <DLabel>
#include "ddropdownmenu.h"
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QPainterPath>
#include <DProgressBar>

#define FormatActionType "format-action-type"

class EditWrapper;
class BottomBar : public QWidget
{
    Q_OBJECT

public:
    enum EndlineFormat{
        Unknow = -1,
        Unix,
        Windows
    };

public:
    explicit BottomBar(QWidget *parent = nullptr);
    ~BottomBar();

    void updatePosition(int row, int column);
    void updateWordCount(int charactorCount);
    void setEncodeName(const QString &name);
    void setCursorStatus(const QString &text);
    void setPalette(const QPalette &palette);
    void updateSize(int size, bool bIsFindOrReplace);
    void setChildEnabled(bool enabled);
    //设置所有焦点　梁卫东　２０２０－０９－１４　１０：５５：２２
    void setChildrenFocus(bool ok,QWidget* preOrderWidget = nullptr);
    void setScaleLabelText(qreal fontSize);
    void setProgress(int progress);

    DDropdownMenu* getEncodeMenu();
    DDropdownMenu* getHighlightMenu();
    static EndlineFormat getEndlineFormat(const QByteArray& text);
    EndlineFormat getEndlineFormat();
    void setEndlineMenuText(EndlineFormat format);
    static int defaultHeight();

protected:
    void paintEvent(QPaintEvent *);

private:
    void initFormatMenu();
    Q_SLOT void onFormatMenuTrigged(QAction* action);
    Q_SLOT void updateSizeMode();

private:
    EditWrapper *m_pWrapper {nullptr};
    DLabel *m_pPositionLabel {nullptr};
    DLabel *m_pCharCountLabel {nullptr};
    DLabel *m_pCursorStatus {nullptr};
    DDropdownMenu *m_pEncodeMenu {nullptr};
    DDropdownMenu *m_pHighlightMenu {nullptr};
    QString m_rowStr {QString()};
    QString m_columnStr {QString()};
    QString m_chrCountStr {QString()};
    bool m_bIsFindOrReplace {false};
    DLabel* m_scaleLabel = nullptr;
    DLabel* m_progressLabel = nullptr;
    DProgressBar* m_progressBar = nullptr;
    DDropdownMenu *m_formatMenu = nullptr;
    EndlineFormat m_endlineFormat = EndlineFormat::Unix;
    QAction* m_unixAction = nullptr;
    QAction* m_windowsAction = nullptr;


public slots:
	//编码按钮/文本类型按钮失去焦点后，设置光标回到文本框里
    void slotSetTextEditFocus();
};

#endif
