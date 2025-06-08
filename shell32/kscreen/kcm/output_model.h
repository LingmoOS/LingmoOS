/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <kscreen/config.h>
#include <kscreen/output.h>

#include <QAbstractListModel>
#include <QPoint>
#include <optional>

class ConfigHandler;

class OutputModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum OutputRoles {
        EnabledRole = Qt::UserRole + 1,
        InternalRole,
        PriorityRole,
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
        ResolutionRole,
        RefreshRateIndexRole,
        RefreshRatesRole,
        ReplicationSourceModelRole,
        ReplicationSourceIndexRole,
        ReplicasModelRole,
        CapabilitiesRole,
        OverscanRole,
        VrrPolicyRole,
        RgbRangeRole,
        InteractiveMoveRole, // This output is currently repositioned interactively
    };

    explicit OutputModel(ConfigHandler *configHandler);
    ~OutputModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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

    bool isMoving() const;

Q_SIGNALS:
    void positionChanged();
    void sizeChanged();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Output {
        Output()
        {
        }
        Output(const Output &output)
            : ptr(output.ptr)
            , pos(output.pos)
        {
        }
        Output(Output &&) noexcept = default;
        Output(KScreen::OutputPtr _ptr, const QPoint &_pos)
            : ptr(_ptr)
            , pos(_pos)
        {
        }
        Output &operator=(const Output &output)
        {
            ptr = output.ptr;
            pos = output.pos;
            posReset = std::nullopt;
            return *this;
        }
        Output &operator=(Output &&) noexcept = default;

        KScreen::OutputPtr ptr;
        QPoint pos;
        // Indicates how to restore position when enabling or un-mirroring a screen.
        // Negative value of x means that the output was the left-most, and restoring it would require shifting all the other ones to the right.
        // Same for the y and shifting down.
        std::optional<QPoint> posReset = std::nullopt;
        bool moving = false;
    };

    void rolesChanged(int outputId, const QVector<int> &roles);
    QModelIndex indexForOutputId(int outputId) const;

    void resetPosition(Output &output);
    void reposition();
    void updatePositions();

    /**
     * @brief Snaps moved output to others
     * @param output the moved output
     * @param dest the desired destination to be adjusted by snapping
     */
    void snap(const Output &output, QPoint &dest);
    void maintainSnapping(const Output &changedOutput, const QSize &oldSize, const QSize &newSize);
    QPoint mostTopLeftLocationOfPositionableOutputOptionallyIgnoringOneOfThem(std::optional<KScreen::OutputPtr> ignored = std::nullopt) const;

    bool setEnabled(int outputIndex, bool enable);

    bool setResolution(int outputIndex, int resIndex);
    bool setRefreshRate(int outputIndex, int refIndex);
    bool setRotation(int outputIndex, KScreen::Output::Rotation rotation);
    bool setAutoRotate(int outputIndex, bool value);
    bool setAutoRotateOnlyInTabletMode(int outputIndex, bool value);

    int resolutionIndex(const KScreen::OutputPtr &output) const;
    int refreshRateIndex(const KScreen::OutputPtr &output) const;
    QSize resolution(const KScreen::OutputPtr &output) const;
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
