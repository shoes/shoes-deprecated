#line 1 "ext/hpricot_scan/hpricot_scan.rl"
/*
 * hpricot_scan.rl
 *
 * $Author: why $
 * $Date: 2006-05-08 22:03:50 -0600 (Mon, 08 May 2006) $
 *
 * Copyright (C) 2006 why the lucky stiff
 */
#include <ruby.h>

#define NO_WAY_SERIOUSLY "*** This should not happen, please send a bug report with the HTML you're parsing to why@whytheluckystiff.net.  So sorry!"

static VALUE sym_xmldecl, sym_doctype, sym_procins, sym_stag, sym_etag, sym_emptytag, sym_comment,
      sym_cdata, sym_text;
static VALUE rb_eHpricotParseError;
static ID s_read, s_to_str;

#define ELE(N) \
  if (tokend > tokstart || text == 1) { \
    VALUE raw_string = Qnil; \
    ele_open = 0; text = 0; \
    if (tokstart != 0 && sym_##N != sym_cdata && sym_##N != sym_text && sym_##N != sym_procins && sym_##N != sym_comment) { \
      raw_string = rb_str_new(tokstart, tokend-tokstart); \
    } \
    rb_yield_tokens(sym_##N, tag, attr, raw_string, taint); \
  }

#define SET(N, E) \
  if (mark_##N == NULL || E == mark_##N) \
    N = rb_str_new2(""); \
  else if (E > mark_##N) \
    N = rb_str_new(mark_##N, E - mark_##N);

#define CAT(N, E) if (NIL_P(N)) { SET(N, E); } else { rb_str_cat(N, mark_##N, E - mark_##N); }

#define SLIDE(N) if ( mark_##N > tokstart ) mark_##N = buf + (mark_##N - tokstart);

#define ATTR(K, V) \
    if (!NIL_P(K)) { \
      if (NIL_P(attr)) attr = rb_hash_new(); \
      rb_hash_aset(attr, K, V); \
    }

#define TEXT_PASS() \
    if (text == 0) \
    { \
      if (ele_open == 1) { \
        ele_open = 0; \
        if (tokstart > 0) { \
          mark_tag = tokstart; \
        } \
      } else { \
        mark_tag = p; \
      } \
      attr = Qnil; \
      tag = Qnil; \
      text = 1; \
    }

#define EBLK(N, T) CAT(tag, p - T + 1); ELE(N);

#line 107 "ext/hpricot_scan/hpricot_scan.rl"



#line 68 "ext/hpricot_scan/hpricot_scan.c"
static const int hpricot_scan_start = 204;

static const int hpricot_scan_error = -1;

#line 110 "ext/hpricot_scan/hpricot_scan.rl"

#define BUFSIZE 16384

void rb_yield_tokens(VALUE sym, VALUE tag, VALUE attr, VALUE raw, int taint)
{
  VALUE ary;
  if (sym == sym_text) {
    raw = tag;
  }
  ary = rb_ary_new3(4, sym, tag, attr, raw);
  if (taint) { 
    OBJ_TAINT(ary);
    OBJ_TAINT(tag);
    OBJ_TAINT(attr);
    OBJ_TAINT(raw);
  }
  rb_yield(ary);
}

VALUE hpricot_scan(VALUE self, VALUE port)
{
  int cs, act, have = 0, nread = 0, curline = 1, text = 0;
  char *tokstart = 0, *tokend = 0, *buf = NULL;

  VALUE attr = Qnil, tag = Qnil, akey = Qnil, aval = Qnil, bufsize = Qnil;
  char *mark_tag = 0, *mark_akey = 0, *mark_aval = 0;
  int done = 0, ele_open = 0, buffer_size = 0;

  int taint = OBJ_TAINTED( port );
  if ( !rb_respond_to( port, s_read ) )
  {
    if ( rb_respond_to( port, s_to_str ) )
    {
      port = rb_funcall( port, s_to_str, 0 );
      StringValue(port);
    }
    else
    {
      rb_raise( rb_eArgError, "bad Hpricot argument, String or IO only please." );
    }
  }

  buffer_size = BUFSIZE;
  if (rb_ivar_defined(self, rb_intern("@buffer_size")) == Qtrue) {
    bufsize = rb_ivar_get(self, rb_intern("@buffer_size"));
    if (!NIL_P(bufsize)) {
      buffer_size = NUM2INT(bufsize);
    }
  }
  buf = ALLOC_N(char, buffer_size);

  
#line 126 "ext/hpricot_scan/hpricot_scan.c"
	{
	cs = hpricot_scan_start;
	tokstart = 0;
	tokend = 0;
	act = 0;
	}
#line 162 "ext/hpricot_scan/hpricot_scan.rl"
  
  while ( !done ) {
    VALUE str;
    char *p = buf + have, *pe;
    int len, space = buffer_size - have;

    if ( space == 0 ) {
      /* We've used up the entire buffer storing an already-parsed token
       * prefix that must be preserved.  Likely caused by super-long attributes.
       * See ticket #13. */
      rb_raise(rb_eHpricotParseError, "ran out of buffer space on element <%s>, starting on line %d.", RSTRING(tag)->ptr, curline);
    }

    if ( rb_respond_to( port, s_read ) )
    {
      str = rb_funcall( port, s_read, 1, INT2FIX(space) );
    }
    else
    {
      str = rb_str_substr( port, nread, space );
    }

    StringValue(str);
    memcpy( p, RSTRING(str)->ptr, RSTRING(str)->len );
    len = RSTRING(str)->len;
    nread += len;

    /* If this is the last buffer, tack on an EOF. */
    if ( len < space ) {
      p[len++] = 0;
      done = 1;
    }

    pe = p + len;
    
#line 169 "ext/hpricot_scan/hpricot_scan.c"
	{
	if ( p == pe )
		goto _out;
	switch ( cs )
	{
tr14:
#line 67 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p;{ {{p = ((tokend))-1;}{goto st218;}} }{p = ((tokend))-1;}}
	goto st204;
tr18:
#line 73 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st204;
tr23:
#line 73 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st204;
tr24:
#line 9 "ext/hpricot_scan/hpricot_scan.rl"
	{curline += 1;}
#line 73 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st204;
tr69:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{	switch( act ) {
	case 8:
	{ ELE(doctype); }
	break;
	case 10:
	{ ELE(stag); }
	break;
	case 12:
	{ ELE(emptytag); }
	break;
	case 15:
	{ TEXT_PASS(); }
	break;
	default: break;
	}
	{p = ((tokend))-1;}}
	goto st204;
tr70:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr76:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr137:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr162:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr264:
#line 67 "ext/hpricot_scan/hpricot_scan.rl"
	{{ {{p = ((tokend))-1;}{goto st218;}} }{p = ((tokend))-1;}}
	goto st204;
tr270:
#line 65 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(xmldecl); }{p = ((tokend))-1;}}
	goto st204;
tr296:
#line 73 "ext/hpricot_scan/hpricot_scan.rl"
	{{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st204;
tr302:
#line 66 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(doctype); }{p = ((tokend))-1;}}
	goto st204;
tr314:
#line 69 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(etag); }{p = ((tokend))-1;}}
	goto st204;
tr318:
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr327:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr330:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
#line 66 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(doctype); }{p = ((tokend))-1;}}
	goto st204;
tr334:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
#line 69 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(etag); }{p = ((tokend))-1;}}
	goto st204;
tr355:
#line 72 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ {{p = ((tokend))-1;}{goto st216;}} }{p = ((tokend))-1;}}
	goto st204;
tr356:
#line 71 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ {{p = ((tokend))-1;}{goto st214;}} }{p = ((tokend))-1;}}
	goto st204;
tr368:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(stag); }{p = ((tokend))-1;}}
	goto st204;
tr369:
#line 70 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ ELE(emptytag); }{p = ((tokend))-1;}}
	goto st204;
st204:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = 0;}
	if ( ++p == pe )
		goto _out204;
case 204:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = p;}
#line 333 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 10: goto tr24;
		case 60: goto tr25;
	}
	goto tr23;
tr25:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 65 "ext/hpricot_scan/hpricot_scan.rl"
	{
    if (text == 1) {
      CAT(tag, p);
      ELE(text);
      text = 0;
    }
    attr = Qnil;
    tag = Qnil;
    mark_tag = NULL;
    ele_open = 1;
  }
#line 73 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 15;}
	goto st205;
st205:
	if ( ++p == pe )
		goto _out205;
case 205:
#line 361 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 33: goto st0;
		case 47: goto st59;
		case 58: goto tr21;
		case 63: goto st145;
		case 95: goto tr21;
	}
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr21;
	} else if ( (*p) >= 65 )
		goto tr21;
	goto tr18;
st0:
	if ( ++p == pe )
		goto _out0;
case 0:
	switch( (*p) ) {
		case 45: goto st1;
		case 68: goto st2;
		case 91: goto st53;
	}
	goto tr296;
st1:
	if ( ++p == pe )
		goto _out1;
case 1:
	if ( (*p) == 45 )
		goto tr356;
	goto tr296;
st2:
	if ( ++p == pe )
		goto _out2;
case 2:
	if ( (*p) == 79 )
		goto st3;
	goto tr296;
st3:
	if ( ++p == pe )
		goto _out3;
case 3:
	if ( (*p) == 67 )
		goto st4;
	goto tr296;
st4:
	if ( ++p == pe )
		goto _out4;
case 4:
	if ( (*p) == 84 )
		goto st5;
	goto tr296;
st5:
	if ( ++p == pe )
		goto _out5;
case 5:
	if ( (*p) == 89 )
		goto st6;
	goto tr296;
st6:
	if ( ++p == pe )
		goto _out6;
case 6:
	if ( (*p) == 80 )
		goto st7;
	goto tr296;
st7:
	if ( ++p == pe )
		goto _out7;
case 7:
	if ( (*p) == 69 )
		goto st8;
	goto tr296;
st8:
	if ( ++p == pe )
		goto _out8;
case 8:
	if ( (*p) == 32 )
		goto st9;
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st9;
	goto tr296;
st9:
	if ( ++p == pe )
		goto _out9;
case 9:
	switch( (*p) ) {
		case 32: goto st9;
		case 58: goto tr309;
		case 95: goto tr309;
	}
	if ( (*p) < 65 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st9;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr309;
	} else
		goto tr309;
	goto tr296;
tr309:
#line 77 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_tag = p; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _out10;
case 10:
#line 469 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr328;
		case 62: goto tr330;
		case 63: goto st10;
		case 91: goto tr331;
		case 95: goto st10;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st10;
		} else if ( (*p) >= 9 )
			goto tr328;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st10;
		} else if ( (*p) >= 65 )
			goto st10;
	} else
		goto st10;
	goto tr296;
tr328:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
	goto st11;
st11:
	if ( ++p == pe )
		goto _out11;
case 11:
#line 500 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st11;
		case 62: goto tr302;
		case 80: goto st12;
		case 83: goto st48;
		case 91: goto st26;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st11;
	goto tr296;
st12:
	if ( ++p == pe )
		goto _out12;
case 12:
	if ( (*p) == 85 )
		goto st13;
	goto tr296;
st13:
	if ( ++p == pe )
		goto _out13;
case 13:
	if ( (*p) == 66 )
		goto st14;
	goto tr296;
