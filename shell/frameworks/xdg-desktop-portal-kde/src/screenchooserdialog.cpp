/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#include "screenchooserdialog.h"
#include "utils.h"
#include "waylandintegration.h"

#include "region-select/SelectionEditor.h"

#include <KLocalizedString>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmawindowmodel.h>

#include <QCoreApplication>
#include <QScreen>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QTimer>
#include <QWindow>

class FilteredWindowModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged)
public:
    FilteredWindowModel(QObject *parent)
        : QSortFilterProxyModel(parent)
    {
    }

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        if (source_parent.isValid())
            return false;

        const auto idx = sourceModel()->index(source_row, 0);
        using KWayland::Client::PlasmaWindowModel;

        return !idx.data(PlasmaWindowModel::SkipTaskbar).toBool() //
            && !idx.data(PlasmaWindowModel::SkipSwitcher).toBool() //
            && idx.data(PlasmaWindowModel::Pid) != QCoreApplication::applicationPid();
    }

    QMap<int, QVariant> itemData(const QModelIndex &index) const override
    {
        using KWayland::Client::PlasmaWindowModel;
        auto ret = QSortFilterProxyModel::itemData(index);
        for (int i = PlasmaWindowModel::AppId; i <= PlasmaWindowModel::Uuid; ++i) {
            ret[i] = index.data(i);
        }
        return ret;
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::CheckStateRole) {
            return false;
        }

        using KWayland::Client::PlasmaWindowModel;
        const QString uuid = index.data(PlasmaWindowModel::Uuid).toString();
        if (value == Qt::Checked) {
            m_selected.insert(index);
        } else {
            m_selected.remove(index);
        }
        Q_EMIT dataChanged(index, index, {role});
        if (m_selected.count() <= 1) {
            Q_EMIT hasSelectionChanged();
        }
        return true;
    }

    QList<QMap<int, QVariant>> selectedWindows() const
    {
        QList<QMap<int, QVariant>> ret;
        ret.reserve(m_selected.size());
        for (const auto &index : m_selected) {
            if (index.isValid())
                ret << itemData(index);
        }
        return ret;
    }

    QHash<int, QByteArray> roleNames() const override
    {
        QHash<int, QByteArray> ret = sourceModel()->roleNames();
        ret.insert(Qt::CheckStateRole, "checked");
        return ret;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
            return {};
        }

        switch (role) {
        case Qt::CheckStateRole:
            return m_selected.contains(index) ? Qt::Checked : Qt::Unchecked;
        default:
            return QSortFilterProxyModel::data(index, role);
        }
        return {};
    }

    bool hasSelection()
    {
        return !m_selected.isEmpty();
    }

    void clearSelection()
    {
        auto selected = m_selected;
        m_selected.clear();

        for (const auto &index : selected) {
            if (index.isValid())
                Q_EMIT dataChanged(index, index, {Qt::CheckStateRole});
        }
        Q_EMIT hasSelectionChanged();
    }

Q_SIGNALS:
    void hasSelectionChanged();

private:
    QSet<QPersistentModelIndex> m_selected;
};

