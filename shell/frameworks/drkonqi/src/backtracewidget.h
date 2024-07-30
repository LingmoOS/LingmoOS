/*******************************************************************
 * backtracewidget.h
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#ifndef BACKTRACEWIDGET__H
#define BACKTRACEWIDGET__H

#include <QWidget>

#include "debugpackageinstaller.h"
#include "ui_backtracewidget.h"

namespace KSyntaxHighlighting
{
class SyntaxHighlighter;
}
class BacktraceRatingWidget;
class BacktraceGenerator;

class BacktraceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BacktraceWidget(BacktraceGenerator *generator, QWidget *parent = nullptr, bool showToggleBacktrace = false);

    bool canInstallDebugPackages() const;

public Q_SLOTS:
    void generateBacktrace();
    void highlightExtraDetailsLabel(bool highlight);
    void focusImproveBacktraceButton();

    void toggleBacktrace(bool show);
    void extraDetailsLinkActivated(QString link);

Q_SIGNALS:
    void stateChanged();

private:
    BacktraceGenerator *m_btGenerator = nullptr;
    Ui::Form ui;
    BacktraceRatingWidget *m_backtraceRatingWidget = nullptr;
    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter = nullptr;
    DebugPackageInstaller *m_debugPackageInstaller = nullptr;

    void setAsLoading();

private Q_SLOTS:
    void loadData();
    void backtraceNewLine(const QString &);

    void regenerateBacktrace();

    void saveClicked();
    void copyClicked();

    void anotherDebuggerRunning();

    void installDebugPackages();
    void debugPackageError(const QString &);
    void debugPackageCanceled();
};

#endif
