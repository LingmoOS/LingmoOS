// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivecore.h"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
namespace fs = std::filesystem;
using namespace ArchiveManager;
void require(bool value, const std::string& message) { if (!value) { std::cerr << message << '\n'; std::exit(1); } }
std::string read(const fs::path& path) { std::ifstream file(path, std::ios::binary); return {std::istreambuf_iterator<char>(file), {}}; }
void text(const fs::path& path, const std::string& content) { std::ofstream file(path, std::ios::binary); file << content; }
struct Temp { fs::path path; ~Temp() { std::error_code error; fs::remove_all(path, error); } };
void fixture(const fs::path& path, const std::string& name, bool symlink = false, bool duplicate = false, bool hardlink = false) {
    auto* writer = archive_write_new(); archive_write_set_format_pax_restricted(writer);
    require(archive_write_open_filename(writer, path.c_str()) == ARCHIVE_OK, "fixture open");
    auto* entry = archive_entry_new(); archive_entry_set_pathname(entry, name.c_str());
    archive_entry_set_filetype(entry, symlink ? AE_IFLNK : AE_IFREG);
    archive_entry_set_perm(entry, 0644); archive_entry_set_size(entry, symlink ? 0 : 4);
    if (symlink) archive_entry_set_symlink(entry, "../../outside");
    if (hardlink) archive_entry_set_hardlink(entry, "../../outside");
    require(archive_write_header(writer, entry) == ARCHIVE_OK, "fixture header");
    if (!symlink) archive_write_data(writer, "evil", 4);
    if (duplicate) {
        require(archive_write_header(writer, entry) == ARCHIVE_OK, "duplicate fixture header");
        archive_write_data(writer, "evil", 4);
    }
    archive_entry_free(entry); archive_write_close(writer); archive_write_free(writer);
}
int main() {
    std::string pattern = "/tmp/lingmo-archive-test-XXXXXX"; require(::mkdtemp(pattern.data()), "temp directory"); Temp temp{pattern};
    const auto root = temp.path; fs::create_directories(root / "source/folder"); fs::create_directory(root / "output");
    text(root / "source/hello.txt", "Hello archive!\n"); text(root / "source/folder/中文.txt", "Unicode payload");
    text(root / "source/empty", ""); fs::create_directory(root / "source/emptydir");
    for (Format format : {Format::Zip, Format::Tar, Format::TarGzip, Format::TarXz}) {
        const auto archive = root / ("test" + formatExtension(format));
        auto made = create({(root / "source").string()}, archive.string(), format); require(made.ok(), "create: " + made.detail);
        auto list = inspect(archive.string()); require(list.ok() && list.entries.size() == 6, "list entries: " + list.detail);
        auto checked = verify(archive.string()); require(checked.ok() && checked.totalBytes == 30, "verify payload: " + checked.detail);
        auto rows = children(list.entries, "source"); require(rows.size() == 4 && rows[0].directory, "directory browsing");
        require(children(list.entries, "source", "HELLO").size() == 1, "search");
        auto out = extract(archive.string(), (root / "output").string()); require(out.ok(), "extract: " + out.detail);
        require(read(fs::path(out.output) / "source/hello.txt") == "Hello archive!\n", "roundtrip file");
        require(read(fs::path(out.output) / "source/folder/中文.txt") == "Unicode payload", "unicode roundtrip");
        auto again = extract(archive.string(), (root / "output").string()); require(again.ok() && again.output != out.output, "no overwrite extraction");
        auto selected = extract(archive.string(), (root / "output").string(), {"source/folder"});
        require(selected.ok() && fs::exists(fs::path(selected.output) / "source/folder/中文.txt")
            && !fs::exists(fs::path(selected.output) / "source/hello.txt"), "selected subtree");
        const auto before = read(archive); require(create({(root / "source").string()}, archive.string(), format).error == Error::Exists, "no replace archive");
        require(read(archive) == before, "existing archive unchanged");
    }
    for (const auto* name : {"../escape", "/absolute", "a/../../escape", "C:/file", "a\\..\\escape"}) {
        fixture(root / "bad.tar", name); auto list = inspect((root / "bad.tar").string());
        require(list.ok() && list.entries.front().unsafe, "unsafe listing marked");
        require(!extract((root / "bad.tar").string(), (root / "output").string()).ok(), "unsafe extraction refused");
    }
    fixture(root / "link.tar", "link", true);
    require(!extract((root / "link.tar").string(), (root / "output").string()).ok(), "symlink refused");
    fixture(root / "duplicate.tar", "same.txt", false, true);
    require(!extract((root / "duplicate.tar").string(), (root / "output").string()).ok(), "duplicate archive file refused");
    fixture(root / "hardlink.tar", "hardlink", false, false, true);
    require(!extract((root / "hardlink.tar").string(), (root / "output").string()).ok(), "hardlink refused");
    {
        auto* writer = archive_write_new(); archive_write_set_format_zip(writer);
        const bool encryption = archive_write_set_format_option(writer, "zip", "encryption", "aes256") == ARCHIVE_OK;
        if (encryption) {
            require(archive_write_set_passphrase(writer, "archive-test-password") == ARCHIVE_OK, "password fixture");
            require(archive_write_open_filename(writer, (root / "encrypted.zip").c_str()) == ARCHIVE_OK, "encrypted open");
            auto* header = archive_entry_new(); archive_entry_set_pathname(header, "secret.txt");
            archive_entry_set_filetype(header, AE_IFREG); archive_entry_set_perm(header, 0600); archive_entry_set_size(header, 6);
            require(archive_write_header(writer, header) == ARCHIVE_OK && archive_write_data(writer, "secret", 6) == 6, "encrypted write");
            archive_entry_free(header); require(archive_write_close(writer) == ARCHIVE_OK, "encrypted close");
        }
        archive_write_free(writer);
        if (encryption) {
            require(!verify((root / "encrypted.zip").string()).ok(), "encrypted needs password");
            Options password; password.password = "wrong";
            require(!extract((root / "encrypted.zip").string(), (root / "output").string(), {}, password).ok(), "wrong password rejected");
            password.password = "archive-test-password";
            require(verify((root / "encrypted.zip").string(), password).ok(), "correct password verifies");
            auto decrypted = extract((root / "encrypted.zip").string(), (root / "output").string(), {}, password);
            require(decrypted.ok() && read(fs::path(decrypted.output) / "secret.txt") == "secret", "correct password extracts");
            std::cout << "Encrypted ZIP checks passed\n";
        } else std::cout << "Encrypted ZIP fixture unavailable in this libarchive\n";
    }
    text(root / "plain.txt", "not an archive"); require(!inspect((root / "plain.txt").string()).ok(), "plain text rejected");
    Options limited; limited.maxBytes = 3;
    require(inspect((root / "test.zip").string(), limited).error == Error::Limit, "declared bytes bounded");
    require(extract((root / "test.zip").string(), (root / "output").string(), {}, limited).error == Error::Limit, "extract bytes bounded");
    limited.maxBytes = 1000; limited.maxEntries = 2;
    require(inspect((root / "test.zip").string(), limited).error == Error::Limit, "entry count bounded");
    Options canceled; canceled.canceled = std::make_shared<std::atomic_bool>(true);
    require(create({(root / "source").string()}, (root / "cancel.zip").string(), Format::Zip, canceled).error == Error::Canceled, "cancel create");
    require(!fs::exists(root / "cancel.zip"), "canceled output absent");
    require(extract((root / "test.zip").string(), (root / "output").string(), {}, canceled).error == Error::Canceled, "cancel extraction");
    require(!create({(root / "source").string()}, (root / "source/self.zip").string(), Format::Zip).ok(), "exclude destination inside source");
    fs::create_symlink(root / "plain.txt", root / "source/link");
    require(!create({(root / "source").string()}, (root / "links.zip").string(), Format::Zip).ok(), "source symlink rejected");
    // Cancel after real bytes were written; staging must be removed on both paths.
    text(root / "large.bin", std::string(2 * 1024 * 1024, 'x'));
    Options midCancel; midCancel.canceled = std::make_shared<std::atomic_bool>(false);
    midCancel.progress = [flag = midCancel.canceled](const Progress&) { flag->store(true); };
    require(create({(root / "large.bin").string()}, (root / "mid-cancel.zip").string(), Format::Zip, midCancel).error == Error::Canceled, "cancel active compression");
    require(!fs::exists(root / "mid-cancel.zip"), "partial compression not published");
    require(create({(root / "large.bin").string()}, (root / "large.zip").string(), Format::Zip).ok(), "large fixture");
    midCancel.canceled->store(false);
    require(extract((root / "large.zip").string(), (root / "output").string(), {}, midCancel).error == Error::Canceled, "cancel active extraction");
    require(!fs::exists(root / "output/large-extracted"), "partial extraction not published");
    require(create({(root / "large.bin").string(), (root / "large.bin").string()}, (root / "duplicate.zip").string(), Format::Zip).error == Error::InvalidPath, "duplicate source names");
    auto truncated = read(root / "test.tar.gz"); truncated.resize(truncated.size() / 2); text(root / "truncated.tar.gz", truncated);
    require(!verify((root / "truncated.tar.gz").string()).ok(), "truncated stream rejected");
    for (const auto& path : fs::directory_iterator(root)) require(!path.path().filename().string().starts_with(".lingmo-"), "compression staging cleaned");
    for (const auto& path : fs::directory_iterator(root / "output")) require(!path.path().filename().string().starts_with(".lingmo-"), "staging cleaned");
    std::string normalized; require(safeArchivePath("./folder/file", normalized) && normalized == "folder/file", "normalization");
    std::cout << "Archive core checks passed\n";
}
