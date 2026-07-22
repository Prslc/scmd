CROSS_COMPILE ?=
CC            ?= $(CROSS_COMPILE)gcc
STRIP         ?= $(CROSS_COMPILE)strip

DIRS   = core io conf log
CFLAGS = -Wall -Wextra -O2 -std=gnu11 -D_GNU_SOURCE $(addprefix -I,$(DIRS))
LDFLAGS = -Wl,--as-needed

OUT    ?= ../scmd/bin/scmd
OBJS   = core/main.o core/init.o core/epoll.o \
         io/netlink.o io/inotify.o io/utils.o \
         conf/ini.o log/log.o

$(OUT): $(OBJS)
	mkdir -p $(dir $(OUT))
	$(CC) $(LDFLAGS) -o $@ $^

core/%.o: core/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

io/%.o: io/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

conf/%.o: conf/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

log/%.o: log/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

PREFIX ?= /data/adb/modules/charge_controller
BINDIR ?= $(PREFIX)/bin

.PHONY: install
install: $(OUT)
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(OUT) $(DESTDIR)$(BINDIR)/
	$(STRIP) $(DESTDIR)$(BINDIR)/scmd

.PHONY: clean
clean:
	rm -f $(OBJS)
	rm -rf ../scmd/bin
