require 'mkmf'

LIBDIR      = Config::CONFIG['libdir']
INCLUDEDIR  = Config::CONFIG['includedir']

HEADER_DIRS = [
  # First search /opt/local for macports
  '/opt/local/include',

  # Then search /usr/local for people that installed from source
  '/usr/local/include',

  # you're terrible, glib
  '/usr/local/include/glib-2.0',
  '/usr/local/lib/glib-2.0/include/',

  # and pango
  '/usr/local/include/pango-1.0',
  '/usr/local/include/cairo',

  # Check the ruby install locations
  INCLUDEDIR,

  # Finally fall back to /usr
  '/usr/include',
]

LIB_DIRS = [
  # First search /opt/local for macports
  '/opt/local/lib',

  # Then search /usr/local for people that installed from source
  '/usr/local/lib',

  # Check the ruby install locations
  LIBDIR,

  # Finally fall back to /usr
  '/usr/lib',
]

#Shoes has a lot of effing deps.

dir_config('glib', HEADER_DIRS, LIB_DIRS)
dir_config('cairo', HEADER_DIRS, LIB_DIRS)
dir_config('pango', HEADER_DIRS, LIB_DIRS)
dir_config('jpeg', HEADER_DIRS, LIB_DIRS)
dir_config('giflib', HEADER_DIRS, LIB_DIRS)

dir_config('shoes', HEADER_DIRS, LIB_DIRS)

unless find_header('glib-2.0/glib.h')
  abort "glib.h is missing.  please install glib."
end

unless find_header('cairo/cairo.h')
  abort "cairo.h is missing. Please install cairo."
end

unless find_header('pango/pangocairo.h')
  abort "pangocairo.h is missing. Please install pango."
end

unless find_header('jpeglib.h')
  abort "jpeglib.h is missing.  please install jpeglib."
end

unless find_header('gif_lib.h')
  abort "gif_lib.h is missing.  please install giflib."
end

# omg only OSX for now
$CPPFLAGS << "-DSHOES_QUARTZ -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -fpascal-strings #{Config::CONFIG["CFLAGS"]} -x objective-c -fobjc-exceptions"
$LDFLAGS << " -lcairo -lpangocairo-1.0 -lgif -ljpeg"

create_makefile('shoes')

# We need this until the next patchlevel of 1.9.2 is released.
system('echo ".m.o:" >> Makefile')
system('echo "	\$(CXX) \$(INCFLAGS) \$(CPPFLAGS) \$(CXXFLAGS) \$(COUTFLAG)\$@ -c \$<" >> Makefile')
