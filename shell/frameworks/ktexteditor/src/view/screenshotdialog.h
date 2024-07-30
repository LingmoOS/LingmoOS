/*
    SPDX-FileCopyrightText: 2023 Waqar Ahmed <waqar.17a@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KTE_Screenshot_dialog_h
#define KTE_Screenshot_dialog_h

#include <KTextEditor/Range>
#include <QDialog>

class QPushButton;
class KateRenderer;
class BaseWidget;
class QScrollArea;
class QToolButton;
class QTimer;
class QMenu;
class QCheckBox;

namespace KTextEditor
{
class ViewPrivate;
}

class ScreenshotDialog : public QDialog
{
public:
    explicit ScreenshotDialog(KTextEditor::Range selRange, KTextEditor::ViewPrivate *parent = nullptr);
    ~ScreenshotDialog() override;

    void renderScreenshot(KateRenderer *renderer);

private:
    void onSaveClicked();
    void onCopyClicked();
    enum {
        DontShowLineNums,
        ShowAbsoluteLineNums,
        ShowActualLineNums,
    };
    void onLineNumChangedClicked(int i);
    void resizeEvent(QResizeEvent *e) override;

private:
    BaseWidget *const m_base;
    const KTextEditor::Range m_selRange;
    QScrollArea *const m_scrollArea;
    QPushButton *const m_saveButton;
    QPushButton *const m_copyButton;
    QPushButton *const m_changeBGColor;
    QToolButton *const m_lineNumButton;
    QCheckBox *const m_extraDecorations;
    QCheckBox *const m_windowDecorations;
    QMenu *const m_lineNumMenu;
    QTimer *m_resizeTimer;
    bool m_firstShow = true;
    bool m_showLineNumbers = true;
    bool m_absoluteLineNumbers = true;
};

#endif
