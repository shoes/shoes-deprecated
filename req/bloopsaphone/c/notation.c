#line 1 "c/notation.rl"
//
// notation.rl
// the musical notation parser
//
// (c) 2009 why the lucky stiff
// See COPYING for the license
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include "bloopsaphone.h"

#define ATOI(X,N) ({ \
  char *Ap = X; \
  int Ai = 0; \
  size_t Al = N; \
  while (Al--) { \
    if ((*Ap >= '0') && (*Ap <= '9')) { \
      Ai = (Ai * 10) + (*Ap - '0'); \
      Ap++; \
    } \
    else break; \
  } \
  Ai; \
})

#define NOTE S->notes[S->nlen]

#define NEXT() \
  NOTE.duration = len; \
  NOTE.octave = oct; \
  mod = 0; \
  tone = 0; \
  len = 4; \
  S->nlen++


#line 42 "c/notation.c"
static const char _bloopnotes_actions[] = {
	0, 1, 0, 1, 3, 1, 4, 1, 
	5, 1, 6, 1, 7, 1, 8, 1, 
	9, 2, 0, 10, 2, 0, 12, 2, 
	0, 13, 2, 3, 12, 2, 4, 13, 
	3, 1, 2, 11, 3, 5, 2, 11, 
	3, 6, 2, 11
};

static const char _bloopnotes_key_offsets[] = {
	0, 0, 11, 13, 16, 17, 17, 19, 
	22, 23, 23, 30, 35, 39, 43, 45
};

static const char _bloopnotes_trans_keys[] = {
	32, 43, 45, 9, 13, 49, 57, 65, 
	71, 97, 103, 49, 57, 58, 48, 57, 
	58, 49, 57, 58, 48, 57, 58, 58, 
	48, 57, 65, 71, 97, 103, 58, 65, 
	71, 97, 103, 65, 71, 97, 103, 35, 
	98, 49, 56, 49, 56, 0
};

static const char _bloopnotes_single_lengths[] = {
	0, 3, 0, 1, 1, 0, 0, 1, 
	1, 0, 1, 1, 0, 2, 0, 0
};

static const char _bloopnotes_range_lengths[] = {
	0, 4, 1, 1, 0, 0, 1, 1, 
	0, 0, 3, 2, 2, 1, 1, 0
};

static const char _bloopnotes_index_offsets[] = {
	0, 0, 8, 10, 13, 15, 16, 18, 
	21, 23, 24, 29, 33, 36, 40, 42
};

static const char _bloopnotes_trans_targs[] = {
	1, 2, 6, 1, 10, 13, 13, 0, 
	3, 1, 5, 4, 1, 5, 1, 1, 
	7, 1, 9, 8, 1, 9, 1, 1, 
	12, 11, 13, 13, 1, 12, 13, 13, 
	1, 13, 13, 1, 14, 14, 15, 1, 
	15, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 0
};

static const char _bloopnotes_trans_actions[] = {
	15, 0, 0, 15, 0, 0, 0, 0, 
	3, 26, 0, 0, 20, 0, 20, 20, 
	5, 29, 0, 0, 23, 0, 23, 23, 
	0, 0, 1, 1, 17, 0, 1, 1, 
	17, 1, 1, 17, 9, 9, 9, 40, 
	7, 36, 32, 26, 20, 20, 20, 29, 
	23, 23, 23, 17, 17, 17, 40, 36, 
	32, 0
};

