/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgpgeditor.h"

#include "detailedconsole.h"
#include "selectsecretkey.h"
#include "kgpgmd5widget.h"
#include "kgpgsettings.h"
#include "keyservers.h"
#include "sourceselect.h"
#include "kgpg.h"
#include "keysmanager.h"
#include "editor/kgpgtextedit.h"
#include "transactions/kgpgdecrypt.h"
#include "transactions/kgpgkeyservergettransaction.h"
#include "transactions/kgpgsigntext.h"
#include "transactions/kgpgverify.h"
#include <kgpgexternalactions.h>

#include <KActionCollection>
#include <KConfigGroup>
#include <KEncodingFileDialog>
#include <KFind>
#include <KFindDialog>
#include <KIO/Job>
#include <KIO/RenameDialog>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRecentFilesAction>
#include <KSharedConfig>
#include <KStandardAction>
#include <KToggleAction>
#include <KToolBar>
#include <KUrlRequester>

#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QMenuBar>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

class KgpgView : public QWidget {
public:
	KgpgView(QWidget *parent, KgpgTextEdit *editor, KToolBar *toolbar);
};

KgpgView::KgpgView(QWidget *parent, KgpgTextEdit *editor, KToolBar *toolbar)
	: QWidget(parent)
{
	QVBoxLayout *vb = new QVBoxLayout(this);
	vb->setSpacing(3);
	vb->addWidget(editor);
	vb->addWidget(toolbar);

	setAcceptDrops(true);
}

KgpgEditor::KgpgEditor(KeysManager *parent, KGpgItemModel *model, Qt::WindowFlags f)
    : KXmlGuiWindow(nullptr, f),
	m_editor(new KgpgTextEdit(this, model, parent)),
	m_recentfiles(nullptr),
    m_find(nullptr),
	m_textchanged(false),
	m_emptytext(true),
	m_model(model),
	m_parent(parent)
{
    // call inits to invoke all other construction parts
    initActions();

    connect(m_editor, &KgpgTextEdit::resetEncoding, this, &KgpgEditor::slotResetEncoding);
    KgpgView *kb = new KgpgView(this, m_editor, toolBar(QLatin1String( "gpgToolBar" )));
    setCentralWidget(kb);
    setCaption(i18n("Untitled"), false);
    m_editredo->setEnabled(false);
    m_editundo->setEnabled(false);
    m_editcopy->setEnabled(false);
    m_editcut->setEnabled(false);

    setObjectName( QLatin1String("editor" ));
    slotSetFont(KGpgSettings::font());
    setupGUI((ToolBar | Keys | StatusBar | Save | Create), QLatin1String( "kgpgeditor.rc" ));
    setAutoSaveSettings(QLatin1String( "Editor" ), true);

    connect(m_editor, &KgpgTextEdit::textChanged, this, &KgpgEditor::modified);
    connect(m_editor, &KgpgTextEdit::newText, this, &KgpgEditor::newText);
    connect(m_editor, &KgpgTextEdit::undoAvailable, this, &KgpgEditor::slotUndoAvailable);
    connect(m_editor, &KgpgTextEdit::redoAvailable, this, &KgpgEditor::slotRedoAvailable);
    connect(m_editor, &KgpgTextEdit::copyAvailable, this, &KgpgEditor::slotCopyAvailable);
}

KgpgEditor::~KgpgEditor()
{
    disconnect(m_editor, &KgpgTextEdit::textChanged, this, &KgpgEditor::modified);
    m_recentfiles->saveEntries( KConfigGroup(KSharedConfig::openConfig(), "Recent Files" ) );
}

void KgpgEditor::openDocumentFile(const QUrl &url, const QString &encoding)
{
    auto downloadJob = KIO::storedGet(url);
    KJobWidgets::setWindow(downloadJob , this);
    downloadJob->exec();
    if(!downloadJob->error())
    {
        QTextStream t(downloadJob->data());
        t.setCodec(encoding.toLatin1().constData());
        m_editor->setPlainText(t.readAll());
        m_docname = url;
        m_textchanged = false;
        m_emptytext = false;
        setCaption(url.fileName(), false);
        m_recentfiles->addUrl(url);
    }
}

