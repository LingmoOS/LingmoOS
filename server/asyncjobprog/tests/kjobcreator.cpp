/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kjobcreator.h"

#include <QApplication>
#include <QIcon>
#include <QPushButton>

#include <KUiServerJobTracker>
#include <KUiServerV2JobTracker>
#include <KWidgetJobTracker>

#include <limits>

TestJob::TestJob(QObject *parent)
    : KJob(parent)
{
}

void TestJob::start()
{
    m_started = true;
}

bool TestJob::started() const
{
    return m_started;
}

void TestJob::finish()
{
    emitResult();
}

void TestJob::setPercentage(int percent)
{
    setPercent(percent);
}

void TestJob::setProcessedMiB(double bytes)
{
    setProcessedAmount(KJob::Bytes, bytes * 1024 * 1024);
}

void TestJob::setTotalMiB(double bytes)
{
    setTotalAmount(KJob::Bytes, bytes * 1024 * 1024);
}

void TestJob::setProcessedFiles(int files)
{
    setProcessedAmount(KJob::Files, files);
}

void TestJob::setTotalFiles(int files)
{
    setTotalAmount(KJob::Files, files);
}

void TestJob::setProcessedDirectories(int directories)
{
    setProcessedAmount(KJob::Directories, directories);
}

void TestJob::setTotalDirectories(int directories)
{
    setTotalAmount(KJob::Directories, directories);
}

void TestJob::setProcessedItems(int items)
{
    setProcessedAmount(KJob::Items, items);
}

void TestJob::setTotalItems(int items)
{
    setTotalAmount(KJob::Items, items);
}

void TestJob::setSpeedMiB(double speed)
{
    emitSpeed(speed * 1024 * 1024);
}

void TestJob::setErrorState(int errorCode, const QString &errorTest)
{
    setError(errorCode);
    setErrorText(errorTest);
}

void TestJob::setSuspendable(bool suspendable)
{
    auto caps = capabilities();
    caps.setFlag(KJob::Suspendable, suspendable);
    setCapabilities(caps);
}

void TestJob::setKillable(bool killable)
{
    auto caps = capabilities();
    caps.setFlag(KJob::Killable, killable);
    setCapabilities(caps);
}

bool TestJob::doKill()
{
    // TODO add checkbox for testing what happens when you try to kill a job that refuses?
    return true;
}

bool TestJob::doSuspend()
{
    return true;
}

bool TestJob::doResume()
{
    return true;
}

TestDialog::TestDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.desktopEntry->setText(QGuiApplication::desktopFileName());

    m_ui.processedBytes->setMaximum(std::numeric_limits<double>::max());
    m_ui.totalBytes->setMaximum(m_ui.processedBytes->maximum());
    m_ui.processedFiles->setMaximum(std::numeric_limits<int>::max());
    m_ui.totalFiles->setMaximum(m_ui.processedFiles->maximum());
    m_ui.processedDirectories->setMaximum(std::numeric_limits<int>::max());
    m_ui.totalDirectories->setMaximum(m_ui.processedDirectories->maximum());
    m_ui.processedItems->setMaximum(std::numeric_limits<int>::max());
    m_ui.totalItems->setMaximum(m_ui.processedItems->maximum());

    m_ui.speed->setMaximum(std::numeric_limits<double>::max());

    m_ui.createButton->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    m_ui.startButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    m_ui.suspendButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    m_ui.killButton->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));
    m_ui.finishButton->setIcon(QIcon::fromTheme(QStringLiteral("view-task")));

    updateUiState();

    connect(m_ui.desktopEntry, &QLineEdit::editingFinished, this, [this] {
        QGuiApplication::setDesktopFileName(m_ui.desktopEntry->text());
        updateUiState();
    });

    connect(m_ui.destUrl, &QLineEdit::editingFinished, this, &TestDialog::updateJob);

    connect(m_ui.emitDescriptionButton, &QPushButton::clicked, this, [this] {
        Q_EMIT m_job->description(m_job,
                                  m_ui.title->text(),
                                  qMakePair(m_ui.descriptionLabel1->text(), m_ui.descriptionValue1->text()),
                                  qMakePair(m_ui.descriptionLabel2->text(), m_ui.descriptionValue2->text()));
    });

    connect(m_ui.percent, &QSlider::valueChanged, this, &TestDialog::updateJob);
    connect(m_ui.processedBytes, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.totalBytes, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.processedFiles, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.totalFiles, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.processedDirectories, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.totalDirectories, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.processedItems, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.totalItems, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::updateJob);
    connect(m_ui.speed, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &TestDialog::updateJob);

    connect(m_ui.infoMessage, &QLineEdit::returnPressed, m_ui.emitInfoMessage, &QPushButton::click);
    connect(m_ui.emitInfoMessage, &QPushButton::clicked, this, [this] {
        Q_EMIT m_job->infoMessage(m_job, m_ui.infoMessage->text(), QString());
    });

    connect(m_ui.errorCombo, qOverload<int>(&QComboBox::activated), this, &TestDialog::updateJob);
    connect(m_ui.errorText, &QLineEdit::editingFinished, this, &TestDialog::updateJob);

    connect(m_ui.suspendableCheck, &QCheckBox::toggled, this, &TestDialog::updateJob);
    connect(m_ui.killableCheck, &QCheckBox::toggled, this, &TestDialog::updateJob);

    connect(m_ui.createButton, &QPushButton::clicked, this, [this] {
        Q_ASSERT(!m_job);
        m_job = new TestJob(this);
        connect(m_job, &KJob::finished, this, [this] {
            m_job = nullptr;
            m_registeredWithWidgetTracker = false;
            m_registeredWithUiServerTracker = false;
            m_registeredWithUiServerV2Tracker = false;
            updateUiState();
        });
        connect(m_job, &KJob::suspended, this, &TestDialog::updateUiState);
        connect(m_job, &KJob::resumed, this, &TestDialog::updateUiState);

        // KJob auto-calculates percent based on processed/total amount, so sync that back into the UI
        connect(m_job, &KJob::percentChanged, this, [this](KJob *job, unsigned long percent) {
            Q_UNUSED(job);
            m_ui.percent->setValue(percent);
        });

        updateJob();
        updateUiState();
    });
    connect(m_ui.startButton, &QPushButton::clicked, this, [this] {
        m_job->start();
        updateUiState();
    });
    connect(m_ui.suspendButton, &QPushButton::clicked, this, [this] {
        if (m_job->isSuspended()) {
            m_job->resume();
        } else {
            m_job->suspend();
        }
    });
    connect(m_ui.killButton, &QPushButton::clicked, this, [this] {
        m_job->kill();
    });
    connect(m_ui.finishButton, &QPushButton::clicked, this, [this] {
        m_job->finish();
    });

    connect(m_ui.registerKWidgetJobTracker, &QPushButton::clicked, this, [this] {
        if (!m_widgetTracker) {
            // not passing "parent" so it spawns a new window
            m_widgetTracker.reset(new KWidgetJobTracker(nullptr));
        }
        m_widgetTracker->registerJob(m_job.data());
        m_registeredWithWidgetTracker = true;
        updateUiState();
    });
    connect(m_ui.registerKUIServerJobTracker, &QPushButton::clicked, this, [this] {
        if (!m_uiServerTracker) {
            m_uiServerTracker.reset(new KUiServerJobTracker(nullptr));
        }
        m_uiServerTracker->registerJob(m_job.data());
        m_registeredWithUiServerTracker = true;
        updateUiState();
    });
    connect(m_ui.registerKUIServerV2JobTracker, &QPushButton::clicked, this, [this] {
        if (!m_uiServerV2Tracker) {
            m_uiServerV2Tracker.reset(new KUiServerV2JobTracker(nullptr));
        }
        m_uiServerV2Tracker->registerJob(m_job.data());
        m_registeredWithUiServerV2Tracker = true;
        updateUiState();
    });
}

