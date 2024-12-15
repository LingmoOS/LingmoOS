PREFIX	:= /usr
ARCH    := x86

all: build

bin:
	mkdir -p bin
	go build -o bin/override_tool tools/override/*.go

build: bin
	@echo build for arch: $(ARCH)
	mkdir -p result
	find schemas -name "*.xml" -exec cp {} result \;
	bin/override_tool -arch $(ARCH)

test: 
	@echo "Testing schemas with glib-compile-shemas..."
	glib-compile-schemas --dry-run result

print_gopath:
	GOPATH="${CURDIR}/${GOPATH_DIR}:${GOPATH}"

install:
	@echo install for arch:$(ARCH)
	mkdir -p $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas
	install -v -m 0644 result/*.xml $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas/
	install -v -m 0644 result/*.override $(DESTDIR)$(PREFIX)/share/glib-2.0/schemas/
	mkdir -p $(DESTDIR)$(PREFIX)/share/lingmo-desktop-schemas
	install -v -m 0644 result/*-override $(DESTDIR)$(PREFIX)/share/lingmo-desktop-schemas

	mkdir -p $(DESTDIR)$(PREFIX)/share/lingmo-appstore/
	install -v -m 0644 lingmo-appstore/$(ARCH)/settings.ini $(DESTDIR)$(PREFIX)/share/lingmo-appstore/

	mkdir -p $(DESTDIR)$(PREFIX)/share/lingmo-app-store/
	install -v -m 0644 lingmo-app-store/$(ARCH)/*.ini $(DESTDIR)$(PREFIX)/share/lingmo-app-store/

clean:
	-rm -rf bin
	-rm -rf result

.PHONY: bin
