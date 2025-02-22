# 定义构建目录
BUILD_DIR = buildpkg
BUILD_TP_DIR = buildpkg_tp
THIRD_PARTY_DIR = third-party
CPU_CORES = $(shell nproc)

# 动态生成需要复制的目录列表
SOURCE_DIRS = \
    boot/splash/BootScreen \
    core \
    daemon \
    fm \
    $(wildcard lib/*) \
    libcoreui \
    posic \
    server/asyncjobprog \
    server/itframework \
    server/locker_server_VirtualPackage \
    $(wildcard shell/bc/*) \
    $(wildcard shell/cc/*) \
    $(wildcard windows/*)

# 默认目标
all-local:
	@echo "Building all packages..."
	$(MAKE) pkg_all

# 准备构建目录
prepare:
	@mkdir -p $(BUILD_DIR)
	@$(foreach dir,$(SOURCE_DIRS),cp -r $(dir) $(BUILD_DIR)/;)

# 构建所有软件包
pkg_all: prepare
	@echo "Building packages with lingmo-pkgbuild..."
	lingmo-pkgbuild -j$(CPU_CORES) -c $(BUILD_DIR)

# 处理第三方软件包
third-party:
	@if [ -d "$(THIRD_PARTY_DIR)" ] && [ -n "$(shell ls -A $(THIRD_PARTY_DIR))" ]; then \
		echo "Processing third-party directories..."; \
		mkdir -p $(BUILD_TP_DIR); \
		cp -r $(THIRD_PARTY_DIR)/* $(BUILD_TP_DIR)/; \
		lingmo-pkgbuild -j$(CPU_CORES) -c $(BUILD_TP_DIR); \
	else \
		echo "No third-party directories found."; \
	fi

# 清理构建目录
clean-local:
	rm -rf $(BUILD_DIR) $(BUILD_TP_DIR)