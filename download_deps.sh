if test -z "$1"
then
  echo "Please include the directory you'd like to use to download the dependencies."
  exit
fi
mkdir $1
cd $1
curl -O http://pkg-config.freedesktop.org/releases/pkg-config-0.20.tar.gz > pkg-config-0.20.tar.gz
curl -L -O http://downloads.sourceforge.net/project/libpng/libpng14/older-releases/1.4.1/libpng-1.4.1.tar.gz > libpng-1.4.1.tar.gz
curl -L -O http://downloads.sourceforge.net/project/giflib/giflib%204.x/giflib-4.1.6/giflib-4.1.6.tar.gz > giflib-4.1.6.tar.gz
curl -O http://ftp.gnu.org/gnu/gettext/gettext-0.17.tar.gz > gettext-0.17.tar.gz
curl -O http://ftp.gnu.org/gnu/libiconv/libiconv-1.13.tar.gz > libiconv-1.13.tar.gz
curl -O http://cgit.freedesktop.org/pixman/snapshot/pixman-0.18.0.tar.gz > pixman-0.18.0.tar.gz
curl -L -O http://ftp.gnome.org/pub/GNOME/sources/pango/1.28/pango-1.28.0.tar.gz > pango-1.28.0.tar.gz
curl -O http://portaudio.com/archives/pa_stable_v19_20071207.tar.gz > pa_stable_v19_20071207.tar.gz
curl -O http://cairographics.org/releases/cairo-1.8.10.tar.gz > cairo-1.8.10.tar.gz
curl -O http://ijg.org/files/jpegsrc.v8a.tar.gz > jpegsrc.v8a.tar.gz
curl -L -O http://ftp.gnome.org/pub/gnome/sources/glib/2.24/glib-2.24.0.tar.gz > glib-2.24.0.tar.gz
curl -O http://ftp.ruby-lang.org/pub/ruby/1.9/ruby-1.9.1-p378.tar.gz > ruby-1.9.1-p378.tar.gz
curl -O http://production.cf.rubygems.org/rubygems/rubygems-1.3.6.tgz > rubygems-1.3.6.tgz
for file in *.tar.gz *.tgz; do tar -xzf $file; echo $file; done && cd -