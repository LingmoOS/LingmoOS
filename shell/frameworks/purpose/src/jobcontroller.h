/*
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef JOBCONTROLLER_H
#define JOBCONTROLLER_H

#include <QObject>

#include "alternativesmodel.h"
#include "job.h"
#include <purpose/purpose_export.h>

namespace Purpose
{
class PURPOSE_EXPORT JobController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AlternativesModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(Purpose::Configuration *configuration READ config NOTIFY configChanged)
    Q_PROPERTY(Purpose::Job *job READ job NOTIFY jobChanged)

public:
    enum State {
        Inactive = 0,
        Configuring,
        Running,
        Finished,
        Cancelled,
        Error,
    };
    Q_ENUM(State)

    AlternativesModel *model() const;
    void setModel(AlternativesModel *model);

    int index() const;
    void setIndex(int index);

    State state() const;

    Configuration *config() const;

    Job *job() const;

    Q_INVOKABLE void configure();
    Q_INVOKABLE void startJob();
    Q_INVOKABLE void cancel();

Q_SIGNALS:
    void modelChanged();
    void indexChanged();
    void stateChanged();
    void configChanged();
    void jobChanged();

private:
    AlternativesModel *m_model = nullptr;
    int m_index = -1;
    Purpose::Configuration *m_configuration = nullptr;
    State m_state = Inactive;
    Job *m_job = nullptr;
};

}
#endif
