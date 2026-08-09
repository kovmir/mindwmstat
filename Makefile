CC ?= cc

X11_CFLAGS := $(shell pkg-config --cflags x11)
X11_LDLIBS := $(shell pkg-config --libs x11)
GIT_VERSION := $(shell git describe --tags --always --dirty)

CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -pedantic
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -D_DEFAULT_SOURCE
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"

ifneq ($(CONSOLE),)
  CFLAGS += -DCONSOLE_OUTPUT
else
  LDLIBS += $(X11_LDLIBS)
  CFLAGS += $(X11_CFLAGS)
endif

ifneq ($(ANIMATION),)
  CFLAGS += -DSTATUS_ANIMATION
endif

LDLIBS += $(LIB_LDLIBS)

BUILD_BIN := mindwmstat
TEST_BIN  := stats_test

PREFIX ?= /usr/local

all: build

build: $(BUILD_BIN)

test: $(TEST_BIN)

$(BUILD_BIN): main.o stats.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(TEST_BIN): stats_test.o stats.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

main.o: config.h stats.h

stats.o: stats.h

install:
	install -Dm755 ./$(BUILD_BIN) "$(DESTDIR)$(PREFIX)/bin/$(BUILD_BIN)"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(BUILD_BIN)"

clean:
	rm -f *.o $(BUILD_BIN) $(TEST_BIN)

.PHONY: all build test install uninstall clean
