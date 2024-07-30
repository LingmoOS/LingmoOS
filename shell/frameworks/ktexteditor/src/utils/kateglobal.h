/*
    SPDX-FileCopyrightText: 2001-2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2009 Erlend Hamberg <ehamberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_GLOBAL_H
#define KATE_GLOBAL_H

#include <ktexteditor_export.h>

#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include <KAboutData>
#include <KSharedConfig>

#include <KTextEditor/Application>
#include <KTextEditor/MainWindow>

#include <QList>
#include <QPointer>

#include <array>
#include <memory>

class QStringListModel;
class QTextToSpeech;

class KateCmd;
class KateModeManager;
class KateGlobalConfig;
class KateDocumentConfig;
class KateViewConfig;
class KateRendererConfig;
namespace KTextEditor
{
class DocumentPrivate;
}
namespace KTextEditor
{
class ViewPrivate;
class Command;
}
class KateScriptManager;
class KDirWatch;
class KateHlManager;
class KateSpellCheckManager;
class KateWordCompletionModel;
class KateAbstractInputModeFactory;
class KateKeywordCompletionModel;
class KateVariableExpansionManager;

namespace KTextEditor
{
/**
 * KTextEditor::EditorPrivate
 * One instance of this class is hold alive during
 * a kate part session, as long as any factory, document
 * or view stay around, here is the place to put things
 * which are needed and shared by all this objects ;)
 */
class KTEXTEDITOR_EXPORT EditorPrivate final : public KTextEditor::Editor
{
    Q_OBJECT

    friend class KTextEditor::Editor;

public:
    // for setDefaultEncoding
    friend class KateDocumentConfig;

private:
    /**
     * Default constructor, private, as singleton
     * @param staticInstance pointer to fill with content of this
     */
    KTEXTEDITOR_NO_EXPORT
    explicit EditorPrivate(QPointer<KTextEditor::EditorPrivate> &staticInstance);

public:
    /**
     * Destructor
     */
    ~EditorPrivate() override;

    /**
     * Create a new document object
     * @param parent parent object
     * @return created KTextEditor::Document
     */
    KTextEditor::Document *createDocument(QObject *parent) override;

    /**
     * Returns a list of all documents of this editor.
     * @return list of all existing documents
     */
    QList<KTextEditor::Document *> documents() override
    {
        return m_documents;
    }

    /**
     * Set the global application object.
     * This will allow the editor component to access
     * the hosting application.
     * @param application application object
     */
    void setApplication(KTextEditor::Application *application) override
    {
        // switch back to dummy application?
        m_application = application ? application : &m_dummyApplication;
    }

    /**
     * Current hosting application, if any set.
     * @return current application object or nullptr
     */
    KTextEditor::Application *application() const override
    {
        return m_application;
    }

    /**
     * General Information about this editor
     */
public:
    /**
     * return the about data
     * @return about data of this editor part
     */
    const KAboutData &aboutData() const override
    {
        return m_aboutData;
    }

    /**
     * Configuration management
     */
public:
    /**
     * Shows a config dialog for the part, changes will be applied
     * to the editor, but not saved anywhere automagically, call
     * writeConfig to save them
     */
    void configDialog(QWidget *parent) override;

    /**
     * Number of available config pages
     * If the editor returns a number < 1, it doesn't support this
     * and the embedding app should use the configDialog () instead
     * @return number of config pages
     */
    int configPages() const override;

    /**
     * returns config page with the given number,
     * config pages from 0 to configPages()-1 are available
     * if configPages() > 0
     */
    KTextEditor::ConfigPage *configPage(int number, QWidget *parent) override;

    /**
     * Kate Part Internal stuff ;)
     */
public:
    /**
     * singleton accessor
     * @return instance of the factory
     */
    static KTextEditor::EditorPrivate *self();

    /**
     * register document at the factory
     * this allows us to loop over all docs for example on config changes
     * @param doc document to register
     */
    void registerDocument(KTextEditor::DocumentPrivate *doc);

    /**
     * unregister document at the factory
     * @param doc document to register
     */
    void deregisterDocument(KTextEditor::DocumentPrivate *doc);

    /**
     * register view at the factory
     * this allows us to loop over all views for example on config changes
     * @param view view to register
     */
    void registerView(KTextEditor::ViewPrivate *view);

