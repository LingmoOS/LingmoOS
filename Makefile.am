# 定义构建目录
BUILD_DIR = buildpkg
BUILD_TP_DIR = buildpkg_tp
BUILD_SC_DIR = buildpkg_sc
THIRD_PARTY_DIR = third-party
CPU_CORES = $(shell nproc)

# 动态生成需要复制的目录列表
SOURCE_DIRS = \
    boot/splash/BootScreen \
    core \
    daemon \
    fm \
    lib/libsys \
    lib/libsysui \
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
	@mkdir -p $(BUILD_DIR) $(BUILD_SC_DIR)
	@$(foreach dir,$(SOURCE_DIRS),cp -r $(dir) $(BUILD_DIR)/;)
	@mv -v $(BUILD_DIR)/asyncjobprog $(BUILD_SC_DIR)/
	@mv -v $(BUILD_DIR)/itframework $(BUILD_SC_DIR)/
	@mv -v $(BUILD_DIR)/locker_server_VirtualPackage $(BUILD_SC_DIR)/
	@mv -v $(BUILD_DIR)/Notepad $(BUILD_SC_DIR)/
	@mv -v $(BUILD_DIR)/Debinstaller $(BUILD_SC_DIR)/
	@mv -v $(BUILD_DIR)/Icons $(BUILD_SC_DIR)/
	@mv -v core32 $(BUILD_SC_DIR)/

# 构建所有软件包
pkg_all: prepare
	@echo "Building packages with lingmo-pkgbuild..."
	lingmo-pkgbuild -j$(CPU_CORES) -c $(BUILD_DIR)
	@echo "Building packages with dpkg..."
	@$(foreach dir,$(wildcard $(BUILD_SC_DIR)/*), \
		$(MAKE) build-pkg DIR=$(dir); \
	)

# 构建单个软件包
build-pkg:
	@if [ -d "$(DIR)" ]; then \
		echo "Processing $(DIR)..."; \
		cd $(DIR) && \
		PKG_NAME=$$(head -1 debian/changelog | awk '{print $$1}') && \
		PKG_VERSION=$$(head -1 debian/changelog | awk -F'[()]' '{print $$2}' | awk -F: '{print $$2}' | awk -F- '{print $$1}') && \
		echo "Package: $$PKG_NAME, Version: $$PKG_VERSION"; \
		tar --exclude='debian' -cf - . | xz -z - > ../$$PKG_NAME_$$PKG_VERSION.orig.tar.xz && \
		apt build-dep -y ./ && \
		dpkg-buildpackage -j$(CPU_CORES); \
	else \
		echo "Directory $(DIR) does not exist."; \
	fi

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

# Buuld Iso Images
image-iso:
	@echo "Nothing..."

# 清理构建目录
clean-local:
	rm -rf $(BUILD_DIR) $(BUILD_TP_DIR) $(BUILD_SC_DIR)
