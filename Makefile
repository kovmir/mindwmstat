CC ?= cc
INSTALL ?= install
PREFIX ?= /usr/local
PROJECT = mindwmstat

CFLAGS += -std=c23
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wstrict-prototypes
CFLAGS += -D_DEFAULT_SOURCE

ifneq ($(CONSOLE),)
  CFLAGS += -DCONSOLE_OUTPUT
else
  LIBS = $(shell pkg-config --libs x11)
endif

ifneq ($(ANIMATION),)
  CFLAGS += -DSTATUS_ANIMATION
endif

LDFLAGS = $(LIBS)

# Fake system files for debug.
BATT_CAP = ./batt_capacity
AC_PLUG  = ./ac_plug
MEMINFO  = ./meminfo

build:
	$(CC) ./$(PROJECT).c $(CFLAGS) -O2 $(LDFLAGS) -o ./$(PROJECT)

debug:
	$(CC) ./$(PROJECT).c \
		$(CFLAGS) -O0 -g \
		-DDEBUG \
		-DDEBUG_BATT_PATH=\"$(BATT_CAP)\" \
		-DDEBUG_AC_PATH=\"$(AC_PLUG)\" \
		-DMEMINFO_PATH=\"$(MEMINFO)\" \
		$(LDFLAGS) -o ./$(PROJECT)

test: debug
	./test.sh $(BATT_CAP) $(AC_PLUG) $(MEMINFO)

gdb: debug
	gdb ./$(PROJECT)

clean:
	rm -f ./$(PROJECT)

install:
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	$(INSTALL) ./$(PROJECT) "$(DESTDIR)$(PREFIX)/bin/$(PROJECT)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(PROJECT)"
	rmdir --ignore-fail-on-non-empty "$(DESTDIR)$(PREFIX)/bin"

.PHONY: all debug test gdb clean install uninstall
