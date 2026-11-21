#include "photostore.h"
#include <gtest/gtest.h>
#include <filesystem>
TEST(PhotoStoreTest, PersistsAndFiltersPhotos) { const auto path = std::filesystem::temp_directory_path() / "lingmo-photos-test.db"; Photos::PhotoStore store(path.string()); Photos::Photo photo; photo.path = "/tmp/photo.jpg"; photo.name = "photo.jpg"; photo.folder = "/tmp"; photo.captured = "2026-11-21"; photo.format = ".jpg"; store.upsert(photo); EXPECT_EQ(store.count(), 1u); EXPECT_EQ(store.search("photo").size(), 1u); store.setFavorite(1, true); EXPECT_EQ(store.photos(true).size(), 1u); }

TEST(PhotoStoreTest, ManagesAlbumsAndMembership) {
    const auto path = std::filesystem::temp_directory_path() / "lingmo-photos-albums-test.db";
    std::filesystem::remove(path);
    Photos::PhotoStore store(path.string());
    Photos::Photo photo;
    photo.path = "/tmp/album-photo.jpg";
    photo.name = "album-photo.jpg";
    photo.folder = "/tmp";
    photo.captured = "2026-11-21";
    photo.format = ".jpg";
    store.upsert(photo);
    const auto id = store.createAlbum("Trips");
    ASSERT_GT(id, 0);
    store.addToAlbum(id, 1);
    ASSERT_EQ(store.albumPhotos(id).size(), 1u);
    store.renameAlbum(id, "Edited Trips");
    ASSERT_EQ(store.albums().front().name, "Edited Trips");
    store.deleteAlbum(id);
    EXPECT_TRUE(store.albums().empty());
}
