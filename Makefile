CC := gcc
CFLAGS := -O3 -Wno-unused-result

MONIT := 0
UPSTART := 0
SYSVINIT := 0
NOWAYOUT := 0

ifeq ($(NOWAYOUT),1)
        CFLAGS += -DNOWAYOUT
endif


EXTRA_UNINSTALL_FILES =
ifeq ($(UPSTART),1)
        EXTRA_UNINSTALL_FILES += /etc/init/watchdog.conf /etc/init.d/watchdog 
else ifeq ($(SYSVINIT),1)
        EXTRA_UNINSTALL_FILES += /etc/init.d/watchdog
endif
ifeq ($(MONIT),1)
        EXTRA_UNINSTALL_FILES += /etc/monit/conf.d/watchdog.conf
endif

all: watchdog

watchdog: watchdog.c
	$(CC) $(CFLAGS) watchdog.c -o watchdog

clean:
	rm -f watchdog

install: all

        ifeq ($(UPSTART),1)
                ifeq ($(SYSVINIT),1)
			$(error UPSTART and SYSVINIT can't both be set)
                endif
        endif

        ifeq ($(MONIT),1)
		install -m 0600 data/watchdog.monit.conf /etc/monit/conf.d/watchdog.conf
        endif

        ifeq ($(UPSTART),1)
		install -m 0644 data/watchdog.upstart.conf /etc/init/watchdog.conf
		ln -sf /lib/init/upstart-job /etc/init.d/watchdog
        endif

        ifeq ($(SYSVINIT),1)
		install -m 0755 data/watchdog.sysvinit.sh /etc/init.d/watchdog
		update-rc.d watchdog defaults 99 00
        endif

	install -s watchdog /usr/local/sbin/watchdog

uninstall:
	rm -f /usr/local/sbin/watchdog

        ifneq ($(strip $(EXTRA_UNINSTALL_FILES)),)
		rm -f $(EXTRA_UNINSTALL_FILES)
        endif

        ifeq ($(SYSVINIT),1)
		update-rc.d -f watchdog remove
        endif
.PHONY: clean install uninstall all
