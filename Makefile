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
# generate header dependencies for incremental builds
DEPFLAGS = -MMD -MP

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

PREFIX ?= /usr/local

BUILD_SRCS := src/main.c src/stats.c src/battery.c
CHECK_SRCS := test/test.c test/stats_test.c test/battery_test.c src/stats.c src/battery.c

BUILD_BIN := build/bin/mindwmstat
CHECK_BIN := build/bin/test

BUILD_OBJS := $(patsubst src/%.c,build/%.o,$(BUILD_SRCS))
CHECK_OBJS := $(patsubst src/%.c,build/%.o,$(filter src/%,$(CHECK_SRCS))) \
              $(patsubst test/%.c,build/%.o,$(filter test/%,$(CHECK_SRCS)))

all: build

build: $(BUILD_BIN)

check: $(CHECK_BIN)
	./$(CHECK_BIN)

$(BUILD_BIN): CFLAGS += $(X11_CFLAGS)
$(BUILD_BIN): LDLIBS += $(X11_LDLIBS)
$(BUILD_BIN): $(BUILD_OBJS) | build/bin
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# test sources include headers from src/
$(CHECK_BIN): CFLAGS += -Isrc
$(CHECK_BIN): $(CHECK_OBJS) | build/bin
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(LDFLAGS) -c $< -o $@

build/%.o: test/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) $(LDFLAGS) -c $< -o $@

build/bin:
	mkdir -p $@

install: build
	install -Dm755 $(BUILD_BIN) "$(DESTDIR)$(PREFIX)/bin/$(notdir $(BUILD_BIN))"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/$(notdir $(BUILD_BIN))"

clean:
	rm -rf build

-include $(BUILD_OBJS:.o=.d)
-include $(CHECK_OBJS:.o=.d)

.PHONY: all build check install uninstall clean
