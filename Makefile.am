SUBDIRS = boot core daemon fm lib libcoreui posic server shell windows

all-local:
	@echo "Building all packages..."
	$(MAKE) pkg_all

pkg_all:
	@echo "Building packages with lingmo-pkgbuild..."
	lingmo-pkgbuild -j$(CPU_CORES) -c $(BUILD_DIR)

third-party:
	@if [ -d "$(THIRD_PARTY_DIR)" ] && [ -n "$(shell ls -A $(THIRD_PARTY_DIR))" ]; then \
		echo "Processing third-party directories..."; \
		mkdir -p $(BUILD_TP_DIR); \
		cp -r $(THIRD_PARTY_DIR)/* $(BUILD_TP_DIR)/; \
		lingmo-pkgbuild -j$(CPU_CORES) -c $(BUILD_TP_DIR); \
	else \
		echo "No third-party directories found."; \
	fi

# clean-local:
#	rm -rf $(BUILD_DIR) $(BUILD_TP_DIR)