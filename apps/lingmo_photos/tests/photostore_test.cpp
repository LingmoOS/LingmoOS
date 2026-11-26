#include "photostore.h"
#include "editengine.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtest/gtest.h>
#include <filesystem>
TEST(PhotoStoreTest, PersistsAndFiltersPhotos) { const auto path = std::filesystem::temp_directory_path() / "lingmo-photos-test.db"; Photos::PhotoStore store(path.string()); Photos::Photo photo; photo.path = "/tmp/photo.jpg"; photo.name = "photo.jpg"; photo.folder = "/tmp"; photo.captured = "2026-11-21"; photo.format = ".jpg"; store.upsert(photo); EXPECT_EQ(store.count(), 1u); EXPECT_EQ(store.search("photo").size(), 1u); const auto stored = store.photoByPath(photo.path); ASSERT_TRUE(stored); EXPECT_EQ(stored->name, photo.name); store.setFavorite(1, true); EXPECT_EQ(store.photos(true).size(), 1u); }

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
    store.addToAlbum(id, 1);
    ASSERT_EQ(store.albumPhotos(id).size(), 1u);
    store.removeFromAlbum(id, 1);
    EXPECT_TRUE(store.albumPhotos(id).empty());
    store.addToAlbum(id, 1);
    store.renameAlbum(id, "Edited Trips");
    ASSERT_EQ(store.albums().front().name, "Edited Trips");
    store.deleteAlbum(id);
    EXPECT_TRUE(store.albums().empty());
    const auto replacement = store.createAlbum("Replacement");
    ASSERT_GT(replacement, 0);
    EXPECT_TRUE(store.albumPhotos(replacement).empty());
}

TEST(PhotoStoreTest, SavesEditedCopy) {
    const auto source = std::filesystem::temp_directory_path() / "lingmo-photos-edit-source.png";
    const auto target = std::filesystem::temp_directory_path() / "lingmo-photos-edit-copy.png";
    GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, 4, 2);
    ASSERT_NE(pixbuf, nullptr);
    gdk_pixbuf_fill(pixbuf, 0x204060ff);
    GError* saveError = nullptr;
    ASSERT_TRUE(gdk_pixbuf_save(pixbuf, source.c_str(), "png", &saveError, nullptr));
    if (saveError)
        g_error_free(saveError);
    g_object_unref(pixbuf);
    Photos::EditParams params;
    params.brightness = .25;
    params.rotation = 1;
    std::string error;
    ASSERT_TRUE(Photos::saveEditedCopy(source.string(), target.string(), params, &error)) << error;
    GError* loadError = nullptr;
    GdkPixbuf* copy = gdk_pixbuf_new_from_file(target.c_str(), &loadError);
    ASSERT_NE(copy, nullptr);
    EXPECT_EQ(gdk_pixbuf_get_width(copy), 2);
    EXPECT_EQ(gdk_pixbuf_get_height(copy), 4);
    if (loadError)
        g_error_free(loadError);
    g_object_unref(copy);
}

TEST(PhotoStoreTest, FlipsEditedCopy) {
    const auto source = std::filesystem::temp_directory_path() / "lingmo-photos-flip-source.png";
    const auto target = std::filesystem::temp_directory_path() / "lingmo-photos-flip-copy.png";
    GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, 2, 1);
    ASSERT_NE(pixbuf, nullptr);
    guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
    pixels[0] = 255; pixels[1] = 0; pixels[2] = 0;
    pixels[3] = 0; pixels[4] = 0; pixels[5] = 255;
    GError* saveError = nullptr;
    ASSERT_TRUE(gdk_pixbuf_save(pixbuf, source.c_str(), "png", &saveError, nullptr));
    if (saveError)
        g_error_free(saveError);
    g_object_unref(pixbuf);

    Photos::EditParams params;
    params.flipped = true;
    std::string error;
    ASSERT_TRUE(Photos::saveEditedCopy(source.string(), target.string(), params, &error)) << error;
    GError* loadError = nullptr;
    GdkPixbuf* copy = gdk_pixbuf_new_from_file(target.c_str(), &loadError);
    ASSERT_NE(copy, nullptr);
    const guchar* flipped = gdk_pixbuf_read_pixels(copy);
    EXPECT_EQ(flipped[0], 0);
    EXPECT_EQ(flipped[2], 255);
    EXPECT_EQ(flipped[3], 255);
    EXPECT_EQ(flipped[5], 0);
    if (loadError)
        g_error_free(loadError);
    g_object_unref(copy);
}
