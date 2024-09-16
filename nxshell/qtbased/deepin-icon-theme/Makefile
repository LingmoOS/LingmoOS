PREFIX = /usr

all: check  cursors-bloom  cursors-bloom-dark cursors-bloom-fantacy

cursors-bloom:
	cd cursors-src/bloom/loginspinner/ && make

cursors-bloom-dark:
	cd cursors-src/bloom-dark/loginspinner/ && make

cursors-bloom-fantacy:
	cd cursors-src/bloom-fantacy/loginspinner/ && make

check: check-same-icon check-perm
	gtk-update-icon-cache bloom || exit 101
	gtk-update-icon-cache bloom-dark || exit 101
	-rm -f bloom/icon-theme.cache
	-rm -f bloom-dark/icon-theme.cache

check-name-unique:
	find bloom -name "*.svg" | xargs -n1 basename | sort | uniq -d | xargs -I '{}' find -name '{}'
	find bloom-dark -name "*.svg" | xargs -n1 basename | sort | uniq -d | xargs -I '{}' find -name '{}'
check-same-icon:
	find bloom -type f | xargs md5sum | sort | uniq --check-chars=32 -d
	find bloom-dark -type f | xargs md5sum | sort | uniq --check-chars=32 -d

check-perm: hicolor-links
	@echo "Fix icon files permission"
	find bloom -type f -exec chmod 644 {} \;
	find bloom-dark -type f -exec chmod 644 {} \;

prepare: check-name-unique check-same-icon
	mkdir -p build

build: prepare convert

convert:
	mkdir -p build
	python tools/convert.py bloom build

clean:
	rm -rf build

install: hicolor-links
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/bloom
	cp -r bloom/* $(DESTDIR)$(PREFIX)/share/icons/bloom
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/bloom-dark
	cp -r bloom-dark/* $(DESTDIR)$(PREFIX)/share/icons/bloom-dark
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/vintage
	cp -r vintage/* $(DESTDIR)$(PREFIX)/share/icons/vintage
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/bloom-classic
	cp -r bloom-classic/* $(DESTDIR)$(PREFIX)/share/icons/bloom-classic
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/bloom-classic-dark
	cp -r bloom-classic-dark/* $(DESTDIR)$(PREFIX)/share/icons/bloom-classic-dark

hicolor-links:
	./tools/hicolor.links bloom hicolor.list ./