static const char _bloopnotes_to_state_actions[] = {
	0, 11, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char _bloopnotes_from_state_actions[] = {
	0, 13, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const char _bloopnotes_eof_trans[] = {
	0, 0, 44, 47, 47, 47, 48, 51, 
	51, 51, 54, 54, 54, 55, 56, 57
};

static const int bloopnotes_start = 1;
static const int bloopnotes_error = 0;

static const int bloopnotes_en_main = 1;

#line 109 "c/notation.rl"


bloopsatrack *
bloops_track(bloops *B, bloopsaphone *P, char *track, int tracklen)
{
  int cs, act, oct = 4, len = 4;
  bloopsatrack *S = (bloopsatrack *)malloc(sizeof(bloopsatrack));
  char tone, mod, *p, *pe, *ts, *te, *eof = 0;

  S->P = P;
  S->nlen = 0;
  S->capa = 1024;
  S->notes = (bloopsanote *)calloc(sizeof(bloopsanote), 1024);

  p = track;
  pe = track + tracklen + 1;

  
#line 142 "c/notation.c"
	{
	cs = bloopnotes_start;
	ts = 0;
	te = 0;
	act = 0;
	}
#line 127 "c/notation.rl"
  
#line 151 "c/notation.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _bloopnotes_actions + _bloopnotes_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 8:
#line 1 "c/notation.rl"
	{ts = p;}
	break;
#line 172 "c/notation.c"
		}
	}

	_keys = _bloopnotes_trans_keys + _bloopnotes_key_offsets[cs];
	_trans = _bloopnotes_index_offsets[cs];

	_klen = _bloopnotes_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _bloopnotes_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
_eof_trans:
	cs = _bloopnotes_trans_targs[_trans];

	if ( _bloopnotes_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _bloopnotes_actions + _bloopnotes_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 42 "c/notation.rl"
	{
    len = ATOI(ts, p - ts);
  }
	break;
	case 1:
#line 46 "c/notation.rl"
	{
    oct = ATOI(p - 1, 1);
  }
	break;
	case 2:
#line 50 "c/notation.rl"
	{
    switch (tone) {
      case 'a': case 'A':
        if (mod == 'b')      NOTE.tone = 'a';
        else if (mod == '#') NOTE.tone = 'b';
        else                 NOTE.tone = 'A';
      break;
      case 'b': case 'B':
        if (mod == 'b')      NOTE.tone = 'b';
        else if (mod == '#') NOTE.tone = 'C';
        else                 NOTE.tone = 'B';
      break;
      case 'c': case 'C':
        if (mod == 'b')      NOTE.tone = 'B';
        else if (mod == '#') NOTE.tone = 'd';
        else                 NOTE.tone = 'C';
      break;
      case 'd': case 'D':
        if (mod == 'b')      NOTE.tone = 'd';
        else if (mod == '#') NOTE.tone = 'e';
        else                 NOTE.tone = 'D';
      break;
      case 'e': case 'E':
        if (mod == 'b')      NOTE.tone = 'e';
        else if (mod == '#') NOTE.tone = 'F';
        else                 NOTE.tone = 'E';
      break;
      case 'f': case 'F':
        if (mod == 'b')      NOTE.tone = 'E';
        else if (mod == '#') NOTE.tone = 'g';
        else                 NOTE.tone = 'F';
      break;
      case 'g': case 'G':
        if (mod == 'b')      NOTE.tone = 'g';
        else if (mod == '#') NOTE.tone = 'a';
        else                 NOTE.tone = 'G';
      break;
    }
  }
	break;
	case 3:
#line 91 "c/notation.rl"
	{ len = 1; }
	break;
	case 4:
#line 92 "c/notation.rl"
	{ len = 1; }
	break;
	case 5:
#line 93 "c/notation.rl"
	{ mod = p[-1]; }
	break;
	case 6:
#line 95 "c/notation.rl"
	{ tone = p[-1]; }
	break;
	case 9:
#line 105 "c/notation.rl"
	{te = p+1;}
	break;
	case 10:
#line 98 "c/notation.rl"
	{te = p;p--;{
      NOTE.tone = 0;
      NEXT();
    }}
	break;
	case 11:
#line 102 "c/notation.rl"
	{te = p;p--;{ NEXT(); }}
	break;
	case 12:
#line 103 "c/notation.rl"
	{te = p;p--;{ oct++; len = 4; }}
	break;
	case 13:
#line 104 "c/notation.rl"
	{te = p;p--;{ oct--; len = 4; }}
	break;
#line 330 "c/notation.c"
		}
	}

_again:
	_acts = _bloopnotes_actions + _bloopnotes_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 7:
#line 1 "c/notation.rl"
	{ts = 0;}
	break;
