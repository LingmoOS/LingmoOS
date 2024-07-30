/*
    SPDX-FileCopyrightText: 2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_APP_COMMANDS_H
#define KATEVI_APP_COMMANDS_H

#include <KTextEditor/Command>
#include <QObject>
#include <QRegularExpression>

namespace KTextEditor
{
class MainWindow;
}

namespace KateVi
{
class AppCommands : public KTextEditor::Command
{
    AppCommands();
    static AppCommands *m_instance;

public:
    ~AppCommands() override;
    bool exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range = KTextEditor::Range::invalid()) override;
    bool help(KTextEditor::View *view, const QString &cmd, QString &msg) override;

    static AppCommands *self()
    {
        if (m_instance == nullptr) {
            m_instance = new AppCommands();
        }
        return m_instance;
    }

private:
    /**
     * @returns a view in the given \p window that does not share a split
     * view with the given \p view. If such view could not be found, then
     * nullptr is returned.
     */
    static KTextEditor::View *findViewInDifferentSplitView(KTextEditor::MainWindow *window, KTextEditor::View *view);

private:
    void closeCurrentDocument();
    void closeDocuments(const QList<KTextEditor::Document *> &documents);
    void closeCurrentView();
    void closeCurrentSplitView();
    void closeOtherSplitViews();
    void quit();

private:
    const QRegularExpression re_write;
    const QRegularExpression re_close;
    const QRegularExpression re_quit;
    const QRegularExpression re_exit;
    const QRegularExpression re_edit;
    const QRegularExpression re_tabedit;
    const QRegularExpression re_new;
    const QRegularExpression re_split;
    const QRegularExpression re_vsplit;
    const QRegularExpression re_vclose;
    const QRegularExpression re_only;
};

class BufferCommands : public KTextEditor::Command
{
    BufferCommands();
    static BufferCommands *m_instance;

public:
    ~BufferCommands() override;
    bool exec(KTextEditor::View *view, const QString &cmd, QString &msg, const KTextEditor::Range &range = KTextEditor::Range::invalid()) override;
    bool help(KTextEditor::View *view, const QString &cmd, QString &msg) override;

    static BufferCommands *self()
    {
        if (m_instance == nullptr) {
            m_instance = new BufferCommands();
        }
        return m_instance;
    }

private:
    void switchDocument(KTextEditor::View *, const QString &doc);
    void prevBuffer(KTextEditor::View *);
    void nextBuffer(KTextEditor::View *);
    void firstBuffer(KTextEditor::View *);
    void lastBuffer(KTextEditor::View *);
    void prevTab(KTextEditor::View *);
    void nextTab(KTextEditor::View *);
    void firstTab(KTextEditor::View *);
    void lastTab(KTextEditor::View *);

    static void activateDocument(KTextEditor::View *, KTextEditor::Document *);
    static QList<KTextEditor::Document *> documents();
};
}

#endif /* KATEVI_APP_COMMANDS_H */
