# 定义变量
BUILD_DIR := Build_Pkgs
AUTO_DIR := $(BUILD_DIR)/auto
DEFAULT_DIR := $(BUILD_DIR)/default
THIRD_PARTY_DIR := thirdparty
THIRD_PARTY_BUILD_DIR := $(BUILD_DIR)/thirdparty
TARGET_DIR := Target/pkgs_out

# 检查是否是管理员用户
IS_ROOT := $(shell id -u)

# 获取CPU核心数
CPU_CORES := $(shell python3 -c "import multiprocessing; print(multiprocessing.cpu_count())")

# 定义构建命令
LINGMO_CMD := $(if $(IS_ROOT),,sudo) lingmo-pkgbuild -j$(CPU_CORES)
DPKG_CMD := dpkg-buildpackage -j$(CPU_CORES) -us -uc

# 默认目标
all: help

# 帮助信息
help:
	@echo "make config-pkgs      - 配置构建目录"
	@echo "make build-pkgs       - 构建软件包"
	@echo "make third-party      - 构建第三方软件包"
	@echo "make clean            - 清理构建产物"
	@echo "make help             - 显示此帮助信息"

# 配置构建目录
config-pkgs:
	@mkdir -p $(AUTO_DIR) $(DEFAULT_DIR)
	@cp -r core daemon fm im/fcitx5-5.1.12 lib/libshell lib/libsys lib/libsysui windows/global windows/plugins $(AUTO_DIR)/
	@cp -r shell/bc/* $(AUTO_DIR)/
	@rm -r $(AUTO_DIR)/{Debinstaller,Icons}
	@cp -r shell/cc/* $(AUTO_DIR)/
	@cp -r shell32/* $(AUTO_DIR)/
	@cp -r shell/bc/Icons core32/ service/asyncjobprog service/itframework service/locker_service $(DEFAULT_DIR)/

# 构建软件包
build-pkgs:
	@cd $(BUILD_DIR) && $(LINGMO_CMD) auto/
	@for dir in $(DEFAULT_DIR)/*; do \
		if [ -d "$$dir" ]; then \
			cd "$$dir" && $(DPKG_CMD); \
		fi \
	done
	@if [ "$(findstring -b,$(MAKEFLAGS))" != "" ]; then \
		mkdir -p $(TARGET_DIR); \
		find . -name "*.deb" -exec mv {} $(TARGET_DIR) \; \
	fi

# 构建第三方软件包
third-party:
	@if [ -d "$(THIRD_PARTY_DIR)" ]; then \
		cp -r $(THIRD_PARTY_DIR) $(THIRD_PARTY_BUILD_DIR); \
		for dir in $(THIRD_PARTY_BUILD_DIR)/*; do \
			if [ -d "$$dir" ]; then \
				cd "$$dir" && $(DPKG_CMD); \
			fi \
		done \
	fi

# 清理构建产物
clean:
	@rm -rf $(BUILD_DIR) $(TARGET_DIR)

.PHONY: help config-pkgs build-pkgs third-party clean