#line 343 "c/notation.c"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _bloopnotes_eof_trans[cs] > 0 ) {
		_trans = _bloopnotes_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}
#line 128 "c/notation.rl"

  return S;
}

bloopsatrack *
bloops_track2(bloops *B, bloopsaphone *P, char *track)
{
  return bloops_track(B, P, track, strlen(track));
}

char *
bloops_track_str(bloopsatrack *track)
{
  char *str = (char *)malloc(sizeof(char) * track->nlen * 6), *ptr = str;
  int i, adv;

  for (i = 0; i < track->nlen; i++)
  {
    if (ptr > str)
      strcat(ptr++, " ");

    if (track->notes[i].duration != 4)
    {
      adv = sprintf(ptr, "%d:", (int)track->notes[i].duration);
      ptr += adv;
    }

    if (track->notes[i].tone)
    {
      char tone[3] = "\0\0\0";
      tone[0] = track->notes[i].tone;
      switch (tone[0]) {
        case 'a': tone[0] = 'A'; tone[1] = 'b'; break;
        case 'b': tone[0] = 'B'; tone[1] = 'b'; break;
        case 'd': tone[0] = 'C'; tone[1] = '#'; break;
        case 'e': tone[0] = 'E'; tone[1] = 'b'; break;
        case 'g': tone[0] = 'F'; tone[1] = '#'; break;
      }
      adv = sprintf(ptr, "%s", tone);
      ptr += adv;

      adv = sprintf(ptr, "%d", (int)track->notes[i].octave);
      ptr += adv;
    }
  }

  return str;
}

float
bloops_note_freq(char note, int octave)
{
  switch (note)
  {
    case 'A': // A
      if (octave <= 0)      return 0.0;
      else if (octave == 1) return 0.121;
      else if (octave == 2) return 0.175;
      else if (octave == 3) return 0.248;
      else if (octave == 4) return 0.353;
      else if (octave == 5) return 0.500;
    break;

    case 'b': // A# or Bb
      if (octave <= 0)      return 0.0;
      else if (octave == 1) return 0.125;
      else if (octave == 2) return 0.181;
      else if (octave == 3) return 0.255;
      else if (octave == 4) return 0.364;
      else if (octave == 5) return 0.515;
    break;

    case 'B': // B
      if (octave <= 0)      return 0.0;
      else if (octave == 1) return 0.129;
      else if (octave == 2) return 0.187;
      else if (octave == 3) return 0.263;
      else if (octave == 4) return 0.374;
      else if (octave == 5) return 0.528;
    break;

    case 'C': // C
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.133;
      else if (octave == 3) return 0.192;
      else if (octave == 4) return 0.271;
      else if (octave == 5) return 0.385;
      else if (octave == 6) return 0.544;
    break;

    case 'd': // C# or Db
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.138;
      else if (octave == 3) return 0.198;
      else if (octave == 4) return 0.279;
      else if (octave == 5) return 0.395;
      else if (octave == 6) return 0.559;
    break;

    case 'D': // D
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.143;
      else if (octave == 3) return 0.202;
      else if (octave == 4) return 0.287;
      else if (octave == 5) return 0.406;
      else if (octave == 6) return 0.575;
    break;

    case 'e': // D# or Eb
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.148;
      else if (octave == 3) return 0.208;
      else if (octave == 4) return 0.296;
      else if (octave == 5) return 0.418;
      else if (octave == 6) return 0.593;
    break;

    case 'E': // E
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.152;
      else if (octave == 3) return 0.214;
      else if (octave == 4) return 0.305;
      else if (octave == 5) return 0.429;
      else if (octave == 6) return 0.608;
    break;

    case 'F': // F
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.155;
      else if (octave == 3) return 0.220;
      else if (octave == 4) return 0.314;
      else if (octave == 5) return 0.441;
    break;

    case 'g': // F# or Gb
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.160;
      else if (octave == 3) return 0.227;
      else if (octave == 4) return 0.323;
      else if (octave == 5) return 0.454;
    break;

    case 'G': // G
      if (octave <= 1)      return 0.0;
      else if (octave == 2) return 0.164;
      else if (octave == 3) return 0.234;
      else if (octave == 4) return 0.332;
      else if (octave == 5) return 0.468;
    break;

    case 'a': // G# or Ab
      if (octave <= 1)      return 0.117;
      else if (octave == 2) return 0.170;
      else if (octave == 3) return 0.242;
      else if (octave == 4) return 0.343;
      else if (octave == 5) return 0.485;
    break;
  }

  return 0.0;
}

