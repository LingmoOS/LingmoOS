/*******************************************************************
 * statuswidget.h
 * SPDX-FileCopyrightText: 2009, 2010 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/
#ifndef STATUSWIDGET__H
#define STATUSWIDGET__H

#include <QEvent>
#include <QLabel>
#include <QStackedWidget>
#include <QTextDocument>

class WrapLabel;
class KBusyIndicatorWidget;
class QHideEvent;

class StatusWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit StatusWidget(QWidget *parent = nullptr);

    void setBusy(const QString &);
    void setIdle(const QString &);

    void addCustomStatusWidget(QWidget *);

private:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

    void setBusyCursor();
    void setIdleCursor();

    WrapLabel *m_statusLabel;

    KBusyIndicatorWidget *m_throbberWidget;
    WrapLabel *m_busyLabel;

    QWidget *m_statusPage;
    QWidget *m_busyPage;

    int m_cursorStackCount;
    bool m_busy;
};

// Dummy class to avoid a QLabel+wordWrap height bug
class WrapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit WrapLabel(QWidget *parent = nullptr)
        : QLabel(parent)
    {
        setWordWrap(true);
    }

    void setText(const QString &text)
    {
        QLabel::setText(text);
        adjustHeight();
    }

    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::ApplicationFontChange || e->type() == QEvent::Resize) {
            adjustHeight();
        }
        return QLabel::event(e);
    }

private:
    void adjustHeight()
    {
        QTextDocument document(text());
        document.setTextWidth(width());
        setMaximumHeight(document.size().height());
    }
};

#endif