st14:
	if ( ++p == pe )
		goto _out14;
case 14:
	if ( (*p) == 76 )
		goto st15;
	goto tr296;
st15:
	if ( ++p == pe )
		goto _out15;
case 15:
	if ( (*p) == 73 )
		goto st16;
	goto tr296;
st16:
	if ( ++p == pe )
		goto _out16;
case 16:
	if ( (*p) == 67 )
		goto st17;
	goto tr296;
st17:
	if ( ++p == pe )
		goto _out17;
case 17:
	if ( (*p) == 32 )
		goto st18;
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st18;
	goto tr296;
st18:
	if ( ++p == pe )
		goto _out18;
case 18:
	switch( (*p) ) {
		case 32: goto st18;
		case 34: goto st19;
		case 39: goto st30;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st18;
	goto tr296;
st19:
	if ( ++p == pe )
		goto _out19;
case 19:
	switch( (*p) ) {
		case 9: goto tr321;
		case 34: goto tr320;
		case 61: goto tr321;
		case 95: goto tr321;
	}
	if ( (*p) < 39 ) {
		if ( 32 <= (*p) && (*p) <= 37 )
			goto tr321;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr321;
		} else if ( (*p) >= 63 )
			goto tr321;
	} else
		goto tr321;
	goto tr296;
tr321:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st20;
st20:
	if ( ++p == pe )
		goto _out20;
case 20:
#line 597 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 9: goto st20;
		case 34: goto tr320;
		case 61: goto st20;
		case 95: goto st20;
	}
	if ( (*p) < 39 ) {
		if ( 32 <= (*p) && (*p) <= 37 )
			goto st20;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st20;
		} else if ( (*p) >= 63 )
			goto st20;
	} else
		goto st20;
	goto tr296;
tr320:
#line 91 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("public_id"), aval); }
	goto st21;
st21:
	if ( ++p == pe )
		goto _out21;
case 21:
#line 624 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st22;
		case 62: goto tr302;
		case 91: goto st26;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st22;
	goto tr296;
st22:
	if ( ++p == pe )
		goto _out22;
case 22:
	switch( (*p) ) {
		case 32: goto st22;
		case 34: goto st23;
		case 39: goto st28;
		case 62: goto tr302;
		case 91: goto st26;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st22;
	goto tr296;
st23:
	if ( ++p == pe )
		goto _out23;
case 23:
	if ( (*p) == 34 )
		goto tr6;
	goto tr222;
tr222:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st24;
st24:
	if ( ++p == pe )
		goto _out24;
case 24:
#line 662 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 34 )
		goto tr6;
	goto st24;
tr6:
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st25;
st25:
	if ( ++p == pe )
		goto _out25;
case 25:
#line 674 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st25;
		case 62: goto tr302;
		case 91: goto st26;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st25;
	goto tr69;
tr331:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
	goto st26;
st26:
	if ( ++p == pe )
		goto _out26;
case 26:
#line 691 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 93 )
		goto st27;
	goto st26;
st27:
	if ( ++p == pe )
		goto _out27;
case 27:
	switch( (*p) ) {
		case 32: goto st27;
		case 62: goto tr302;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st27;
	goto tr69;
st28:
	if ( ++p == pe )
		goto _out28;
case 28:
	if ( (*p) == 39 )
		goto tr6;
	goto tr182;
tr182:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st29;
st29:
	if ( ++p == pe )
		goto _out29;
case 29:
#line 721 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 39 )
		goto tr6;
	goto st29;
st30:
	if ( ++p == pe )
		goto _out30;
case 30:
	switch( (*p) ) {
		case 9: goto tr322;
		case 39: goto tr323;
		case 61: goto tr322;
		case 95: goto tr322;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto tr322;
		} else if ( (*p) >= 32 )
			goto tr322;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr322;
		} else if ( (*p) >= 63 )
			goto tr322;
	} else
		goto tr322;
	goto tr296;
tr322:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st31;
st31:
	if ( ++p == pe )
		goto _out31;
case 31:
#line 758 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 9: goto st31;
		case 39: goto tr303;
		case 61: goto st31;
		case 95: goto st31;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st31;
		} else if ( (*p) >= 32 )
			goto st31;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 63 )
			goto st31;
	} else
		goto st31;
	goto tr296;
tr42:
#line 91 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("public_id"), aval); }
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st32;
tr303:
#line 91 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("public_id"), aval); }
	goto st32;
tr323:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 91 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("public_id"), aval); }
	goto st32;
st32:
	if ( ++p == pe )
		goto _out32;
case 32:
#line 800 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 9: goto st33;
		case 32: goto st33;
		case 33: goto st31;
		case 39: goto tr303;
		case 62: goto tr302;
		case 91: goto st26;
		case 95: goto st31;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 13 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st31;
		} else if ( (*p) >= 10 )
			goto st22;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 61 )
			goto st31;
	} else
		goto st31;
	goto tr296;
st33:
	if ( ++p == pe )
		goto _out33;
case 33:
	switch( (*p) ) {
		case 9: goto st33;
		case 32: goto st33;
		case 34: goto st23;
		case 39: goto tr301;
		case 62: goto tr302;
		case 91: goto st26;
		case 95: goto st31;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 13 ) {
			if ( 33 <= (*p) && (*p) <= 37 )
				goto st31;
		} else if ( (*p) >= 10 )
			goto st22;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st31;
		} else if ( (*p) >= 61 )
			goto st31;
	} else
		goto st31;
	goto tr296;
tr44:
#line 91 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("public_id"), aval); }
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st34;
tr301:
#line 91 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("public_id"), aval); }
	goto st34;
st34:
	if ( ++p == pe )
		goto _out34;
case 34:
#line 867 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 9: goto tr186;
		case 32: goto tr186;
		case 33: goto tr188;
		case 39: goto tr42;
		case 62: goto tr184;
		case 91: goto tr185;
		case 95: goto tr188;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 13 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto tr188;
		} else if ( (*p) >= 10 )
			goto tr187;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr188;
		} else if ( (*p) >= 61 )
			goto tr188;
	} else
		goto tr188;
	goto tr182;
tr186:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st35;
st35:
	if ( ++p == pe )
		goto _out35;
case 35:
#line 900 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 9: goto st35;
		case 32: goto st35;
		case 34: goto st37;
		case 39: goto tr44;
		case 62: goto tr40;
		case 91: goto st40;
		case 95: goto st47;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 13 ) {
			if ( 33 <= (*p) && (*p) <= 37 )
				goto st47;
		} else if ( (*p) >= 10 )
			goto st36;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st47;
		} else if ( (*p) >= 61 )
			goto st47;
	} else
		goto st47;
	goto st29;
tr187:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st36;
st36:
	if ( ++p == pe )
		goto _out36;
case 36:
#line 933 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st36;
		case 34: goto st37;
		case 39: goto tr39;
		case 62: goto tr40;
		case 91: goto st40;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st36;
	goto st29;
st37:
	if ( ++p == pe )
		goto _out37;
case 37:
	switch( (*p) ) {
		case 34: goto tr63;
		case 39: goto tr224;
	}
	goto tr223;
tr223:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st38;
st38:
	if ( ++p == pe )
		goto _out38;
case 38:
#line 961 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr63;
		case 39: goto tr64;
	}
	goto st38;
tr63:
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st39;
tr183:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st39;
st39:
	if ( ++p == pe )
		goto _out39;
case 39:
#line 979 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st39;
		case 39: goto tr6;
		case 62: goto tr40;
		case 91: goto st40;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st39;
	goto st29;
tr40:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 66 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 8;}
	goto st206;
tr184:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 66 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 8;}
	goto st206;
st206:
	if ( ++p == pe )
		goto _out206;
case 206:
#line 1007 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 39 )
		goto tr6;
	goto st29;
tr185:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st40;
st40:
	if ( ++p == pe )
		goto _out40;
case 40:
#line 1019 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr35;
		case 93: goto st42;
	}
	goto st40;
tr35:
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st41;
st41:
	if ( ++p == pe )
		goto _out41;
case 41:
#line 1033 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st41;
		case 62: goto tr27;
		case 93: goto st27;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st41;
	goto st26;
tr27:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 66 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 8;}
	goto st207;
st207:
	if ( ++p == pe )
		goto _out207;
case 207:
#line 1052 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 93 )
		goto st27;
	goto st26;
st42:
	if ( ++p == pe )
		goto _out42;
case 42:
	switch( (*p) ) {
		case 32: goto st42;
		case 39: goto tr6;
		case 62: goto tr40;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st42;
	goto st29;
tr64:
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st43;
tr224:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st43;
st43:
	if ( ++p == pe )
		goto _out43;
case 43:
#line 1082 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st43;
		case 34: goto tr6;
		case 62: goto tr61;
		case 91: goto st44;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st43;
	goto st24;
tr61:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 66 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 8;}
	goto st208;
st208:
	if ( ++p == pe )
		goto _out208;
case 208:
#line 1102 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 34 )
		goto tr6;
	goto st24;
st44:
	if ( ++p == pe )
		goto _out44;
case 44:
	switch( (*p) ) {
		case 34: goto tr35;
		case 93: goto st45;
	}
	goto st44;
st45:
	if ( ++p == pe )
		goto _out45;
case 45:
	switch( (*p) ) {
		case 32: goto st45;
		case 34: goto tr6;
		case 62: goto tr61;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st45;
	goto st24;
tr39:
#line 92 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("system_id"), aval); }
	goto st46;
st46:
	if ( ++p == pe )
		goto _out46;
case 46:
#line 1135 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr183;
		case 39: goto tr6;
		case 62: goto tr184;
		case 91: goto tr185;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr183;
	goto tr182;
tr188:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st47;
st47:
	if ( ++p == pe )
		goto _out47;
case 47:
#line 1153 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 9: goto st47;
		case 39: goto tr42;
		case 61: goto st47;
		case 95: goto st47;
	}
	if ( (*p) < 40 ) {
		if ( (*p) > 33 ) {
			if ( 35 <= (*p) && (*p) <= 37 )
				goto st47;
		} else if ( (*p) >= 32 )
			goto st47;
	} else if ( (*p) > 59 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st47;
		} else if ( (*p) >= 63 )
			goto st47;
	} else
		goto st47;
	goto st29;
st48:
	if ( ++p == pe )
		goto _out48;
case 48:
	if ( (*p) == 89 )
		goto st49;
	goto tr296;
st49:
	if ( ++p == pe )
		goto _out49;
case 49:
	if ( (*p) == 83 )
		goto st50;
	goto tr296;
st50:
	if ( ++p == pe )
		goto _out50;
case 50:
	if ( (*p) == 84 )
		goto st51;
	goto tr296;
st51:
	if ( ++p == pe )
		goto _out51;
case 51:
	if ( (*p) == 69 )
		goto st52;
	goto tr296;
st52:
	if ( ++p == pe )
		goto _out52;
case 52:
	if ( (*p) == 77 )
		goto st21;
	goto tr296;
st53:
	if ( ++p == pe )
		goto _out53;
case 53:
	if ( (*p) == 67 )
		goto st54;
	goto tr296;
st54:
	if ( ++p == pe )
		goto _out54;
case 54:
	if ( (*p) == 68 )
		goto st55;
	goto tr296;
st55:
	if ( ++p == pe )
		goto _out55;
