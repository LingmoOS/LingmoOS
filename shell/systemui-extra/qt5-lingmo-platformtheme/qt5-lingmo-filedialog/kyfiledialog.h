/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#ifndef KYNATIVEFILEDIALOG_H
#define KYNATIVEFILEDIALOG_H
#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QCompleter>
#include <QListView>
#include <QStringListModel>
#include <QTranslator>
#include "qpa/qplatformdialoghelper.h"
#include <explorer-qt/FMWindowIface.h>

#define OPEN_ACTIONS_SEPARATOR "open-actions-seperator"
#define CREATE_ACTIONS_SEPARATOR "create-actions-seperator"
#define VIEW_ACTIONS_SEPARATOR "view-actions-seperator"
#define FILE_OPERATION_ACTIONS_SEPARATOR "file-operation-actions-seperator"
#define PLUGIN_ACTIONS_SEPARATOR "plugin-actions-seperator"
#define PROPERTIES_ACTIONS_SEPARATOR "properties-actions-seperator"
#define COMPUTER_ACTIONS_SEPARATOR "computer-actions-seperator"
#define TRASH_ACTIONS_SEPARATOR "trash-actions-seperator"
#define OPEN_IN_NEW_WINDOW_ACTION "open-in-new-window-action"
#define OPEN_IN_NEW_TAB_ACTION "open-in-new-tab-action"
#define ADD_TO_BOOKMARK_ACTION "add-to-bookmark-action"
#define OPEN_ACTION "open-action"
#define OPEN_WITH_ACTION "open-with-action"
#define OPEN_SELECTED_FILES_ACTION "open-selected-files-action"
#define CREATE_ACTION "create-action"
#define VIEW_TYPE_ACTION "view-type-action"
#define SORT_TYPE_ACTION "sort-type-action"
#define SORT_ORDER_ACTION "sort-order-action")
#define SORT_PREFERENCES_ACTION "sort-preferences-action"
#define COPY_ACTION "copy-action"
#define CUT_ACTION "cut-action"
#define TRASH_ACTION "trash-action"
#define DELETE_ACTION "delete-action"
#define RENAME_ACTION "rename-action"
#define PASTE_ACTION "paste-action"
#define REFRESH_ACTION "refresh-action"
#define SELECT_ALL_ACTION "select-all-action"
#define REVERSE_SELECT_ACTION "reverse-select-action"
#define PROPERTIES_ACTION "properties-action"
#define FORMAT_ACTION "format-action"
#define CLEAN_THE_TRASH_ACTION "clean-the-trash-action"
#define RESTORE_ACTION "restore-action"
#define CLEAN_THE_RECENT_ACTION "clean-the-recent-action"

QT_BEGIN_NAMESPACE
namespace Ui { class KyNativeFileDialog; }
QT_END_NAMESPACE

namespace Peony {
class DirectoryViewContainer;
class DirectoryViewWidget;
class FMWindowIface;
}

namespace LINGMOFileDialog {

class Ui_KyFileDialog;
class FileDialogSideBar;
class FileDialogPathBar;
class KyNativeFileDialogPrivate;
class KyFileDialogHelper;

class KyNativeFileDialog : public QDialog, public Peony::FMWindowIface
{
    Q_OBJECT
    friend class KyFileDialogHelper;

public:
    enum ViewMode { List, Icon };
    KyNativeFileDialog(QWidget *parent = nullptr);
    ~KyNativeFileDialog();

    Peony::FMWindowIface *create(const QString &uri);
    Peony::FMWindowIface *create(const QStringList &uris);


    const QString getCurrentUri();
    const QStringList getCurrentSelections();
    const QStringList getCurrentAllFileUris();
    const QStringList getCurrentSelectionsList();
    const QList<std::shared_ptr<Peony::FileInfo>> getCurrentSelectionFileInfos();

    void setCurrentSelections(QStringList selections);

    Qt::SortOrder getCurrentSortOrder();
    int getCurrentSortColumn();

    bool getWindowShowHidden();
    bool getWindowUseDefaultNameSortOrder();
    bool getWindowSortFolderFirst();

    void refresh();
    void forceStopLoading();

    void setShowHidden(bool showHidden);
    void setUseDefaultNameSortOrder(bool use);
    void setSortFolderFirst(bool set);

    void setCurrentSelectionUris(const QStringList &uris);
    void setCurrentSortOrder (Qt::SortOrder order);
    void setCurrentSortColumn (int sortColumn);

    void editUri(const QString &uri);
    void editUris(const QStringList &uris);

    bool getFilterWorking();

    void beginSwitchView(const QString &viewId);


    void setDirectory(const QString &directory);
    void setDirectory(const QDir &directory);
    QDir directory() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void setDirectoryUrl(const QUrl &directory);
    QUrl directoryUrl() const;

    void selectUrl(const QUrl &url);
    QList<QUrl> selectedUrls() const;

    void setNameFilterDetailsVisible(bool enabled);
    bool isNameFilterDetailsVisible() const;

    void setNameFilter(const QString &filter);
    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString &filter);
    QString selectedMimeTypeFilter() const;
    QString selectedNameFilter() const;
    void selectNameFilterByIndex(int index);
    void selectNameFilterCurrentIndex(int index);
    int selectNameFilterIndex() const;

    QDir::Filters filter();
    void setFilter(QDir::Filters filters);

