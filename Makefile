CC = gcc
PKG_CONFIG = pkg-config
PREFIX = /usr/local
TARGET = featherbar
SRC = featherbar.c

CFLAGS += -Wall -Wextra $(shell $(PKG_CONFIG) --cflags xft)
LDLIBS += $(shell $(PKG_CONFIG) --libs xft) -lX11

.PHONY: all install clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)