void KgpgEditor::openEncryptedDocumentFile(const QUrl &url)
{
    m_editor->slotDroppedFile(url);
}

void KgpgEditor::slotSetFont(QFont myFont)
{
    m_editor->setFont(myFont);
}

void KgpgEditor::closeWindow()
{
    m_recentfiles->saveEntries( KConfigGroup(KSharedConfig::openConfig(), "Recent Files" ) );
    close();
}

void KgpgEditor::saveOptions()
{
    KGpgSettings::setFirstRun(false);
    KGpgSettings::self()->save();
}

void KgpgEditor::initActions()
{
    KStandardAction::openNew(this, &KgpgEditor::slotFileNew, actionCollection());
    KStandardAction::open(this, &KgpgEditor::slotFileOpen, actionCollection());
    KStandardAction::save(this, &KgpgEditor::slotFileSave, actionCollection());
    KStandardAction::saveAs(this, &KgpgEditor::slotFileSaveAs, actionCollection());
    KStandardAction::close(this, &KgpgEditor::slotFileClose, actionCollection());
    KStandardAction::paste(this, &KgpgEditor::slotEditPaste, actionCollection());
    KStandardAction::print(this, &KgpgEditor::slotFilePrint, actionCollection());
    KStandardAction::selectAll(this, &KgpgEditor::slotSelectAll, actionCollection());
    KStandardAction::find(this, &KgpgEditor::slotFind, actionCollection());
    KStandardAction::findNext(this, &KgpgEditor::slotFindNext, actionCollection());
    KStandardAction::findPrev(this, &KgpgEditor::slotFindPrev, actionCollection());
    actionCollection()->addAction(KStandardAction::Preferences, QLatin1String( "options_configure" ),
                                  this, SLOT(slotOptions()));

    m_editundo = KStandardAction::undo(this, &KgpgEditor::slotundo, actionCollection());
    m_editredo = KStandardAction::redo(this, &KgpgEditor::slotredo, actionCollection());
    m_editcopy = KStandardAction::copy(this, &KgpgEditor::slotEditCopy, actionCollection());
    m_editcut  = KStandardAction::cut(this, &KgpgEditor::slotEditCut, actionCollection());

    m_recentfiles = KStandardAction::openRecent(this, SLOT(openDocumentFile(QUrl)), this);
    menuBar()->addAction(m_recentfiles);

    m_recentfiles->loadEntries( KConfigGroup(KSharedConfig::openConfig(), "Recent Files" ) );
    m_recentfiles->setMaxItems(KGpgSettings::recentFiles());

    QAction *action = actionCollection()->addAction(QLatin1String("file_encrypt"), this, &KgpgEditor::slotFilePreEnc);
    action->setIcon(QIcon::fromTheme( QLatin1String( "document-encrypt" )));
    action->setText(i18n("&Encrypt File..."));

    action = actionCollection()->addAction(QLatin1String("file_decrypt"), this, &KgpgEditor::slotFilePreDec);
    action->setIcon(QIcon::fromTheme( QLatin1String( "document-decrypt" )));
    action->setText(i18n("&Decrypt File..."));

    action = actionCollection()->addAction(QLatin1String("key_manage"), this, &KgpgEditor::slotKeyManager);
    action->setIcon(QIcon::fromTheme( QLatin1String( "kgpg" )));
    action->setText(i18n("&Open Key Manager"));

    action = actionCollection()->addAction(QLatin1String("sign_generate"), this, &KgpgEditor::slotPreSignFile);
    action->setText(i18n("&Generate Signature..."));
    action->setIcon(QIcon::fromTheme( QLatin1String( "document-sign-key" )));

    action = actionCollection()->addAction(QLatin1String("sign_verify"), this, &KgpgEditor::slotPreVerifyFile);
    action->setText(i18n("&Verify Signature..."));

    action = actionCollection()->addAction(QLatin1String("sign_check"), this, &KgpgEditor::slotCheckMd5);
    action->setText(i18n("&Check MD5 Sum..."));

    m_encodingaction = actionCollection()->add<KToggleAction>(QLatin1String("charsets"), this, &KgpgEditor::slotSetCharset);
    m_encodingaction->setText(i18n("&Unicode (utf-8) Encoding"));

    actionCollection()->addAction(m_recentfiles->objectName(), m_recentfiles);

    action = actionCollection()->addAction(QLatin1String("text_encrypt"), m_editor, &KgpgTextEdit::slotEncode);
    action->setIcon(QIcon::fromTheme( QLatin1String( "document-encrypt" )));
    action->setText(i18n("En&crypt"));

    action = actionCollection()->addAction(QLatin1String("text_decrypt"), m_editor, &KgpgTextEdit::slotDecode);
    action->setIcon(QIcon::fromTheme( QLatin1String( "document-decrypt" )));
    action->setText(i18n("&Decrypt"));

    action = actionCollection()->addAction(QLatin1String("text_sign_verify"), m_editor, &KgpgTextEdit::slotSignVerify);
    action->setIcon(QIcon::fromTheme( QLatin1String( "document-sign-key" )));
    action->setText(i18n("S&ign/Verify"));
}

