# Build a Redis module: src/module.c -> module.so
#
#   make          # builds ./module.so
#   make clean    # removes build artifacts

CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2 -fPIC -std=gnu99 -Isrc
LDFLAGS ?= -shared

# macOS needs different link flags; Linux is the default target here.
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS = -bundle -undefined dynamic_lookup
endif

TARGET  := module.so
SRC     := src/module.c

all: $(TARGET)

$(TARGET): $(SRC) src/redismodule.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