#define KEY(name) key = (void *)&P->name


#line 528 "c/notation.c"
static const char _bloopserial_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	5, 1, 6, 1, 7, 1, 8, 1, 
	9, 1, 10, 1, 11, 1, 12, 1, 
	13, 1, 14, 1, 15, 1, 16, 1, 
	17, 1, 18, 1, 19, 1, 20, 1, 
	21, 1, 22, 1, 23, 1, 24, 1, 
	25, 1, 26, 1, 27, 1, 29, 1, 
	30, 1, 31, 1, 32, 1, 33, 1, 
	34, 1, 35, 1, 36, 2, 1, 3, 
	2, 1, 35, 2, 4, 28, 3, 1, 
	3, 35
};

static const unsigned char _bloopserial_key_offsets[] = {
	0, 0, 3, 4, 7, 13, 15, 18, 
	20, 23, 25, 26, 27, 28, 29, 32, 
	33, 34, 35, 36, 39, 41, 42, 43, 
	44, 47, 48, 49, 50, 51, 54, 55, 
	56, 57, 60, 62, 63, 66, 67, 68, 
	69, 70, 73, 76, 77, 78, 79, 82, 
	83, 86, 87, 88, 89, 90, 93, 96, 
	97, 98, 99, 102, 103, 104, 105, 106, 
	109, 110, 111, 112, 115, 116, 118, 119, 
	120, 121, 124, 125, 126, 127, 128, 129, 
	130, 133, 137, 138, 139, 140, 143, 144, 
	145, 146, 147, 150, 151, 152, 153, 154, 
	155, 158, 159, 160, 161, 164, 165, 166, 
	167, 170, 175, 176, 177, 178, 179, 182, 
	183, 184, 185, 186, 187, 188, 189, 190, 
	191, 192, 193, 194, 198, 199, 200, 201, 
	202, 205, 206, 207, 210, 211, 212, 213, 
	214, 217, 218, 219, 220, 221, 224, 237, 
	240, 245, 248
};

static const char _bloopserial_trans_keys[] = {
	114, 115, 116, 112, 32, 9, 13, 32, 
	45, 9, 13, 48, 57, 48, 57, 46, 
	48, 57, 48, 57, 46, 48, 57, 48, 
	57, 112, 101, 101, 100, 32, 9, 13, 
	116, 97, 99, 107, 32, 9, 13, 101, 
	115, 99, 97, 121, 32, 9, 13, 108, 
	105, 100, 101, 32, 9, 13, 114, 101, 
	113, 32, 9, 13, 112, 115, 102, 32, 
	9, 13, 119, 101, 101, 112, 32, 9, 
	13, 105, 112, 115, 109, 105, 116, 32, 
	9, 13, 102, 32, 9, 13, 119, 101, 
	101, 112, 32, 9, 13, 104, 115, 117, 
	97, 115, 101, 32, 9, 13, 119, 101, 
	101, 112, 32, 9, 13, 110, 99, 104, 
	32, 9, 13, 101, 112, 115, 101, 97, 
	116, 32, 9, 13, 111, 110, 97, 110, 
	99, 101, 32, 9, 13, 108, 113, 117, 
	119, 105, 100, 101, 32, 9, 13, 117, 
	97, 114, 101, 32, 9, 13, 115, 116, 
	97, 105, 110, 32, 9, 13, 101, 101, 
	112, 32, 9, 13, 121, 112, 101, 32, 
	9, 13, 32, 110, 115, 9, 13, 111, 
	105, 115, 101, 97, 105, 113, 119, 116, 
	111, 111, 116, 104, 110, 101, 117, 97, 
	114, 101, 100, 105, 111, 115, 101, 108, 
	97, 121, 32, 9, 13, 98, 101, 32, 
	9, 13, 108, 117, 109, 101, 32, 9, 
	13, 112, 101, 101, 100, 32, 9, 13, 
	32, 97, 100, 102, 104, 108, 112, 114, 
	115, 116, 118, 9, 13, 32, 9, 13, 
	32, 9, 13, 48, 57, 32, 9, 13, 
	32, 9, 13, 48, 57, 0
};

