/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGEDITOR_H
#define KGPGEDITOR_H

#include <KXmlGuiWindow>
#include <QUrl>
#include <QTextCodec>

class KToggleAction;
class QAction;
class KFind;

class KgpgTextEdit;
class KGpgItemModel;
class KeysManager;
class KRecentFilesAction;

class KgpgEditor : public KXmlGuiWindow
{
    Q_OBJECT
    friend class KgpgView;

    KgpgEditor() = delete;
public:
    KgpgEditor(KeysManager *parent, KGpgItemModel *model, Qt::WindowFlags f);
    ~KgpgEditor() override;

    void openEncryptedDocumentFile(const QUrl &url);

    KgpgTextEdit * const m_editor;
    KRecentFilesAction *m_recentfiles;

Q_SIGNALS:
    void openChangeFont();
    void openConfigDialog();

public Q_SLOTS:
    void openDocumentFile(const QUrl &url, const QString &encoding = QString());
    void slotSetFont(QFont myFont);
    void closeWindow();

protected:
    void saveOptions();
    void initActions();
    bool queryClose() override;
    bool saveBeforeClear();

private Q_SLOTS:
    // File menu
    void slotFileNew();
    void slotFileOpen();
    bool slotFileSave();
    bool slotFileSaveAs();
    void slotFilePrint();
    void slotFilePreEnc();
    void slotFilePreDec();
    void slotKeyManager();
    void slotFileClose();

    // Edit menu
    void slotundo();
    void slotredo();
    void slotEditCut();
    void slotEditCopy();
    void slotEditPaste();
    void slotSelectAll();
    void slotFind();
    void slotFindNext();
    void slotFindPrev();
    void slotFindText();

    // Coding menu
    void slotSetCharset();
    void slotResetEncoding(bool enc);
    bool checkEncoding(QTextCodec *codec);

    // Signing menu
    void slotPreSignFile();
    void slotSignFile(const QUrl &url);
    void slotSignFileFin(int);
    void slotPreVerifyFile();
    void slotVerifyFile(const QUrl &url);
    void slotCheckMd5();
    void importSignatureKey(const QString &id, const QString &fileName);

    // Options menu
    void slotOptions();

    void slotUndoAvailable(const bool v);
    void slotRedoAvailable(const bool v);
    void slotCopyAvailable(const bool v);

    void modified();
    void newText();

    void slotLibraryDone();

    void slotDownloadKeysFinished(const QStringList &ids);

private:
    QString m_textencoding;

    KToggleAction *m_encodingaction;
    QAction *m_editundo;
    QAction *m_editredo;
    QAction *m_editcopy;
    QAction *m_editcut;
    KFind *m_find;
    QUrl m_docname;

    bool m_textchanged;		//< text was changed since last save
    bool m_emptytext;		//< this was not saved to a file ever

    KGpgItemModel *m_model;
    KeysManager *m_parent;
};

#endif // KGPGEDITOR_H
