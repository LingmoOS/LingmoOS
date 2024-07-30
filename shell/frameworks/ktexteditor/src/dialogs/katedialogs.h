/*
    SPDX-FileCopyrightText: 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2006-2016 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2009 Michel Ludwig <michel.ludwig@kdemail.net>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_DIALOGS_H
#define KATE_DIALOGS_H

#include "kateconfigpage.h"
#include "katehighlight.h"
#include "kateviewhelpers.h"

#include <ktexteditor/attribute.h>
#include <ktexteditor/document.h>

#include <sonnet/configwidget.h>
#include <sonnet/dictionarycombobox.h>

#include <QColor>
#include <QDialog>
#include <QTabWidget>
#include <QTreeWidget>

class ModeConfigPage;
namespace KTextEditor
{
class DocumentPrivate;
}
namespace KTextEditor
{
class ViewPrivate;
}
namespace KTextEditor
{
class Message;
}

namespace KIO
{
class Job;
class TransferJob;
}

class KShortcutsEditor;
class QSpinBox;
class KProcess;

class QCheckBox;
class QLabel;
class QCheckBox;
class QKeyEvent;
class QTemporaryFile;
class QTableWidget;

namespace Ui
{
class TextareaAppearanceConfigWidget;
class BordersAppearanceConfigWidget;
class NavigationConfigWidget;
class EditConfigWidget;
class IndentationConfigWidget;
class OpenSaveConfigWidget;
class OpenSaveConfigAdvWidget;
class CompletionConfigTab;
class SpellCheckConfigWidget;
class StatusbarConfigWidget;
}

class KateGotoBar : public KateViewBarWidget
{
public:
    explicit KateGotoBar(KTextEditor::View *view, QWidget *parent = nullptr);

    void closed() override;

public:
    void updateData();

protected:
    void gotoLine();
    void gotoClipboard();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *, QEvent *) override;
    void showEvent(QShowEvent *event) override;

private:
    KTextEditor::View *const m_view;
    QSpinBox *m_gotoRange = nullptr;
    QToolButton *m_modifiedUp = nullptr;
    QToolButton *m_modifiedDown = nullptr;
    int m_wheelDelta = 0; // To accumulate "wheel-deltas" to become e.g. a touch-pad usable
};

class KateDictionaryBar : public KateViewBarWidget
{
public:
    explicit KateDictionaryBar(KTextEditor::ViewPrivate *view, QWidget *parent = nullptr);
    ~KateDictionaryBar() override;

public:
    void updateData();

protected:
    void dictionaryChanged(const QString &dictionary);

private:
    KTextEditor::ViewPrivate *m_view;
    Sonnet::DictionaryComboBox *m_dictionaryComboBox;
};

class KateIndentConfigTab : public KateConfigPage
{
public:
    explicit KateIndentConfigTab(QWidget *parent);
    ~KateIndentConfigTab() override;
    QString name() const override;

protected:
    Ui::IndentationConfigWidget *ui;

public:
    void apply() override;
    void reload() override;
    void reset() override
    {
    }
    void defaults() override
    {
    }

private:
    void slotChanged();
    void showWhatsThis(const QString &text);
};

class KateCompletionConfigTab : public KateConfigPage
{
public:
    explicit KateCompletionConfigTab(QWidget *parent);
    ~KateCompletionConfigTab() override;
    QString name() const override;

protected:
    Ui::CompletionConfigTab *ui;

public:
    void apply() override;
    void reload() override;
    void reset() override
    {
    }
    void defaults() override
    {
    }

private:
    void showWhatsThis(const QString &text);
};

class KateEditGeneralConfigTab : public KateConfigPage
{
public:
    explicit KateEditGeneralConfigTab(QWidget *parent);
    ~KateEditGeneralConfigTab() override;
    QString name() const override;

private:
    Ui::EditConfigWidget *ui;

    enum SetOfCharsToEncloseSelection {
        None,
        MarkDown,
        NonLetters,
        MirrorChar,
        UserData // Ensure to keep it at bottom of this list
    };

public:
    void apply() override;
    void reload() override;
    void reset() override
    {
    }
    void defaults() override
    {
    }
};

class KateNavigationConfigTab : public KateConfigPage
{
public:
    explicit KateNavigationConfigTab(QWidget *parent);
    ~KateNavigationConfigTab() override;
    QString name() const override;

private:
    Ui::NavigationConfigWidget *ui;

    void initMulticursorModifierComboBox();

public:
    void apply() override;
    void reload() override;
    void reset() override
    {
    }
    void defaults() override
    {
    }
};

class KateSpellCheckConfigTab : public KateConfigPage
{
public:
    explicit KateSpellCheckConfigTab(QWidget *parent);
    ~KateSpellCheckConfigTab() override;
    QString name() const override;

protected:
    Ui::SpellCheckConfigWidget *ui;
    Sonnet::ConfigWidget *m_sonnetConfigWidget;

public:
    void apply() override;
    void reload() override;
    void reset() override
    {
    }
    void defaults() override
    {
    }

private:
    void showWhatsThis(const QString &text);
};

class KateEditConfigTab : public KateConfigPage
{
public:
    explicit KateEditConfigTab(QWidget *parent);
    ~KateEditConfigTab() override;
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public:
    void apply() override;
    void reload() override;
    void reset() override;
    void defaults() override;

private:
    KateEditGeneralConfigTab *editConfigTab;
    KateNavigationConfigTab *navigationConfigTab;
    KateIndentConfigTab *indentConfigTab;
    KateCompletionConfigTab *completionConfigTab;
    KateSpellCheckConfigTab *spellCheckConfigTab;
    std::vector<KateConfigPage *> m_inputModeConfigTabs;
};

class KateViewDefaultsConfig : public KateConfigPage
{
public:
    explicit KateViewDefaultsConfig(QWidget *parent);
    ~KateViewDefaultsConfig() override;
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public:
    void apply() override;
    void reload() override;
    void reset() override;
    void defaults() override;

private:
    Ui::TextareaAppearanceConfigWidget *const textareaUi;
    Ui::BordersAppearanceConfigWidget *const bordersUi;
    Ui::StatusbarConfigWidget *const statusBarUi;
};

class KateSaveConfigTab : public KateConfigPage
{
public:
    explicit KateSaveConfigTab(QWidget *parent);
    ~KateSaveConfigTab() override;
    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public:
    void apply() override;
    void reload() override;
    void reset() override;
    void defaults() override;
    void swapFileModeChanged(int);

protected:
    // why?
    // KComboBox *m_encoding, *m_encodingDetection, *m_eol;
    QCheckBox *cbLocalFiles, *cbRemoteFiles;
    QCheckBox *replaceTabs, *removeSpaces, *allowEolDetection;
    class QSpinBox *blockCount;
    class QLabel *blockCountLabel;

private:
    Ui::OpenSaveConfigWidget *ui;
    Ui::OpenSaveConfigAdvWidget *uiadv;
    ModeConfigPage *modeConfigPage;
};

/**
 * This dialog will prompt the user for what do with a file that is
 * modified on disk.
 * If the file wasn't deleted, it has a 'diff' button, which will create
 * a diff file (using diff(1)) and launch that using OpenUrlJob.
 */
class KateModOnHdPrompt : public QObject
{
    Q_OBJECT
public:
    enum Status {
        Reload = 1, // 0 is QDialog::Rejected
        Save,
        Overwrite,
        Ignore,
        Close
    };
    KateModOnHdPrompt(KTextEditor::DocumentPrivate *doc, KTextEditor::Document::ModifiedOnDiskReason modtype, const QString &reason);
    ~KateModOnHdPrompt() override;

Q_SIGNALS:
    void saveAsTriggered();
    void ignoreTriggered();
    void reloadTriggered();
    void autoReloadTriggered();
    void closeTriggered();

private Q_SLOTS:
    /**
     * Show a diff between the document text and the disk file.
     */
    void slotDiff();

private Q_SLOTS:
    void slotDataAvailable(); ///< read data from the process
    void slotPDone(); ///< Runs the diff file when done

private:
    KTextEditor::DocumentPrivate *m_doc;
    QPointer<KTextEditor::Message> m_message;
    QString m_fullDiffPath;
    KProcess *m_proc;
    QTemporaryFile *m_diffFile;
    QAction *m_diffAction;
};

#endif
