# Makefile for PennMUSH 

# - System configuration - #

VERSION=1.8.3
PATCHLEVEL=8

#
# This section of the file should be automatically configured by
# the Configure script. If it doesn't work, you might try starting
# from the Makefile.old that's included instead, and reporting
# your problem (including this Makefile) to pennmush-bugs@pennmush.org
#
# If you want to profile the code, add -pg -a -DPROFILING to CCFLAGS
# and (probably) remove -O
#
MAKE=/usr/bin/make
CC=gcc -std=gnu99

SQL_CFLAGS=-I/usr/include/mysql  -DBIG_JOINS=1  -fno-strict-aliasing   -DUNIV_LINUX -DUNIV_LINUX  
SQL_LDFLAGS=-Wl,-Bsymbolic-functions -rdynamic -L/usr/lib/mysql -lmysqlclient  

CCFLAGS=-ggdb -O -W -Wall -pedantic -Wno-comment -fstack-protector -Wa,--noexecstack -I/usr/include/ -I.. -I../hdrs   -I/usr/include/python2.7
LDFLAGS= -Wl,-z,noexecstack -L/usr/lib -L/usr/lib -lpython2.7 -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions
CLIBS=-lpcre -lz -lcrypt -lm  -lssl -lcrypto -lpcre -lssl -lcrypto  -lssl -lcrypto      -L/usr/lib -lz -lpthread -ldl  -lutil
INSTALL=/usr/bin/install -c
INSTALLDIR=$installdir
CP=/bin/cp
CHMOD=/bin/chmod
INSTALL_LINKS=ln -s ../src/netmud netmush; ln -s ../src/info_slave info_slave

# stupid SYS V shell
SHELL=/bin/sh
# Where to install with 'make globalinstall'
GLOBAL_INSTALL=${exec_prefix}/libexec

all: config.h options.h autogen game/mush.cnf
	@echo "Making all in src."
	(cd src; /usr/bin/make all "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" "MAKE=$(MAKE)" \
	"MAKEFLAGS=$(MAKEFLAGS)" "SQL_CFLAGS=$(SQL_CFLAGS)" \
	"SQL_LDFLAGS=$(SQL_LDFLAGS)")
	@echo "If the make was successful, use 'make install' to install links."

config.h: configure
	@echo "Looks like your configure has been updated."
	@echo "Run that first. If you did just run configure and"
	@echo "it said that config.h was unchanged, 'touch config.h'"
	@echo "to suppress this message and continue compiling."
	@exit 1

options.h: options.h.dist
	@echo "Please use 'make update' to update your options.h file from options.h.dist"
	@echo "You must cp options.h.dist to options.h and edit it."
	@exit 1

autogen: hdrs/cmds.h hdrs/funs.h hdrs/switches.h

hdrs/cmds.h: src/cmds.c src/command.c src/cque.c src/help.c src/set.c src/sql.c Patchlevel
	/usr/bin/perl utils/mkcmds.pl commands

hdrs/switches.h: src/SWITCHES Patchlevel
	/usr/bin/perl utils/mkcmds.pl switches

src/switchinc.c: src/SWITCHES Patchlevel
	/usr/bin/perl utils/mkcmds.pl switches

hdrs/funs.h: src/fun*.c src/bsd.c src/conf.c src/extmail.c src/help.c src/markup.c src/wiz.c src/sql.c Patchlevel
	/usr/bin/perl utils/mkcmds.pl functions