static const char _bloopserial_single_lengths[] = {
	0, 3, 1, 1, 2, 0, 1, 0, 
	1, 0, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 2, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 2, 1, 1, 1, 1, 1, 
	1, 1, 3, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 3, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 2, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 4, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 3, 1, 1, 1, 1, 3, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 4, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 11, 1, 
	1, 1, 1
};

static const char _bloopserial_range_lengths[] = {
	0, 0, 0, 1, 2, 1, 1, 1, 
	1, 1, 0, 0, 0, 0, 1, 0, 
	0, 0, 0, 1, 0, 0, 0, 0, 
	1, 0, 0, 0, 0, 1, 0, 0, 
	0, 1, 0, 0, 1, 0, 0, 0, 
	0, 1, 0, 0, 0, 0, 1, 0, 
	1, 0, 0, 0, 0, 1, 0, 0, 
	0, 0, 1, 0, 0, 0, 0, 1, 
	0, 0, 0, 1, 0, 0, 0, 0, 
	0, 1, 0, 0, 0, 0, 0, 0, 
	1, 0, 0, 0, 0, 1, 0, 0, 
	0, 0, 1, 0, 0, 0, 0, 0, 
	1, 0, 0, 0, 1, 0, 0, 0, 
	1, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 0, 1, 0, 0, 0, 0, 
	1, 0, 0, 0, 0, 1, 1, 1, 
	2, 1, 2
};

static const short _bloopserial_index_offsets[] = {
	0, 0, 4, 6, 9, 14, 16, 19, 
	21, 24, 26, 28, 30, 32, 34, 37, 
	39, 41, 43, 45, 48, 51, 53, 55, 
	57, 60, 62, 64, 66, 68, 71, 73, 
	75, 77, 80, 83, 85, 88, 90, 92, 
	94, 96, 99, 103, 105, 107, 109, 112, 
	114, 117, 119, 121, 123, 125, 128, 132, 
	134, 136, 138, 141, 143, 145, 147, 149, 
	152, 154, 156, 158, 161, 163, 166, 168, 
	170, 172, 175, 177, 179, 181, 183, 185, 
	187, 190, 195, 197, 199, 201, 204, 206, 
	208, 210, 212, 215, 217, 219, 221, 223, 
	225, 228, 230, 232, 234, 237, 239, 241, 
	243, 246, 251, 253, 255, 257, 259, 263, 
	265, 267, 269, 271, 273, 275, 277, 279, 
	281, 283, 285, 287, 292, 294, 296, 298, 
	300, 303, 305, 307, 310, 312, 314, 316, 
	318, 321, 323, 325, 327, 329, 332, 345, 
	348, 352, 355
};