bool KgpgEditor::queryClose()
{
	bool b = saveBeforeClear();
	if (b) {
		m_editor->clear();
		newText();
	}
	return b;
}

bool KgpgEditor::saveBeforeClear()
{
    if (m_textchanged)
    {
        QString fname;
        if (m_docname.fileName().isEmpty())
            fname = i18n("Untitled");
        else
            fname = m_docname.fileName();

        QString msg = i18n("The document \"%1\" has changed.\nDo you want to save it?", fname);
        QString caption = i18n("Close the document");
        int res = KMessageBox::warningYesNoCancel(this, msg, caption, KStandardGuiItem::save(), KStandardGuiItem::discard());
        if (res == KMessageBox::Yes)
            return slotFileSave();
        else
        if (res == KMessageBox::No)
            return true;
        else
            return false;
    }

    return true;
}

void KgpgEditor::slotFileNew()
{
    if (saveBeforeClear())
    {
        m_editor->clear();
        newText();
    }
}

void KgpgEditor::slotFileOpen()
{
    if (saveBeforeClear())
    {
        KEncodingFileDialog::Result loadResult;
        loadResult = KEncodingFileDialog::getOpenUrlAndEncoding(QString(), QUrl(), QString(), this);
        QUrl url = loadResult.URLs.first();
        m_textencoding = loadResult.encoding;

        if(!url.isEmpty())
            openDocumentFile(url, m_textencoding);
    }
}

bool KgpgEditor::slotFileSave()
{
    QString filn = m_docname.path();
    if (filn.isEmpty())
        return slotFileSaveAs();

    QTextCodec *cod = QTextCodec::codecForName(m_textencoding.toLatin1());

    if (cod == nullptr) {
		KMessageBox::error(this, i18n("The document could not been saved, as the selected codec is not supported."));
		return false;
    }

    if (!checkEncoding(cod))
    {
        KMessageBox::error(this, i18n("The document could not been saved, as the selected encoding cannot encode every unicode character in it."));
        return false;
    }

    if (m_docname.isLocalFile())
    {
        QFile f(filn);
        if (!f.open(QIODevice::WriteOnly))
        {
            KMessageBox::error(this, i18n("The document could not be saved, please check your permissions and disk space."));
            return false;
        }

        QTextStream t(&f);
        t.setCodec(cod);
        t << m_editor->toPlainText();
        f.close();
    }
    else
    {
        auto uploadJob = KIO::storedPut(cod->fromUnicode(m_editor->toPlainText()), m_docname, -1);
        KJobWidgets::setWindow(uploadJob , this);
        uploadJob->exec();
        if(uploadJob->error())
        {
            KMessageBox::error(this, i18n("The document could not be saved, please check your permissions and disk space."));
            return false;
        }
    }

    m_textchanged = false;
    m_emptytext = false;
    setCaption(m_docname.fileName(), false);
    return true;
}

