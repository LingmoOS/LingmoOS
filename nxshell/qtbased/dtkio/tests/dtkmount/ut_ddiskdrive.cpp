// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkmount_global.h"
#include "ddiskdrive.h"
#include "udisks2_interface.h"

#include <gtest/gtest.h>
#include <stubext.h>

DMOUNT_USE_NAMESPACE

class TestDDiskDrive : public testing::Test
{
public:
    void SetUp() override { }
    void TearDown() override { m_stub.clear(); }

    stub_ext::StubExt m_stub;

    DDiskDrive m_drv { "test" };
};

TEST_F(TestDDiskDrive, path)
{
    m_stub.set_lamda(&QDBusAbstractInterface::path, [] { __DBG_STUB_INVOKE__ return "Test"; });
    EXPECT_EQ("Test", m_drv.path());
}

TEST_F(TestDDiskDrive, canPowerOff)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::canPowerOff, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.canPowerOff());
}

TEST_F(TestDDiskDrive, configuration)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::configuration, [] { __DBG_STUB_INVOKE__ return QVariantMap(); });
    EXPECT_NO_FATAL_FAILURE(m_drv.configuration());
    EXPECT_TRUE(m_drv.configuration().count() == 0);
}

TEST_F(TestDDiskDrive, connectionBus)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::connectionBus, [] { __DBG_STUB_INVOKE__ return "usb"; });
    EXPECT_EQ("usb", m_drv.connectionBus());
}

TEST_F(TestDDiskDrive, ejectable)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::ejectable, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.ejectable());
}

TEST_F(TestDDiskDrive, id)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::id, [] { __DBG_STUB_INVOKE__ return "testId"; });
    EXPECT_EQ("testId", m_drv.id());
}

TEST_F(TestDDiskDrive, media)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::media, [] { __DBG_STUB_INVOKE__ return "cd"; });
    EXPECT_EQ("cd", m_drv.media());
}

TEST_F(TestDDiskDrive, mediaAvailable)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::mediaAvailable, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.mediaAvailable());
}

TEST_F(TestDDiskDrive, mediaChangeDetected)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::mediaChangeDetected, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.mediaChangeDetected());
}

TEST_F(TestDDiskDrive, mediaCompatibility)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::mediaCompatibility, [] { __DBG_STUB_INVOKE__ return QStringList { "cd", "dvd" }; });
    EXPECT_EQ(2, m_drv.mediaCompatibility().count());
    EXPECT_EQ("cd", m_drv.mediaCompatibility().first());
}

TEST_F(TestDDiskDrive, mediaRemovable)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::mediaRemovable, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.mediaRemovable());
}

TEST_F(TestDDiskDrive, model)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::model, [] { __DBG_STUB_INVOKE__ return "model"; });
    EXPECT_EQ("model", m_drv.model());
}

TEST_F(TestDDiskDrive, optical)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::optical, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.optical());
}

TEST_F(TestDDiskDrive, opticalBlank)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::opticalBlank, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.opticalBlank());
}

TEST_F(TestDDiskDrive, opticalNumAudioTracks)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::opticalNumAudioTracks, [] { __DBG_STUB_INVOKE__ return 1; });
    EXPECT_EQ(1, m_drv.opticalNumAudioTracks());
}

TEST_F(TestDDiskDrive, opticalNumDataTracks)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::opticalNumDataTracks, [] { __DBG_STUB_INVOKE__ return 1; });
    EXPECT_EQ(1, m_drv.opticalNumDataTracks());
}

TEST_F(TestDDiskDrive, opticalNumSessions)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::opticalNumSessions, [] { __DBG_STUB_INVOKE__ return 1; });
    EXPECT_EQ(1, m_drv.opticalNumSessions());
}

TEST_F(TestDDiskDrive, opticalNumTracks)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::opticalNumTracks, [] { __DBG_STUB_INVOKE__ return 1; });
    EXPECT_EQ(1, m_drv.opticalNumTracks());
}

