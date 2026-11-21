// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "clipboard/selection.h"

using namespace Lingmo::Clipboard;

namespace {

class Source : public DataSource
{
public:
    std::function<void()> onCancel;
    int requests = 0;

    void send(const std::string &, int fd) override
    {
        ++requests;
        close(fd);
    }

    void cancelled() override
    {
        if (onCancel)
            onCancel();
    }
};

TEST(Selection, OwnershipGenerationsAndReuse)
{
    SelectionController c;
    auto a = std::make_shared<Source>();
    a->offer("text/plain");
    c.setRecipient(&c);
    ASSERT_TRUE(c.setSource(a));
    DataOffer offer(&c);
    EXPECT_TRUE(offer.valid());
    c.setRecipient(nullptr);
    EXPECT_FALSE(offer.valid());
    EXPECT_EQ(a, c.source());
    c.setRecipient(&c);
    EXPECT_FALSE(offer.valid());
    EXPECT_FALSE(c.setSource(a));
    auto b = std::make_shared<Source>();
    ASSERT_TRUE(c.setSource(b));
    EXPECT_FALSE(a->alive());
    EXPECT_FALSE(c.setSource(a));
    c.sourceDestroyed(a.get());
    EXPECT_EQ(b, c.source());
    b->invalidate();
    EXPECT_FALSE(c.source());
}

TEST(Selection, ReplacementCancellationMayReplaceAgain)
{
    SelectionController c;
    auto a = std::make_shared<Source>();
    auto b = std::make_shared<Source>();
    auto d = std::make_shared<Source>();
    ASSERT_TRUE(c.setSource(a));
    a->onCancel = [&] {
        EXPECT_EQ(b, c.source());
        EXPECT_TRUE(c.setSource(d));
    };
    EXPECT_FALSE(c.setSource(b));
    EXPECT_EQ(d, c.source());
    EXPECT_FALSE(b->alive());
}

TEST(Selection, NotificationMayDestroyController)
{
    auto c = std::make_unique<SelectionController>();
    auto a = std::make_shared<Source>();
    const auto changed = c->selectionChanged.connect([&] {
        c.reset();
    });
    EXPECT_FALSE(c->setSource(a));
    EXPECT_FALSE(c);
}

TEST(Selection, SerialOrderingFocusEpochAndOpaqueMime)
{
    SelectionController c;
    int other;
    c.setRecipient(&c);
    c.recordSerial(&other, 1);
    EXPECT_FALSE(c.authorize(&c, 1));
    c.recordSerial(&c, UINT32_MAX);
    c.recordSerial(&c, 0);
    EXPECT_TRUE(c.authorize(&c, 0));
    EXPECT_FALSE(c.authorize(&c, UINT32_MAX));
    EXPECT_TRUE(c.authorize(&c, 0)); // Same triggering event may clear/replace.
    c.setRecipient(&other);
    c.setRecipient(&c);
    EXPECT_FALSE(c.authorize(&c, 0));
    Source s;
    EXPECT_TRUE(s.offer("Text/Plain"));
    EXPECT_FALSE(s.supports("text/plain"));
    EXPECT_TRUE(s.offer("Text/Plain"));
    EXPECT_EQ(1u, s.formats().size());
    EXPECT_FALSE(s.offer(std::string(1025, 'x')));
    s.markUsed();
    EXPECT_TRUE(s.offer("text/html"));
}

} // namespace

// --- Session lock: the secure screen's own clipboard --------------------------

TEST(Selection, IsolationHidesTheSessionSelectionAndRestoresIt)
{
    SelectionController c;
    auto session = std::make_shared<Source>();
    session->offer("text/plain");
    c.setRecipient(&c);
    ASSERT_TRUE(c.setSource(session));
    DataOffer sessionOffer(&c);
    ASSERT_TRUE(sessionOffer.valid());

    c.setIsolated(true);

    // Nothing for the secure client to be offered, and the offer the session
    // already held cannot be read from either.
    EXPECT_TRUE(c.isIsolated());
    EXPECT_EQ(nullptr, c.source());
    EXPECT_EQ(nullptr, c.recipient());
    EXPECT_FALSE(sessionOffer.valid());
    // The source itself is untouched: it is put aside, not cancelled.
    EXPECT_TRUE(session->alive());

    c.setIsolated(false);

    EXPECT_FALSE(c.isIsolated());
    EXPECT_EQ(session, c.source());
    EXPECT_EQ(&c, c.recipient());
    // Offers taken before the lock stay dead; the session takes a fresh one.
    EXPECT_FALSE(sessionOffer.valid());
    DataOffer afterUnlock(&c);
    EXPECT_TRUE(afterUnlock.valid());
    EXPECT_EQ(std::vector<std::string> {"text/plain"}, afterUnlock.formats());
}

TEST(Selection, TheSecureScreenCannotSetOrReadASelection)
{
    SelectionController c;
    auto session = std::make_shared<Source>();
    session->offer("text/plain");
    c.setRecipient(&c);
    ASSERT_TRUE(c.setSource(session));
    c.recordSerial(&c, 7);

    c.setIsolated(true);

    // The auth client typing a password must not be able to leave it on the
    // session's clipboard, for the session or for a clipboard manager.
    auto secret = std::make_shared<Source>();
    secret->offer("text/plain");
    EXPECT_FALSE(c.setSource(secret));
    EXPECT_EQ(nullptr, c.source());
    // Nor can a stale serial from before the lock authorize anything.
    EXPECT_FALSE(c.authorize(&c, 7));

    c.setIsolated(false);

    // The session's own selection is what comes back, not the secure one.
    EXPECT_EQ(session, c.source());
}

TEST(Selection, IsolationSurvivesTheSessionSourceDyingWhileBlocked)
{
    SelectionController c;
    auto session = std::make_shared<Source>();
    session->offer("text/plain");
    ASSERT_TRUE(c.setSource(session));

    c.setIsolated(true);
    session->invalidate();
    c.setIsolated(false);

    // Nothing to restore, and nothing dangling either.
    EXPECT_EQ(nullptr, c.source());
}
