/*
 * SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QAbstractListModel>
#include <QScreen>
#include <QSet>

class Output
{
public:
    enum OutputType {
        Unknown,
        Laptop,
        Monitor,
        Television,
        Workspace,
        Virtual,
        Region,
    };

    Output(OutputType outputType, QScreen *screen, const QString &display, const QString &uniqueId, const QString &name)
        : m_outputType(outputType)
        , m_screen(screen)
        , m_display(display)
        , m_uniqueId(uniqueId)
        , m_name(name)
    {
    }

    QScreen *screen() const
    {
        return m_screen;
    }

    QString name() const
    {
        return m_name;
    }

    QString iconName() const
    {
        switch (m_outputType) {
        case Laptop:
            return QStringLiteral("computer-laptop");
        case Television:
            return QStringLiteral("video-television");
        default:
            return QStringLiteral("video-display");
        }
    }

    QString display() const
    {
        return m_display;
    }

    QString uniqueId() const
    {
        return m_uniqueId;
    }

    OutputType outputType() const
    {
        return m_outputType;
    }

private:
    OutputType m_outputType;
    QScreen *m_screen = nullptr;
    QString m_display;
    QString m_uniqueId;
    QString m_name;
};

class OutputsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged)
public:
    enum Option {
        None = 0,
        WorkspaceIncluded = 0x1,
        VirtualIncluded = 0x2,
        RegionIncluded = 0x4,
    };
    Q_ENUM(Option)
    Q_DECLARE_FLAGS(Options, Option)

    enum Roles {
        ScreenRole = Qt::UserRole,
        NameRole,
    };

    OutputsModel(Options o, QObject *parent);
    ~OutputsModel() override;

    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    const Output &outputAt(int row) const;
    QList<Output> selectedOutputs() const;
    bool hasSelection() const
    {
        return !m_selectedRows.isEmpty();
    }

public Q_SLOTS:
    void clearSelection();

Q_SIGNALS:
    void hasSelectionChanged();

private:
    QList<Output> m_outputs;
    QSet<quint32> m_selectedRows;
};
