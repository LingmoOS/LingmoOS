// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QWidget>

#include <QPlainTextEdit>

class QLabel;
class QStackedWidget;

namespace GrandSearch {
namespace text_preview {

class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    using QPlainTextEdit::QPlainTextEdit;
protected:
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
};

class TextView : public QWidget
{
    Q_OBJECT
public:
    static bool checkUosAiInstalled();

    explicit TextView(QWidget *parent = nullptr);
    void initUI();
    void setSource(const QString &path);
    static QString toUnicode(const QByteArray &data);
    void showErrorPage();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
     PlainTextEdit *m_browser = nullptr;
     QLabel *m_errLabel = nullptr;
     QStackedWidget *m_stackedWidget = nullptr;
     bool m_isShowAiToolBar = false;
};

}}

#endif // TEXTVIEW_H
