#pragma once
#include <string>
#include <filesystem>

class LingmoPkgBuilder {
public:
    enum class PackageType {
        Native,    // 原生包
        Quilt     // 带补丁的包
    };

    // 只保留一个构造函数，用于从源目录构建
    LingmoPkgBuilder(const std::filesystem::path& sourceDir, PackageType type = PackageType::Native);

    static bool buildFromDirectory(const std::filesystem::path& sourceDir, 
                                 const std::string& outputPath);

    void setMaintainer(const std::string& maintainer);
    void setDescription(const std::string& description);
    void addFile(const std::string& sourcePath, const std::string& destPath);
    bool build(const std::filesystem::path& sourceDir);

    static void setGlobalBuildDir(const std::filesystem::path& buildDir) {
        s_buildDir = buildDir;
    }
    
    static void setGlobalOutputDir(const std::filesystem::path& outputDir) {
        s_outputDir = outputDir;
    }

    // 添加设置并行构建数的静态方法
    static void setThreadCount(int threads) {
        s_threadCount = threads;
    }

    // 添加签名相关的设置
    static void setSignBuild(bool sign) {
        s_signBuild = sign;
    }
    
    static void setSignKey(const std::string& key) {
        s_signKey = key;
    }

    // 添加检查构建依赖的静态方法
    static bool checkBuildDependencies(const std::filesystem::path& sourceDir);

    // 添加清理构建目录的静态方法
    static void cleanBuildDir() {
        if (std::filesystem::exists(s_buildDir)) {
            std::filesystem::remove_all(s_buildDir);
        }
    }

private:
    bool createControlFile();
    bool createDataArchive();
    bool createDebianBinary();
    bool createOrigTarball() const;
    bool isNativePackage() const { return m_packageType == PackageType::Native; }

    bool parseControlFile(const std::filesystem::path& controlFile);
    bool parseChangelogFile(const std::filesystem::path& changelogFile);
    bool copyDebianFiles(const std::filesystem::path& debianDir);
    bool copyArtifacts(const std::string& packageName) const;

    std::string m_packageName;
    std::string m_version;
    std::string m_architecture;
    std::string m_maintainer;
    std::string m_description;
    std::filesystem::path m_tempDir;

    PackageType m_packageType;

    static std::filesystem::path s_buildDir;
    static std::filesystem::path s_outputDir;
    static int s_threadCount;  // 存储并行构建数
    static bool s_signBuild;         // 是否签名
    static std::string s_signKey;    // 签名密钥

    static bool runCommand(const std::string& cmd);  // 用于执行命令并检查结果
}; 