case 55:
	if ( (*p) == 65 )
		goto st56;
	goto tr296;
st56:
	if ( ++p == pe )
		goto _out56;
case 56:
	if ( (*p) == 84 )
		goto st57;
	goto tr296;
st57:
	if ( ++p == pe )
		goto _out57;
case 57:
	if ( (*p) == 65 )
		goto st58;
	goto tr296;
st58:
	if ( ++p == pe )
		goto _out58;
case 58:
	if ( (*p) == 91 )
		goto tr355;
	goto tr296;
st59:
	if ( ++p == pe )
		goto _out59;
case 59:
	switch( (*p) ) {
		case 58: goto tr338;
		case 95: goto tr338;
	}
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr338;
	} else if ( (*p) >= 65 )
		goto tr338;
	goto tr296;
tr338:
#line 77 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_tag = p; }
	goto st60;
st60:
	if ( ++p == pe )
		goto _out60;
case 60:
#line 1274 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr332;
		case 62: goto tr334;
		case 63: goto st60;
		case 95: goto st60;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st60;
		} else if ( (*p) >= 9 )
			goto tr332;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st60;
		} else if ( (*p) >= 65 )
			goto st60;
	} else
		goto st60;
	goto tr296;
tr332:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
	goto st61;
st61:
	if ( ++p == pe )
		goto _out61;
case 61:
#line 1304 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st61;
		case 62: goto tr314;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st61;
	goto tr296;
tr21:
#line 77 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_tag = p; }
	goto st62;
st62:
	if ( ++p == pe )
		goto _out62;
case 62:
#line 1320 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr324;
		case 47: goto tr326;
		case 62: goto tr327;
		case 63: goto st62;
		case 95: goto st62;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr324;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st62;
		} else if ( (*p) >= 65 )
			goto st62;
	} else
		goto st62;
	goto tr296;
tr324:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
	goto st63;
st63:
	if ( ++p == pe )
		goto _out63;
case 63:
#line 1348 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st63;
		case 47: goto st66;
		case 62: goto tr318;
		case 63: goto tr316;
		case 95: goto tr316;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st63;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr316;
		} else if ( (*p) >= 65 )
			goto tr316;
	} else
		goto tr316;
	goto tr296;
tr360:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st64;
tr316:
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st64;
st64:
	if ( ++p == pe )
		goto _out64;
case 64:
#line 1398 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr364;
		case 47: goto tr366;
		case 61: goto tr367;
		case 62: goto tr368;
		case 63: goto st64;
		case 95: goto st64;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr364;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st64;
		} else if ( (*p) >= 65 )
			goto st64;
	} else
		goto st64;
	goto tr69;
tr71:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st65;
tr364:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st65;
tr132:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st65;
st65:
	if ( ++p == pe )
		goto _out65;
case 65:
#line 1443 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st65;
		case 47: goto tr361;
		case 61: goto st67;
		case 62: goto tr162;
		case 63: goto tr360;
		case 95: goto tr360;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st65;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr360;
		} else if ( (*p) >= 65 )
			goto tr360;
	} else
		goto tr360;
	goto tr69;
tr361:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st66;
tr366:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st66;
tr326:
#line 80 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(tag, p); }
	goto st66;
st66:
	if ( ++p == pe )
		goto _out66;
case 66:
#line 1486 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 62 )
		goto tr369;
	goto tr69;
tr367:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st67;
st67:
	if ( ++p == pe )
		goto _out67;
case 67:
#line 1498 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr157;
		case 32: goto tr157;
		case 34: goto st142;
		case 39: goto st143;
		case 47: goto tr161;
		case 60: goto tr69;
		case 62: goto tr162;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr158;
	} else if ( (*p) >= 9 )
		goto tr157;
	goto tr156;
tr156:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st68;
st68:
	if ( ++p == pe )
		goto _out68;
case 68:
#line 1522 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr66;
		case 32: goto tr66;
		case 47: goto tr68;
		case 60: goto tr69;
		case 62: goto tr70;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr67;
	} else if ( (*p) >= 9 )
		goto tr66;
	goto st68;
tr3:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st69;
tr66:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st69;
st69:
	if ( ++p == pe )
		goto _out69;
case 69:
#line 1551 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st69;
		case 47: goto tr361;
		case 62: goto tr162;
		case 63: goto tr360;
		case 95: goto tr360;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st69;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr360;
		} else if ( (*p) >= 65 )
			goto tr360;
	} else
		goto tr360;
	goto tr69;
tr84:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st70;
tr67:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st70;
st70:
	if ( ++p == pe )
		goto _out70;
case 70:
#line 1586 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr66;
		case 32: goto tr66;
		case 47: goto tr74;
		case 60: goto tr69;
		case 62: goto tr76;
		case 63: goto tr73;
		case 95: goto tr73;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr67;
		} else if ( (*p) >= 9 )
			goto tr66;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr73;
		} else if ( (*p) >= 65 )
			goto tr73;
	} else
		goto tr73;
	goto st68;
tr73:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st71;
tr165:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st71;
st71:
	if ( ++p == pe )
		goto _out71;
case 71:
#line 1647 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr132;
		case 32: goto tr132;
		case 47: goto tr135;
		case 60: goto tr69;
		case 61: goto tr136;
		case 62: goto tr137;
		case 63: goto st71;
		case 95: goto st71;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr133;
		} else if ( (*p) >= 9 )
			goto tr132;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st71;
		} else if ( (*p) >= 65 )
			goto st71;
	} else
		goto st71;
	goto st68;
tr72:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st72;
tr133:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st72;
st72:
	if ( ++p == pe )
		goto _out72;
case 72:
#line 1693 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr71;
		case 32: goto tr71;
		case 47: goto tr74;
		case 60: goto tr69;
		case 61: goto st74;
		case 62: goto tr76;
		case 63: goto tr73;
		case 95: goto tr73;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr72;
		} else if ( (*p) >= 9 )
			goto tr71;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr73;
		} else if ( (*p) >= 65 )
			goto tr73;
	} else
		goto tr73;
	goto st68;
tr68:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st73;
tr74:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st73;
tr135:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st73;
tr161:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st73;
tr230:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st73;
tr231:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st73;
st73:
	if ( ++p == pe )
		goto _out73;
case 73:
#line 1792 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr66;
		case 32: goto tr66;
		case 47: goto tr68;
		case 60: goto tr69;
		case 62: goto tr70;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr67;
	} else if ( (*p) >= 9 )
		goto tr66;
	goto st68;
tr136:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st74;
tr158:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st74;
st74:
	if ( ++p == pe )
		goto _out74;
case 74:
#line 1818 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr226;
		case 32: goto tr226;
		case 34: goto st77;
		case 39: goto st141;
		case 47: goto tr230;
		case 60: goto tr69;
		case 62: goto tr70;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr227;
	} else if ( (*p) >= 9 )
		goto tr226;
	goto tr156;
tr163:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st75;
tr226:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st75;
st75:
	if ( ++p == pe )
		goto _out75;
case 75:
#line 1851 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr163;
		case 32: goto tr163;
		case 34: goto st142;
		case 39: goto st143;
		case 47: goto tr161;
		case 60: goto tr69;
		case 62: goto tr162;
		case 63: goto tr165;
		case 95: goto tr165;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr164;
		} else if ( (*p) >= 9 )
			goto tr163;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr165;
		} else if ( (*p) >= 65 )
			goto tr165;
	} else
		goto tr165;
	goto tr156;
tr164:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st76;
tr227:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st76;
st76:
	if ( ++p == pe )
		goto _out76;
case 76:
#line 1895 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr226;
		case 32: goto tr226;
		case 34: goto st77;
		case 39: goto st141;
		case 47: goto tr231;
		case 60: goto tr69;
		case 62: goto tr76;
		case 63: goto tr165;
		case 95: goto tr165;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr227;
		} else if ( (*p) >= 9 )
			goto tr226;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr165;
		} else if ( (*p) >= 65 )
			goto tr165;
	} else
		goto tr165;
	goto tr156;
st77:
	if ( ++p == pe )
		goto _out77;
case 77:
	switch( (*p) ) {
		case 13: goto tr248;
		case 32: goto tr248;
		case 34: goto tr84;
		case 47: goto tr246;
		case 60: goto tr199;
		case 62: goto tr250;
		case 92: goto tr195;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr249;
	} else if ( (*p) >= 9 )
		goto tr248;
	goto tr189;
tr189:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st78;
st78:
	if ( ++p == pe )
		goto _out78;
case 78:
#line 1949 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr92;
		case 32: goto tr92;
		case 34: goto tr84;
		case 47: goto tr95;
		case 60: goto st80;
		case 62: goto tr96;
		case 92: goto st94;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr93;
	} else if ( (*p) >= 9 )
		goto tr92;
	goto st78;
tr11:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st79;
tr92:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st79;
tr201:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st79;
tr216:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st79;
tr248:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st79;
st79:
	if ( ++p == pe )
		goto _out79;
case 79:
#line 1999 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st79;
		case 34: goto tr3;
		case 47: goto tr48;
		case 62: goto tr50;
		case 63: goto tr47;
		case 92: goto st81;
		case 95: goto tr47;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st79;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr47;
		} else if ( (*p) >= 65 )
			goto tr47;
	} else
		goto tr47;
	goto st80;
tr199:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st80;
st80:
	if ( ++p == pe )
		goto _out80;
case 80:
#line 2029 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr3;
		case 92: goto st81;
	}
	goto st80;
tr200:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st81;
st81:
	if ( ++p == pe )
		goto _out81;
case 81:
#line 2043 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr11;
		case 92: goto st81;
	}
	goto st80;
tr47:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st82;
tr202:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st82;
st82:
	if ( ++p == pe )
		goto _out82;
case 82:
#line 2085 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr122;
		case 34: goto tr3;
		case 47: goto tr124;
		case 61: goto tr125;
		case 62: goto tr126;
		case 63: goto st82;
		case 92: goto st81;
		case 95: goto st82;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr122;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st82;
		} else if ( (*p) >= 65 )
			goto st82;
	} else
		goto st82;
	goto st80;
tr98:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st83;
tr122:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st83;
tr144:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st83;
st83:
	if ( ++p == pe )
		goto _out83;
case 83:
#line 2132 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st83;
		case 34: goto tr3;
		case 47: goto tr48;
		case 61: goto st85;
		case 62: goto tr50;
		case 63: goto tr47;
		case 92: goto st81;
		case 95: goto tr47;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st83;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr47;
		} else if ( (*p) >= 65 )
			goto tr47;
	} else
		goto tr47;
	goto st80;
tr48:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st84;
tr124:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st84;
tr203:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st84;
st84:
	if ( ++p == pe )
		goto _out84;
case 84:
#line 2181 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr3;
		case 62: goto tr45;
		case 92: goto st81;
	}
	goto st80;
tr45:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 70 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 12;}
	goto st209;
tr50:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr96:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr103:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr126:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr149:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr204:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr250:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
tr251:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st209;
st209:
	if ( ++p == pe )
		goto _out209;
case 209:
#line 2313 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr3;
		case 92: goto st81;
	}
	goto st80;
tr125:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st85;
st85:
	if ( ++p == pe )
		goto _out85;