static const unsigned char _bloopserial_trans_targs[] = {
	2, 10, 15, 0, 3, 0, 4, 4, 
	0, 4, 5, 4, 8, 0, 6, 0, 
	7, 6, 0, 144, 0, 9, 8, 0, 
	146, 0, 11, 0, 12, 0, 13, 0, 
	14, 0, 4, 4, 0, 16, 0, 17, 
	0, 18, 0, 19, 0, 4, 4, 0, 
	21, 25, 0, 22, 0, 23, 0, 24, 
	0, 4, 4, 0, 26, 0, 27, 0, 
	28, 0, 29, 0, 4, 4, 0, 31, 
	0, 32, 0, 33, 0, 4, 4, 0, 
	35, 37, 0, 36, 0, 4, 4, 0, 
	38, 0, 39, 0, 40, 0, 41, 0, 
	4, 4, 0, 43, 47, 49, 0, 44, 
	0, 45, 0, 46, 0, 4, 4, 0, 
	48, 0, 4, 4, 0, 50, 0, 51, 
	0, 52, 0, 53, 0, 4, 4, 0, 
	55, 59, 64, 0, 56, 0, 57, 0, 
	58, 0, 4, 4, 0, 60, 0, 61, 
	0, 62, 0, 63, 0, 4, 4, 0, 
	65, 0, 66, 0, 67, 0, 4, 4, 
	0, 69, 0, 70, 74, 0, 71, 0, 
	72, 0, 73, 0, 4, 4, 0, 75, 
	0, 76, 0, 77, 0, 78, 0, 79, 
	0, 80, 0, 4, 4, 0, 82, 86, 
	91, 97, 0, 83, 0, 84, 0, 85, 
	0, 4, 4, 0, 87, 0, 88, 0, 
	89, 0, 90, 0, 4, 4, 0, 92, 
	0, 93, 0, 94, 0, 95, 0, 96, 
	0, 4, 4, 0, 98, 0, 99, 0, 
	100, 0, 4, 4, 0, 102, 0, 103, 
	0, 104, 0, 105, 105, 0, 105, 106, 
	110, 105, 0, 107, 0, 108, 0, 109, 
	0, 142, 0, 111, 117, 119, 0, 112, 
	0, 113, 0, 114, 0, 115, 0, 116, 
	0, 142, 0, 118, 0, 142, 0, 120, 
	0, 121, 0, 122, 0, 142, 0, 124, 
	129, 132, 137, 0, 125, 0, 126, 0, 
	127, 0, 128, 0, 4, 4, 0, 130, 
	0, 131, 0, 4, 4, 0, 133, 0, 
	134, 0, 135, 0, 136, 0, 4, 4, 
	0, 138, 0, 139, 0, 140, 0, 141, 
	0, 4, 4, 0, 143, 1, 20, 30, 
	34, 42, 54, 68, 81, 101, 123, 143, 
	0, 143, 143, 142, 145, 145, 144, 142, 
	145, 145, 142, 145, 145, 146, 142, 142, 
	142, 142, 142, 0
};

static const char _bloopserial_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 7, 7, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 0, 5, 0, 1, 0, 0, 
	5, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 9, 9, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 11, 11, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 13, 13, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 15, 15, 0, 0, 
	0, 0, 0, 0, 0, 17, 17, 0, 
	0, 0, 0, 0, 0, 19, 19, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	21, 21, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 23, 23, 0, 
	0, 0, 25, 25, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 27, 27, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 29, 29, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 31, 31, 0, 
	0, 0, 0, 0, 0, 0, 45, 45, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 33, 33, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 35, 35, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 37, 37, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 39, 39, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 41, 41, 0, 0, 0, 0, 0, 
	0, 0, 43, 43, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 63, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 59, 0, 0, 0, 61, 0, 0, 
	0, 0, 0, 0, 0, 57, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 51, 51, 0, 0, 
	0, 0, 0, 47, 47, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 75, 75, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 49, 49, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 67, 69, 69, 0, 78, 
	0, 0, 65, 3, 3, 0, 72, 67, 
	78, 65, 72, 0
};

static const char _bloopserial_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 53, 0, 
	0, 0, 0
};

static const char _bloopserial_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 55, 0, 
	0, 0, 0
};

static const short _bloopserial_eof_trans[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 360, 
	361, 362, 363
};

static const int bloopserial_start = 142;
static const int bloopserial_error = 0;

static const int bloopserial_en_main = 142;

#line 345 "c/notation.rl"


