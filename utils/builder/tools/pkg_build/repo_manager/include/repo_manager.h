#pragma once
#include <string>
#include <filesystem>

namespace lingmo {

class RepoManager {
public:
    // 初始化仓库
    static bool initRepo(const std::filesystem::path& repoDir, const std::string& codename);

    // 导入单个 changes 文件
    static bool importChanges(const std::filesystem::path& repoDir, 
                            const std::filesystem::path& changesFile,
                            const std::string& codename);

    // 导入目录中的所有 changes 文件
    static bool importChangesDir(const std::filesystem::path& repoDir,
                               const std::filesystem::path& directory,
                               const std::string& codename);

    // 导入 deb 包
    static bool importDeb(const std::filesystem::path& repoDir,
                         const std::filesystem::path& debFile,
                         const std::string& codename,
                         const std::string& component = "main");

    // 导入目录中的所有 deb 包
    static bool importDebDir(const std::filesystem::path& repoDir,
                           const std::filesystem::path& directory,
                           const std::string& codename,
                           const std::string& component = "main");

private:
    // 创建仓库配置文件
    static bool createRepoConfig(const std::filesystem::path& repoDir, 
                               const std::string& codename);

    // 检查 reprepro 是否可用
    static bool checkReprepro();

    // 执行 reprepro 命令
    static bool runRepreproCommand(const std::string& cmd);

    static bool s_repreproChecked;
};

} // namespace lingmo 