case 85:
#line 2327 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr190;
		case 32: goto tr190;
		case 34: goto tr192;
		case 39: goto st140;
		case 47: goto tr194;
		case 60: goto st80;
		case 62: goto tr50;
		case 92: goto tr195;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr191;
	} else if ( (*p) >= 9 )
		goto tr190;
	goto tr189;
tr190:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st86;
st86:
	if ( ++p == pe )
		goto _out86;
case 86:
#line 2352 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr196;
		case 32: goto tr196;
		case 34: goto tr192;
		case 39: goto st140;
		case 47: goto tr194;
		case 60: goto st80;
		case 62: goto tr50;
		case 92: goto tr195;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr197;
	} else if ( (*p) >= 9 )
		goto tr196;
	goto tr189;
tr196:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st87;
tr242:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st87;
st87:
	if ( ++p == pe )
		goto _out87;
case 87:
#line 2386 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr196;
		case 32: goto tr196;
		case 34: goto tr192;
		case 39: goto st140;
		case 47: goto tr194;
		case 60: goto st80;
		case 62: goto tr50;
		case 63: goto tr198;
		case 92: goto tr195;
		case 95: goto tr198;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr197;
		} else if ( (*p) >= 9 )
			goto tr196;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr198;
		} else if ( (*p) >= 65 )
			goto tr198;
	} else
		goto tr198;
	goto tr189;
tr197:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st88;
tr243:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st88;
st88:
	if ( ++p == pe )
		goto _out88;
case 88:
#line 2431 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr242;
		case 32: goto tr242;
		case 34: goto tr244;
		case 39: goto st96;
		case 47: goto tr247;
		case 60: goto st80;
		case 62: goto tr103;
		case 63: goto tr198;
		case 92: goto tr195;
		case 95: goto tr198;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr243;
		} else if ( (*p) >= 9 )
			goto tr242;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr198;
		} else if ( (*p) >= 65 )
			goto tr198;
	} else
		goto tr198;
	goto tr189;
tr244:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st89;
st89:
	if ( ++p == pe )
		goto _out89;
case 89:
#line 2467 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr248;
		case 32: goto tr248;
		case 34: goto tr84;
		case 47: goto tr247;
		case 60: goto tr199;
		case 62: goto tr251;
		case 63: goto tr198;
		case 92: goto tr195;
		case 95: goto tr198;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr249;
		} else if ( (*p) >= 9 )
			goto tr248;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr198;
		} else if ( (*p) >= 65 )
			goto tr198;
	} else
		goto tr198;
	goto tr189;
tr94:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st90;
tr93:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st90;
tr260:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st90;
tr249:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st90;
st90:
	if ( ++p == pe )
		goto _out90;
case 90:
#line 2524 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr92;
		case 32: goto tr92;
		case 34: goto tr84;
		case 47: goto tr101;
		case 60: goto st80;
		case 62: goto tr103;
		case 63: goto tr100;
		case 92: goto st94;
		case 95: goto tr100;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr93;
		} else if ( (*p) >= 9 )
			goto tr92;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr100;
		} else if ( (*p) >= 65 )
			goto tr100;
	} else
		goto tr100;
	goto st78;
tr100:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st91;
tr198:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st91;
st91:
	if ( ++p == pe )
		goto _out91;
case 91:
#line 2587 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr144;
		case 32: goto tr144;
		case 34: goto tr84;
		case 47: goto tr147;
		case 60: goto st80;
		case 61: goto tr148;
		case 62: goto tr149;
		case 63: goto st91;
		case 92: goto st94;
		case 95: goto st91;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr145;
		} else if ( (*p) >= 9 )
			goto tr144;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st91;
		} else if ( (*p) >= 65 )
			goto st91;
	} else
		goto st91;
	goto st78;
tr99:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st92;
tr145:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st92;
st92:
	if ( ++p == pe )
		goto _out92;
case 92:
#line 2635 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr98;
		case 32: goto tr98;
		case 34: goto tr84;
		case 47: goto tr101;
		case 60: goto st80;
		case 61: goto st95;
		case 62: goto tr103;
		case 63: goto tr100;
		case 92: goto st94;
		case 95: goto tr100;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr99;
		} else if ( (*p) >= 9 )
			goto tr98;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr100;
		} else if ( (*p) >= 65 )
			goto tr100;
	} else
		goto tr100;
	goto st78;
tr95:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st93;
tr101:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st93;
tr147:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st93;
tr194:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st93;
tr246:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st93;
tr247:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st93;
st93:
	if ( ++p == pe )
		goto _out93;
case 93:
#line 2736 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr92;
		case 32: goto tr92;
		case 34: goto tr84;
		case 47: goto tr95;
		case 60: goto st80;
		case 62: goto tr96;
		case 92: goto st94;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr93;
	} else if ( (*p) >= 9 )
		goto tr92;
	goto st78;
tr195:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st94;
st94:
	if ( ++p == pe )
		goto _out94;
case 94:
#line 2760 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr92;
		case 32: goto tr92;
		case 34: goto tr94;
		case 47: goto tr95;
		case 60: goto st80;
		case 62: goto tr96;
		case 92: goto st94;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr93;
	} else if ( (*p) >= 9 )
		goto tr92;
	goto st78;
tr148:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st95;
tr191:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st95;
st95:
	if ( ++p == pe )
		goto _out95;
case 95:
#line 2788 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr242;
		case 32: goto tr242;
		case 34: goto tr244;
		case 39: goto st96;
		case 47: goto tr246;
		case 60: goto st80;
		case 62: goto tr96;
		case 92: goto tr195;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr243;
	} else if ( (*p) >= 9 )
		goto tr242;
	goto tr189;
st96:
	if ( ++p == pe )
		goto _out96;
case 96:
	switch( (*p) ) {
		case 13: goto tr258;
		case 32: goto tr258;
		case 34: goto tr263;
		case 39: goto tr94;
		case 47: goto tr256;
		case 60: goto tr215;
		case 62: goto tr261;
		case 92: goto tr211;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr259;
	} else if ( (*p) >= 9 )
		goto tr258;
	goto tr205;
tr205:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st97;
st97:
	if ( ++p == pe )
		goto _out97;
case 97:
#line 2833 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr105;
		case 32: goto tr105;
		case 34: goto tr80;
		case 39: goto tr94;
		case 47: goto tr108;
		case 60: goto st99;
		case 62: goto tr109;
		case 92: goto st129;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr106;
	} else if ( (*p) >= 9 )
		goto tr105;
	goto st97;
tr51:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st98;
tr105:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st98;
tr218:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st98;
tr258:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st98;
st98:
	if ( ++p == pe )
		goto _out98;
case 98:
#line 2878 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st98;
		case 34: goto tr10;
		case 39: goto tr11;
		case 47: goto tr55;
		case 62: goto tr57;
		case 63: goto tr54;
		case 92: goto st122;
		case 95: goto tr54;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st98;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr54;
		} else if ( (*p) >= 65 )
			goto tr54;
	} else
		goto tr54;
	goto st99;
tr215:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st99;
st99:
	if ( ++p == pe )
		goto _out99;
case 99:
#line 2909 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr10;
		case 39: goto tr11;
		case 92: goto st122;
	}
	goto st99;
tr10:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st100;
tr78:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st100;
tr178:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st100;
tr225:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st100;
tr238:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st100;
st100:
	if ( ++p == pe )
		goto _out100;
case 100:
#line 2950 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st100;
		case 39: goto tr3;
		case 47: goto tr31;
		case 62: goto tr33;
		case 63: goto tr30;
		case 92: goto st102;
		case 95: goto tr30;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st100;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr30;
		} else if ( (*p) >= 65 )
			goto tr30;
	} else
		goto tr30;
	goto st101;
tr176:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st101;
st101:
	if ( ++p == pe )
		goto _out101;
case 101:
#line 2980 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr3;
		case 92: goto st102;
	}
	goto st101;
tr177:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st102;
st102:
	if ( ++p == pe )
		goto _out102;
case 102:
#line 2994 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr10;
		case 92: goto st102;
	}
	goto st101;
tr30:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st103;
tr179:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st103;
st103:
	if ( ++p == pe )
		goto _out103;
case 103:
#line 3036 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr117;
		case 39: goto tr3;
		case 47: goto tr119;
		case 61: goto tr120;
		case 62: goto tr121;
		case 63: goto st103;
		case 92: goto st102;
		case 95: goto st103;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr117;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st103;
		} else if ( (*p) >= 65 )
			goto st103;
	} else
		goto st103;
	goto st101;
tr85:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st104;
tr117:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st104;
tr138:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st104;
st104:
	if ( ++p == pe )
		goto _out104;
case 104:
#line 3083 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st104;
		case 39: goto tr3;
		case 47: goto tr31;
		case 61: goto st106;
		case 62: goto tr33;
		case 63: goto tr30;
		case 92: goto st102;
		case 95: goto tr30;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st104;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr30;
		} else if ( (*p) >= 65 )
			goto tr30;
	} else
		goto tr30;
	goto st101;
tr31:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st105;
tr119:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st105;
tr180:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st105;
st105:
	if ( ++p == pe )
		goto _out105;
case 105:
#line 3132 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr3;
		case 62: goto tr28;
		case 92: goto st102;
	}
	goto st101;
tr28:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 70 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 12;}
	goto st210;
tr33:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr82:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr90:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr121:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr143:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr181:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr240:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
tr241:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st210;
st210:
	if ( ++p == pe )
		goto _out210;
case 210:
#line 3264 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr3;
		case 92: goto st102;
	}
	goto st101;
tr120:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st106;
st106:
	if ( ++p == pe )
		goto _out106;
case 106:
#line 3278 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr167;
		case 32: goto tr167;
		case 34: goto st136;
		case 39: goto tr170;
		case 47: goto tr171;
		case 60: goto st101;
		case 62: goto tr33;
		case 92: goto tr172;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr168;
	} else if ( (*p) >= 9 )
		goto tr167;
	goto tr166;
tr166:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st107;
st107:
	if ( ++p == pe )
		goto _out107;
case 107:
#line 3303 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr78;
		case 32: goto tr78;
		case 39: goto tr84;
		case 47: goto tr81;
		case 60: goto st101;
		case 62: goto tr82;
		case 92: goto st112;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr79;
	} else if ( (*p) >= 9 )
		goto tr78;
	goto st107;
tr80:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st108;
tr79:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st108;
tr263:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st108;
tr239:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st108;
st108:
	if ( ++p == pe )
		goto _out108;
case 108:
#line 3349 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr78;
		case 32: goto tr78;
		case 39: goto tr84;
		case 47: goto tr88;
		case 60: goto st101;
		case 62: goto tr90;
		case 63: goto tr87;
		case 92: goto st112;
		case 95: goto tr87;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr79;
		} else if ( (*p) >= 9 )
			goto tr78;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr87;
		} else if ( (*p) >= 65 )
			goto tr87;
	} else
		goto tr87;
	goto st107;
tr87:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st109;
tr175:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st109;
st109:
	if ( ++p == pe )
		goto _out109;
case 109:
#line 3412 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr138;
		case 32: goto tr138;
		case 39: goto tr84;
		case 47: goto tr141;
		case 60: goto st101;
		case 61: goto tr142;
		case 62: goto tr143;
		case 63: goto st109;
		case 92: goto st112;
		case 95: goto st109;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr139;
		} else if ( (*p) >= 9 )
			goto tr138;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st109;
		} else if ( (*p) >= 65 )
			goto st109;
	} else
		goto st109;
	goto st107;
