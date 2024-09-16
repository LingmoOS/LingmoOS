// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "filechooser.h"
#include "request.h"
#include "utils.h"

#include <QLoggingCategory>
#include <QFileDialog>
#include <QDBusMetaType>

// Keep in sync with qflatpakfiledialog from flatpak-platform-plugin
Q_DECLARE_METATYPE(FileChooserPortal::Filter)
Q_DECLARE_METATYPE(FileChooserPortal::Filters)
Q_DECLARE_METATYPE(FileChooserPortal::FilterList)
Q_DECLARE_METATYPE(FileChooserPortal::FilterListList)
// used for options - choices
Q_DECLARE_METATYPE(FileChooserPortal::Choice)
Q_DECLARE_METATYPE(FileChooserPortal::Choices)
Q_DECLARE_METATYPE(FileChooserPortal::Option)
Q_DECLARE_METATYPE(FileChooserPortal::OptionList)

QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::Filter &filter)
{
    arg.beginStructure();
    arg << filter.type << filter.filterString;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::Filter &filter)
{
    uint type;
    QString filterString;
    arg.beginStructure();
    arg >> type >> filterString;
    filter.type = type;
    filter.filterString = filterString;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::FilterList &filterList)
{
    arg.beginStructure();
    arg << filterList.userVisibleName << filterList.filters;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::FilterList &filterList)
{
    QString userVisibleName;
    FileChooserPortal::Filters filters;
    arg.beginStructure();
    arg >> userVisibleName >> filters;
    filterList.userVisibleName = userVisibleName;
    filterList.filters = filters;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::Choice &choice)
{
    arg.beginStructure();
    arg << choice.id << choice.value;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::Choice &choice)
{
    QString id;
    QString value;
    arg.beginStructure();
    arg >> id >> value;
    choice.id = id;
    choice.value = value;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::Option &option)
{
    arg.beginStructure();
    arg << option.id << option.label << option.choices << option.initialChoiceId;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::Option &option)
{
    QString id;
    QString label;
    FileChooserPortal::Choices choices;
    QString initialChoiceId;
    arg.beginStructure();
    arg >> id >> label >> choices >> initialChoiceId;
    option.id = id;
    option.label = label;
    option.choices = choices;
    option.initialChoiceId = initialChoiceId;
    arg.endStructure();
    return arg;
}

Q_LOGGING_CATEGORY(fileChooserCategory, "xdg-dde-filechooser")

FileChooserPortal::FileChooserPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    qCDebug(fileChooserCategory) << "init dde-filechooser";

    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<Filters>();
    qDBusRegisterMetaType<FilterList>();
    qDBusRegisterMetaType<FilterListList>();
    qDBusRegisterMetaType<Choice>();
    qDBusRegisterMetaType<Choices>();
    qDBusRegisterMetaType<Option>();
    qDBusRegisterMetaType<OptionList>();
}

