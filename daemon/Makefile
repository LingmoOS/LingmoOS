CXX = g++
CXXFLAGS = -Wall -O2
PREFIX = /usr
BINDIR = $(PREFIX)/bin
SYSTEMDDIR = /usr/lib/systemd/system

all: lingmo-watchdog

lingmo-watchdog: lingmo-watchdog.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

install:
	install -Dm755 lingmo-watchdog $(DESTDIR)$(BINDIR)/lingmo-watchdog
	install -Dm644 lingmo-watchdog.service $(DESTDIR)$(SYSTEMDDIR)/lingmo-watchdog.service

clean:
	rm -f lingmo-watchdog

.PHONY: all install clean 