bool KgpgEditor::slotFileSaveAs()
{
	KEncodingFileDialog::Result saveResult;
	saveResult = KEncodingFileDialog::getSaveUrlAndEncoding(QString(), QUrl(), QString(), this);
	QUrl url;

	if (!saveResult.URLs.empty())
		url = saveResult.URLs.first();

	if(!url.isEmpty()) {
		const QString selectedEncoding = saveResult.encoding;
		if (url.isLocalFile()) {
			QFile f(url.path());
			if (f.exists()) {
				const QString message = i18n("Overwrite existing file %1?", url.fileName());
				int result = KMessageBox::warningContinueCancel(this, message, QString(), KStandardGuiItem::overwrite());
				if (result == KMessageBox::Cancel)
					return false;
			}
			f.close();
		} else {
            auto statJob = KIO::statDetails(url, KIO::StatJob::DestinationSide, {});
			KJobWidgets::setWindow(statJob, this);
			statJob->exec();
			if (!statJob->error()) {
				const QString message = i18n("Overwrite existing file %1?", url.fileName());
				int result = KMessageBox::warningContinueCancel(this, message, QString(), KStandardGuiItem::overwrite());
				if (result == KMessageBox::Cancel)
					return false;
                        }
		}

		m_docname = url;
		m_textencoding = selectedEncoding;
		slotFileSave();
		return true;
	}

	return false;
}

void KgpgEditor::slotFilePrint()
{
    QPrinter prt;
    QPointer<QPrintDialog> printDialog = new QPrintDialog(&prt, this);
    if (printDialog->exec() == QDialog::Accepted) {
        int width = prt.width();
        int height = prt.height();
        QPainter painter(&prt);
        painter.drawText(0, 0, width, height, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip, m_editor->toPlainText());
    }
    delete printDialog;
}

void KgpgEditor::slotFind()
{
	QPointer<KFindDialog> fd = new KFindDialog(this);

	if (m_find) {
		fd->setOptions(m_find->options());
		fd->setPattern(m_find->pattern());
	}

	if (fd->exec() != QDialog::Accepted) {
		delete fd;
		return;
	}

	if (!m_find) {
		m_find = new KFind(fd->pattern(), fd->options(), this);

		if (m_find->options() & KFind::FromCursor)
			m_find->setData(m_editor->toPlainText(), m_editor->textCursor().selectionStart());
		else
			m_find->setData(m_editor->toPlainText());
        connect(m_find, &KFind::textFound, m_editor, &KgpgTextEdit::slotHighlightText);
		connect(m_find, &KFind::findNext, this, &KgpgEditor::slotFindText);
	} else {
		m_find->setPattern(fd->pattern());
		m_find->setOptions(fd->options());
		m_find->resetCounts();
	}

	slotFindText();
	delete fd;
}

void KgpgEditor::slotFindNext()
{
    slotFindText();
}

void KgpgEditor::slotFindPrev()
{
    if(!m_find)
    {
       slotFind();
       return;
    }
    long oldopt = m_find->options();
    long newopt = oldopt ^ KFind::FindBackwards;
    m_find->setOptions(newopt);
    slotFindText();
    if (m_find) {
        m_find->setOptions(oldopt);
    }
}

void KgpgEditor::slotFindText()
{
    if (!m_find)
    {
        slotFind();
        return;
    }

    if (m_find->find() == KFind::NoMatch)
    {
        if (m_find->numMatches() == 0)
        {
            m_find->displayFinalDialog();
            delete m_find;
            m_find = nullptr;
        }
        else
        {
            if (m_find->shouldRestart(true, false))
            {
                m_find->setData(m_editor->toPlainText());
                slotFindText();
            }
            else
                m_find->closeFindNextDialog();
        }
    }
}

void KgpgEditor::slotFilePreEnc()
{
    QList<QUrl> urls = QFileDialog::getOpenFileUrls(this, i18n("Open File to Encode"), QUrl(), i18n("*|All Files"));
    if (urls.isEmpty())
        return;

    KGpgExternalActions::encryptFiles(m_parent, urls);
}

