//
// rubyext.c
// the ruby binding to bloopsaphone
//
// (c) 2009 why the lucky stiff
//
#include <ruby.h>
#include "bloopsaphone.h"

static VALUE cBloops, cSound, cTrack;

#ifndef RSTRING_LEN
#define RSTRING_LEN(str) RSTRING(str)->len
#define RSTRING_PTR(str) RSTRING(str)->ptr
#endif

//
// Main Bloops object
//
static void
rb_bloops_free(bloops *B)
{
  bloops_destroy(B);
}

VALUE
rb_bloops_alloc(VALUE klass)
{
  bloops *B = bloops_new();
  return Data_Wrap_Struct(klass, NULL, rb_bloops_free, B);
}

VALUE
rb_bloops_clear(VALUE self)
{
  bloops *B;
  Data_Get_Struct(self, bloops, B);
  bloops_clear(B);
  return self;
}

VALUE
rb_bloops_play(VALUE self)
{
  bloops *B;
  Data_Get_Struct(self, bloops, B);
  bloops_play(B);
  return self;
}

VALUE
rb_bloops_is_stopped(VALUE self)
{
  bloops *B;
  Data_Get_Struct(self, bloops, B);
  return bloops_is_done(B) ? Qtrue : Qfalse;
}

VALUE
rb_bloops_get_tempo(VALUE self)
{
  bloops *B;
  Data_Get_Struct(self, bloops, B);
  return INT2NUM(B->tempo);
}

VALUE
rb_bloops_set_tempo(VALUE self, VALUE tempo)
{
  bloops *B;
  Data_Get_Struct(self, bloops, B);
  bloops_tempo(B, NUM2INT(tempo));
  return tempo;
}

//
// Instrument creation
//
static void
rb_bloops_sound_free(bloopsaphone *sound)
{
  free(sound);
}

VALUE
rb_bloops_load(VALUE self, VALUE fname)
{
  bloops *B;
  bloopsaphone *P;
  Data_Get_Struct(self, bloops, B);

  StringValue(fname);
  P = bloops_sound_file(B, RSTRING_PTR(fname));
  if (P == NULL) return Qnil;
  return Data_Wrap_Struct(cSound, NULL, rb_bloops_sound_free, P);
}

VALUE
rb_bloops_sound(VALUE self, VALUE type)
{
  bloopsaphone *P = bloops_square();
  P->type = (unsigned char)NUM2INT(type);
  return Data_Wrap_Struct(cSound, NULL, rb_bloops_sound_free, P);
}

VALUE
rb_bloops_get_type(VALUE self)
{
  bloopsaphone *P;
  Data_Get_Struct(self, bloopsaphone, P);
  return INT2NUM((int)P->type);
}

VALUE
rb_bloops_set_type(VALUE self, VALUE type)
{
  bloopsaphone *P;
  Data_Get_Struct(self, bloopsaphone, P);
  P->type = (unsigned char)NUM2INT(type);
  return type;
}

#define SOUND_ACCESSOR(name) \
  VALUE rb_bloops_get_##name(VALUE self) { \
    bloopsaphone *P; \
    Data_Get_Struct(self, bloopsaphone, P); \
    return rb_float_new(P->name); \
  } \
  VALUE rb_bloops_set_##name(VALUE self, VALUE f) { \
    bloopsaphone *P; \
    Data_Get_Struct(self, bloopsaphone, P); \
    P->name = (float)NUM2DBL(f); \
    return f; \
  }

SOUND_ACCESSOR(arp);
SOUND_ACCESSOR(aspeed);
SOUND_ACCESSOR(attack);
SOUND_ACCESSOR(decay);
SOUND_ACCESSOR(dslide);
SOUND_ACCESSOR(freq);
SOUND_ACCESSOR(hpf);
SOUND_ACCESSOR(hsweep);
SOUND_ACCESSOR(limit);
SOUND_ACCESSOR(lpf);
SOUND_ACCESSOR(lsweep);
SOUND_ACCESSOR(phase);
SOUND_ACCESSOR(psweep);
SOUND_ACCESSOR(repeat);
SOUND_ACCESSOR(resonance);
SOUND_ACCESSOR(slide);
SOUND_ACCESSOR(square);
SOUND_ACCESSOR(sustain);
SOUND_ACCESSOR(sweep);
SOUND_ACCESSOR(punch);
SOUND_ACCESSOR(vibe);
SOUND_ACCESSOR(vspeed);
SOUND_ACCESSOR(vdelay);
SOUND_ACCESSOR(volume);

//
// Individual track object
//
static void
rb_bloops_track_free(bloopsatrack *track)
{
  bloops_track_destroy(track);
}

VALUE
rb_bloops_tune(VALUE self, VALUE sound, VALUE notes)
{
  int i;
  bloops *B;
  bloopsaphone *phone;
  bloopsatrack *track;
  Data_Get_Struct(self, bloops, B);
  Data_Get_Struct(sound, bloopsaphone, phone);

  StringValue(notes);
  track = bloops_track(B, phone, RSTRING_PTR(notes), RSTRING_LEN(notes));

  for (i = 0; i < BLOOPS_MAX_TRACKS; i++)
    if (B->tracks[i] == NULL) {
      bloops_track_at(B, track, i);
      break;
    }
  return Data_Wrap_Struct(cTrack, NULL, rb_bloops_track_free, track);
}

VALUE
rb_bloops_track_str(VALUE self)
{
  char *str;
  VALUE obj;
  bloopsatrack *track;
  Data_Get_Struct(self, bloopsatrack, track);

  str = bloops_track_str(track);
  obj = rb_str_new2(str);
  free(str);

  return obj;
}

