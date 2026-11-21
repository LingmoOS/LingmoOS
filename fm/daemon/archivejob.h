#ifndef ARCHIVEJOB_H
#define ARCHIVEJOB_H

#include <zip.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

struct archive;

// Compresses files into a ZIP archive, or extracts an archive, on a thread of
// its own. The callbacks run on that thread.
class ArchiveJob
{
public:
    enum Operation {
        Compress,
        Extract,
    };

    ArchiveJob(Operation operation, std::vector<std::string> inputs, std::string outputPath);
    ~ArchiveJob();

    ArchiveJob(const ArchiveJob &) = delete;
    ArchiveJob &operator=(const ArchiveJob &) = delete;

    void start();
    void cancel();
    // False unless the request is the one waiting for a password.
    bool answerPassword(uint32_t request, std::string password);

    std::function<void(int progress)> onProgress;
    std::function<void(const std::string &fileName)> onCurrentFile;
    // An entry needs a password; retry when the ones given so far did not
    // fit. The job waits for answerPassword() or cancel().
    std::function<void(uint32_t request, bool retry)> onPasswordRequested;
    // The password given last opened the entry.
    std::function<void()> onPasswordAccepted;
    std::function<void(bool success, bool canceled, const std::string &error, const std::string &outputPath)>
            onCompleted;

private:
    struct CompressionEntry {
        std::string sourcePath;
        std::string archivePath;
        bool directory;
        uint64_t size;
        bool symbolicLink;
        std::string linkTarget;
    };

    void run();
    bool compress(std::string *error);
    bool extract(std::string *error);
    bool extractVolumes(const std::vector<std::string> &volumes, std::string *error);

    bool collectCompressionEntries(const std::string &sourcePath, const std::string &archivePath,
                                   std::vector<CompressionEntry> *entries, std::string *error) const;
    bool archiveEntryPath(const std::string &archiveName, std::string *relativePath, bool *directory) const;
    bool extractEntryData(struct archive *reader, int fd, uint64_t total, std::string *error);
    bool extractSingleFile(struct archive *reader, bool empty, const std::string &parentPath, uint64_t total,
                           std::string *error);

    void emitProgress(uint64_t processed, uint64_t total);
    void progressChanged(int progress);
    void currentFileChanged(const std::string &fileName);
    bool isCanceled() const;
    const char *requestPassword();
    void settlePassword();

    static void zipProgressCallback(zip_t *archive, double progress, void *state);
    static int zipCancelCallback(zip_t *archive, void *state);
    static const char *passwordCallback(struct archive *archive, void *state);

    Operation m_operation;
    std::vector<std::string> m_inputs;
    std::string m_outputPath;
    std::atomic_bool m_cancelRequested;
    int m_lastPercent = -1;
    std::mutex m_passwordMutex;
    std::condition_variable m_passwordCondition;
    // The request waiting for a password, 0 for none.
    uint32_t m_passwordRequest = 0;
    uint32_t m_lastPasswordRequest = 0;
    std::optional<std::string> m_answer;
    // Read by the archive reader until the next request.
    std::string m_password;
    int m_passwordsGiven = 0;
    bool m_passwordPending = false;
    std::thread m_thread;
};

#endif // ARCHIVEJOB_H