void KgpgEditor::slotFilePreDec()
{
    QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open File to Decode"), QUrl(), i18n("*|All Files"));
    if (url.isEmpty())
        return;

    QString oldname = url.fileName();
    QString newname;

    if (oldname.endsWith(QLatin1String(".gpg")) || oldname.endsWith(QLatin1String(".asc")) || oldname.endsWith(QLatin1String(".pgp")))
        oldname.chop(4);
    else
        oldname.append(QLatin1String( ".clear" ));
    oldname.prepend(url.adjusted(QUrl::RemoveFilename).path());

    QPointer<QDialog> popn = new QDialog(this);
    popn->setWindowTitle(i18n("Decrypt File To"));
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(popn);
    popn->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    popn->setModal( true );

    SrcSelect *page = new SrcSelect();
    mainLayout->addWidget(page);
    page->newFilename->setUrl(QUrl(oldname));
    page->newFilename->setMode(KFile::File);
    page->newFilename->setWindowTitle(i18n("Save File"));

    page->checkClipboard->setText(i18n("Editor"));
    page->resize(page->minimumSize());
    popn->resize(popn->minimumSize());

    page->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(page->buttonBox, &QDialogButtonBox::accepted, popn.data(), &QDialog::accept);
    connect(page->buttonBox, &QDialogButtonBox::rejected, popn.data(), &QDialog::reject);

    if (popn->exec() == QDialog::Accepted)
    {
        if (page->checkFile->isChecked())
            newname = page->newFilename->url().path();
    }
    else
    {
        delete popn;
        return;
    }
    delete popn;

    if (!newname.isEmpty())
    {
        QFile fgpg(newname);
        if (fgpg.exists()) {
		QPointer<KIO::RenameDialog> over = new KIO::RenameDialog(this, i18n("File Already Exists"), QUrl(), QUrl::fromLocalFile(newname), KIO::RenameDialog_Overwrite);

		if (over->exec() != QDialog::Accepted) {
			delete over;
			return;
		}
		newname = over->newDestUrl().path();
		delete over;
        }

	KGpgDecrypt *decr = new KGpgDecrypt(this, url, QUrl(newname));
	connect(decr, &KGpgDecrypt::done, this, &KgpgEditor::slotLibraryDone);
	decr->start();
    }
    else
        openEncryptedDocumentFile(url);
}

void
KgpgEditor::slotLibraryDone()
{
	sender()->deleteLater();
}

void KgpgEditor::slotKeyManager()
{
	m_parent->show();
	m_parent->raise();
}

void KgpgEditor::slotFileClose()
{
    saveOptions();
    close();
}

void KgpgEditor::slotundo()
{
    m_editor->undo();
}

void KgpgEditor::slotredo()
{
    m_editor->redo();
}

void KgpgEditor::slotEditCut()
{
    m_editor->cut();
}

void KgpgEditor::slotEditCopy()
{
    m_editor->copy();
}

void KgpgEditor::slotEditPaste()
{
    m_editor->paste();
}

void KgpgEditor::slotSelectAll()
{
    m_editor->selectAll();
}

void KgpgEditor::slotSetCharset()
{
    if (!m_encodingaction->isChecked())
        m_editor->setPlainText(QString::fromUtf8(m_editor->toPlainText().toLatin1()));
    else
    {
        if (checkEncoding(QTextCodec::codecForLocale()))
            return;
        m_editor->setPlainText(QLatin1String( m_editor->toPlainText().toUtf8() ));
    }
}

bool KgpgEditor::checkEncoding(QTextCodec *codec)
{
    return codec->canEncode(m_editor->toPlainText());
}

void KgpgEditor::slotResetEncoding(bool enc)
{
    m_encodingaction->setChecked(enc);
}

void KgpgEditor::slotPreSignFile()
{
    // create a detached signature for a chosen file
    QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open File to Sign"), QUrl(), i18n("*|All Files"));
    if (!url.isEmpty())
        slotSignFile(url);
}