ScreenChooserDialog::ScreenChooserDialog(const QString &appName, bool multiple, ScreenCastPortal::SourceTypes types)
    : QuickDialog()
{
    Q_ASSERT(types != 0);

    QVariantMap props = {
        {"title", i18n("Screen Sharing")},
        {"multiple", multiple},
    };

    int numberOfMonitors = 0;
    if (types & ScreenCastPortal::Monitor) {
        auto model =
            new OutputsModel(OutputsModel::Options(OutputsModel::WorkspaceIncluded | OutputsModel::VirtualIncluded | OutputsModel::RegionIncluded), this);
        props.insert("outputsModel", QVariant::fromValue<QObject *>(model));
        numberOfMonitors += model->rowCount(QModelIndex());
        connect(this, &ScreenChooserDialog::clearSelection, model, &OutputsModel::clearSelection);
    }

    int numberOfWindows = 0;
    if (types & ScreenCastPortal::Window) {
        auto model = new KWayland::Client::PlasmaWindowModel(WaylandIntegration::plasmaWindowManagement());
        auto windowsProxy = new FilteredWindowModel(this);
        windowsProxy->setSourceModel(model);
        props.insert("windowsModel", QVariant::fromValue<QObject *>(windowsProxy));
        connect(this, &ScreenChooserDialog::clearSelection, windowsProxy, &FilteredWindowModel::clearSelection);
        numberOfWindows += model->rowCount(QModelIndex());
    }

    const QString applicationName = Utils::applicationName(appName);

    QString mainText;

    // App asked for monitors and windows
    if (types & ScreenCastPortal::Monitor && types & ScreenCastPortal::Window) {
        if (appName.isEmpty()) {
            mainText = i18n("Choose what to share with the requesting application:");
        } else {
            mainText = i18n("Choose what to share with %1:", applicationName);
        }
    }

    // App only asked for monitors
    else if (types & ScreenCastPortal::Monitor) {
        if (numberOfMonitors == 1) {
            if (appName.isEmpty()) {
                mainText = i18n("Share this screen with the requesting application?");
            } else {
                mainText = i18n("Share this screen with %1?", applicationName);
            }
        } else {
            if (multiple) {
                if (appName.isEmpty()) {
                    mainText = i18n("Choose screens to share with the requesting application:");
                } else {
                    mainText = i18n("Choose screens to share with %1:", applicationName);
                }
            } else {
                if (appName.isEmpty()) {
                    mainText = i18n("Choose which screen to share with the requesting application:");
                } else {
                    mainText = i18n("Choose which screen to share with %1:", applicationName);
                }
            }
        }
    }

    // App only asked for windows
    else if (types & ScreenCastPortal::Window) {
        if (numberOfWindows == 1) {
            if (appName.isEmpty()) {
                mainText = i18n("Share this window with the requesting application?");
            } else {
                mainText = i18n("Share this window with %1?", applicationName);
            }
        } else {
            if (multiple) {
                if (appName.isEmpty()) {
                    mainText = i18n("Choose windows to share with the requesting application:");
                } else {
                    mainText = i18n("Choose windows to share with %1:", applicationName);
                }
            } else {
                if (appName.isEmpty()) {
                    mainText = i18n("Choose which window to share with the requesting application:");
                } else {
                    mainText = i18n("Choose which window to share with %1:", applicationName);
                }
            }
        }
    }
    props.insert("mainText", mainText);

    create(QStringLiteral("qrc:/ScreenChooserDialog.qml"), props);
    connect(m_theDialog, SIGNAL(clearSelection()), this, SIGNAL(clearSelection()));
}

ScreenChooserDialog::~ScreenChooserDialog() = default;

QList<Output> ScreenChooserDialog::selectedOutputs() const
{
    OutputsModel *model = dynamic_cast<OutputsModel *>(m_theDialog->property("outputsModel").value<QObject *>());
    if (!model) {
        return {};
    }
    return model->selectedOutputs();
}

QList<QMap<int, QVariant>> ScreenChooserDialog::selectedWindows() const
{
    FilteredWindowModel *model = dynamic_cast<FilteredWindowModel *>(m_theDialog->property("windowsModel").value<QObject *>());
    if (!model) {
        return {};
    }
    return model->selectedWindows();
}

QRect ScreenChooserDialog::selectedRegion() const
{
    return m_region;
}

void ScreenChooserDialog::setRegion(const QRect region)
{
    m_region = region;
}

bool ScreenChooserDialog::allowRestore() const
{
    return m_theDialog->property("allowRestore").toBool();
}

void ScreenChooserDialog::accept()
{
    bool valid = true;

    for (const auto &output : selectedOutputs()) {
        if (output.outputType() == Output::OutputType::Region) {
            QScopedPointer<SelectionEditor, QScopedPointerDeleteLater> selectionEditor(new SelectionEditor(this));

            valid = selectionEditor->exec();
            setRegion(selectionEditor->rect());

            break;
        }
    }

    if (!valid) {
        // if we selected rectangular region, but didn't actually choose a region, start over
        QTimer::singleShot(0, m_theDialog, SLOT(present()));
    } else {
        QuickDialog::accept();
    }
}

#include "screenchooserdialog.moc"
