CC ?= cc
INSTALL ?= install
PREFIX ?= /usr/local
PROJECT = mindwmstat

CFLAGS += -std=c23
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wstrict-prototypes
CFLAGS += -O3
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

build:
	$(CC) ./$(PROJECT).c $(CFLAGS) $(LDFLAGS) -o ./$(PROJECT)

debug:
	$(CC) ./$(PROJECT).c $(CFLAGS) -g $(LDFLAGS) -o ./$(PROJECT)

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

.PHONY: all debug gdb memcheck memcheck_v memcheck_full clean install uninstall