hdrs/patches.h: patches/*
	/usr/bin/perl utils/mkcmds.pl patches

install: localized all
	-rm -f game/netmush
	-rm -f game/info_slave
	(cd game; $(INSTALL_LINKS))
	(cd game/txt; make)
	@echo "If you plan to run multiple MUSHes, consider running 'make customize'"

netmud: 
	(cd src; make netmud "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"SQL_CFLAGS=$(SQL_CFLAGS)" "SQL_LDFLAGS=$(SQL_LDFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

access:
	utils/make_access_cnf.sh game

pennmush.pot:
	(cd src; make ../po/pennmush.pot)

localized:
	-echo "Localizing for your locale..."
	-(cd po; make localized)

portmsg:
	(cd src; make portmsg "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

ssl_slave:
	(cd src; make ssl_slave "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" "MAKE=$(MAKE)" \
	"MAKEFLAGS=$(MAKEFLAGS)")

versions: CHANGES*
	-@rm -rf CHANGES*~ CHANGES*bak
	@utils/mkvershlp.pl game/txt/hlp CHANGES*

safety:
	$(CP) src/*.c /var/pennmush-bak/src
	$(CP) hdrs/*.h /var/pennmush-bak/hdrs
	$(CP) * /var/pennmush-bak

distdepend: hdrs/funs.h hdrs/cmds.h
	(cd src; /usr/bin/make depend "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

local-files:
	$(CP) -f src/cmdlocal.dst src/cmdlocal.c
	$(CP) -f src/flaglocal.dst src/flaglocal.c
	$(CP) -f src/funlocal.dst src/funlocal.c
	$(CP) -f src/local.dst src/local.c

# REQUIRES GNU INDENT! DON'T INDENT WITH ANYTHING ELSE!
indent:
	@(cd src; make indent)

customize: update-conf
	-@/usr/bin/perl utils/customize.pl

# The default place to find the runtime files is in this directory,
# but it can be overridden with env variables so people can use
# other game directories.
GAMEDIR=game

update-conf: game/mushcnf.dst game/aliascnf.dst game/restrictcnf.dst game/namescnf.dst
	-@/usr/bin/touch game/mushcnf.dst
	-@/usr/bin/perl utils/update-cnf.pl $(GAMEDIR)/mush.cnf game/mushcnf.dst
	-@/usr/bin/touch game/aliascnf.dst
	-@/usr/bin/perl utils/update-cnf.pl $(GAMEDIR)/alias.cnf game/aliascnf.dst
	-@/usr/bin/touch game/restrictcnf.dst
	-@/usr/bin/perl utils/update-cnf.pl $(GAMEDIR)/restrict.cnf game/restrictcnf.dst
	-@if [ ! -f $(GAMEDIR)/names.cnf ]; then $(CP) game/namescnf.dst $(GAMEDIR)/names.cnf; fi

$(GAMEDIR)/alias.cnf: game/aliascnf.dst
	-@/usr/bin/touch game/aliascnf.dst
	-@/usr/bin/perl utils/update-cnf.pl $(GAMEDIR)/alias.cnf game/aliascnf.dst

$(GAMEDIR)/restrict.cnf: game/restrictcnf.dst
	-@/usr/bin/touch game/restrictcnf.dst
	-@/usr/bin/perl utils/update-cnf.pl $(GAMEDIR)/restrict.cnf game/restrictcnf.dst

$(GAMEDIR)/names.cnf: game/namescnf.dst
	if [ ! -f game/names.cnf ]; then \
		$(CP) game/namescnf.dst $(GAMEDIR)/names.cnf \
	fi

$(GAMEDIR)/mush.cnf: game/mushcnf.dst
	-@/usr/bin/touch game/mushcnf.dst
	-@/usr/bin/perl utils/update-cnf.pl $(GAMEDIR)/mush.cnf game/mushcnf.dst

update: update-hdr update-conf

update-hdr:
	-@/usr/bin/touch options.h.dist
	-@sleep 2
	-@/usr/bin/perl utils/update.pl options.h options.h.dist

test: netmud
	(cd test; /usr/bin/perl alltests.pl)

clean:
	(cd src; make clean)
	(cd game; rm -f netmush info_slave)

distclean: 
	(cd hdrs; rm -f *.orig *~ \#* *.rej *.bak funs.h cmds.h buildinf.h patches.h)
	(cd utils; rm -f *.orig *~ \#* *.rej *.bak mkcmds.sh *.o)
	(cd game; rm -rf *.log netmush info_slave *.orig *.rej *~ *.bak mush.cnf)
	(cd src; make distclean; rm -f Makefile)
	(cd game/txt; make clean)
	(rm -rf .config Makefile config.h config.sh options.h)

totallyclean: distclean 
	(cd hdrs; rm -rf *.rej)
	(cd src; rm -rf *.rej)
	-rm -f Makefile

diffs:
	@make indent > /dev/null 2>&1
	@make versions > /dev/null 2>&1
	@make touchswitches > /dev/null 2>&1
	@make autogen > /dev/null 2>&1
	@(prcs diff -r$(VS) -N pennmush `cat MANIFEST` | grep -v 'Index:')

commit: indent
	@svn commit

patch: versions
	@make-patch-header
	@make diffs

etags: 
	(cd src; make etags)

ctags:
	(cd src; make ctags)

touchswitches:
	@/usr/bin/touch src/SWITCHES

dist.tar: 
	svn checkout svn+ssh://svn.pennmush.org/svn/pennmush/$(VERSION)/trunk
	sed -e "s#^#pennmush-$(VERSION)p$(PATCHLEVEL)/#" < trunk/MANIFEST \
		> DISTFILES
	mv -f trunk pennmush-$(VERSION)p$(PATCHLEVEL)
	tar -cvf dist.tar --files-from=DISTFILES
	-gpg -sb /tmp/dist.tar
	bzip2 -k /tmp/dist.tar
	openssl dgst -sha1 -out /tmp/dist.tar.bz2.sha1 /tmp/dist.tar.bz2 
	gzip -k /tmp/dist.tar
	openssl dgst -sha1 -out /tmp/dist.tar.gz.sha1 /tmp/dist.tar.gz
	-rm -rf pennmush-$(VERSION)p$(PATCHLEVEL) DISTFILES

globalinstall: install
	(cd game/txt; make clean compose.sh)
#	$(INSTALLDIR) $(GLOBAL_INSTALL)
	$(CP) -R game/* $(GLOBAL_INSTALL)
	rm -f $(GLOBAL_INSTALL)/netmush $(GLOBAL_INSTALL)/info_slave
	$(INSTALL) config.sh $(GLOBAL_INSTALL)/config.sh
	$(INSTALL) src/netmud $(GLOBAL_INSTALL)/netmush
	$(INSTALL) src/info_slave utils/ln-dir.sh $(GLOBAL_INSTALL)
	$(CHMOD) a+rX -R $(GLOBAL_INSTALL)
	@echo "** Files installed in $(GLOBAL_INSTALL). Feel free to move them."
	@echo "** You can run $(GLOBAL_INSTALL)/ln-dir.sh to create a user directory,"
	@echo "** or symlink that to somewhere easier to run. You may wish to strip them."