uint FileChooserPortal::OpenFile(const QDBusObjectPath &handle,
                                 const QString &app_id,
                                 const QString &parent_window,
                                 const QString &title,
                                 const QVariantMap &options,
                                 QVariantMap &results)
{
    Q_UNUSED(app_id);

    qCDebug(fileChooserCategory) << __FUNCTION__ << "args:"
                                 << "\n handle:" << handle.path()
                                 << "\n app_id:" << app_id
                                 << "\n parent_window:" << parent_window
                                 << "\n title:" << title
                                 << "\n options:" << options
                                 << "\n results:" << results;

    // Whether to make the dialog modal. Default is yes.
    DECLEAR_PARA_WITH_FALLBACK(modal, toBool, true);
    // Whether to allow selection of multiple files. Default is no.
    DECLEAR_PARA_WITH_FALLBACK(multiple, toBool, false);
    // Whether to select for folders instead of files. Default is to select files.
    DECLEAR_PARA_WITH_FALLBACK(directory, toBool, false);
    // The label for the accept button. Mnemonic underlines are allowed.
    const QString &acceptText = parseAcceptLabel(options);

    QStringList nameFilters;
    QStringList mimeTypeFilters;
    QString selectedMimeTypeFilter;
    // mapping between filter strings and actual filters
    QMap<QString, FilterList> allFilters;

    parseFilters(options, nameFilters, mimeTypeFilters, allFilters, selectedMimeTypeFilter);

    // open directory
    if (options.contains(QStringLiteral("directory"))) {
        directory = options.value("directory").toBool();
    }

    if (directory && !options.contains(QStringLiteral("choices"))) {
        QFileDialog dirDialog;
        dirDialog.setWindowTitle(title);
        dirDialog.setModal(modal);
        dirDialog.setFileMode(QFileDialog::Directory);
        dirDialog.setOptions(QFileDialog::ShowDirsOnly);
        dirDialog.setSupportedSchemes(QStringList{QStringLiteral("file")});
        if (!acceptText.isEmpty()) {
            dirDialog.setLabelText(QFileDialog::Accept, acceptText);
        }

        dirDialog.winId(); // Trigger window creation

        Utils::setParentWindow(&dirDialog, parent_window);
        auto request = new Request(handle, QVariant(), &dirDialog);
        connect(request, &Request::closeRequested, &dirDialog, &QFileDialog::reject);

        if (dirDialog.exec() != QDialog::Accepted) {
            return 1;
        }

        const auto urls = dirDialog.selectedUrls();
        if (urls.empty()) {
            return 2;
        }

        results.insert(QStringLiteral("uris"), QUrl::toStringList(urls, QUrl::FullyEncoded));
        results.insert(QStringLiteral("writable"), true);

        return 0;
    }

    // handle choices
    if (options.contains(QStringLiteral("choices"))) {
        OptionList optionList = qdbus_cast<OptionList>(options.value(QStringLiteral("choices")));
        //        optionsWidget.reset(CreateChoiceControls(optionList, checkboxes, comboboxes));
    }

    if (options.contains(QStringLiteral("multiple"))) {
        multiple = options.value("multiple").toBool();
    }

    QFileDialog fileDialog;
    Utils::setParentWindow(&fileDialog, parent_window);
    auto request = new Request(handle, QVariant(), &fileDialog);
    connect(request, &Request::closeRequested, &fileDialog, &QFileDialog::reject);
    fileDialog.setWindowTitle(title);
    fileDialog.setModal(modal);
    fileDialog.setFileMode(multiple ? QFileDialog::FileMode::ExistingFiles : QFileDialog::FileMode::ExistingFile);
    if (!acceptText.isEmpty()) {
        fileDialog.setLabelText(QFileDialog::Accept, acceptText);
    }

    bool bMimeFilters = false;
    if (!mimeTypeFilters.isEmpty()) {
        fileDialog.setMimeTypeFilters(mimeTypeFilters);
        fileDialog.selectMimeTypeFilter(selectedMimeTypeFilter);
        bMimeFilters = true;
    } else if (!nameFilters.isEmpty()) {
        fileDialog.setNameFilters(nameFilters);
    }

    if (fileDialog.exec() != QDialog::Accepted)
        return 1;

    const auto &urls = fileDialog.selectedUrls();
    if (urls.isEmpty()) {
        qCDebug(fileChooserCategory) << "Failed to open file: no local file selected";
        return 2;
    }

    results.insert(QStringLiteral("uris"), QUrl::toStringList(urls, QUrl::FullyEncoded));
    results.insert(QStringLiteral("writable"), true);
    results.insert(QStringLiteral("choices"), ""); // TODO

    // try to map current filter back to one of the predefined ones
    QString selectedFilter;
    if (bMimeFilters) {
        selectedFilter = fileDialog.selectedMimeTypeFilter();
    } else {
        selectedFilter = fileDialog.selectedNameFilter();
    }
    if (allFilters.contains(selectedFilter)) {
        results.insert(QStringLiteral("current_filter"), QVariant::fromValue<FilterList>(allFilters.value(selectedFilter)));
    }

    return 0;
}