bloopsaphone *
bloops_sound_file(bloops *B, char *fname)
{
  FILE *fp;
  struct stat stats;
  int cs, act, len;
  float fval;
  void *key;
  char *str, *p, *pe, *pf, *ts, *te, *eof = 0;
  bloopsaphone *P;

  if (stat(fname, &stats) == -1)
    return NULL;

  fp = fopen(fname, "rb");
  if (!fp)
    return NULL;

  len = stats.st_size;
  str = (char *)malloc(stats.st_size + 1);
  if (fread(str, 1, stats.st_size, fp) != stats.st_size)
    goto done;

  p = str;
  pe = str + len + 1;
  p[len] = '\0';

  P = bloops_square();
  
#line 867 "c/notation.c"
	{
	cs = bloopserial_start;
	ts = 0;
	te = 0;
	act = 0;
	}
#line 376 "c/notation.rl"
  
#line 876 "c/notation.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _bloopserial_actions + _bloopserial_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 30:
#line 1 "c/notation.rl"
	{ts = p;}
	break;
#line 897 "c/notation.c"
		}
	}

	_keys = _bloopserial_trans_keys + _bloopserial_key_offsets[cs];
	_trans = _bloopserial_index_offsets[cs];

	_klen = _bloopserial_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _bloopserial_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
_eof_trans:
	cs = _bloopserial_trans_targs[_trans];

	if ( _bloopserial_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _bloopserial_actions + _bloopserial_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 299 "c/notation.rl"
	{
    fval = ATOI(ts, p - ts) * 1.0f;
  }
	break;
	case 1:
#line 303 "c/notation.rl"
	{
    fval = ATOI(pf, p - pf) * pow(0.1f, p - pf);
  }
	break;
	case 2:
#line 307 "c/notation.rl"
	{ pf = p; }
	break;
	case 3:
#line 308 "c/notation.rl"
	{ fval *= -1.0f; }
	break;
	case 4:
#line 309 "c/notation.rl"
	{ KEY(volume); }
	break;
	case 5:
#line 310 "c/notation.rl"
	{ KEY(arp); }
	break;
	case 6:
#line 311 "c/notation.rl"
	{ KEY(aspeed); }
	break;
	case 7:
#line 312 "c/notation.rl"
	{ KEY(attack); }
	break;
	case 8:
#line 313 "c/notation.rl"
	{ KEY(decay); }
	break;
	case 9:
#line 314 "c/notation.rl"
	{ KEY(dslide); }
	break;
	case 10:
#line 315 "c/notation.rl"
	{ KEY(freq); }
	break;
	case 11:
#line 316 "c/notation.rl"
	{ KEY(hpf); }
	break;
	case 12:
#line 317 "c/notation.rl"
	{ KEY(hsweep); }
	break;
	case 13:
#line 318 "c/notation.rl"
	{ KEY(limit); }
	break;
	case 14:
#line 319 "c/notation.rl"
	{ KEY(lpf); }
	break;
	case 15:
#line 320 "c/notation.rl"
	{ KEY(lsweep); }
	break;
	case 16:
#line 321 "c/notation.rl"
	{ KEY(phase); }
	break;
	case 17:
#line 322 "c/notation.rl"
	{ KEY(psweep); }
	break;
	case 18:
#line 323 "c/notation.rl"
	{ KEY(repeat); }
	break;
	case 19:
#line 324 "c/notation.rl"
	{ KEY(resonance); }
	break;
	case 20:
#line 325 "c/notation.rl"
	{ KEY(slide); }
	break;
	case 21:
#line 326 "c/notation.rl"
	{ KEY(square); }
	break;
	case 22:
#line 327 "c/notation.rl"
	{ KEY(sustain); }
	break;
	case 23:
#line 328 "c/notation.rl"
	{ KEY(sweep); }
	break;
	case 24:
#line 329 "c/notation.rl"
	{ KEY(punch); }
	break;
	case 25:
#line 330 "c/notation.rl"
	{ KEY(vibe); }
	break;
	case 26:
#line 331 "c/notation.rl"
	{ KEY(vspeed); }
	break;
	case 27:
#line 332 "c/notation.rl"
	{ KEY(vdelay); }
	break;
	case 28:
#line 333 "c/notation.rl"
	{ KEY(volume); }
	break;
	case 31:
#line 337 "c/notation.rl"
	{te = p+1;{ P->type = BLOOPS_SQUARE; }}
	break;
	case 32:
#line 338 "c/notation.rl"
	{te = p+1;{ P->type = BLOOPS_SAWTOOTH; }}
	break;
	case 33:
#line 339 "c/notation.rl"
	{te = p+1;{ P->type = BLOOPS_SINE; }}
	break;
	case 34:
#line 340 "c/notation.rl"
	{te = p+1;{ P->type = BLOOPS_NOISE; }}
	break;
	case 35:
#line 336 "c/notation.rl"
	{te = p;p--;{ *((float *)key) = fval; }}
	break;
	case 36:
#line 341 "c/notation.rl"
	{te = p;p--;}
	break;
#line 1106 "c/notation.c"
		}
	}

