# Create all of the file tasks necessary to copy a tree of files
#
# source_files  - a collection of the source files to copy
# source_root   - the common root dir of the source files
# dest_root     - the common root dir to which files should be copied
# invoking_task - the rake task name that will invoke tasks created here. The
#                 invoking_task depends on these files existing in dest_root.
def create_copy_file_tasks(source_files, source_root, dest_root, invoking_task)
  source_files.each do |source|
    target = source.pathmap("%{#{source_root},#{dest_root}}p")
    directory File.dirname(target)
    file target => [File.dirname(target), source] do |t|
      cp source, target
    end
    task invoking_task => target
  end
end

# Create all of the tasks necessary to compile a c-extension for a library, and
# copy the compiled extension
#
# source_root   - the extension's source root (must contain 'extconf.rb')
# dest_root     - the directory to which the compiled extension should be copied
# invoking_task - the rake task name that will invoke tasks created here. The
#                 invoking_task depends on these files existing in dest_root.
def create_compile_ext_tasks(source_root, dest_root, invoking_task)
  compiled_ext = "#{source_root}/#{SPECIAL_BUNDLE_NAMES[File.basename(source_root)] || File.basename(source_root)}.bundle"
  create_copy_file_tasks(FileList[compiled_ext], source_root, dest_root, invoking_task)
  file compiled_ext => FileList["#{source_root}/*.c"] do
    cd source_root do
      `ruby extconf.rb; make >/dev/null 2>&1`
    end
  end
end

# Normal case (default): bundle name matches parent dir name
#   Example: /req/chipmunk/ext/chipmunk/chipmunk.bundle
#
# Special case (specify here): bundle name is different from paernt dir name
#   Example: /req/binject/ext/binject_c/binject.bundle
SPECIAL_BUNDLE_NAMES = {'binject_c' => 'binject', 'sqlite3' => 'sqlite3_native'}
DIST_RUBY_LIB        = "dist/ruby/lib"
BUNDLE_DIR           = "#{DIST_RUBY_LIB}/#{SHOES_RUBY_ARCH}"
GEM_DIR              = "dist/ruby/gems/#{RUBY_V}"
GEMSPEC_DIR          = "#{GEM_DIR}/specifications"
BUNDLED_LIBS         = %w[binject ftsearch chipmunk]
GEMS                 = %w[hpricot json sqlite3 redcarpet2]
SPECIAL_EXT_DIRS     = {'json' => 'lib/json/ext'}

# Copy Ruby source to dist
create_copy_file_tasks(FileList["#{EXT_RUBY_LIBRUBY}/**/*.{rb,so,bundle}"], EXT_RUBY_LIBRUBY, DIST_RUBY_LIB, :req)

# Compile gem extensions. Copy gem's ruby source and compiled extensions to
# dist/
GEMS.each do |gem_name|
  gem_root_dir = "req/#{gem_name}"
  gemspec = FileList["#{gem_root_dir}/{*.gemspec,gemspec}"].first
  gem_full_name = eval(File.read(gemspec)).full_name
  gemspec_target = "#{GEMSPEC_DIR}/#{gem_full_name}.gemspec"
  dest_dir = "#{GEM_DIR}/gems/#{gem_full_name}"
  ext_dir = "#{dest_dir}/#{SPECIAL_EXT_DIRS[gem_name] || 'lib'}"

  FileList["#{gem_root_dir}/ext/*"].each do |ext|
    create_compile_ext_tasks(ext, ext_dir, :req)
  end
  create_copy_file_tasks(FileList["#{gem_root_dir}/lib/**/*.rb"], gem_root_dir, dest_dir, :req)
  # Treat gemspecs differently, because they get renamed.
  file gemspec_target => [GEMSPEC_DIR, gemspec] do
    cp gemspec, gemspec_target
  end
  task :req => gemspec_target
end
directory GEMSPEC_DIR

# Compile bundled library extensions. Copy library's ruby source and compiled
# extensions to dist/
#
# (These libs are not packaged as gems. If they were, they wouldn't require
# special treatment.)
BUNDLED_LIBS.each do |lib|
  FileList["req/#{lib}/ext/*"].each do |ext|
    create_compile_ext_tasks(ext, BUNDLE_DIR, :req)
  end
  create_copy_file_tasks(FileList["req/#{lib}/lib/**/*.rb"], "req/#{lib}/lib", DIST_RUBY_LIB, :req)
end

# TODO: Is the code in this task still necessary?
task :common_build => :req do
  if RUBY_LIB_BASE != 'lib'
    Dir.chdir(File.join(Dir.pwd,"dist/ruby")) { ln_s "lib", RUBY_LIB_BASE }
  end
  unless ENV['STANDARD']
    %w[soap wsdl xsd].each do |libn|
      rm_rf "dist/ruby/lib/#{libn}"
    end
  end
end

