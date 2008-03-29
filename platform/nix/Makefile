# shoes - abstract windowing for gtk, osx, and win32
# by why the lucky stiff, released to you under the MIT license

SRC = shoes/app.c shoes/canvas.c shoes/dialogs.c shoes/image.c shoes/internal.c shoes/ruby.c shoes/world.c
OBJ = ${SRC:.c=.o}

PREFIX = /usr/local
INCS = -I. -I/usr/include -I/usr/local/include
LIBS = -L/usr/lib -L/usr/local/lib -lcairo -lpangocairo-1.0 -lgif -ljpeg

REVISION = `git rev-list HEAD | wc -l`
RUBY_INCS = `ruby -rrbconfig -e 'puts Config::CONFIG["archdir"]'`
RUBY_LIBS = `ruby -rrbconfig -e 'puts Config::CONFIG["LIBRUBYARG_SHARED"]'`
RUBY_PREFIX = `ruby -rrbconfig -e 'puts Config::CONFIG["prefix"]'`
RUBY_SO = `ruby -rrbconfig -e 'puts Config::CONFIG["RUBY_SO_NAME"]'`
CAIRO_CFLAGS = `pkg-config --cflags cairo`
CAIRO_LIB = `pkg-config --libs cairo`
PANGO_CFLAGS = `pkg-config --cflags pango`
PANGO_LIB = `pkg-config --libs pango`
GTK_CFLAGS = `pkg-config --cflags gtk+-2.0`
GTK_LIB = `pkg-config --libs gtk+-2.0`

VERSION = 0.r${REVISION}
CFLAGS = -DSHOES_GTK -fPIC ${INCS} ${CAIRO_CFLAGS} ${PANGO_CFLAGS} ${GTK_CFLAGS} -I${RUBY_INCS}
LDFLAGS = -fPIC ${LIBS} ${CAIRO_LIB} ${PANGO_LIB} ${GTK_LIB} ${RUBY_LIBS}
OPTIONS =

ifeq (${DEBUG}, 1)
	CFLAGS += -DDEBUG
	OPTIONS += DEBUG
endif

ifeq (${VIDEO}, 1)
	CFLAGS += -DVIDEO
	LIBS += -lvlc
	OPTIONS += VIDEO
endif

all: clean options shoes

options:
	@echo shoes build options:
	@echo "CC       = ${CC}"
	@echo "RUBY     = ${RUBY_PREFIX}"
	@echo "OPTIONS  =${OPTIONS}"
	@ruby -v

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
	@echo 'LD_LIBRARY_PATH="$$APPPATH/../lib/shoes" $$APPPATH/../lib/shoes/shoes-bin $$@' >> dist/shoes.launch 
	@chmod 755 dist/shoes.launch

shoes: dist/shoes.launch
	@mkdir -p dist/ruby
	@ln -s ${RUBY_PREFIX}/lib/ruby/1.8 dist/ruby/lib
	@cp ${RUBY_PREFIX}/lib/lib${RUBY_SO}.so dist
	@ln -s lib${RUBY_SO}.so dist/libruby.so.1.8
	@cp -r lib dist/lib
	@cp -r rubygems/* dist/lib
	@cp -r samples dist/samples
	@cp -r static dist/static
	@rm -rf dist/**/.svn
	@cp README COPYING dist

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

install: all dist/shoes.launch
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
