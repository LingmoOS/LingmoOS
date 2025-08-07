/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KEYSMANAGER_H
#define KEYSMANAGER_H

#include "ui_adduid.h"

#include "core/kgpgkey.h"
#include "model/kgpgitemmodel.h"
#include "model/kgpgitemnode.h"

#include <KToggleAction>
#include <KXmlGuiWindow>

#include <QClipboard>
#include <QSet>
#include <QUrl>

class KJob;

class KSelectAction;

class QAction;
class QEvent;
class QKeySequence;
class QLineEdit;
class QMenu;

class KeyServer;
class KgpgEditor;
class KeyServer;
class KeyListProxyModel;
class KeyTreeView;
class KGpgAddUid;
class KGpgDelKey;
class KGpgImport;
class KGpgTransactionJob;

class KStatusNotifierItem;


class AddUid : public QWidget, public Ui::AddUid
{
public:
  explicit AddUid( QWidget *parent )
    : QWidget( parent )
  {
    setupUi( this );
  }
};

class KeysManager : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit KeysManager(QWidget *parent = nullptr);
    ~KeysManager() override;

    KGpgItemModel *getModel();

    KgpgEditor *s_kgpgEditor;

    void saveToggleOpts(void);
    void showTrayMessage(const QString &message);

    /**
     * @brief returns the shortcut to go to the default key in a key selection
     */
    QKeySequence goDefaultShortcut() const;

private:
    KToggleAction *sTrust;
    KToggleAction *sCreat;
    KToggleAction *sExpi;
    KToggleAction *sSize;
    KToggleAction *hPublic;
    KToggleAction *longId;
    KSelectAction *photoProps;
    KSelectAction *trustProps;

Q_SIGNALS:
    void readAgainOptions();
    void certificate(QString);
    void closeAsked();
    void fontChanged(QFont);

public Q_SLOTS:
    void slotGenerateKey();
    void refreshkey();
    void readAllOptions();
    void showKeyInfo(const QString &keyID);
    void slotSetDefaultKey(const QString &newID);
    void showKeyManager();
    bool importRemoteKey(const QString &keyIDs);
    bool importRemoteKeys(const QStringList &keyIDs, const bool dialog = true);
    void showKeyServer();
    void showOptions();
    void slotOpenEditor();
    void slotImport(const QString &text);
    void slotImport(const QList<QUrl> &files);
    /**
     * When you click on "encrypt the clipboard" in the systray,
     * this slot will open the dialog to choose a key and encrypt the
     * clipboard.
     */
    void clipEncrypt();
    void clipDecrypt();
    void clipSign();
    void slotImportDone(int ret);

protected:
    bool eventFilter(QObject *, QEvent *e) override;
    void removeFromGroups(KGpgKeyNode *nd);
    void setDefaultKeyNode(KGpgKeyNode *key);

private Q_SLOTS:
    void slotGenerateKeyDone(KJob *job);

    void slotShowTrust();
    void slotShowSize();
    void slotShowCreation();
    void slotShowExpiration();

    void slotAddUidFin(int res);
    void slotDelPhotoFinished(int res);
    void quitApp();
    void slotShowLongId(bool);
    void slotSetTrustFilter(int);
    void slotGotoDefaultKey();
    void slotDelUid();
    void slotDelUidDone(int);
    void slotPrimUid();
    void slotPrimUidDone(int result);
    void slotAddUid();
    void slotAddUidEnable(const QString &name);
    void slotUpdatePhoto();
    void slotDeletePhoto();
    void slotAddPhoto();
    void slotAddPhotoFinished(int res);
    void slotSetPhotoSize(int size);
    void slotShowPhoto();
    void revokeWidget();
    void slotRevokeDialogFinished(int result);
    void slotRevokeGenerated(int result);
    void doPrint(const QString &txt);
    void checkList();
    void slotManpage();
    void slotTip();
    void slotExportFinished(int result);
    void slotProcessExportMail(int result);
    void slotProcessExportClip(int result);
    void readOptions();
    void slotSetDefKey();
    void confirmdeletekey();
    void deleteseckey();
    void signkey();
    void signuid();
    void caff();
    void slotCaffDone();
    void delsignkey();
    void preimportsignkey();
    void importallsignkey();
    void signatureResult(int success);
    void delsignatureResult(int success);
    void defaultAction(const QModelIndex &);
    void defaultAction(KGpgNode *);
    void slotDefaultAction();
    void showProperties(KGpgNode *);
    void keyproperties();
    void slotexport();
    void slotexportsec();
    void slotExportSecFinished(int result);

    void slotMenu(const QPoint &);

    void slotPreImportKey();
    void slotSendEmail();
    void slotedit();

    /**
     * @brief start an "add to addressbook" operation
     *
     * This searches if given id already exists in the addressbook.
     * The search result is handled in slotAddressbookSearchResult()
     */
    void addToKAB();

    /**
     * @brief add or change the addressbook entry
     * @param job the search job
     *
     * This handles the result of the search started in addToKAB().
     */
    void slotAddressbookSearchResult(KJob *job);

    void editGroup();
    void createNewGroup();
    void deleteGroup();
    void renameGroup();
    void slotImportRevokeTxt(const QString &revokeText);
    void refreshKeyFromServer();
    void slotKeyRefreshDone(int result);
    void slotregenerate();
    void secretKeyDeleted(int);
    void getMissingSigs(QSet<QString> &missingKeys, const KGpgExpandableNode *nd);
    void slotEditDone(int exitcode);
    void importRemoteFinished(int result);
    void slotDelKeyDone(int ret);
    void slotSetClip(int result);
    void slotOpenKeyUrl();

