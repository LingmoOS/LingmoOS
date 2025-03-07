# 定义构建目录
BUILD_DIR = buildpkg
BUILD_TP_DIR = buildpkg_tp
BUILD_SC_DIR = buildpkg_sc
THIRD_PARTY_DIR = third-party
CORE_EXTRA_FILE = core_extra.build
TAR = tar
TAR_OPTIONS = -Jcvf
EXCLUDE_DIR = --exclude=debian
CPU_CORES = $(shell nproc)

PKG_TARGETS = $(shell awk '{print $$1 "_" $$2 ".orig.tar.xz"}' $(CORE_EXTRA_FILE))

# 定义打包规则
%.orig.tar.xz:
	@echo "Pack: $@"
	$(eval PKG_NAME = $(shell echo $@ | awk -F_ '{print $$1}'))
	$(eval VERSION = $(shell echo $@ | awk -F_ '{print $$2}' | awk -F. '{print $$1 "." $$2 "." $$3}'))
	$(eval SRC_DIR = $(shell awk -v pkg=$(PKG_NAME) -v ver=$(VERSION) '$$1 == pkg && $$2 == ver {print $$3}' $(CORE_EXTRA_FILE)))
	$(TAR) $(TAR_OPTIONS) $@ $(EXCLUDE_DIR) $(SRC_DIR)

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
		$(MAKE) build-pkg; \
	)

# 构建单个软件包
build-pkg:
	@bash core_extra.build

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
