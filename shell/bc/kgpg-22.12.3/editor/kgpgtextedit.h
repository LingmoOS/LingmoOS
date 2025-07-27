/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2009, 2010, 2011 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGTEXTEDIT_H
#define KGPGTEXTEDIT_H

#include <QString>

#include <KTextEdit>
#include <QUrl>

class QDragEnterEvent;
class QDropEvent;

class KGpgItemModel;
class KeysManager;

class KgpgTextEdit : public KTextEdit
{
    Q_OBJECT

public:
    explicit KgpgTextEdit(QWidget *parent, KGpgItemModel *model, KeysManager *manager);
    ~KgpgTextEdit() override;

    void signVerifyText(const QString &message);
    void openDroppedFile(const QUrl &url, const bool probe);

Q_SIGNALS:
    void newText();
    void resetEncoding(bool);
    void verifyFinished();

public Q_SLOTS:
    void slotDroppedFile(const QUrl &url);
    void slotEncode();
    void slotDecode();
    void slotSign(const QString &message);
    void slotVerify(const QString &message);
    void slotSignVerify();
    void slotHighlightText(const QString &, const int matchingindex, const int matchedlength);
    void slotVerifyDone(int result);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private:
    void verifyKeyNeeded(const QString &id);

private Q_SLOTS:
    void slotEncodeUpdate(int result);
    void slotSignUpdate(int result);

    void slotDecryptDone(int result);

private:
    QString m_tempfile;

    int m_posstart;
    int m_posend;

    KGpgItemModel *m_model;
    KeysManager *m_keysmanager;
};

#endif // KGPGVIEW_H