uint FileChooserPortal::SaveFile(const QDBusObjectPath &handle,
                                 const QString &app_id,
                                 const QString &parent_window,
                                 const QString &title,
                                 const QVariantMap &options,
                                 QVariantMap &results)
{
    qCDebug(fileChooserCategory) << __FUNCTION__ << "args:"
                                 << "\n handle:" << handle.path()
                                 << "\n app_id:" << app_id
                                 << "\n parent_window:" << parent_window
                                 << "\n title:" << title
                                 << "\n options:" << options
                                 << "\n results:" << results;


    // Whether to make the dialog modal. Default is yes.
    DECLEAR_PARA_WITH_FALLBACK(modal, toBool, true);
    // The label for the accept button. Mnemonic underlines are allowed.
    const QString &acceptText = parseAcceptLabel(options);
    // A suggested filename.
    DECLEAR_PARA(current_name, toString);
    // A suggested folder to save the file in.
    DECLEAR_PARA(current_folder, toString);
    // The current file (when saving an existing file).
    DECLEAR_PARA(current_file, toString);

    QStringList nameFilters;
    QStringList mimeTypeFilters;
    QString selectedMimeTypeFilter;
    // mapping between filter strings and actual filters
    QMap<QString, FilterList> allFilters;
    parseFilters(options, nameFilters, mimeTypeFilters, allFilters, selectedMimeTypeFilter);

    if (options.contains(QStringLiteral("choices"))) {
        OptionList optionList = qdbus_cast<OptionList>(options.value(QStringLiteral("choices")));
    }

    QFileDialog fileDialog;
    Utils::setParentWindow(&fileDialog, parent_window);
    auto request = new Request(handle, QVariant(), &fileDialog);
    connect(request, &Request::closeRequested, &fileDialog, &QFileDialog::reject);

    fileDialog.setWindowTitle(title);
    fileDialog.setModal(modal);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, false);
    fileDialog.setDirectory(current_folder);
    fileDialog.selectFile(current_file);
    fileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    if (!acceptText.isEmpty()) {
        fileDialog.setLabelText(QFileDialog::Accept, acceptText);
    }

    bool bMimeFilters = false;
    if (!mimeTypeFilters.isEmpty()) {
        fileDialog.setMimeTypeFilters(mimeTypeFilters);
        fileDialog.selectMimeTypeFilter(selectedMimeTypeFilter);
        bMimeFilters = true;
    } else if (!nameFilters.isEmpty()) {
        fileDialog.setNameFilters(nameFilters);
    }

    if (fileDialog.exec() != QDialog::Accepted)
        return 1;

    const auto &urls = fileDialog.selectedUrls();
    results.insert(QStringLiteral("uris"), QUrl::toStringList(urls, QUrl::FullyEncoded));
    results.insert(QStringLiteral("choices"), ""); // TODO
    results.insert(QStringLiteral("current_filter"), ""); // TODO

    return 0;
}

uint FileChooserPortal::SaveFiles(const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QString &title, const QVariantMap &options, QVariantMap &results)
{
    qCDebug(fileChooserCategory) << __FUNCTION__ << "args:"
                                 << "\n handle:" << handle.path()
                                 << "\n app_id:" << app_id
                                 << "\n parent_window:" << parent_window
                                 << "\n title:" << title
                                 << "\n options:" << options
                                 << "\n results:" << results;


    // Whether the dialog should be modal. Default is yes.
    DECLEAR_PARA_WITH_FALLBACK(modal, toBool, true);
    // Label for the accept button. Mnemonic underlines are allowed.
    const QString &acceptText = parseAcceptLabel(options);

    // Suggested folder to save the files in. The byte array is expected to be null-terminated.
    DECLEAR_PARA(current_folder, toString);
    // An array of file names to be saved. The array and byte arrays are expected to be null-terminated.
    DECLEAR_PARA(files, toStringList);

    QStringList nameFilters;
    QStringList mimeTypeFilters;
    QString selectedMimeTypeFilter;
    // mapping between filter strings and actual filters
    QMap<QString, FilterList> allFilters;
    parseFilters(options, nameFilters, mimeTypeFilters, allFilters, selectedMimeTypeFilter);

    if (options.contains(QStringLiteral("choices"))) {
        OptionList optionList = qdbus_cast<OptionList>(options.value(QStringLiteral("choices")));
    }

    QFileDialog fileDialog;
    Utils::setParentWindow(&fileDialog, parent_window);

    auto *request = new Request(handle, QVariant(), this);
    connect(request, &Request::closeRequested, &fileDialog, &QFileDialog::reject);

    fileDialog.setWindowTitle(title);
    fileDialog.setModal(modal);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite, false);
    fileDialog.setDirectory(current_folder);
    fileDialog.setFileMode(QFileDialog::FileMode::ExistingFiles);
    if (!acceptText.isEmpty()) {
        fileDialog.setLabelText(QFileDialog::Accept, acceptText);
    }

    bool bMimeFilters = false;
    if (!mimeTypeFilters.isEmpty()) {
        fileDialog.setMimeTypeFilters(mimeTypeFilters);
        fileDialog.selectMimeTypeFilter(selectedMimeTypeFilter);
        bMimeFilters = true;
    } else if (!nameFilters.isEmpty()) {
        fileDialog.setNameFilters(nameFilters);
    }

    if (fileDialog.exec() != QDialog::Accepted)
        return 1;

    const auto &urls = fileDialog.selectedUrls();
    results.insert(QStringLiteral("uris"), QUrl::toStringList(urls, QUrl::FullyEncoded));
    results.insert(QStringLiteral("choices"), ""); // TODO

    return 0;
}