_again:
	_acts = _bloopserial_actions + _bloopserial_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 29:
#line 1 "c/notation.rl"
	{ts = 0;}
	break;
#line 1119 "c/notation.c"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _bloopserial_eof_trans[cs] > 0 ) {
		_trans = _bloopserial_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}
#line 377 "c/notation.rl"

done:
  fclose(fp);
  return P;
}

char *
bloops_sound_str(bloops *B, bloopsaphone *P)
{
  char *lines = (char *)malloc(4096), *str = lines;
  bloopsaphone *sq = bloops_square();
  if (P->type == BLOOPS_SQUARE)
    str += sprintf(str, "type square\n");
  else if (P->type == BLOOPS_SAWTOOTH)
    str += sprintf(str, "type sawtooth\n");
  else if (P->type == BLOOPS_SINE)
    str += sprintf(str, "type sine\n");
  else if (P->type == BLOOPS_NOISE)
    str += sprintf(str, "type noise\n");

  if (P->volume != sq->volume)
    str += sprintf(str, "volume %0.3f\n", P->volume);
  if (P->punch != sq->punch)
    str += sprintf(str, "punch %0.3f\n", P->punch);
  if (P->attack != sq->attack)
    str += sprintf(str, "attack %0.3f\n", P->attack);
  if (P->sustain != sq->sustain)
    str += sprintf(str, "sustain %0.3f\n", P->sustain);
  if (P->decay != sq->decay)
    str += sprintf(str, "decay %0.3f\n", P->decay);
  if (P->freq != sq->freq)
    str += sprintf(str, "freq %0.3f\n", P->freq);
  if (P->limit != sq->limit)
    str += sprintf(str, "limit %0.3f\n", P->limit);
  if (P->slide != sq->slide)
    str += sprintf(str, "slide %0.3f\n", P->slide);
  if (P->dslide != sq->dslide)
    str += sprintf(str, "dslide %0.3f\n", P->dslide);
  if (P->square != sq->square)
    str += sprintf(str, "square %0.3f\n", P->square);
  if (P->sweep != sq->sweep)
    str += sprintf(str, "sweep %0.3f\n", P->sweep);
  if (P->vibe != sq->vibe)
    str += sprintf(str, "vibe %0.3f\n", P->vibe);
  if (P->vspeed != sq->vspeed)
    str += sprintf(str, "vspeed %0.3f\n", P->vspeed);
  if (P->vdelay != sq->vdelay)
    str += sprintf(str, "vdelay %0.3f\n", P->vdelay);
  if (P->lpf != sq->lpf)
    str += sprintf(str, "lpf %0.3f\n", P->lpf);
  if (P->lsweep != sq->lsweep)
    str += sprintf(str, "lsweep %0.3f\n", P->lsweep);
  if (P->resonance != sq->resonance)
    str += sprintf(str, "resonance %0.3f\n", P->resonance);
  if (P->hpf != sq->hpf)
    str += sprintf(str, "hpf %0.3f\n", P->hpf);
  if (P->hsweep != sq->hsweep)
    str += sprintf(str, "hsweep %0.3f\n", P->hsweep);
  if (P->arp != sq->arp)
    str += sprintf(str, "arp %0.3f\n", P->arp);
  if (P->aspeed != sq->aspeed)
    str += sprintf(str, "aspeed %0.3f\n", P->aspeed);
  if (P->phase != sq->phase)
    str += sprintf(str, "phase %0.3f\n", P->phase);
  if (P->psweep != sq->psweep)
    str += sprintf(str, "psweep %0.3f\n", P->psweep);
  if (P->repeat != sq->repeat)
    str += sprintf(str, "repeat %0.3f\n", P->repeat);

  free(sq);
  return lines;
}
