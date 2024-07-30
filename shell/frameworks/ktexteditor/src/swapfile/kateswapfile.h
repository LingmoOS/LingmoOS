/*
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2010 Diana-Victoria Tiriplica <diana.tiriplica@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SWAPFILE_H
#define KATE_SWAPFILE_H

#include <QDataStream>
#include <QFile>
#include <QObject>
#include <QPointer>

class QTimer;
namespace KTextEditor
{
class ViewPrivate;
class DocumentPrivate;
class Message;
class Range;
class Cursor;
class Document;
}

namespace Kate
{
/**
 * Class for tracking editing actions.
 * In case Kate crashes, this can be used to replay all edit actions to
 * recover the lost data.
 */
class SwapFile : public QObject
{
public:
    explicit SwapFile(KTextEditor::DocumentPrivate *document);
    ~SwapFile() override;
    bool shouldRecover() const;

    void fileClosed();
    QString fileName();

    KTextEditor::DocumentPrivate *document();

private:
    void setTrackingEnabled(bool trackingEnabled);
    void removeSwapFile();
    bool updateFileName();
    bool isValidSwapFile(QDataStream &stream, bool checkDigest) const;

private:
    KTextEditor::DocumentPrivate *m_document;
    bool m_trackingEnabled;

protected:
    void fileSaved(const QString &filename);
    void fileLoaded(const QString &filename);
    void modifiedChanged();

    void startEditing();
    void finishEditing();

    void wrapLine(KTextEditor::Document *, const KTextEditor::Cursor position);
    void unwrapLine(KTextEditor::Document *, int line);
    void insertText(KTextEditor::Document *, const KTextEditor::Cursor position, const QString &text);
    void removeText(KTextEditor::Document *, KTextEditor::Range range, const QString &);

public:
    void discard();
    void recover();
    bool recover(QDataStream &, bool checkDigest = true);
    void configChanged();

private:
    QDataStream m_stream;
    QFile m_swapfile;
    bool m_recovered;
    bool m_needSync;
    static QTimer *s_timer;

protected:
    void writeFileToDisk();

private:
    static QTimer *syncTimer();

public:
    void showSwapFileMessage();
    void showDiff();

private:
    QPointer<KTextEditor::Message> m_swapMessage;
};
}

#endif // KATE_SWAPFILE_H