QString FileChooserPortal::parseAcceptLabel(const QVariantMap &options)
{
    QString acceptLabel;
    if (options.contains(QStringLiteral("accept_label"))) {
        acceptLabel = options.value(QStringLiteral("accept_label")).toString();
        // 'accept_label' allows mnemonic underlines, but Qt uses '&' character, so replace/escape accordingly
        // to keep literal '&'s and transform mnemonic underlines to the Qt equivalent using '&' for mnemonic
        acceptLabel.replace(QChar::fromLatin1('&'), QStringLiteral("&&"));
        const int mnemonic_pos = acceptLabel.indexOf(QChar::fromLatin1('_'));
        if (mnemonic_pos != -1) {
            acceptLabel.replace(mnemonic_pos, 1, QChar::fromLatin1('&'));
        }
    }
    return acceptLabel;
}

void FileChooserPortal::parseFilters(const QVariantMap &options,
                                     QStringList &nameFilters,
                                     QStringList &mimeTypeFilters,
                                     QMap<QString, FilterList> &allFilters,
                                     QString &selectedMimeTypeFilter)
{
    if (options.contains(QStringLiteral("filters"))) {
        const FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
        for (const FilterList &filterList : filterListList) {
            QStringList filterStrings;
            for (const Filter &filterStruct : filterList.filters) {
                // a glob-style pattern (indicated by 0) or a mimetype (indicated by 1)
                if (filterStruct.type == 0) {
                    filterStrings << filterStruct.filterString;
                } else {
                    mimeTypeFilters << filterStruct.filterString;
                    allFilters[filterStruct.filterString] = filterList;
                }
            }

            if (!filterStrings.isEmpty()) {
                QString userVisibleName = filterList.userVisibleName;
                const QString filterString = filterStrings.join(QLatin1Char(' '));
                const QString nameFilter = QStringLiteral("%1(%2)").arg(userVisibleName, filterString);
                nameFilters << nameFilter;
                allFilters[filterList.userVisibleName] = filterList;
            }
        }
    }

    if (options.contains(QStringLiteral("current_filter"))) {
        FilterList filterList = qdbus_cast<FilterList>(options.value(QStringLiteral("current_filter")));
        // only one set of data is valid
        if (filterList.filters.size() != 1) {
            qCDebug(fileChooserCategory) << "Ignoring 'current_filter' parameter with 0 or multiple filters specified.";
            return;
        }

        Filter filterStruct = filterList.filters.at(0);
        if (filterStruct.type == 0) {
            QString userVisibleName = filterList.userVisibleName;
            QString nameFilter = QStringLiteral("%1|(%2)").arg(filterStruct.filterString, userVisibleName);
            nameFilters.removeAll(nameFilter);
            nameFilters.push_front(nameFilter);
        } else {
            selectedMimeTypeFilter = filterStruct.filterString;
        }
    }
}
