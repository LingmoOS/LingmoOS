需要的依赖:
cmake
libcjson1
libcjson-dev

源码编译执行:
cmake . && make && echo "" && echo "" && ./getSystemVersion


获取系统版本号接口: version_t getSystemVersion();

typedef struct {
    char    *os_version;            //系统版本
    char    *update_version;        //补丁版本
} version_t;
