#include <ruby.h>
#include "version.h"

void shoes_version_init() {
    // for compatibily pre 3.2.22
    rb_const_set(cTypes, rb_intern("RELEASE_NAME"), rb_str_new2(SHOES_RELEASE_NAME));
    rb_const_set(cTypes, rb_intern("RELEASE_TYPE"), rb_str_new2(SHOES_STYLE));
    rb_const_set(cTypes, rb_intern("RELEASE_ID"), INT2NUM(SHOES_RELEASE_ID));
    rb_const_set(cTypes, rb_intern("REVISION"), INT2NUM(SHOES_REVISION));
    rb_const_set(cTypes, rb_intern("RELEASE_BUILD_DATE"), rb_str_new2(SHOES_BUILD_DATE));
    rb_const_set(cTypes, rb_intern("VERSION"), rb_str_new2(SHOES_RELEASE_NAME));
    // post 3.2.22 constants
    rb_const_set(cTypes, rb_intern("VERSION_NUMBER"), rb_str_new2(SHOES_VERSION_NUMBER));
    rb_const_set(cTypes, rb_intern("VERSION_MAJOR"), INT2NUM(SHOES_VERSION_MAJOR));
    rb_const_set(cTypes, rb_intern("VERSION_MINOR"), INT2NUM(SHOES_VERSION_MINOR));
    rb_const_set(cTypes, rb_intern("VERSION_TINY"), INT2NUM(SHOES_VERSION_TINY));
    rb_const_set(cTypes, rb_intern("VERSION_NAME"), rb_str_new2(SHOES_VERSION_NAME));
    rb_const_set(cTypes, rb_intern("VERSION_REVISION"), INT2NUM(SHOES_VERSION_REVISION));
    rb_const_set(cTypes, rb_intern("VERSION_DATE"), rb_str_new2(SHOES_VERSION_DATE));
    rb_const_set(cTypes, rb_intern("VERSION_PLATFORM"), rb_str_new2(SHOES_VERSION_PLATFORM));
}