void KgpgEditor::slotSignFile(const QUrl &url)
{
    // create a detached signature for a chosen file
    if (!url.isEmpty())
    {
        QString signKeyID;
        QPointer<KgpgSelectSecretKey> opts = new KgpgSelectSecretKey(this, m_model, false);
        if (opts->exec() == QDialog::Accepted) {
            signKeyID = opts->getKeyID();
        } else {
            delete opts;
            return;
        }

        delete opts;

	KGpgSignText::SignOptions sopts = KGpgSignText::DetachedSignature;
	if (KGpgSettings::asciiArmor())
		sopts |= KGpgSignText::AsciiArmored;

	KGpgSignText *signt = new KGpgSignText(this, signKeyID, QList<QUrl>({url}), sopts);
	connect(signt, &KGpgSignText::done, this, &KgpgEditor::slotSignFileFin);
	signt->start();
    }
}

void KgpgEditor::slotSignFileFin(int)
{
	sender()->deleteLater();
}

void KgpgEditor::slotPreVerifyFile()
{
    QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open File to Verify"), QUrl(), i18n("*|All Files"));
    slotVerifyFile(url);
}

void KgpgEditor::slotVerifyFile(const QUrl &url)
{
	if (url.isEmpty())
		return;

	QString sigfile;
	if (!url.fileName().endsWith(QLatin1String(".sig"))) {
		sigfile = url.path() + QLatin1String( ".sig" );
		if (!QFile::exists(sigfile)) {
			sigfile = url.path() + QLatin1String( ".asc" );
			// if no .asc or .sig signature file included, assume the file is internally signed
			if (!QFile::exists(sigfile))
				sigfile.clear();
		}
	}

	QList<QUrl> chkfiles;
	if (sigfile.isEmpty())
		chkfiles << url;
	else
		chkfiles << QUrl::fromLocalFile(sigfile);

	KGpgVerify *verify = new KGpgVerify(this, chkfiles);
	connect(verify, &KGpgVerify::done, m_editor, &KgpgTextEdit::slotVerifyDone);
	verify->start();
}

void KgpgEditor::slotCheckMd5()
{
	// display md5 sum for a chosen file
	QUrl url = QFileDialog::getOpenFileUrl(this, i18n("Open File to Verify"), QUrl(), i18n("*|All Files"));
	if (!url.isEmpty()) {
		QPointer<Md5Widget> mdwidget = new Md5Widget(this, url);
		mdwidget->exec();
		delete mdwidget;
	}
}

void KgpgEditor::importSignatureKey(const QString &id, const QString &fileName)
{
	sender()->deleteLater();

        if (KMessageBox::questionYesNo(nullptr,
			i18n("<qt><b>Missing signature:</b><br />Key id: %1<br /><br />Do you want to import this key from a keyserver?</qt>", id),
			fileName, KGuiItem(i18n("Import")), KGuiItem(i18n("Do Not Import"))) != KMessageBox::Yes)
		return;

	KeyServer *ks = new KeyServer(this);

	connect(ks, &KeyServer::importFinished, this, &KgpgEditor::slotDownloadKeysFinished);
	connect(ks, &KeyServer::importFailed, ks, &KeyServer::deleteLater);

	ks->startImport(QStringList(id), QString(),QLatin1String( qgetenv("http_proxy") ));
}

void KgpgEditor::slotDownloadKeysFinished(const QStringList &ids)
{
	m_model->refreshKeys(ids);

	sender()->deleteLater();
}

void KgpgEditor::slotOptions()
{
	m_parent->showOptions();
}

void KgpgEditor::modified()
{
	QString capt = m_docname.fileName();
	if (m_emptytext) {
		m_textchanged = !m_editor->toPlainText().isEmpty();
		if (capt.isEmpty())
			capt = i18n("Untitled");
	} else if (!m_textchanged) {
		m_textchanged = true;
	}
	setCaption(capt, m_textchanged);
	m_editor->document()->setModified(m_textchanged);
}

void KgpgEditor::slotUndoAvailable(const bool v)
{
	m_editundo->setEnabled(v);
}

void KgpgEditor::slotRedoAvailable(const bool v)
{
	m_editredo->setEnabled(v);
}

void KgpgEditor::slotCopyAvailable(const bool v)
{
	m_editcopy->setEnabled(v);
	m_editcut->setEnabled(v);
}

void KgpgEditor::newText()
{
	m_textchanged = false;
	m_emptytext = true;
	m_docname.clear();
	setCaption(i18n("Untitled"), false);
	slotResetEncoding(false);
}
