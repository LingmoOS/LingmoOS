/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef OUTPUTMODEL_H
#define OUTPUTMODEL_H

#include <kscreen/config.h>
#include <kscreen/output.h>
#include <kscreen/mode.h>

#include <QAbstractListModel>
#include <QPoint>

class ConfigHandler;

class OutputModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum OutputRoles {
        EnabledRole = Qt::UserRole + 1,
        InternalRole,
        PrimaryRole,
        SizeRole,
        /** Position in the graphical view relative to some arbitrary but fixed origin. */
        PositionRole,
        /** Position for backend relative to most northwest display corner. */
        NormalizedPositionRole,
        AutoRotateRole,
        AutoRotateOnlyInTabletModeRole,
        RotationRole,
        ScaleRole,
        ResolutionIndexRole,
        ResolutionsRole,
        RefreshRateIndexRole,
        RefreshRatesRole,
        ReplicationSourceModelRole,
        ReplicationSourceIndexRole,
        ReplicasModelRole
    };

    explicit OutputModel(ConfigHandler *configHandler);
    ~OutputModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;

    void add(const KScreen::OutputPtr &output);
    void remove(int outputId);

    /**
     * Resets the origin for calculation of positions to the most northwest display corner
     * while keeping the normalized positions untouched.
     *
     * @return true if some (unnormalized) output position changed on this call, otherwise false.
     */
    bool normalizePositions();
    bool positionsNormalized() const;

Q_SIGNALS:
    void positionChanged();
    void sizeChanged();
    void changed();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Output {
        Output() {}
        Output(const Output &output)
            : ptr(output.ptr)
            , pos(output.pos)
        {}
        Output(Output &&) noexcept = default;
        Output(KScreen::OutputPtr _ptr, const QPoint &_pos)
            : ptr(_ptr)
            , pos(_pos)
        {}
        Output &operator=(const Output &output) {
            ptr = output.ptr;
            pos = output.pos;
            posReset = QPoint(-1, -1);
            return *this;
        }
        Output &operator=(Output &&) noexcept = default;

        KScreen::OutputPtr ptr;
        QPoint pos;
        QPoint posReset = QPoint(-1, -1);
    };

    void roleChanged(int outputId, OutputRoles role);

    void resetPosition(const Output &output);
    void reposition();
    void updatePositions();
    void updateOrder();
    QPoint originDelta() const;

    /**
     * @brief Snaps moved output to others
     * @param output the moved output
     * @param dest the desired destination to be adjusted by snapping
     */
    void snap(const Output &output, QPoint &dest);

    bool setEnabled(int outputIndex, bool enable);

    bool setResolution(int outputIndex, int resIndex);
    bool setRefreshRate(int outputIndex, int refIndex);
    bool setRotation(int outputIndex, KScreen::Output::Rotation rotation);
    bool setAutoRotate(int outputIndex, bool value);
    bool setAutoRotateOnlyInTabletMode(int outputIndex, bool value);

    int resolutionIndex(const KScreen::OutputPtr &output) const;
    int refreshRateIndex(const KScreen::OutputPtr &output) const;
    QVariantList resolutionsStrings(const KScreen::OutputPtr &output) const;
    QVector<QSize> resolutions(const KScreen::OutputPtr &output) const;
    QVector<float> refreshRates(const KScreen::OutputPtr &output) const;

    bool positionable(const Output &output) const;

    QStringList replicationSourceModel(const KScreen::OutputPtr &output) const;
    bool setReplicationSourceIndex(int outputIndex, int sourceIndex);
    int replicationSourceIndex(int outputIndex) const;
    int replicationSourceId(const Output &output) const;

    QVariantList replicasModel(const KScreen::OutputPtr &output) const;

    QVector<Output> m_outputs;

    ConfigHandler *m_config;
};

#endif
