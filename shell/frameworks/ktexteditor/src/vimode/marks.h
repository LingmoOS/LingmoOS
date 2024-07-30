/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_VIMODE_MARKS_H
#define KATE_VIMODE_MARKS_H

#include <KConfigGroup>
#include <KTextEditor/Document>

#include <QMap>

namespace KTextEditor
{
class DocumentPrivate;
class Cursor;
class MovingCursor;
}

namespace KateVi
{
class InputModeManager;

class Marks : public QObject
{
public:
    explicit Marks(InputModeManager *imm);

    /** JBOS == Just a Bunch Of Shortcuts **/
    void setStartEditYanked(const KTextEditor::Cursor pos);
    void setFinishEditYanked(const KTextEditor::Cursor pos);
    void setLastChange(const KTextEditor::Cursor pos);
    void setInsertStopped(const KTextEditor::Cursor pos);
    void setSelectionStart(const KTextEditor::Cursor pos);
    void setSelectionFinish(const KTextEditor::Cursor pos);
    void setUserMark(const QChar &mark, const KTextEditor::Cursor pos);

    KTextEditor::Cursor getStartEditYanked() const;
    KTextEditor::Cursor getFinishEditYanked() const;
    KTextEditor::Cursor getLastChange() const;
    KTextEditor::Cursor getInsertStopped() const;
    KTextEditor::Cursor getSelectionStart() const;
    KTextEditor::Cursor getSelectionFinish() const;
    KTextEditor::Cursor getMarkPosition(const QChar &mark) const;

    void writeSessionConfig(KConfigGroup &config) const;
    void readSessionConfig(const KConfigGroup &config);

    QString getMarksOnTheLine(int line) const;

private:
    void syncViMarksAndBookmarks();
    static bool isShowable(const QChar &mark);

    void setMark(const QChar &mark, const KTextEditor::Cursor pos);

private:
    void markChanged(KTextEditor::Document *doc, KTextEditor::Mark mark, KTextEditor::Document::MarkChangeAction action);

private:
    InputModeManager *m_inputModeManager;
    KTextEditor::DocumentPrivate *m_doc;

    QMap<QChar, KTextEditor::MovingCursor *> m_marks;
    bool m_settingMark;
};
}

#endif // KATE_VIMODE_MARKS_H
