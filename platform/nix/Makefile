# shoes - abstract windowing for gtk, osx, and win32
# by why the lucky stiff, released to you under the MIT license

SRC = shoes/app.c shoes/canvas.c shoes/effects.c shoes/image.c shoes/internal.c shoes/ruby.c shoes/world.c shoes/native/gtk.c shoes/http/curl.c
OBJ = ${SRC:.c=.o}

PREFIX = /usr/local
INCS = -I. -I/usr/include -I/usr/local/include
LIBS = -L/usr/lib -L/usr/local/lib -lcairo -lpangocairo-1.0 -lgif -ljpeg -lrt

REVISION = `git rev-list HEAD | wc -l`
RUBY_INCS = `ruby -rrbconfig -e 'puts Config::CONFIG["archdir"]'`
RUBY_SO = `ruby -rrbconfig -e 'puts Config::CONFIG["RUBY_SO_NAME"]'`
RUBY_LIBS = `ruby -rrbconfig -e 'puts Config::CONFIG["LIBRUBYARG_SHARED"]'` -l${RUBY_SO}
RUBY_PREFIX = `ruby -rrbconfig -e 'puts Config::CONFIG["prefix"]'`
RUBY_PLATFORM = `ruby -rrbconfig -e 'puts RUBY_PLATFORM'`
RUBY_VERSION = `ruby -rrbconfig -e 'puts Config::CONFIG["ruby_version"]'`
CAIRO_CFLAGS = `pkg-config --cflags cairo`
CAIRO_LIB = `pkg-config --libs cairo`
PANGO_CFLAGS = `pkg-config --cflags pango`
PANGO_LIB = `pkg-config --libs pango`
GTK_CFLAGS = `pkg-config --cflags gtk+-2.0`
GTK_LIB = `pkg-config --libs gtk+-2.0`
CURL_CFLAGS = `curl-config --cflags`
CURL_LIB = `curl-config --libs`
RELEASE_NAME = `cat shoes/version.h | ruby -ne 'puts $$1.downcase if ~/SHOES_RELEASE_NAME "(.+)"/'`
HPRICOT_NAME = `ruby -rubygems -e 'puts eval(File.read("req/hpricot/gemspec")).full_name'`
SQLITE_NAME = `ruby -rubygems -e 'puts eval(File.read("req/sqlite3/gemspec")).full_name'`
JSON_NAME = `ruby -rubygems -e 'puts eval(File.read("req/json/gemspec")).full_name'`
GEM_DIR = "dist/ruby/gems/${RUBY_VERSION}"
SHOES_IDENT = "shoes ${RELEASE_NAME} (0.r${REVISION}) [${RUBY_PLATFORM}]"

VERSION = 0.r${REVISION}
CFLAGS = -DSHOES_GTK -fPIC ${INCS} ${CAIRO_CFLAGS} ${PANGO_CFLAGS} ${GTK_CFLAGS} ${CURL_CFLAGS} -I${RUBY_INCS}
LDFLAGS = -fPIC ${LIBS} ${CAIRO_LIB} ${PANGO_LIB} ${GTK_LIB} ${CURL_LIB} ${RUBY_LIBS}
OPTIONS =

ifeq (${DEBUG}, 1)
	CFLAGS += -DDEBUG
	OPTIONS += DEBUG
	SHOES_IDENT += +debug
endif

ifeq (${VIDEO}, 1)
	VLC_0_8 = $(shell vlc --version 2>/dev/null | ruby -ne 'puts $$1 if ~/VLC version (\d+\.\d+)/')
	CFLAGS += -DVIDEO $(if $(filter 0.8, $(VLC_0_8)), -DVLC_0_8, -I/usr/include/vlc/plugins)
	LIBS += -lvlc
	OPTIONS += VIDEO
	SHOES_IDENT += +video
endif

all: options dist/shoes.launch

options:
	@echo build options: ${SHOES_IDENT}
	@echo "CC       = ${CC}"
	@echo "RUBY     = ${RUBY_PREFIX}"
	@echo "OPTIONS  =${OPTIONS}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} -o $@ $<

dist/libshoes.so: ${OBJ} 
	@echo CC -o $@
	@mkdir dist
	@${CC} -o $@ ${OBJ} ${LDFLAGS} -shared

dist/shoes-bin: dist/libshoes.so bin/main.o
	@echo CC -o $@
	@${CC} -o $@ ${LDFLAGS} bin/main.o -Ldist -lshoes

