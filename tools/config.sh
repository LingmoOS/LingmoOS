#!/bin/bash

# 定义源目录和目标目录
SRC_DIRS=(
    "boot/splash/BootScreen"
        "core"
            "core32"
                "daemon"
                    "fm"
                        "lib/*"
                            "libcoreui"
                                "posic"
                                    "server/asyncjobprog"
                                        "server/itframework"
                                            "shell/bc/*"
                                                "shell/cc/*"
                                                    "windows/*"
                                                    )

                                                    DEST_DIR="buildpkg"

                                                    # 遍历源目录并复制到目标目录
                                                    for dir in "${SRC_DIRS[@]}"; do
                                                        # 使用 rsync 复制目录及其内容
                                                            rsync -av --relative "$dir" "$DEST_DIR"
                                                            done

                                                            echo "复制完成！"