    /**
     * unregister view at the factory
     * @param view view to unregister
     */
    void deregisterView(KTextEditor::ViewPrivate *view);

    /**
     * return a list of all registered views
     * @return all known views
     */
    const std::vector<KTextEditor::ViewPrivate *> &views()
    {
        return m_views;
    }

    /**
     * global dirwatch
     * @return dirwatch instance
     */
    KDirWatch *dirWatch()
    {
        return m_dirWatch;
    }

    /**
     * The global configuration of katepart, e.g. katepartrc
     * @return global shared access to katepartrc config
     */
    static KSharedConfigPtr config();

    /**
     * global mode manager
     * used to manage the modes centrally
     * @return mode manager
     */
    KateModeManager *modeManager()
    {
        return m_modeManager;
    }

    /**
     * fallback document config
     * @return default config for all documents
     */
    KateDocumentConfig *documentConfig()
    {
        return m_documentConfig;
    }

    /**
     * fallback view config
     * @return default config for all views
     */
    KateViewConfig *viewConfig()
    {
        return m_viewConfig;
    }

    /**
     * fallback renderer config
     * @return default config for all renderers
     */
    KateRendererConfig *rendererConfig()
    {
        return m_rendererConfig;
    }

    /**
     * Global script collection
     */
    KateScriptManager *scriptManager()
    {
        return m_scriptManager;
    }

    /**
     * hl manager
     * @return hl manager
     */
    KateHlManager *hlManager()
    {
        return m_hlManager;
    }

    /**
     * command manager
     * @return command manager
     */
    KateCmd *cmdManager()
    {
        return m_cmdManager;
    }

    /**
     * spell check manager
     * @return spell check manager
     */
    KateSpellCheckManager *spellCheckManager()
    {
        return m_spellCheckManager;
    }

    /**
     * global instance of the simple word completion mode
     * @return global instance of the simple word completion mode
     */
    KateWordCompletionModel *wordCompletionModel()
    {
        return m_wordCompletionModel;
    }

    /**
     * Global instance of the language-aware keyword completion model
     * @return global instance of the keyword completion model
     */
    KateKeywordCompletionModel *keywordCompletionModel()
    {
        return m_keywordCompletionModel;
    }

    /**
     * query for command
     * @param cmd name of command to query for
     * @return found command or 0
     */
    KTextEditor::Command *queryCommand(const QString &cmd) const override;

    /**
     * Get a list of all registered commands.
     * \return list of all commands
     */
    QList<KTextEditor::Command *> commands() const override;

    /**
     * Get a list of available commandline strings.
     * \return commandline strings
     */
    QStringList commandList() const override;

    /**
     * Copy text to clipboard an remember it in the history
     * @param text text to copy to clipboard, does nothing if empty!
     * @param fileName fileName of the text to copy, used for highlighting
     */
    void copyToClipboard(const QString &text, const QString &fileName);

    /**
     * A clipboard entry stores the copied text and the filename of
     * the copied text.
     */
    struct ClipboardEntry {
        /**
         * The copied text
         */
        QString text;
        /**
         * The file name of the file containing the copied text,
         * used for syntax highlighting
         */
        QString fileName;
    };

    friend inline bool operator==(const ClipboardEntry &lhs, const ClipboardEntry &rhs)
    {
        return lhs.text == rhs.text && lhs.fileName == rhs.fileName;
    }

    /**
     * Clipboard history, filled with text we ever copied
     * to clipboard via copyToClipboard.
     */
    const QList<ClipboardEntry> &clipboardHistory() const
    {
        return m_clipboardHistory;
    }

    /**
     * Dummy main window to be null safe.
     * @return dummy main window
     */
    KTextEditor::MainWindow *dummyMainWindow()
    {
        return &m_dummyMainWindow;
    }

    /**
     * @return list of available input mode factories
     */
    const std::array<std::unique_ptr<KateAbstractInputModeFactory>, KTextEditor::View::ViInputMode + 1> &inputModeFactories()
    {
        return m_inputModeFactories;
    }

    /**
     * Search pattern history shared among simple/power search instances.
     */
    QStringListModel *searchHistoryModel();

    /**
     * Replace pattern history shared among simple/power search instances.
     */
    QStringListModel *replaceHistoryModel();

    /**
     * Call this function to store the history models to the application config.
     */
    void saveSearchReplaceHistoryModels();