private:
    KGpgItemModel *imodel;
    KeyListProxyModel *iproxy;
    KeyTreeView *iview;

    KGpgAddUid *m_adduid;
    KGpgTransactionJob *m_genkey;
    KGpgDelKey *m_delkey;

    QString globalkeyID;
    QString searchString;

    QList<KGpgSignableNode *> signList;
    QList<KGpgKeyNode *> refreshList;
    QHash<KJob *, KGpgNode *> m_addIds;	///< user ids to add to addressbook

    QClipboard::Mode m_clipboardmode;

    QMenu *m_popuppub;          // popup on a public key
    QMenu *m_popupsec;          // popup on a secret key
    QMenu *m_popupgroup;        // popup on a group
    QMenu *m_popupout;          // popup there is no key or when the user don't right-click on a key
    QMenu *m_popupsig;          // popup on a signature
    QMenu *m_popupphoto;        // popup on a photo
    QMenu *m_popupuid;          // popup on an user id
    QMenu *m_popuporphan;       // popup on an orphan key

    QLineEdit *m_listviewsearch;
    QDialog *addUidWidget;
    AddUid *keyUid;

    QLabel m_statusBarLabel;

    QAction *importSignatureKey;
    QAction *importAllSignKeys;
    QAction *signKey;
    QAction *signUid;
    QAction *signMailUid;
    QAction *refreshKey;
    QAction *editKey;
    QAction *setPrimUid;
    QAction *delUid;
    QAction *delSignKey;
    QAction *deleteKey;
    QAction *editCurrentGroup;
    QAction *delGroup;
    QAction *setDefaultKey;
    QAction *kserver;
    QAction *openEditor;
    QAction *goToDefaultKey;
    QAction *exportPublicKey;
    QAction *m_sendEmail;
    QAction *createGroup;
    QAction *m_groupRename;
    QAction *m_revokeKey;

    bool showTipOfDay;
    bool m_signuids;

    int keyCount;

    KGpgKeyNode *terminalkey; // the key currently edited in a terminal

    void startImport(KGpgImport *import);

    // react to network status changes
    bool m_online;
    void toggleNetworkActions(bool online);

    KStatusNotifierItem *m_trayicon;
    void setupTrayIcon();

    void setActionDescriptions(int cnt);
    /**
     * @brief show a message in the status bar
     * @param msg the text to show
     * @param keep if the text should stay visible or may be hidden after a while
     */
    void changeMessage(const QString &msg, const bool keep = false);
    /**
     * @brief update the key and group counter in the status bar
     */
    void updateStatusCounter();
    /**
     * @brief sign the next key from signList
     * @param localsign if signature should be a local (not exportable) one
     * @param checklevel how careful the identity of the key was checked
     */
    void signLoop(const bool localsign, const int checklevel);
    /**
     * @brief Opens the console when the user want to sign a key manually.
     * @param signer key to sign with
     * @param keyid key to sign
     * @param checking how careful the identify was checked
     * @param local if signature should be local (not exportable)
     */
    void signKeyOpenConsole(const QString &signer, const QString &keyid, const int checking, const bool local);
};

#endif // KEYSMANAGER_H
