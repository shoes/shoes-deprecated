#!/bin/bash
if test -z "$1"
then
  echo "Please include the directory where the dependencies' source code resides."
  exit
fi
rm -rf /tmp/dep
cd $1

#make clean all deps
cd pkg-config-0.20
make clean
cd ../libpng-1.4.1
make clean
cd ../jpeg-8a
make clean
cd ../giflib-4.1.6
make clean
cd ../gettext-0.17
make clean
cd ../libiconv-1.13
make clean
cd ../glib-2.24.0
make clean
cd ../pixman-0.18.0
make clean
cd ../cairo-1.8.10
make clean
cd ../pango-1.28.0
make clean
cd ../portaudio
make clean
cd ../ruby-1.9.2-p180
make clean

#build pkg-config
cd ../pkg-config-0.20
./configure --prefix=/tmp/dep
make && make install
export MACOSX_DEPLOYMENT_TARGET=10.6
export PATH=/tmp/dep/bin:$PATH
export CFLAGS="-I/tmp/dep/include -w"
export LDFLAGS=-L/tmp/dep/lib
export PKG_CONFIG_PATH=/tmp/dep/lib/pkgconfig

#build libpng
cd ../libpng-1.4.1
./configure --prefix=/tmp/dep
make && make install

#build libjpeg
cd ../jpeg-8a/
./configure --prefix=/tmp/dep --enable-shared
make && make install

#build giflib
cd ../giflib-4.1.6
./configure --prefix=/tmp/dep
make && make install

#build gettext
cd ../gettext-0.17
./configure --prefix=/tmp/dep
make && make install

#build libiconv
cd ../libiconv-1.13
./configure --prefix=/tmp/dep
make && make install

#build glib 
cd ../glib-2.24.0
./configure --prefix=/tmp/dep --disable-visibility
make && make install

#build pixman 
cd ../pixman-0.18.0
./configure --prefix=/tmp/dep
make && make install

#build cairo 
cd ../cairo-1.8.10
./configure --prefix=/tmp/dep --enable-quartz=yes --enable-quartz-font=yes --enable-xlib=no
make && make install

#build pango 
cd ../pango-1.28.0
./configure --prefix=/tmp/dep --with-x=no
make && make install

#build Port Audio 
cd ../portaudio/
./configure --prefix=/tmp/dep --disable-mac-universal
make && make install

#build ruby
cd ../ruby-1.9.2-p180
./configure --prefix=/tmp/dep --enable-shared
make && make install