tr86:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st110;
tr139:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st110;
st110:
	if ( ++p == pe )
		goto _out110;
case 110:
#line 3460 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr85;
		case 32: goto tr85;
		case 39: goto tr84;
		case 47: goto tr88;
		case 60: goto st101;
		case 61: goto st113;
		case 62: goto tr90;
		case 63: goto tr87;
		case 92: goto st112;
		case 95: goto tr87;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr86;
		} else if ( (*p) >= 9 )
			goto tr85;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr87;
		} else if ( (*p) >= 65 )
			goto tr87;
	} else
		goto tr87;
	goto st107;
tr81:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st111;
tr88:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st111;
tr141:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st111;
tr171:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st111;
tr236:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st111;
tr237:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st111;
st111:
	if ( ++p == pe )
		goto _out111;
case 111:
#line 3561 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr78;
		case 32: goto tr78;
		case 39: goto tr84;
		case 47: goto tr81;
		case 60: goto st101;
		case 62: goto tr82;
		case 92: goto st112;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr79;
	} else if ( (*p) >= 9 )
		goto tr78;
	goto st107;
tr172:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st112;
st112:
	if ( ++p == pe )
		goto _out112;
case 112:
#line 3585 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr78;
		case 32: goto tr78;
		case 39: goto tr80;
		case 47: goto tr81;
		case 60: goto st101;
		case 62: goto tr82;
		case 92: goto st112;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr79;
	} else if ( (*p) >= 9 )
		goto tr78;
	goto st107;
tr142:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st113;
tr168:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st113;
st113:
	if ( ++p == pe )
		goto _out113;
case 113:
#line 3613 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr232;
		case 32: goto tr232;
		case 34: goto st116;
		case 39: goto tr235;
		case 47: goto tr236;
		case 60: goto st101;
		case 62: goto tr82;
		case 92: goto tr172;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr233;
	} else if ( (*p) >= 9 )
		goto tr232;
	goto tr166;
tr173:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st114;
tr232:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st114;
st114:
	if ( ++p == pe )
		goto _out114;
case 114:
#line 3647 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr173;
		case 32: goto tr173;
		case 34: goto st136;
		case 39: goto tr170;
		case 47: goto tr171;
		case 60: goto st101;
		case 62: goto tr33;
		case 63: goto tr175;
		case 92: goto tr172;
		case 95: goto tr175;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr174;
		} else if ( (*p) >= 9 )
			goto tr173;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr175;
		} else if ( (*p) >= 65 )
			goto tr175;
	} else
		goto tr175;
	goto tr166;
tr174:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st115;
tr233:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st115;
st115:
	if ( ++p == pe )
		goto _out115;
case 115:
#line 3692 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr232;
		case 32: goto tr232;
		case 34: goto st116;
		case 39: goto tr235;
		case 47: goto tr237;
		case 60: goto st101;
		case 62: goto tr90;
		case 63: goto tr175;
		case 92: goto tr172;
		case 95: goto tr175;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr233;
		} else if ( (*p) >= 9 )
			goto tr232;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr175;
		} else if ( (*p) >= 65 )
			goto tr175;
	} else
		goto tr175;
	goto tr166;
st116:
	if ( ++p == pe )
		goto _out116;
case 116:
	switch( (*p) ) {
		case 13: goto tr258;
		case 32: goto tr258;
		case 34: goto tr80;
		case 39: goto tr260;
		case 47: goto tr256;
		case 60: goto tr215;
		case 62: goto tr261;
		case 92: goto tr211;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr259;
	} else if ( (*p) >= 9 )
		goto tr258;
	goto tr205;
tr107:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st117;
tr106:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st117;
tr259:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st117;
st117:
	if ( ++p == pe )
		goto _out117;
case 117:
#line 3764 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr105;
		case 32: goto tr105;
		case 34: goto tr80;
		case 39: goto tr94;
		case 47: goto tr114;
		case 60: goto st99;
		case 62: goto tr116;
		case 63: goto tr113;
		case 92: goto st129;
		case 95: goto tr113;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr106;
		} else if ( (*p) >= 9 )
			goto tr105;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr113;
		} else if ( (*p) >= 65 )
			goto tr113;
	} else
		goto tr113;
	goto st97;
tr113:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st118;
tr214:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st118;
st118:
	if ( ++p == pe )
		goto _out118;
case 118:
#line 3828 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr150;
		case 32: goto tr150;
		case 34: goto tr80;
		case 39: goto tr94;
		case 47: goto tr153;
		case 60: goto st99;
		case 61: goto tr154;
		case 62: goto tr155;
		case 63: goto st118;
		case 92: goto st129;
		case 95: goto st118;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr151;
		} else if ( (*p) >= 9 )
			goto tr150;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st118;
		} else if ( (*p) >= 65 )
			goto st118;
	} else
		goto st118;
	goto st97;
tr111:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st119;
tr127:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st119;
tr150:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st119;
st119:
	if ( ++p == pe )
		goto _out119;
case 119:
#line 3881 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st119;
		case 34: goto tr10;
		case 39: goto tr11;
		case 47: goto tr55;
		case 61: goto st123;
		case 62: goto tr57;
		case 63: goto tr54;
		case 92: goto st122;
		case 95: goto tr54;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto st119;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr54;
		} else if ( (*p) >= 65 )
			goto tr54;
	} else
		goto tr54;
	goto st99;
tr54:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st120;
tr219:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 94 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    akey = Qnil;
    aval = Qnil;
    mark_akey = NULL;
    mark_aval = NULL;
  }
#line 79 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_akey = p; }
	goto st120;
st120:
	if ( ++p == pe )
		goto _out120;
case 120:
#line 3941 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr127;
		case 34: goto tr10;
		case 39: goto tr11;
		case 47: goto tr129;
		case 61: goto tr130;
		case 62: goto tr131;
		case 63: goto st120;
		case 92: goto st122;
		case 95: goto st120;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr127;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st120;
		} else if ( (*p) >= 65 )
			goto st120;
	} else
		goto st120;
	goto st99;
tr55:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st121;
tr129:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st121;
tr220:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st121;
st121:
	if ( ++p == pe )
		goto _out121;
case 121:
#line 3991 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr10;
		case 39: goto tr11;
		case 62: goto tr52;
		case 92: goto st122;
	}
	goto st99;
tr52:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 70 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 12;}
	goto st211;
tr57:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr109:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr116:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr131:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr155:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr221:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr261:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
tr262:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 68 "ext/hpricot_scan/hpricot_scan.rl"
	{act = 10;}
	goto st211;
st211:
	if ( ++p == pe )
		goto _out211;
case 211:
#line 4124 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr10;
		case 39: goto tr11;
		case 92: goto st122;
	}
	goto st99;
tr217:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st122;
st122:
	if ( ++p == pe )
		goto _out122;
case 122:
#line 4139 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr51;
		case 39: goto tr51;
		case 92: goto st122;
	}
	goto st99;
tr130:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st123;
st123:
	if ( ++p == pe )
		goto _out123;
case 123:
#line 4154 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr206;
		case 32: goto tr206;
		case 34: goto tr208;
		case 39: goto tr209;
		case 47: goto tr210;
		case 60: goto st99;
		case 62: goto tr57;
		case 92: goto tr211;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr207;
	} else if ( (*p) >= 9 )
		goto tr206;
	goto tr205;
tr206:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st124;
st124:
	if ( ++p == pe )
		goto _out124;
case 124:
#line 4179 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr212;
		case 32: goto tr212;
		case 34: goto tr208;
		case 39: goto tr209;
		case 47: goto tr210;
		case 60: goto st99;
		case 62: goto tr57;
		case 92: goto tr211;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr213;
	} else if ( (*p) >= 9 )
		goto tr212;
	goto tr205;
tr212:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st125;
tr252:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st125;
st125:
	if ( ++p == pe )
		goto _out125;
case 125:
#line 4213 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr212;
		case 32: goto tr212;
		case 34: goto tr208;
		case 39: goto tr209;
		case 47: goto tr210;
		case 60: goto st99;
		case 62: goto tr57;
		case 63: goto tr214;
		case 92: goto tr211;
		case 95: goto tr214;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr213;
		} else if ( (*p) >= 9 )
			goto tr212;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr214;
		} else if ( (*p) >= 65 )
			goto tr214;
	} else
		goto tr214;
	goto tr205;
tr213:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st126;
tr253:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st126;
st126:
	if ( ++p == pe )
		goto _out126;
case 126:
#line 4258 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr252;
		case 32: goto tr252;
		case 34: goto tr254;
		case 39: goto tr255;
		case 47: goto tr257;
		case 60: goto st99;
		case 62: goto tr116;
		case 63: goto tr214;
		case 92: goto tr211;
		case 95: goto tr214;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr253;
		} else if ( (*p) >= 9 )
			goto tr252;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr214;
		} else if ( (*p) >= 65 )
			goto tr214;
	} else
		goto tr214;
	goto tr205;
tr254:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st127;
st127:
	if ( ++p == pe )
		goto _out127;
case 127:
#line 4294 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr258;
		case 32: goto tr258;
		case 34: goto tr80;
		case 39: goto tr260;
		case 47: goto tr257;
		case 60: goto tr215;
		case 62: goto tr262;
		case 63: goto tr214;
		case 92: goto tr211;
		case 95: goto tr214;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr259;
		} else if ( (*p) >= 9 )
			goto tr258;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr214;
		} else if ( (*p) >= 65 )
			goto tr214;
	} else
		goto tr214;
	goto tr205;
tr108:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st128;
tr114:
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st128;
tr153:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st128;
tr210:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st128;
tr256:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
	goto st128;
tr257:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
#line 101 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    ATTR(akey, aval);
  }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st128;
st128:
	if ( ++p == pe )
		goto _out128;
case 128:
#line 4395 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr105;
		case 32: goto tr105;
		case 34: goto tr80;
		case 39: goto tr94;
		case 47: goto tr108;
		case 60: goto st99;
		case 62: goto tr109;
		case 92: goto st129;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr106;
	} else if ( (*p) >= 9 )
		goto tr105;
	goto st97;
tr211:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st129;
st129:
	if ( ++p == pe )
		goto _out129;
case 129:
#line 4420 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr105;
		case 32: goto tr105;
		case 34: goto tr107;
		case 39: goto tr107;
		case 47: goto tr108;
		case 60: goto st99;
		case 62: goto tr109;
		case 92: goto st129;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr106;
	} else if ( (*p) >= 9 )
		goto tr105;
	goto st97;
tr255:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st130;
st130:
	if ( ++p == pe )
		goto _out130;
case 130:
#line 4445 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr258;
		case 32: goto tr258;
		case 34: goto tr263;
		case 39: goto tr94;
		case 47: goto tr257;
		case 60: goto tr215;
		case 62: goto tr262;
		case 63: goto tr214;
		case 92: goto tr211;
		case 95: goto tr214;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr259;
		} else if ( (*p) >= 9 )
			goto tr258;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr214;
		} else if ( (*p) >= 65 )
			goto tr214;
	} else
		goto tr214;
	goto tr205;
