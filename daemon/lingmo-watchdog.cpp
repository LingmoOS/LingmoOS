#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <fstream>
#include <sstream>

bool isProcessRunning(const std::string& processName) {
    std::string cmd = "pgrep -f " + processName + " > /dev/null";
    return (system(cmd.c_str()) == 0);
}

void startFileManager() {
    system("lingmo-filemanager --desktop &");
}

int main() {
    // 将进程变成守护进程
    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "Fork failed" << std::endl;
        exit(1);
    }
    
    if (pid > 0) {
        exit(0); // 父进程退出
    }
    
    // 创建新会话
    setsid();
    
    // 更改工作目录
    chdir("/");
    
    // 关闭标准文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // 主循环监控
    while (true) {
        if (!isProcessRunning("lingmo-filemanager")) {
            startFileManager();
        }
        
        // 每5秒检查一次
        sleep(3);
    }
    
    return 0;
} 