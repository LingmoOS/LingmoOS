PREFIX = /usr

ifndef USE_GCCGO
    GOBUILD = go build
else
   LDFLAGS = $(shell pkg-config --libs gio-2.0)
   GOBUILD = go build -compiler gccgo -gccgoflags "${LDFLAGS}"
endif

LANGUAGES = $(basename $(notdir $(wildcard misc/po/*.po)))

all: build

build/locale/%/LC_MESSAGES/lingmo-feedback.mo:misc/po/%.po
	mkdir -p $(@D)
	msgfmt -o $@ $<

translate: $(addsuffix /LC_MESSAGES/lingmo-feedback.mo, $(addprefix build/locale/, ${LANGUAGES}))

build: check translate
	mkdir -p build
	(cd cli; go build -o ../build/feedbackserver feedbackserver.go)
	lingmo-policy-ts-convert ts2policy misc/com.lingmo.lingmo-feedback.policy.in misc/ts/com.lingmo.lingmo-feedback.policy build/com.lingmo.lingmo-feedback.policy
	lingmo-desktop-ts-convert ts2desktop misc/lingmo-feedback.desktop.in misc/ts/lingmo-feedback.desktop build/lingmo-feedback.desktop

ts:
	lingmo-policy-ts-convert policy2ts misc/com.lingmo.lingmo-feedback.policy.in misc/ts/com.lingmo.lingmo-feedback.policy
	lingmo-desktop-ts-convert desktop2ts misc/lingmo-feedback.desktop.in misc/ts/lingmo-feedback.desktop

pot:
	xgettext -LShell --from-code UTF-8 --keyword=gettext -o misc/po/lingmo-feedback-cli.pot cli/lingmo-feedback-cli.sh lingmo-feedback

install:
	install -dm0755 ${DESTDIR}${PREFIX}/bin/
	mkdir -p ${DESTDIR}/var/lib/lingmo-feedback/
	cp -rf feedback_logpath.json ${DESTDIR}/var/lib/lingmo-feedback/feedback_logpath.json
	install -m0755 cli/lingmo-feedback-cli.sh ${DESTDIR}${PREFIX}/bin/lingmo-feedback-cli
	install -m0755 build/feedbackserver ${DESTDIR}${PREFIX}/bin/
	install -m0755 lingmo-feedback ${DESTDIR}${PREFIX}/bin/
	install -dm0755 ${DESTDIR}${PREFIX}/share/applications
	install -m0755 build/lingmo-feedback.desktop ${DESTDIR}${PREFIX}/share/applications/
	cp -rf misc/icons ${DESTDIR}${PREFIX}/share/
	install -dm0755 ${DESTDIR}${PREFIX}/share/polkit-1/actions/
	install -m0644 build/com.lingmo.lingmo-feedback.policy ${DESTDIR}${PREFIX}/share/polkit-1/actions/
	install -dm0755 ${DESTDIR}${PREFIX}/share/locale/
	cp -r build/locale/* ${DESTDIR}${PREFIX}/share/locale

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/lingmo-feedback-cli
	rm -f /var/lib/lingmo-feedback/feedback_logpath.json
	rm -f ${DESTDIR}${PREFIX}/share/applications/lingmo-feedback.desktop
	rm -f ${DESTDIR}${PREFIX}/share/icons/hicolor/scalable/apps/lingmo-feedback.svg

check:
	bash --norc -n -- cli/lingmo-feedback-cli.sh

clean:
	$(RM) -r build