dist/shoes.launch: dist/shoes-bin
	@cp platform/nix/shoes.launch dist/
	@mkdir -p dist/ruby
	@ln -s ${RUBY_PREFIX}/lib/ruby/1.8 dist/ruby/lib
	@cp ${RUBY_PREFIX}/lib/lib${RUBY_SO}.so dist
	@ln -s lib${RUBY_SO}.so dist/libruby.so.1.8
	@cp -r lib dist/lib
	@cp -r req/rubygems/* dist/lib
	@mkdir -p ${GEM_DIR}/specifications
	@mkdir -p ${GEM_DIR}/gems/${HPRICOT_NAME}
	@cp -r req/hpricot/lib ${GEM_DIR}/gems/${HPRICOT_NAME}/lib
	@cd req/hpricot/ext/hpricot_scan && ruby extconf.rb && make
	@cp req/hpricot/ext/hpricot_scan/*.so ${GEM_DIR}/gems/${HPRICOT_NAME}/lib
	@cp req/hpricot/gemspec ${GEM_DIR}/specifications/${HPRICOT_NAME}.gemspec
	@mkdir -p ${GEM_DIR}/gems/${SQLITE_NAME}
	@cp -r req/sqlite3/lib ${GEM_DIR}/gems/${SQLITE_NAME}/lib
	@cd req/sqlite3/ext/sqlite3 && ruby extconf.rb && make
	@cp req/sqlite3/ext/sqlite3/*.so ${GEM_DIR}/gems/${SQLITE_NAME}/lib
	@cp req/sqlite3/gemspec ${GEM_DIR}/specifications/${SQLITE_NAME}.gemspec
	@mkdir -p ${GEM_DIR}/gems/${JSON_NAME}
	@cp -r req/json/lib ${GEM_DIR}/gems/${JSON_NAME}/lib
	@cd req/json/ext/generator && ruby extconf.rb && make
	@cd req/json/ext/parser && ruby extconf.rb && make
	@mkdir -p ${GEM_DIR}/gems/${JSON_NAME}/lib/json/ext
	@cp req/json/ext/**/*.so ${GEM_DIR}/gems/${JSON_NAME}/lib/json/ext
	@cp req/json/gemspec ${GEM_DIR}/specifications/${JSON_NAME}.gemspec
	@cp -r req/ftsearch/lib/* dist/lib
	@cd req/ftsearch/ext/ftsearchrt && ruby extconf.rb && make
	@cp req/ftsearch/ext/ftsearchrt/*.so dist/lib
	@cd req/binject/ext/binject_c && ruby extconf.rb && make
	@cp req/binject/ext/binject_c/*.so dist/lib
	@cp -r samples dist/samples
	@cp -r static dist/static
	@rm -rf dist/**/.svn
	@echo ${SHOES_IDENT} > dist/VERSION.txt
	@cp README COPYING dist
	@echo 'LD_LIBRARY_PATH="$$APPPATH/../lib/shoes" $$APPPATH/../lib/shoes/shoes-bin $$@' >> dist/shoes.launch 
	@chmod 755 dist/shoes.launch

clean:
	@echo cleaning
	@rm -rf dist
	@rm -f ${OBJ} bin/main.o shoes-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p shoes-${VERSION}
	@cp -R COPYING Makefile README bin shoes samples static \
		shoes-${VERSION}
	@rm -f shoes-${VERSION}/bin/main.skel
	@rm -rf shoes-${VERSION}/**/.svn
	@tar -cf shoes-${VERSION}.tar shoes-${VERSION}
	@gzip shoes-${VERSION}.tar
	@rm -rf shoes-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f dist/shoes.launch ${DESTDIR}${PREFIX}/bin/shoes
	@chmod 755 ${DESTDIR}${PREFIX}/bin/shoes
	@echo installing libraries to ${DESTDIR}${PREFIX}/lib/shoes
	@rm -rf ${DESTDIR}${PREFIX}/lib/shoes
	@mkdir -p ${DESTDIR}${PREFIX}/lib/shoes
	@cp -r dist/* ${DESTDIR}${PREFIX}/lib/shoes/

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/shoes
	# @echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	# @rm -f ${DESTDIR}${MANPREFIX}/man1/shoes.1

.PHONY: all options clean dist install uninstall
