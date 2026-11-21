// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remoteurl.h"

#include <gtest/gtest.h>

TEST(RemoteUrlTest, ReadsServerAddresses)
{
    const RemoteUrl smb = RemoteUrl::parse("smb://127.0.0.1/test");
    EXPECT_EQ(smb.scheme, "smb");
    EXPECT_EQ(smb.host, "127.0.0.1");
    EXPECT_EQ(smb.port, -1);
    EXPECT_EQ(smb.path, "/test");
    EXPECT_TRUE(smb.user.empty());

    const RemoteUrl sftp = RemoteUrl::parse("SFTP://Some%20One@Example.ORG:2222/home/a%20b");
    EXPECT_EQ(sftp.scheme, "sftp");
    EXPECT_EQ(sftp.user, "Some One");
    EXPECT_EQ(sftp.host, "example.org");
    EXPECT_EQ(sftp.port, 2222);
    EXPECT_EQ(sftp.path, "/home/a b");

    const RemoteUrl v6 = RemoteUrl::parse("sftp://[::1]:22");
    EXPECT_EQ(v6.host, "::1");
    EXPECT_EQ(v6.port, 22);
    EXPECT_EQ(v6.hostForUrl(), "[::1]");
}

TEST(RemoteUrlTest, WritesEncodedAddresses)
{
    RemoteUrl url;
    url.scheme = "sftp";
    url.user = "me@home";
    url.host = "server";
    url.port = 2222;
    EXPECT_EQ(url.toString(), "sftp://me%40home@server:2222");

    url.port = -1;
    url.user.clear();
    url.path = "/a b";
    EXPECT_EQ(url.toString(), "sftp://server/a%20b");
    EXPECT_EQ(RemoteUrl::parse(url.toString()).path, "/a b");
}

TEST(RemoteUrlTest, RefusesWhatIsNoUrl)
{
    EXPECT_FALSE(RemoteUrl::parse("").isValid());
    EXPECT_FALSE(RemoteUrl::parse("server/share").isValid());
    EXPECT_FALSE(RemoteUrl::parse("smb://host:99999").isValid());
}