tr208:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st131;
st131:
	if ( ++p == pe )
		goto _out131;
case 131:
#line 4481 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr218;
		case 34: goto tr10;
		case 39: goto tr216;
		case 47: goto tr220;
		case 62: goto tr221;
		case 63: goto tr219;
		case 92: goto tr217;
		case 95: goto tr219;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr218;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr219;
		} else if ( (*p) >= 65 )
			goto tr219;
	} else
		goto tr219;
	goto tr215;
tr209:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st132;
st132:
	if ( ++p == pe )
		goto _out132;
case 132:
#line 4512 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr218;
		case 34: goto tr225;
		case 39: goto tr11;
		case 47: goto tr220;
		case 62: goto tr221;
		case 63: goto tr219;
		case 92: goto tr217;
		case 95: goto tr219;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr218;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr219;
		} else if ( (*p) >= 65 )
			goto tr219;
	} else
		goto tr219;
	goto tr215;
tr154:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
	goto st133;
tr207:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st133;
st133:
	if ( ++p == pe )
		goto _out133;
case 133:
#line 4547 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr252;
		case 32: goto tr252;
		case 34: goto tr254;
		case 39: goto tr255;
		case 47: goto tr256;
		case 60: goto st99;
		case 62: goto tr109;
		case 92: goto tr211;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr253;
	} else if ( (*p) >= 9 )
		goto tr252;
	goto tr205;
tr112:
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st134;
tr151:
#line 87 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(akey, p); }
#line 83 "ext/hpricot_scan/hpricot_scan.rl"
	{ 
    if (*(p-1) == '"' || *(p-1) == '\'') { SET(aval, p-1); }
    else { SET(aval, p); }
  }
	goto st134;
st134:
	if ( ++p == pe )
		goto _out134;
case 134:
#line 4584 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr111;
		case 32: goto tr111;
		case 34: goto tr80;
		case 39: goto tr94;
		case 47: goto tr114;
		case 60: goto st99;
		case 61: goto st133;
		case 62: goto tr116;
		case 63: goto tr113;
		case 92: goto st129;
		case 95: goto tr113;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr112;
		} else if ( (*p) >= 9 )
			goto tr111;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr113;
		} else if ( (*p) >= 65 )
			goto tr113;
	} else
		goto tr113;
	goto st97;
tr235:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st135;
st135:
	if ( ++p == pe )
		goto _out135;
case 135:
#line 4621 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr238;
		case 32: goto tr238;
		case 39: goto tr84;
		case 47: goto tr237;
		case 60: goto tr176;
		case 62: goto tr241;
		case 63: goto tr175;
		case 92: goto tr172;
		case 95: goto tr175;
	}
	if ( (*p) < 45 ) {
		if ( (*p) > 10 ) {
			if ( 11 <= (*p) && (*p) <= 12 )
				goto tr239;
		} else if ( (*p) >= 9 )
			goto tr238;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr175;
		} else if ( (*p) >= 65 )
			goto tr175;
	} else
		goto tr175;
	goto tr166;
st136:
	if ( ++p == pe )
		goto _out136;
case 136:
	switch( (*p) ) {
		case 34: goto tr10;
		case 39: goto tr216;
		case 92: goto tr217;
	}
	goto tr215;
tr170:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st137;
st137:
	if ( ++p == pe )
		goto _out137;
case 137:
#line 4666 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr178;
		case 39: goto tr3;
		case 47: goto tr180;
		case 62: goto tr181;
		case 63: goto tr179;
		case 92: goto tr177;
		case 95: goto tr179;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr178;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr179;
		} else if ( (*p) >= 65 )
			goto tr179;
	} else
		goto tr179;
	goto tr176;
tr167:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st138;
st138:
	if ( ++p == pe )
		goto _out138;
case 138:
#line 4696 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr173;
		case 32: goto tr173;
		case 34: goto st136;
		case 39: goto tr170;
		case 47: goto tr171;
		case 60: goto st101;
		case 62: goto tr33;
		case 92: goto tr172;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr174;
	} else if ( (*p) >= 9 )
		goto tr173;
	goto tr166;
tr192:
#line 82 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); }
	goto st139;
st139:
	if ( ++p == pe )
		goto _out139;
case 139:
#line 4721 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr201;
		case 34: goto tr3;
		case 47: goto tr203;
		case 62: goto tr204;
		case 63: goto tr202;
		case 92: goto tr200;
		case 95: goto tr202;
	}
	if ( (*p) < 45 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr201;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr202;
		} else if ( (*p) >= 65 )
			goto tr202;
	} else
		goto tr202;
	goto tr199;
st140:
	if ( ++p == pe )
		goto _out140;
case 140:
	switch( (*p) ) {
		case 34: goto tr225;
		case 39: goto tr11;
		case 92: goto tr217;
	}
	goto tr215;
st141:
	if ( ++p == pe )
		goto _out141;
case 141:
	switch( (*p) ) {
		case 13: goto tr238;
		case 32: goto tr238;
		case 39: goto tr84;
		case 47: goto tr236;
		case 60: goto tr176;
		case 62: goto tr240;
		case 92: goto tr172;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr239;
	} else if ( (*p) >= 9 )
		goto tr238;
	goto tr166;
st142:
	if ( ++p == pe )
		goto _out142;
case 142:
	switch( (*p) ) {
		case 34: goto tr3;
		case 92: goto tr200;
	}
	goto tr199;
st143:
	if ( ++p == pe )
		goto _out143;
case 143:
	switch( (*p) ) {
		case 39: goto tr3;
		case 92: goto tr177;
	}
	goto tr176;
tr157:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st144;
st144:
	if ( ++p == pe )
		goto _out144;
case 144:
#line 4798 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 13: goto tr163;
		case 32: goto tr163;
		case 34: goto st142;
		case 39: goto st143;
		case 47: goto tr161;
		case 60: goto tr69;
		case 62: goto tr162;
	}
	if ( (*p) > 10 ) {
		if ( 11 <= (*p) && (*p) <= 12 )
			goto tr164;
	} else if ( (*p) >= 9 )
		goto tr163;
	goto tr156;
st145:
	if ( ++p == pe )
		goto _out145;
case 145:
	switch( (*p) ) {
		case 58: goto tr339;
		case 95: goto tr339;
		case 120: goto tr340;
	}
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr339;
	} else if ( (*p) >= 65 )
		goto tr339;
	goto tr296;
tr339:
#line 46 "ext/hpricot_scan/hpricot_scan.rl"
	{ TEXT_PASS(); }
	goto st146;
st146:
	if ( ++p == pe )
		goto _out146;
case 146:
#line 4837 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st212;
		case 63: goto st146;
		case 95: goto st146;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st146;
		} else if ( (*p) >= 9 )
			goto st212;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st146;
		} else if ( (*p) >= 65 )
			goto st146;
	} else
		goto st146;
	goto tr296;
st212:
	if ( ++p == pe )
		goto _out212;
case 212:
	if ( (*p) == 32 )
		goto st212;
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st212;
	goto tr14;
tr340:
#line 46 "ext/hpricot_scan/hpricot_scan.rl"
	{ TEXT_PASS(); }
	goto st147;
st147:
	if ( ++p == pe )
		goto _out147;
case 147:
#line 4875 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st212;
		case 63: goto st146;
		case 95: goto st146;
		case 109: goto st148;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st146;
		} else if ( (*p) >= 9 )
			goto st212;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st146;
		} else if ( (*p) >= 65 )
			goto st146;
	} else
		goto st146;
	goto tr296;
st148:
	if ( ++p == pe )
		goto _out148;
case 148:
	switch( (*p) ) {
		case 32: goto st212;
		case 63: goto st146;
		case 95: goto st146;
		case 108: goto st149;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st146;
		} else if ( (*p) >= 9 )
			goto st212;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st146;
		} else if ( (*p) >= 65 )
			goto st146;
	} else
		goto st146;
	goto tr296;
st149:
	if ( ++p == pe )
		goto _out149;
case 149:
	switch( (*p) ) {
		case 32: goto tr16;
		case 63: goto st146;
		case 95: goto st146;
	}
	if ( (*p) < 48 ) {
		if ( (*p) > 13 ) {
			if ( 45 <= (*p) && (*p) <= 46 )
				goto st146;
		} else if ( (*p) >= 9 )
			goto tr16;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st146;
		} else if ( (*p) >= 65 )
			goto st146;
	} else
		goto st146;
	goto tr296;
tr16:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
	goto st213;
st213:
	if ( ++p == pe )
		goto _out213;
case 213:
#line 4954 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto tr16;
		case 118: goto st150;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr16;
	goto tr14;
st150:
	if ( ++p == pe )
		goto _out150;
case 150:
	if ( (*p) == 101 )
		goto st151;
	goto tr264;
st151:
	if ( ++p == pe )
		goto _out151;
case 151:
	if ( (*p) == 114 )
		goto st152;
	goto tr264;
st152:
	if ( ++p == pe )
		goto _out152;
case 152:
	if ( (*p) == 115 )
		goto st153;
	goto tr264;
st153:
	if ( ++p == pe )
		goto _out153;
case 153:
	if ( (*p) == 105 )
		goto st154;
	goto tr264;
st154:
	if ( ++p == pe )
		goto _out154;
case 154:
	if ( (*p) == 111 )
		goto st155;
	goto tr264;
st155:
	if ( ++p == pe )
		goto _out155;
case 155:
	if ( (*p) == 110 )
		goto st156;
	goto tr264;
st156:
	if ( ++p == pe )
		goto _out156;
case 156:
	switch( (*p) ) {
		case 32: goto st156;
		case 61: goto st157;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st156;
	goto tr264;
st157:
	if ( ++p == pe )
		goto _out157;
case 157:
	switch( (*p) ) {
		case 32: goto st157;
		case 34: goto st158;
		case 39: goto st200;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st157;
	goto tr264;
st158:
	if ( ++p == pe )
		goto _out158;
case 158:
	if ( (*p) == 95 )
		goto tr282;
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr282;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr282;
		} else if ( (*p) >= 65 )
			goto tr282;
	} else
		goto tr282;
	goto tr264;
tr282:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st159;
st159:
	if ( ++p == pe )
		goto _out159;
case 159:
#line 5053 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr275;
		case 95: goto st159;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st159;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st159;
		} else if ( (*p) >= 65 )
			goto st159;
	} else
		goto st159;
	goto tr264;
tr275:
#line 88 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("version"), aval); }
	goto st160;
st160:
	if ( ++p == pe )
		goto _out160;
case 160:
#line 5078 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st161;
		case 62: goto tr270;
		case 63: goto st162;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st161;
	goto tr264;
st161:
	if ( ++p == pe )
		goto _out161;