TEST_F(TestDDiskDrive, removable)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::removable, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(m_drv.removable());
}

TEST_F(TestDDiskDrive, revision)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::revision, [] { __DBG_STUB_INVOKE__ return "revision"; });
    EXPECT_EQ("revision", m_drv.revision());
}

TEST_F(TestDDiskDrive, rotationRate)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::rotationRate, [] { __DBG_STUB_INVOKE__ return 1; });
    EXPECT_EQ(1, m_drv.rotationRate());
}

TEST_F(TestDDiskDrive, seat)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::seat, [] { __DBG_STUB_INVOKE__ return "seat"; });
    EXPECT_EQ("seat", m_drv.seat());
}

TEST_F(TestDDiskDrive, serial)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::serial, [] { __DBG_STUB_INVOKE__ return "serial"; });
    EXPECT_EQ("serial", m_drv.serial());
}

TEST_F(TestDDiskDrive, siblingId)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::siblingId, [] { __DBG_STUB_INVOKE__ return "siblingId"; });
    EXPECT_EQ("siblingId", m_drv.siblingId());
}

TEST_F(TestDDiskDrive, size)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::size, [] { __DBG_STUB_INVOKE__ return 102400; });
    EXPECT_EQ(102400, m_drv.size());
}

TEST_F(TestDDiskDrive, sortKey)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::sortKey, [] { __DBG_STUB_INVOKE__ return "sortKey"; });
    EXPECT_EQ("sortKey", m_drv.sortKey());
}

TEST_F(TestDDiskDrive, timeDetected)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::timeDetected, [] { __DBG_STUB_INVOKE__ return 1990; });
    EXPECT_EQ(1990, m_drv.timeDetected());
}

TEST_F(TestDDiskDrive, timeMediaDetected)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::timeMediaDetected, [] { __DBG_STUB_INVOKE__ return 2022; });
    EXPECT_EQ(2022, m_drv.timeMediaDetected());
}

TEST_F(TestDDiskDrive, vendor)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::vendor, [] { __DBG_STUB_INVOKE__ return "deepin"; });
    EXPECT_EQ("deepin", m_drv.vendor());
}

TEST_F(TestDDiskDrive, wwn)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::wWN, [] { __DBG_STUB_INVOKE__ return "Deepin"; });
    EXPECT_EQ("Deepin", m_drv.WWN());
}

TEST_F(TestDDiskDrive, eject)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::Eject, [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<>(); });
    m_stub.set_lamda(&QDBusPendingCall::waitForFinished, [] { __DBG_STUB_INVOKE__ });
    m_stub.set_lamda(&QDBusPendingCall::error, [] { __DBG_STUB_INVOKE__ return QDBusError {}; });
    EXPECT_NO_FATAL_FAILURE(m_drv.eject({}));
}

TEST_F(TestDDiskDrive, pwoerOff)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::PowerOff, [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<>(); });
    m_stub.set_lamda(&QDBusPendingCall::waitForFinished, [] { __DBG_STUB_INVOKE__ });
    m_stub.set_lamda(&QDBusPendingCall::error, [] { __DBG_STUB_INVOKE__ return QDBusError {}; });
    EXPECT_NO_FATAL_FAILURE(m_drv.powerOff({}));
}

TEST_F(TestDDiskDrive, setConfiguration)
{
    m_stub.set_lamda(&OrgFreedesktopUDisks2DriveInterface::SetConfiguration, [] { __DBG_STUB_INVOKE__ return QDBusPendingReply<>(); });
    m_stub.set_lamda(&QDBusPendingCall::waitForFinished, [] { __DBG_STUB_INVOKE__ });
    m_stub.set_lamda(&QDBusPendingCall::error, [] { __DBG_STUB_INVOKE__ return QDBusError {}; });
    EXPECT_NO_FATAL_FAILURE(m_drv.setConfiguration({}, {}));
}