    /**
     * Returns the variable expansion manager.
     */
    KateVariableExpansionManager *variableExpansionManager();

    /**
     * Trigger delayed emission of config changed.
     */
    void triggerConfigChanged();

    void copyToMulticursorClipboard(const QStringList &texts);

    QStringList multicursorClipboard() const;

    /**
     * text to speech engine to be use by the view actions, constructed on demand.
     * @param view the view that want to use it
     */
    QTextToSpeech *speechEngine(KTextEditor::ViewPrivate *view);

private Q_SLOTS:
    /**
     * Emit configChanged if needed.
     * Used to bundle emissions.
     */
    void emitConfigChanged();

    /**
     * Was the view that started the current speak output destroyed?
     */
    void speechEngineUserDestoyed();

    /**
     * Speech error occured.
     * @param view view to signal error to
     * @param errorString error to show
     */
    void speechError(KTextEditor::ViewPrivate *view, const QString &errorString);

Q_SIGNALS:
    /**
     * Emitted if the history of clipboard changes via copyToClipboard
     */
    void clipboardHistoryChanged();

protected:
    bool eventFilter(QObject *, QEvent *) override;

private Q_SLOTS:
    void updateColorPalette();

private:
    /**
     * about data (authors and more)
     */
    KAboutData m_aboutData;

    /**
     * registered docs, map from general to specialized pointer
     */
    QList<KTextEditor::Document *> m_documents;

    /**
     * registered views
     */
    std::vector<KTextEditor::ViewPrivate *> m_views;

    /**
     * global dirwatch object
     */
    KDirWatch *m_dirWatch;

    /**
     * mode manager
     */
    KateModeManager *m_modeManager;

    /**
     * global config
     */
    KateGlobalConfig *m_globalConfig;

    /**
     * fallback document config
     */
    KateDocumentConfig *m_documentConfig;

    /**
     * fallback view config
     */
    KateViewConfig *m_viewConfig;

    /**
     * fallback renderer config
     */
    KateRendererConfig *m_rendererConfig;

    /**
     * internal commands
     */
    std::array<KTextEditor::Command *, 5> m_cmds;

    /**
     * script manager
     */
    KateScriptManager *m_scriptManager;

    /**
     * hl manager
     */
    KateHlManager *m_hlManager;

    /**
     * command manager
     */
    KateCmd *m_cmdManager;

    /**
     * variable expansion manager
     */
    KateVariableExpansionManager *m_variableExpansionManager;

    /**
     * spell check manager
     */
    KateSpellCheckManager *m_spellCheckManager;

    /**
     * global instance of the simple word completion mode
     */
    KateWordCompletionModel *m_wordCompletionModel;

    /**
     * global instance of the language-specific keyword completion model
     */
    KateKeywordCompletionModel *m_keywordCompletionModel;

    /**
     * clipboard history
     */
    QList<ClipboardEntry> m_clipboardHistory;

    /**
     * Dummy application object to be null safe
     */
    KTextEditor::Application m_dummyApplication;

    /**
     * access to application
     */
    QPointer<KTextEditor::Application> m_application;

    /**
     * Dummy main window to be null safe
     */
    KTextEditor::MainWindow m_dummyMainWindow;

    /**
     * input modes factories
     * for all input modes in the KTextEditor::View::InputMode we have here an entry
     */
    std::array<std::unique_ptr<KateAbstractInputModeFactory>, KTextEditor::View::ViInputMode + 1> m_inputModeFactories;

    /**
     * simple list that stores text copied
     * from all cursors selection
     * It's main purpose is providing multi-paste
     * support.
     */
    QStringList m_multicursorClipboard;

    /**
     * Shared history models for search & replace.
     */
    QStringListModel *m_searchHistoryModel;
    QStringListModel *m_replaceHistoryModel;

    /**
     * We collapse configChanged signals to avoid that e.g.
     * Document/View/Renderer/... updates cause X emitted signals in a row.
     * This bool tells if we still shall emit a signal in the delayed connected
     * slot.
     */
    bool m_configWasChanged = false;

    /**
     * text to speech engine to be use by the view actions, constructed on demand
     */
    QTextToSpeech *m_speechEngine = nullptr;

    /**
     * view that triggered last speech action
     * used to show error messages and to stop output on view destruction
     */
    QPointer<KTextEditor::ViewPrivate> m_speechEngineLastUser;
};

}

#endif
