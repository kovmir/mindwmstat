CC ?= cc

X11_CFLAGS := $(shell pkg-config --cflags x11)
X11_LDLIBS := $(shell pkg-config --libs x11)
GIT_VERSION := $(shell git describe --tags --always --dirty 2>/dev/null)

CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -D_DEFAULT_SOURCE

ifneq ($(GIT_VERSION),)
  CFLAGS += -DGIT_VERSION='"$(GIT_VERSION)"'
endif

ifneq ($(CONSOLE),)
  CFLAGS += -DCONSOLE_OUTPUT
  X11_LDLIBS :=
  X11_CFLAGS :=
endif

ifneq ($(ANIMATION),)
  CFLAGS += -DSTATUS_ANIMATION
endif

LDLIBS += $(LIB_LDLIBS)

BUILD_BIN := mindwmstat
CHECK_BIN  := test

PREFIX ?= /usr/local

all: build

build: $(BUILD_BIN)

check: $(CHECK_BIN)

$(BUILD_BIN): LDLIBS += $(X11_LDLIBS)
$(BUILD_BIN): CFLAGS += $(X11_CFLAGS)
$(BUILD_BIN): main.o stats.o battery.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(CHECK_BIN): battery_test.o battery.o stats_test.o stats.o test.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

main.o: config.h stats.h battery.h

test.o: stats_test.h battery_test.h

stats.o: stats.h

battery.o: battery.h

install:
	install -Dm755 ./$(BUILD_BIN) "$(DESTDIR)$(PREFIX)/bin/$(BUILD_BIN)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(BUILD_BIN)"

clean:
	rm -f *.o $(BUILD_BIN) $(CHECK_BIN)

.PHONY: all build install uninstall clean
