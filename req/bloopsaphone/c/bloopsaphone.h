//
// bloopsaphone.h
// the chiptune maker for portaudio
//
#ifndef BLOOPSAPHONE_H
#define BLOOPSAPHONE_H

#define BLOOPSAPHONE_VERSION "1.0"

#define BLOOPS_STOP 0
#define BLOOPS_PLAY 1
#define BLOOPS_MUTE 2

#define BLOOPS_SQUARE   0
#define BLOOPS_SAWTOOTH 1
#define BLOOPS_SINE     2
#define BLOOPS_NOISE    3

typedef struct {
  unsigned char type, pan;
  float volume;
  float punch;
  float attack;
  float sustain;
  float decay;
  float freq, limit, slide, dslide; // pitch
  float square, sweep;              // square wave
  float vibe, vspeed, vdelay;       // vibrato
  float lpf, lsweep, resonance, hpf, hsweep;
                                    // hi-pass, lo-pass
  float arp, aspeed;                // arpeggiator
  float phase, psweep;              // phaser
  float repeat;                     // repeats?
} bloopsaphone;

#define BLOOPS_HI_OCTAVE 8

typedef struct {
  char tone, octave, duration;
} bloopsanote;

typedef struct {
  bloopsaphone *P;
  int nlen, capa;
  bloopsanote *notes;

  int frames, nextnote[2];
  float volume, freq;
  unsigned char playing;
  int stage, time, length[3];
  double period, maxperiod, slide, dslide;
  float square, sweep;
  int phase, iphase, phasex;
  float fphase, dphase;
  float phaser[1024];
  float noise[32];
  float filter[8];
  float vibe, vspeed, vdelay;
  int repeat, limit;
  double arp;
  int atime, alimit;
} bloopsatrack;

#define BLOOPS_MAX_TRACKS 64
#define BLOOPS_MAX_CHANNELS 64

typedef struct {
  int tempo;
  float volume;
  bloopsatrack *tracks[BLOOPS_MAX_TRACKS];
  unsigned char play;
} bloops;

typedef struct {
  bloops *B[BLOOPS_MAX_CHANNELS];
  void *stream;
} bloopsmix;

//
// the api
//
bloops *bloops_new();
void bloops_destroy(bloops *);
bloopsaphone *bloops_square();
bloopsaphone *bloops_load(char *);
void bloops_clear(bloops *);
void bloops_tempo(bloops *, int tempo);
void bloops_track_at(bloops *, bloopsatrack *, int);
void bloops_track_destroy(bloopsatrack *);
void bloops_play(bloops *);
void bloops_stop(bloops *);
int bloops_is_done(bloops *);
bloopsatrack *bloops_track(bloops *, bloopsaphone *, char *, int);
bloopsatrack *bloops_track2(bloops *, bloopsaphone *, char *);
char *bloops_track_str(bloopsatrack *);
float bloops_note_freq(char, int);
bloopsaphone *bloops_sound_file(bloops *, char *);
char *bloops_sound_str(bloops *, bloopsaphone *);
 
#endif
