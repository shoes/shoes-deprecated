#include <sqlite3_ruby.h>

VALUE mSqlite3;
VALUE cSqlite3Blob;

static VALUE libversion(VALUE UNUSED(klass))
{
  return INT2NUM(sqlite3_libversion_number());
}

void Init_sqlite3_native()
{
  /*
   * SQLite3 is a wrapper around the popular database
   * sqlite[http://sqlite.org].
   *
   * For an example of usage, see SQLite3::Database.
   */
  mSqlite3     = rb_define_module("SQLite3");

  /* A class for differentiating between strings and blobs, when binding them
   * into statements.
   */
  cSqlite3Blob = rb_define_class_under(mSqlite3, "Blob", rb_cString);

  /* Initialize the sqlite3 library */
  sqlite3_initialize();

  init_sqlite3_database();
  init_sqlite3_statement();

  rb_define_singleton_method(mSqlite3, "libversion", libversion, 0);
}