case 161:
	switch( (*p) ) {
		case 32: goto st161;
		case 62: goto tr270;
		case 63: goto st162;
		case 101: goto st163;
		case 115: goto st176;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st161;
	goto tr264;
st162:
	if ( ++p == pe )
		goto _out162;
case 162:
	if ( (*p) == 62 )
		goto tr270;
	goto tr264;
st163:
	if ( ++p == pe )
		goto _out163;
case 163:
	if ( (*p) == 110 )
		goto st164;
	goto tr264;
st164:
	if ( ++p == pe )
		goto _out164;
case 164:
	if ( (*p) == 99 )
		goto st165;
	goto tr264;
st165:
	if ( ++p == pe )
		goto _out165;
case 165:
	if ( (*p) == 111 )
		goto st166;
	goto tr264;
st166:
	if ( ++p == pe )
		goto _out166;
case 166:
	if ( (*p) == 100 )
		goto st167;
	goto tr264;
st167:
	if ( ++p == pe )
		goto _out167;
case 167:
	if ( (*p) == 105 )
		goto st168;
	goto tr264;
st168:
	if ( ++p == pe )
		goto _out168;
case 168:
	if ( (*p) == 110 )
		goto st169;
	goto tr264;
st169:
	if ( ++p == pe )
		goto _out169;
case 169:
	if ( (*p) == 103 )
		goto st170;
	goto tr264;
st170:
	if ( ++p == pe )
		goto _out170;
case 170:
	switch( (*p) ) {
		case 32: goto st170;
		case 61: goto st171;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st170;
	goto tr264;
st171:
	if ( ++p == pe )
		goto _out171;
case 171:
	switch( (*p) ) {
		case 32: goto st171;
		case 34: goto st172;
		case 39: goto st198;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st171;
	goto tr264;
st172:
	if ( ++p == pe )
		goto _out172;
case 172:
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr283;
	} else if ( (*p) >= 65 )
		goto tr283;
	goto tr264;
tr283:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st173;
st173:
	if ( ++p == pe )
		goto _out173;
case 173:
#line 5198 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 34: goto tr277;
		case 95: goto st173;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st173;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st173;
		} else if ( (*p) >= 65 )
			goto st173;
	} else
		goto st173;
	goto tr264;
tr277:
#line 89 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("encoding"), aval); }
	goto st174;
st174:
	if ( ++p == pe )
		goto _out174;
case 174:
#line 5223 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st175;
		case 62: goto tr270;
		case 63: goto st162;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st175;
	goto tr264;
st175:
	if ( ++p == pe )
		goto _out175;
case 175:
	switch( (*p) ) {
		case 32: goto st175;
		case 62: goto tr270;
		case 63: goto st162;
		case 115: goto st176;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st175;
	goto tr264;
st176:
	if ( ++p == pe )
		goto _out176;
case 176:
	if ( (*p) == 116 )
		goto st177;
	goto tr264;
st177:
	if ( ++p == pe )
		goto _out177;
case 177:
	if ( (*p) == 97 )
		goto st178;
	goto tr264;
st178:
	if ( ++p == pe )
		goto _out178;
case 178:
	if ( (*p) == 110 )
		goto st179;
	goto tr264;
st179:
	if ( ++p == pe )
		goto _out179;
case 179:
	if ( (*p) == 100 )
		goto st180;
	goto tr264;
st180:
	if ( ++p == pe )
		goto _out180;
case 180:
	if ( (*p) == 97 )
		goto st181;
	goto tr264;
st181:
	if ( ++p == pe )
		goto _out181;
case 181:
	if ( (*p) == 108 )
		goto st182;
	goto tr264;
st182:
	if ( ++p == pe )
		goto _out182;
case 182:
	if ( (*p) == 111 )
		goto st183;
	goto tr264;
st183:
	if ( ++p == pe )
		goto _out183;
case 183:
	if ( (*p) == 110 )
		goto st184;
	goto tr264;
st184:
	if ( ++p == pe )
		goto _out184;
case 184:
	if ( (*p) == 101 )
		goto st185;
	goto tr264;
st185:
	if ( ++p == pe )
		goto _out185;
case 185:
	switch( (*p) ) {
		case 32: goto st185;
		case 61: goto st186;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st185;
	goto tr264;
st186:
	if ( ++p == pe )
		goto _out186;
case 186:
	switch( (*p) ) {
		case 32: goto st186;
		case 34: goto st187;
		case 39: goto st193;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st186;
	goto tr264;
st187:
	if ( ++p == pe )
		goto _out187;
case 187:
	switch( (*p) ) {
		case 110: goto tr291;
		case 121: goto tr292;
	}
	goto tr264;
tr291:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st188;
st188:
	if ( ++p == pe )
		goto _out188;
case 188:
#line 5348 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 111 )
		goto st189;
	goto tr264;
st189:
	if ( ++p == pe )
		goto _out189;
case 189:
	if ( (*p) == 34 )
		goto tr279;
	goto tr264;
tr279:
#line 90 "ext/hpricot_scan/hpricot_scan.rl"
	{ SET(aval, p); ATTR(rb_str_new2("standalone"), aval); }
	goto st190;
st190:
	if ( ++p == pe )
		goto _out190;
case 190:
#line 5367 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 32: goto st190;
		case 62: goto tr270;
		case 63: goto st162;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st190;
	goto tr264;
tr292:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st191;
st191:
	if ( ++p == pe )
		goto _out191;
case 191:
#line 5384 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 101 )
		goto st192;
	goto tr264;
st192:
	if ( ++p == pe )
		goto _out192;
case 192:
	if ( (*p) == 115 )
		goto st189;
	goto tr264;
st193:
	if ( ++p == pe )
		goto _out193;
case 193:
	switch( (*p) ) {
		case 110: goto tr405;
		case 121: goto tr406;
	}
	goto tr264;
tr405:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st194;
st194:
	if ( ++p == pe )
		goto _out194;
case 194:
#line 5412 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 111 )
		goto st195;
	goto tr264;
st195:
	if ( ++p == pe )
		goto _out195;
case 195:
	if ( (*p) == 39 )
		goto tr279;
	goto tr264;
tr406:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st196;
st196:
	if ( ++p == pe )
		goto _out196;
case 196:
#line 5431 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 101 )
		goto st197;
	goto tr264;
st197:
	if ( ++p == pe )
		goto _out197;
case 197:
	if ( (*p) == 115 )
		goto st195;
	goto tr264;
st198:
	if ( ++p == pe )
		goto _out198;
case 198:
	if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr383;
	} else if ( (*p) >= 65 )
		goto tr383;
	goto tr264;
tr383:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st199;
st199:
	if ( ++p == pe )
		goto _out199;
case 199:
#line 5460 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr277;
		case 95: goto st199;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st199;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st199;
		} else if ( (*p) >= 65 )
			goto st199;
	} else
		goto st199;
	goto tr264;
st200:
	if ( ++p == pe )
		goto _out200;
case 200:
	if ( (*p) == 95 )
		goto tr382;
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto tr382;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr382;
		} else if ( (*p) >= 65 )
			goto tr382;
	} else
		goto tr382;
	goto tr264;
tr382:
#line 78 "ext/hpricot_scan/hpricot_scan.rl"
	{ mark_aval = p; }
	goto st201;
st201:
	if ( ++p == pe )
		goto _out201;
case 201:
#line 5503 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 39: goto tr275;
		case 95: goto st201;
	}
	if ( (*p) < 48 ) {
		if ( 45 <= (*p) && (*p) <= 46 )
			goto st201;
	} else if ( (*p) > 58 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st201;
		} else if ( (*p) >= 65 )
			goto st201;
	} else
		goto st201;
	goto tr264;
tr409:
#line 51 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st214;
tr411:
#line 51 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st214;
tr412:
#line 9 "ext/hpricot_scan/hpricot_scan.rl"
	{curline += 1;}
#line 51 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st214;
tr414:
#line 51 "ext/hpricot_scan/hpricot_scan.rl"
	{{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st214;
tr415:
#line 50 "ext/hpricot_scan/hpricot_scan.rl"
	{ EBLK(comment, 3); {goto st204;} }
#line 50 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{p = ((tokend))-1;}}
	goto st214;
st214:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = 0;}
	if ( ++p == pe )
		goto _out214;
case 214:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = p;}
#line 5552 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 10: goto tr412;
		case 45: goto tr413;
	}
	goto tr411;
tr413:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
	goto st215;
st215:
	if ( ++p == pe )
		goto _out215;
case 215:
#line 5566 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 45 )
		goto st202;
	goto tr409;
st202:
	if ( ++p == pe )
		goto _out202;
case 202:
	if ( (*p) == 62 )
		goto tr415;
	goto tr414;
tr416:
#line 56 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st216;
tr418:
#line 56 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st216;
tr419:
#line 9 "ext/hpricot_scan/hpricot_scan.rl"
	{curline += 1;}
#line 56 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st216;
tr421:
#line 56 "ext/hpricot_scan/hpricot_scan.rl"
	{{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st216;
tr422:
#line 55 "ext/hpricot_scan/hpricot_scan.rl"
	{ EBLK(cdata, 3); {goto st204;} }
#line 55 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{p = ((tokend))-1;}}
	goto st216;
st216:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = 0;}
	if ( ++p == pe )
		goto _out216;
case 216:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = p;}
#line 5609 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 10: goto tr419;
		case 93: goto tr420;
	}
	goto tr418;
tr420:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;}
	goto st217;
st217:
	if ( ++p == pe )
		goto _out217;
case 217:
#line 5623 "ext/hpricot_scan/hpricot_scan.c"
	if ( (*p) == 93 )
		goto st203;
	goto tr416;
st203:
	if ( ++p == pe )
		goto _out203;
case 203:
	if ( (*p) == 62 )
		goto tr422;
	goto tr421;
tr423:
#line 61 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st218;
tr424:
#line 60 "ext/hpricot_scan/hpricot_scan.rl"
	{ EBLK(procins, 2); {goto st204;} }
#line 60 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{p = ((tokend))-1;}}
	goto st218;
tr425:
#line 61 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st218;
tr426:
#line 9 "ext/hpricot_scan/hpricot_scan.rl"
	{curline += 1;}
#line 61 "ext/hpricot_scan/hpricot_scan.rl"
	{tokend = p+1;{ TEXT_PASS(); }{p = ((tokend))-1;}}
	goto st218;
st218:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = 0;}
	if ( ++p == pe )
		goto _out218;
case 218:
#line 1 "ext/hpricot_scan/hpricot_scan.rl"
	{tokstart = p;}
#line 5662 "ext/hpricot_scan/hpricot_scan.c"
	switch( (*p) ) {
		case 10: goto tr426;
		case 62: goto tr424;
		case 63: goto st219;
	}
	goto tr425;
st219:
	if ( ++p == pe )
		goto _out219;
