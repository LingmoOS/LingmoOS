/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PageChoosePPD.h"
#include "ui_PageChoosePPD.h"

#include <kde-add-printer_log.h>

#include <KCupsRequest.h>
#include <KIO/FileCopyJob>
#include <KLocalizedString>
#include <SelectMakeModel.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>

PageChoosePPD::PageChoosePPD(const QVariantMap &args, QWidget *parent)
    : GenericPage(parent)
    , ui(new Ui::PageChoosePPD)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    m_layout = new QStackedLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    ui->gridLayout->addLayout(m_layout, 1, 3);
    m_selectMM = new SelectMakeModel(this);
    connect(m_selectMM, &SelectMakeModel::changed, this, &PageChoosePPD::checkSelected);
    m_layout->addWidget(m_selectMM);

    // Setup the busy cursor
    connect(m_selectMM, &SelectMakeModel::changed, this, &PageChoosePPD::notWorking);

    if (!args.isEmpty()) {
        // set our args
        setValues(args);
    }
}

PageChoosePPD::~PageChoosePPD()
{
    removeTempPPD();

    delete ui;
}

void PageChoosePPD::setValues(const QVariantMap &args)
{
    m_args = args;

    if (args[ADDING_PRINTER].toBool()) {
        qCDebug(PM_ADD_PRINTER) << args;
        working();
        removeTempPPD();
        const QString deviceId = args[KCUPS_DEVICE_ID].toString();
        QString make;
        QString makeAndModel = args[KCUPS_DEVICE_MAKE_AND_MODEL].toString();
        const QString deviceURI = args[KCUPS_DEVICE_URI].toString();

        // If
        QUrl url(deviceURI + QLatin1String(".ppd"));
        if (url.scheme() == QLatin1String("ipp")) {
            auto tempFile = new QTemporaryFile;
            tempFile->setFileTemplate(QLatin1String("print-manager-XXXXXX.ppd"));
            tempFile->open();
            url.setScheme(QLatin1String("http"));
            if (url.port() < 0) {
                url.setPort(631);
            }
            qCDebug(PM_ADD_PRINTER) << deviceURI << url;
            KJob *job = KIO::file_copy(url, QUrl::fromLocalFile(tempFile->fileName()), -1, KIO::Overwrite | KIO::HideProgressInfo);
            job->setProperty("URI", deviceURI);
            connect(job, &KJob::result, this, &PageChoosePPD::resultJob);
        }

        // Get the make from the device id
        for (const QString &pair : deviceId.split(QLatin1Char(';'))) {
            if (pair.startsWith(QLatin1String("MFG:"))) {
                make = pair.section(QLatin1Char(':'), 1);
                break;
            }
        }

        if (makeAndModel.isEmpty()) {
            // Get the model  from the device id
            for (const QString &pair : deviceId.split(QLatin1Char(';'))) {
                if (pair.startsWith(QLatin1String("MDL:"))) {
                    // Build the make and model string
                    if (make.isNull()) {
                        makeAndModel = pair.section(QLatin1Char(':'), 1);
                    } else {
                        makeAndModel = make + QLatin1Char(' ') + pair.section(QLatin1Char(':'), 1);
                    }
                    break;
                }
            }
        }

        // if the device info is empty use the make and model
        // so we can have a nice name for the new printer on the next page
        if (!args.contains(KCUPS_DEVICE_INFO) && !makeAndModel.isEmpty()) {
            m_args[KCUPS_DEVICE_INFO] = makeAndModel;
        }

        m_selectMM->setDeviceInfo(deviceId, make, makeAndModel, deviceURI);
        m_isValid = true;
    } else {
        m_isValid = false;
    }
}

bool PageChoosePPD::isValid() const
{
    return m_isValid;
}

QVariantMap PageChoosePPD::values() const
{
    if (!isValid()) {
        return m_args;
    }

    QVariantMap ret = m_args;
    if (canProceed()) {
        if (!m_ppdFile.isNull()) {
            ret[FILENAME] = m_ppdFile;
        } else if (m_selectMM->isFileSelected()) {
            ret[FILENAME] = m_selectMM->selectedPPDFileName();
        } else {
            ret[PPD_NAME] = m_selectMM->selectedPPDName();
        }
    }
    return ret;
}

bool PageChoosePPD::canProceed() const
{
    // It can proceed if a PPD file (local or not) is provided    bool changed = false;
    bool allow = false;

    if (m_selectMM->isFileSelected()) {
        allow = !m_selectMM->selectedPPDFileName().isNull();
    } else if (!m_ppdFile.isNull()) {
        allow = true;
    } else {
        allow = !m_selectMM->selectedPPDName().isNull();
    }

    qCDebug(PM_ADD_PRINTER) << allow;
    return allow;
}

void PageChoosePPD::checkSelected()
{
    Q_EMIT allowProceed(canProceed());
}

void PageChoosePPD::selectDefault()
{
}

void PageChoosePPD::resultJob(KJob *job)
{
    if (!job->error() && job->property("URI").toString() == m_args[KCUPS_DEVICE_URI].toString()) {
        auto fileCopyJob = qobject_cast<KIO::FileCopyJob *>(job);

        // Make sure this job is for the current device
        m_ppdFile = fileCopyJob->destUrl().toLocalFile();
        m_isValid = false;
        Q_EMIT proceed();
    }
}

void PageChoosePPD::removeTempPPD()
{
    if (!m_ppdFile.isEmpty()) {
        QFile::remove(m_ppdFile);
        m_ppdFile.clear();
    }
}

#include "moc_PageChoosePPD.cpp"
