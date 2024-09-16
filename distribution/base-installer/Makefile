ifndef TARGETS
TARGETS=pkgdetails run-debootstrap
endif

CFLAGS += -Wall -g -D_GNU_SOURCE

ifdef DEBUG
CFLAGS:=$(CFLAGS) -g3
else
CFLAGS:=$(CFLAGS) -Os -fomit-frame-pointer
endif

STRIP := strip

all: $(TARGETS)

pkgdetails: pkgdetails.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run-debootstrap: run-debootstrap.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -ldebconfclient -ldebian-installer

small: CFLAGS:=-Os $(CFLAGS)
small: $(TARGETS)
	$(STRIP) --remove-section=.comment --remove-section=.note $^
	ls -l $^

clean:
	-rm -f $(TARGETS)

test:
	$(MAKE) -C kernel test