//
// Ruby extension startup
//
void
Init_bloops()
{
  cBloops = rb_define_class("Bloops", rb_cObject);
  rb_define_alloc_func(cBloops, rb_bloops_alloc);
  rb_define_method(cBloops, "clear", rb_bloops_clear, 0);
  rb_define_method(cBloops, "load", rb_bloops_load, 1);
  rb_define_method(cBloops, "play", rb_bloops_play, 0);
  rb_define_method(cBloops, "sound", rb_bloops_sound, 1);
  rb_define_method(cBloops, "stopped?", rb_bloops_is_stopped, 0);
  rb_define_method(cBloops, "tempo", rb_bloops_get_tempo, 0);
  rb_define_method(cBloops, "tempo=", rb_bloops_set_tempo, 1);
  rb_define_method(cBloops, "tune", rb_bloops_tune, 2);

  rb_const_set(cBloops, rb_intern("SQUARE"), INT2NUM(BLOOPS_SQUARE));
  rb_const_set(cBloops, rb_intern("SAWTOOTH"), INT2NUM(BLOOPS_SAWTOOTH));
  rb_const_set(cBloops, rb_intern("SINE"), INT2NUM(BLOOPS_SINE));
  rb_const_set(cBloops, rb_intern("NOISE"), INT2NUM(BLOOPS_NOISE));

  cSound = rb_define_class_under(cBloops, "Sound", rb_cObject);
  rb_define_method(cSound, "arp", rb_bloops_get_arp, 0);
  rb_define_method(cSound, "arp=", rb_bloops_set_arp, 1);
  rb_define_method(cSound, "aspeed", rb_bloops_get_aspeed, 0);
  rb_define_method(cSound, "aspeed=", rb_bloops_set_aspeed, 1);
  rb_define_method(cSound, "attack", rb_bloops_get_attack, 0);
  rb_define_method(cSound, "attack=", rb_bloops_set_attack, 1);
  rb_define_method(cSound, "decay", rb_bloops_get_decay, 0);
  rb_define_method(cSound, "decay=", rb_bloops_set_decay, 1);
  rb_define_method(cSound, "dslide", rb_bloops_get_dslide, 0);
  rb_define_method(cSound, "dslide=", rb_bloops_set_dslide, 1);
  rb_define_method(cSound, "freq", rb_bloops_get_freq, 0);
  rb_define_method(cSound, "freq=", rb_bloops_set_freq, 1);
  rb_define_method(cSound, "hpf", rb_bloops_get_hpf, 0);
  rb_define_method(cSound, "hpf=", rb_bloops_set_hpf, 1);
  rb_define_method(cSound, "hsweep", rb_bloops_get_hsweep, 0);
  rb_define_method(cSound, "hsweep=", rb_bloops_set_hsweep, 1);
  rb_define_method(cSound, "limit", rb_bloops_get_limit, 0);
  rb_define_method(cSound, "limit=", rb_bloops_set_limit, 1);
  rb_define_method(cSound, "lpf", rb_bloops_get_lpf, 0);
  rb_define_method(cSound, "lpf=", rb_bloops_set_lpf, 1);
  rb_define_method(cSound, "lsweep", rb_bloops_get_lsweep, 0);
  rb_define_method(cSound, "lsweep=", rb_bloops_set_lsweep, 1);
  rb_define_method(cSound, "phase", rb_bloops_get_phase, 0);
  rb_define_method(cSound, "phase=", rb_bloops_set_phase, 1);
  rb_define_method(cSound, "psweep", rb_bloops_get_psweep, 0);
  rb_define_method(cSound, "psweep=", rb_bloops_set_psweep, 1);
  rb_define_method(cSound, "punch", rb_bloops_get_punch, 0);
  rb_define_method(cSound, "punch=", rb_bloops_set_punch, 1);
  rb_define_method(cSound, "repeat", rb_bloops_get_repeat, 0);
  rb_define_method(cSound, "repeat=", rb_bloops_set_repeat, 1);
  rb_define_method(cSound, "resonance", rb_bloops_get_resonance, 0);
  rb_define_method(cSound, "resonance=", rb_bloops_set_resonance, 1);
  rb_define_method(cSound, "slide", rb_bloops_get_slide, 0);
  rb_define_method(cSound, "slide=", rb_bloops_set_slide, 1);
  rb_define_method(cSound, "square", rb_bloops_get_square, 0);
  rb_define_method(cSound, "square=", rb_bloops_set_square, 1);
  rb_define_method(cSound, "sweep", rb_bloops_get_sweep, 0);
  rb_define_method(cSound, "sweep=", rb_bloops_set_sweep, 1);
  rb_define_method(cSound, "sustain", rb_bloops_get_sustain, 0);
  rb_define_method(cSound, "sustain=", rb_bloops_set_sustain, 1);
  rb_define_method(cSound, "type", rb_bloops_get_type, 0);
  rb_define_method(cSound, "type=", rb_bloops_set_type, 1);
  rb_define_method(cSound, "vibe", rb_bloops_get_vibe, 0);
  rb_define_method(cSound, "vibe=", rb_bloops_set_vibe, 1);
  rb_define_method(cSound, "vspeed", rb_bloops_get_vspeed, 0);
  rb_define_method(cSound, "vspeed=", rb_bloops_set_vspeed, 1);
  rb_define_method(cSound, "vdelay", rb_bloops_get_vdelay, 0);
  rb_define_method(cSound, "vdelay=", rb_bloops_set_vdelay, 1);
  rb_define_method(cSound, "volume", rb_bloops_get_volume, 0);
  rb_define_method(cSound, "volume=", rb_bloops_set_volume, 1);

  cTrack = rb_define_class_under(cBloops, "Track", rb_cObject);
  rb_define_method(cTrack, "to_s", rb_bloops_track_str, 0);
}