TestDialog::~TestDialog() = default;

void TestDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    if (m_job) {
        m_job->kill(KJob::EmitResult);
    }
}

void TestDialog::updateUiState()
{
    m_ui.destUrl->setEnabled(m_job);

    m_ui.emitDescriptionButton->setEnabled(m_job);
    m_ui.emitInfoMessage->setEnabled(m_job);

    m_ui.processedBytes->setEnabled(m_job);
    m_ui.totalBytes->setEnabled(m_job);
    m_ui.processedFiles->setEnabled(m_job);
    m_ui.totalFiles->setEnabled(m_job);
    m_ui.processedDirectories->setEnabled(m_job);
    m_ui.totalDirectories->setEnabled(m_job);
    m_ui.processedItems->setEnabled(m_job);
    m_ui.totalItems->setEnabled(m_job);

    m_ui.percent->setEnabled(m_job);
    m_ui.speed->setEnabled(m_job);
    m_ui.errorCombo->setEnabled(m_job);
    m_ui.errorText->setEnabled(m_job);

    m_ui.createButton->setEnabled(!m_job);
    m_ui.startButton->setEnabled(m_job && !m_job->started());
    m_ui.suspendButton->setEnabled(m_job);
    m_ui.suspendButton->setChecked(m_job && m_job->isSuspended());
    m_ui.killButton->setEnabled(m_job);
    m_ui.finishButton->setEnabled(m_job);

    m_ui.registerKWidgetJobTracker->setEnabled(m_job && !m_registeredWithWidgetTracker);
    m_ui.registerKUIServerJobTracker->setEnabled(m_job && !m_registeredWithUiServerTracker);
    m_ui.registerKUIServerV2JobTracker->setEnabled(m_job && !m_registeredWithUiServerV2Tracker && !QGuiApplication::desktopFileName().isEmpty());
}

void TestDialog::updateJob()
{
    if (!m_job) {
        return;
    }

    m_job->setProperty("destUrl", m_ui.destUrl->text());
    m_job->setProperty("immediateProgressReporting", m_ui.immediateCheck->isChecked());
    m_job->setFinishedNotificationHidden(m_ui.transientCheck->isChecked());

    m_job->setPercentage(m_ui.percent->value());

    m_job->setProcessedMiB(m_ui.processedBytes->value());
    m_job->setTotalMiB(m_ui.totalBytes->value());
    m_job->setProcessedFiles(m_ui.processedFiles->value());
    m_job->setTotalFiles(m_ui.totalFiles->value());
    m_job->setProcessedDirectories(m_ui.processedDirectories->value());
    m_job->setTotalDirectories(m_ui.totalDirectories->value());
    m_job->setProcessedItems(m_ui.processedItems->value());
    m_job->setTotalItems(m_ui.totalItems->value());

    m_job->setSpeedMiB(m_ui.speed->value());

    m_job->setErrorState(m_ui.errorCombo->currentIndex(), m_ui.errorText->text());

    m_job->setSuspendable(m_ui.suspendableCheck->isChecked());
    m_job->setKillable(m_ui.killableCheck->isChecked());
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kjobcreator"));
    app.setDesktopFileName(QStringLiteral("org.kde.dolphin"));

    TestDialog dialog;
    dialog.show();

    return app.exec();
}
