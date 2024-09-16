PREFIX = /usr

ifndef USE_GCCGO
    GOBUILD = go build
else
   LDFLAGS = $(shell pkg-config --libs gio-2.0)
   GOBUILD = go build -compiler gccgo -gccgoflags "${LDFLAGS}"
endif

LANGUAGES = $(basename $(notdir $(wildcard misc/po/*.po)))

all: build

build/locale/%/LC_MESSAGES/deepin-feedback.mo:misc/po/%.po
	mkdir -p $(@D)
	msgfmt -o $@ $<

translate: $(addsuffix /LC_MESSAGES/deepin-feedback.mo, $(addprefix build/locale/, ${LANGUAGES}))

build: check translate
	mkdir -p build
	(cd cli; go build -o ../build/feedbackserver feedbackserver.go)
	deepin-policy-ts-convert ts2policy misc/com.deepin.deepin-feedback.policy.in misc/ts/com.deepin.deepin-feedback.policy build/com.deepin.deepin-feedback.policy
	deepin-desktop-ts-convert ts2desktop misc/deepin-feedback.desktop.in misc/ts/deepin-feedback.desktop build/deepin-feedback.desktop

ts:
	deepin-policy-ts-convert policy2ts misc/com.deepin.deepin-feedback.policy.in misc/ts/com.deepin.deepin-feedback.policy
	deepin-desktop-ts-convert desktop2ts misc/deepin-feedback.desktop.in misc/ts/deepin-feedback.desktop

pot:
	xgettext -LShell --from-code UTF-8 --keyword=gettext -o misc/po/deepin-feedback-cli.pot cli/deepin-feedback-cli.sh deepin-feedback

install:
	install -dm0755 ${DESTDIR}${PREFIX}/bin/
	mkdir -p ${DESTDIR}/var/lib/deepin-feedback/
	cp -rf feedback_logpath.json ${DESTDIR}/var/lib/deepin-feedback/feedback_logpath.json
	install -m0755 cli/deepin-feedback-cli.sh ${DESTDIR}${PREFIX}/bin/deepin-feedback-cli
	install -m0755 build/feedbackserver ${DESTDIR}${PREFIX}/bin/
	install -m0755 deepin-feedback ${DESTDIR}${PREFIX}/bin/
	install -dm0755 ${DESTDIR}${PREFIX}/share/applications
	install -m0755 build/deepin-feedback.desktop ${DESTDIR}${PREFIX}/share/applications/
	cp -rf misc/icons ${DESTDIR}${PREFIX}/share/
	install -dm0755 ${DESTDIR}${PREFIX}/share/polkit-1/actions/
	install -m0644 build/com.deepin.deepin-feedback.policy ${DESTDIR}${PREFIX}/share/polkit-1/actions/
	install -dm0755 ${DESTDIR}${PREFIX}/share/locale/
	cp -r build/locale/* ${DESTDIR}${PREFIX}/share/locale

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/deepin-feedback-cli
	rm -f /var/lib/deepin-feedback/feedback_logpath.json
	rm -f ${DESTDIR}${PREFIX}/share/applications/deepin-feedback.desktop
	rm -f ${DESTDIR}${PREFIX}/share/icons/hicolor/scalable/apps/deepin-feedback.svg

check:
	bash --norc -n -- cli/deepin-feedback-cli.sh

clean:
	$(RM) -r build