case 219:
	if ( (*p) == 62 )
		goto tr424;
	goto tr423;
	}
	_out204: cs = 204; goto _out; 
	_out205: cs = 205; goto _out; 
	_out0: cs = 0; goto _out; 
	_out1: cs = 1; goto _out; 
	_out2: cs = 2; goto _out; 
	_out3: cs = 3; goto _out; 
	_out4: cs = 4; goto _out; 
	_out5: cs = 5; goto _out; 
	_out6: cs = 6; goto _out; 
	_out7: cs = 7; goto _out; 
	_out8: cs = 8; goto _out; 
	_out9: cs = 9; goto _out; 
	_out10: cs = 10; goto _out; 
	_out11: cs = 11; goto _out; 
	_out12: cs = 12; goto _out; 
	_out13: cs = 13; goto _out; 
	_out14: cs = 14; goto _out; 
	_out15: cs = 15; goto _out; 
	_out16: cs = 16; goto _out; 
	_out17: cs = 17; goto _out; 
	_out18: cs = 18; goto _out; 
	_out19: cs = 19; goto _out; 
	_out20: cs = 20; goto _out; 
	_out21: cs = 21; goto _out; 
	_out22: cs = 22; goto _out; 
	_out23: cs = 23; goto _out; 
	_out24: cs = 24; goto _out; 
	_out25: cs = 25; goto _out; 
	_out26: cs = 26; goto _out; 
	_out27: cs = 27; goto _out; 
	_out28: cs = 28; goto _out; 
	_out29: cs = 29; goto _out; 
	_out30: cs = 30; goto _out; 
	_out31: cs = 31; goto _out; 
	_out32: cs = 32; goto _out; 
	_out33: cs = 33; goto _out; 
	_out34: cs = 34; goto _out; 
	_out35: cs = 35; goto _out; 
	_out36: cs = 36; goto _out; 
	_out37: cs = 37; goto _out; 
	_out38: cs = 38; goto _out; 
	_out39: cs = 39; goto _out; 
	_out206: cs = 206; goto _out; 
	_out40: cs = 40; goto _out; 
	_out41: cs = 41; goto _out; 
	_out207: cs = 207; goto _out; 
	_out42: cs = 42; goto _out; 
	_out43: cs = 43; goto _out; 
	_out208: cs = 208; goto _out; 
	_out44: cs = 44; goto _out; 
	_out45: cs = 45; goto _out; 
	_out46: cs = 46; goto _out; 
	_out47: cs = 47; goto _out; 
	_out48: cs = 48; goto _out; 
	_out49: cs = 49; goto _out; 
	_out50: cs = 50; goto _out; 
	_out51: cs = 51; goto _out; 
	_out52: cs = 52; goto _out; 
	_out53: cs = 53; goto _out; 
	_out54: cs = 54; goto _out; 
	_out55: cs = 55; goto _out; 
	_out56: cs = 56; goto _out; 
	_out57: cs = 57; goto _out; 
	_out58: cs = 58; goto _out; 
	_out59: cs = 59; goto _out; 
	_out60: cs = 60; goto _out; 
	_out61: cs = 61; goto _out; 
	_out62: cs = 62; goto _out; 
	_out63: cs = 63; goto _out; 
	_out64: cs = 64; goto _out; 
	_out65: cs = 65; goto _out; 
	_out66: cs = 66; goto _out; 
	_out67: cs = 67; goto _out; 
	_out68: cs = 68; goto _out; 
	_out69: cs = 69; goto _out; 
	_out70: cs = 70; goto _out; 
	_out71: cs = 71; goto _out; 
	_out72: cs = 72; goto _out; 
	_out73: cs = 73; goto _out; 
	_out74: cs = 74; goto _out; 
	_out75: cs = 75; goto _out; 
	_out76: cs = 76; goto _out; 
	_out77: cs = 77; goto _out; 
	_out78: cs = 78; goto _out; 
	_out79: cs = 79; goto _out; 
	_out80: cs = 80; goto _out; 
	_out81: cs = 81; goto _out; 
	_out82: cs = 82; goto _out; 
	_out83: cs = 83; goto _out; 
	_out84: cs = 84; goto _out; 
	_out209: cs = 209; goto _out; 
	_out85: cs = 85; goto _out; 
	_out86: cs = 86; goto _out; 
	_out87: cs = 87; goto _out; 
	_out88: cs = 88; goto _out; 
	_out89: cs = 89; goto _out; 
	_out90: cs = 90; goto _out; 
	_out91: cs = 91; goto _out; 
	_out92: cs = 92; goto _out; 
	_out93: cs = 93; goto _out; 
	_out94: cs = 94; goto _out; 
	_out95: cs = 95; goto _out; 
	_out96: cs = 96; goto _out; 
	_out97: cs = 97; goto _out; 
	_out98: cs = 98; goto _out; 
	_out99: cs = 99; goto _out; 
	_out100: cs = 100; goto _out; 
	_out101: cs = 101; goto _out; 
	_out102: cs = 102; goto _out; 
	_out103: cs = 103; goto _out; 
	_out104: cs = 104; goto _out; 
	_out105: cs = 105; goto _out; 
	_out210: cs = 210; goto _out; 
	_out106: cs = 106; goto _out; 
	_out107: cs = 107; goto _out; 
	_out108: cs = 108; goto _out; 
	_out109: cs = 109; goto _out; 
	_out110: cs = 110; goto _out; 
	_out111: cs = 111; goto _out; 
	_out112: cs = 112; goto _out; 
	_out113: cs = 113; goto _out; 
	_out114: cs = 114; goto _out; 
	_out115: cs = 115; goto _out; 
	_out116: cs = 116; goto _out; 
	_out117: cs = 117; goto _out; 
	_out118: cs = 118; goto _out; 
	_out119: cs = 119; goto _out; 
	_out120: cs = 120; goto _out; 
	_out121: cs = 121; goto _out; 
	_out211: cs = 211; goto _out; 
	_out122: cs = 122; goto _out; 
	_out123: cs = 123; goto _out; 
	_out124: cs = 124; goto _out; 
	_out125: cs = 125; goto _out; 
	_out126: cs = 126; goto _out; 
	_out127: cs = 127; goto _out; 
	_out128: cs = 128; goto _out; 
	_out129: cs = 129; goto _out; 
	_out130: cs = 130; goto _out; 
	_out131: cs = 131; goto _out; 
	_out132: cs = 132; goto _out; 
	_out133: cs = 133; goto _out; 
	_out134: cs = 134; goto _out; 
	_out135: cs = 135; goto _out; 
	_out136: cs = 136; goto _out; 
	_out137: cs = 137; goto _out; 
	_out138: cs = 138; goto _out; 
	_out139: cs = 139; goto _out; 
	_out140: cs = 140; goto _out; 
	_out141: cs = 141; goto _out; 
	_out142: cs = 142; goto _out; 
	_out143: cs = 143; goto _out; 
	_out144: cs = 144; goto _out; 
	_out145: cs = 145; goto _out; 
	_out146: cs = 146; goto _out; 
	_out212: cs = 212; goto _out; 
	_out147: cs = 147; goto _out; 
	_out148: cs = 148; goto _out; 
	_out149: cs = 149; goto _out; 
	_out213: cs = 213; goto _out; 
	_out150: cs = 150; goto _out; 
	_out151: cs = 151; goto _out; 
	_out152: cs = 152; goto _out; 
	_out153: cs = 153; goto _out; 
	_out154: cs = 154; goto _out; 
	_out155: cs = 155; goto _out; 
	_out156: cs = 156; goto _out; 
	_out157: cs = 157; goto _out; 
	_out158: cs = 158; goto _out; 
	_out159: cs = 159; goto _out; 
	_out160: cs = 160; goto _out; 
	_out161: cs = 161; goto _out; 
	_out162: cs = 162; goto _out; 
	_out163: cs = 163; goto _out; 
	_out164: cs = 164; goto _out; 
	_out165: cs = 165; goto _out; 
	_out166: cs = 166; goto _out; 
	_out167: cs = 167; goto _out; 
	_out168: cs = 168; goto _out; 
	_out169: cs = 169; goto _out; 
	_out170: cs = 170; goto _out; 
	_out171: cs = 171; goto _out; 
	_out172: cs = 172; goto _out; 
	_out173: cs = 173; goto _out; 
	_out174: cs = 174; goto _out; 
	_out175: cs = 175; goto _out; 
	_out176: cs = 176; goto _out; 
	_out177: cs = 177; goto _out; 
	_out178: cs = 178; goto _out; 
	_out179: cs = 179; goto _out; 
	_out180: cs = 180; goto _out; 
	_out181: cs = 181; goto _out; 
	_out182: cs = 182; goto _out; 
	_out183: cs = 183; goto _out; 
	_out184: cs = 184; goto _out; 
	_out185: cs = 185; goto _out; 
	_out186: cs = 186; goto _out; 
	_out187: cs = 187; goto _out; 
	_out188: cs = 188; goto _out; 
	_out189: cs = 189; goto _out; 
	_out190: cs = 190; goto _out; 
	_out191: cs = 191; goto _out; 
	_out192: cs = 192; goto _out; 
	_out193: cs = 193; goto _out; 
	_out194: cs = 194; goto _out; 
	_out195: cs = 195; goto _out; 
	_out196: cs = 196; goto _out; 
	_out197: cs = 197; goto _out; 
	_out198: cs = 198; goto _out; 
	_out199: cs = 199; goto _out; 
	_out200: cs = 200; goto _out; 
	_out201: cs = 201; goto _out; 
	_out214: cs = 214; goto _out; 
	_out215: cs = 215; goto _out; 
	_out202: cs = 202; goto _out; 
	_out216: cs = 216; goto _out; 
	_out217: cs = 217; goto _out; 
	_out203: cs = 203; goto _out; 
	_out218: cs = 218; goto _out; 
	_out219: cs = 219; goto _out; 

	_out: {}
	}
#line 197 "ext/hpricot_scan/hpricot_scan.rl"
    
    if ( cs == hpricot_scan_error ) {
      free(buf);
      if ( !NIL_P(tag) )
      {
        rb_raise(rb_eHpricotParseError, "parse error on element <%s>, starting on line %d.\n" NO_WAY_SERIOUSLY, RSTRING(tag)->ptr, curline);
      }
      else
      {
        rb_raise(rb_eHpricotParseError, "parse error on line %d.\n" NO_WAY_SERIOUSLY, curline);
      }
    }
    
    if ( done && ele_open )
    {
      ele_open = 0;
      if (tokstart > 0) {
        mark_tag = tokstart;
        tokstart = 0;
        text = 1;
      }
    }

    if ( tokstart == 0 )
    {
      have = 0;
      /* text nodes have no tokstart because each byte is parsed alone */
      if ( mark_tag != NULL && text == 1 )
      {
        if (done)
        {
          if (mark_tag < p-1)
          {
            CAT(tag, p-1);
            ELE(text);
          }
        }
        else
        {
          CAT(tag, p);
        }
      }
      mark_tag = buf;
    }
    else
    {
      have = pe - tokstart;
      memmove( buf, tokstart, have );
      SLIDE(tag);
      SLIDE(akey);
      SLIDE(aval);
      tokend = buf + (tokend - tokstart);
      tokstart = buf;
    }
  }
  free(buf);
}

void Init_hpricot_scan()
{
  VALUE mHpricot = rb_define_module("Hpricot");
  rb_define_attr(rb_singleton_class(mHpricot), "buffer_size", 1, 1);
  rb_define_singleton_method(mHpricot, "scan", hpricot_scan, 1);
  rb_eHpricotParseError = rb_define_class_under(mHpricot, "ParseError", rb_eException);

  s_read = rb_intern("read");
  s_to_str = rb_intern("to_str");
  sym_xmldecl = ID2SYM(rb_intern("xmldecl"));
  sym_doctype = ID2SYM(rb_intern("doctype"));
  sym_procins = ID2SYM(rb_intern("procins"));
  sym_stag = ID2SYM(rb_intern("stag"));
  sym_etag = ID2SYM(rb_intern("etag"));
  sym_emptytag = ID2SYM(rb_intern("emptytag"));
  sym_comment = ID2SYM(rb_intern("comment"));
  sym_cdata = ID2SYM(rb_intern("cdata"));
  sym_text = ID2SYM(rb_intern("text"));
}
