/***************************************************************************
 *   This file is part of Kate build plugin                                *
 *   SPDX-FileCopyrightText: 2014 Kåre Särs <kare.sars@iki.fi>             *
 *                                                                         *
 *   SPDX-License-Identifier: LGPL-2.0-or-later                            *
 ***************************************************************************/

#ifndef TargetModel_h
#define TargetModel_h

#include <QAbstractItemModel>
#include <QByteArray>

class TargetModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    struct Command {
        QString name;
        QString buildCmd;
        QString runCmd;
    };
    struct TargetSet {
        TargetSet(const QString &_name, const QString &_workDir);
        QString name;
        QString workDir;
        QList<Command> commands;
    };

    TargetModel(QObject *parent = nullptr);
    ~TargetModel() override;

public Q_SLOTS:

    /** This function clears all the target-sets */
    void clear();

    /** This function adds a target set and returns the model-index of the newly
     * inserted target-set */
    QModelIndex addTargetSet(const QString &setName, const QString &workDir);

    /** This function insert a target set and returns the model-index of the newly
     * inserted target-set */
    QModelIndex insertTargetSet(int row, const QString &setName, const QString &workDir);

    /** This function adds a new command to a target-set and returns the model index */
    QModelIndex addCommand(const QModelIndex &parentIndex, const QString &cmdName, const QString &buildCmd, const QString &runCmd);

    /** This function copies the target(-set) the model index points to and returns
     * the model index of the copy. */
    QModelIndex copyTargetOrSet(const QModelIndex &index);

    /** This function deletes the index */
    void deleteItem(const QModelIndex &index);

    /** This function deletes the target-set with the same name */
    void deleteTargetSet(const QString &targetSet);

    void moveRowUp(const QModelIndex &index);
    void moveRowDown(const QModelIndex &index);

    const QList<TargetSet> targetSets() const
    {
        return m_targets;
    }

    /** Static functions that could be in a different file */
    static const QString command(const QModelIndex &itemIndex);
    static const QString cmdName(const QModelIndex &itemIndex);
    static const QString workDir(const QModelIndex &itemIndex);
    static const QStringList searchPaths(const QModelIndex &itemIndex);
    static const QString targetName(const QModelIndex &itemIndex);

Q_SIGNALS:

public:
    static const quint32 InvalidIndex = 0xFFFFFFFF;
    // Model-View model functions
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QList<TargetSet> m_targets;
};

#endif