    void setViewMode(ViewMode mode);
    ViewMode viewMode() const;

    void setFileMode(QFileDialog::FileMode mode);
    QFileDialog::FileMode fileMode();

    void setAcceptMode(QFileDialog::AcceptMode mode);
    QFileDialog::AcceptMode acceptMode() const;

    void setLabelText(QFileDialog::DialogLabel label, const QString &text);
    QString labelText(QFileDialog::DialogLabel label) const;

    void setOptions(QFileDialog::Options options);
    void setOption(QFileDialog::Option option, bool on = true);
    bool testOption(QFileDialog::Option option) const;
    QFileDialog::Options options() const;
    void setCurrentInputName(const QString &name);
    Peony::DirectoryViewContainer *getCurrentPage();
    Peony::DirectoryViewContainer *getCurrentPage() const;

    Peony::DirectoryViewWidget *containerView() const;

    void  setComBoxItems(const QStringList &filters);

    bool isDir(QString path);

    void updateMaximizeState();

    void intiContainerSort();

    void refreshContainerSort();

    void refreshCompleter();

    bool doSave(QStringList sFiles);

    bool doOpen(QStringList sFiles);

    bool saveMessageBox(QString name);

    void setHelper(KyFileDialogHelper* helepr);

    void setShortCuts();

    void initialViewId();

    void delayShow();

    void discardDelayedShow();

    QString copyEditText();

    void installTranslate(QString local);

Q_SIGNALS:
    void switchViewRequest(const QString &viewId);

    void fileSelected(const QUrl &file);
    void filesSelected(const QList<QUrl> &files);
    void currentChanged(const QUrl &path);
    void directoryEntered(const QUrl &directory);
    void selectedNameFilterChanged();
    void filterSelected(const QString &filter);
    void locationChangeEnd();

public Q_SLOTS:
    void goToUri(const QString &uri, bool addToHistory = true, bool forceUpdate = false);
    void goBack();
    void goForward();
    void goToParent();
    void onSwitchView();
    void updateWindowState();
    QString selectName();
    void updateStatusBar();
    void onNewFolder();
    void setSortType();
    void searchButtonClicked();
    void setSearchMode(bool mode);
    void lineEditTextChange(QString text);
    void containerMenuRequest(const QPoint &pos);
    void updateTableModel(bool tableMode);
    void updateSearchProgressBar();

protected:
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    Ui_KyFileDialog *mKyFileDialogUi = nullptr;
    QScopedPointer<KyNativeFileDialogPrivate>  d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), KyNativeFileDialog)
    void onAcceptButtonClicked();
    void onRejectButtonClicked();
    void onCurrentInputNameChanged();
    void handleEnterPressed();
    void updateAcceptButtonState();
    bool checkSaveFileExsits(QString path);
    void isTableModel();

private:
    bool m_searchMode = false;
//    QFileSystemModel *m_fileSystemModel = nullptr;
    QCompleter * m_completer = nullptr;
    QListView *m_listView = nullptr;
    QStringList m_CurrentPathAllFiles;
    KyFileDialogHelper *m_fileDialogHelper = nullptr;
    bool isInitialGoToUriNum = true;
    bool m_shortcutsSet = false;
    QString m_last_no_SearchPath;
    bool m_isClearSearchKey = false;
    QStringListModel *m_model = nullptr;
    QString m_copyEditText;
    bool m_istableModel = false;
    QMap<int, QString> m_nameFilterMap;
    QStringList m_uris_to_edit;
    bool m_needSearch = false;

    QTranslator *m_translator = nullptr;
    QTranslator *m_translator0 = nullptr;
    QTranslator *m_translator1 = nullptr;
    QTranslator *m_translator2 = nullptr;
    QTranslator *m_translator3 = nullptr;

};

class KyFileDialogHelper : public QPlatformFileDialogHelper
{
    Q_OBJECT
public:
     explicit KyFileDialogHelper();

    ~KyFileDialogHelper() override;


    virtual void exec() override;
    virtual bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;
    virtual void hide() override;

    virtual bool defaultNameFilterDisables() const override;
    virtual void setDirectory(const QUrl &directory) override;
    virtual QUrl directory() const override;
    virtual void selectFile(const QUrl &filename) override;
    virtual QList<QUrl> selectedFiles() const override;
    virtual void setFilter() override;
    virtual void selectNameFilter(const QString &filter) override;

    virtual void selectMimeTypeFilter(const QString &filter) override;
    virtual QString selectedNameFilter() const override;
    virtual QString selectedMimeTypeFilter() const override;

    virtual bool isSupportedUrl(const QUrl &url) const override;

    bool isViewInitialFinished();

    bool isShow();

public Q_SLOTS:
    void viewInitialFinished();
Q_SIGNALS:
    void fileSelected(const QUrl &file);
    void filesSelected(const QList<QUrl> &files);
    void currentChanged(const QUrl &path);
    void directoryEntered(const QUrl &directory);
    void filterSelected(const QString &filter);


private:
    KyNativeFileDialog*  mKyFileDialog = nullptr;
    QUrl m_selectedFiles;
    bool m_viewInitialFinished = false;
    bool m_isShow = false;
    QUrl m_initialDirectory;
    QList<QUrl> m_initialSelectFiles;

private:
    void initDialog();
};
}

#endif // KYNATIVEFILEDIALOG_H
