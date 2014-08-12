#include <ruby.h>
#include <assert.h>

#ifdef HAVE_RUBY_ENCODING_H
#include <ruby/encoding.h>
# define ASSOCIATE_INDEX(s,enc)  rb_enc_associate_index((s), rb_enc_to_index(enc))
#else
# define ASSOCIATE_INDEX(s,enc)
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(arr)  RARRAY(arr)->len
#define RARRAY_PTR(arr)  RARRAY(arr)->ptr
#define RSTRING_LEN(str) RSTRING(str)->len
#define RSTRING_PTR(str) RSTRING(str)->ptr
#endif

static ID unpack_id;
static VALUE U_fmt, C_fmt;

/* give GCC hints for better branch prediction
 * (we layout branches so that ASCII characters are handled faster) */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#  define likely(x)		__builtin_expect (!!(x), 1)
#  define unlikely(x)		__builtin_expect (!!(x), 0)
#else
#  define unlikely(x)		(x)
#  define likely(x)		(x)
#endif

/* pass-through certain characters for CP-1252 */
#define p(x) (x-128)

static const int cp_1252[] = {
	8364,		/* 128 => 8364, euro sign */
	p(129),		/* 129 => 129,  pass-through */
	8218,		/* 130 => 8218, single low-9 quotation mark */
	402,		/* 131 =>  402, latin small letter f with hook */
	8222,		/* 132 => 8222, double low-9 quotation mark */
	8230,		/* 133 => 8230, horizontal ellipsis */
	8224,		/* 134 => 8224, dagger */
	8225,		/* 135 => 8225, double dagger */
	710,		/* 136 =>  710, modifier letter circumflex accent */
	8240,		/* 137 => 8240, per mille sign */
	352,		/* 138 =>  352, latin capital letter s with caron */
	8249,		/* 139 => 8249, single left-pointing angle quotation mark */
	338,		/* 140 =>  338, latin capital ligature oe */
	p(141),		/* 141 =>  141, pass-through */
	381,		/* 142 =>  381, latin capital letter z with caron */
	p(143),		/* 143 =>  143, pass-through */
	p(144),		/* 144 =>  144, pass-through */
	8216,		/* 145 => 8216, left single quotation mark */
	8217,		/* 146 => 8217, right single quotation mark */
	8220,		/* 147 => 8220, left double quotation mark */
	8221,		/* 148 => 8221, right double quotation mark */
	8226,		/* 149 => 8226, bullet */
	8211,		/* 150 => 8211, en dash */
	8212,		/* 151 => 8212, em dash */
	732,		/* 152 =>  732, small tilde */
	8482,		/* 153 => 8482, trade mark sign */
	353,		/* 154 =>  353, latin small letter s with caron */
	8250,		/* 155 => 8250, single right-pointing angle quotation mark */
	339,		/* 156 =>  339, latin small ligature oe */
	p(157),		/* 157 =>  157, pass-through */
	382,		/* 158 =>  382, latin small letter z with caron */
	376		/* 159 =>  376} latin capital letter y with diaeresis */
};

#define VALID_VALUE(n) \
	(n >= 0x20 && n <= 0xD7FF) || \
	    (n >= 0xE000 && n <= 0xFFFD) || \
	    (n >= 0x10000 && n <= 0x10FFFF)

#define CP_1252_ESCAPE(n) do { \
	if (n >= 128 && n <= 159) \
		n = cp_1252[n - 128]; \
	} while(0)

static inline size_t bytes_for(int n)
{
	if (n < 1000)
		return sizeof("&#999;") - 1;
	if (n < 10000)
		return sizeof("&#9999;") - 1;
	if (n < 100000)
		return sizeof("&#99999;") - 1;
	if (n < 1000000)
		return sizeof("&#999999;") - 1;
	/* if (n < 10000000), we won't have cases above 0x10FFFF */
	return sizeof("&#9999999;") - 1;
}

static size_t escape(char *buf, int n)
{

#define return_const_len(x) do { \
	memcpy(buf, x, sizeof(x) - 1); \
	return (sizeof(x) - 1); \
} while (0)

	/* handle ASCII first */
	if (likely(n < 128)) {
		if (likely(n >= 0x20 || n == '\t' || n == '\n' || n == '\r')) {
			if (unlikely(n == '"'))
				return_const_len("&quot;");
			if (unlikely(n == '&'))
				return_const_len("&amp;");
			if (unlikely(n == '<'))
				return_const_len("&lt;");
			if (unlikely(n == '>'))
				return_const_len("&gt;");
			buf[0] = (char)n;
			return 1;
		}

		buf[0] = '*';
		return 1;
	}

#undef return_const_len

	CP_1252_ESCAPE(n);

	if (VALID_VALUE(n)) {
		/* return snprintf(buf, sizeof("&#1114111;"), "&#%i;", n); */
		static const char digitmap[] = "0123456789";
		size_t rv = sizeof("&#;") - 1;
		buf += bytes_for(n);
		*--buf = ';';
		do {
			*--buf = digitmap[(int)(n % 10)];
			++rv;
		} while (n /= 10);
		*--buf = '#';
		*--buf = '&';
		return rv;
	}
	buf[0] = '*';
	return 1;
}

static VALUE unpack_utf8(VALUE self)
{
	return rb_funcall(self, unpack_id, 1, U_fmt);
}

static VALUE unpack_uchar(VALUE self)
{
	return rb_funcall(self, unpack_id, 1, C_fmt);
}

/*
 * escapes strings for XML
 * The double-quote (") character is translated to "&quot;"
 */
static VALUE fast_xs(VALUE self)
{
	long i;
	VALUE array;
	char *c;
	size_t s_len;
	VALUE *tmp;
	VALUE rv;

	array = rb_rescue(unpack_utf8, self, unpack_uchar, self);

	for (tmp = RARRAY_PTR(array), s_len = i = RARRAY_LEN(array);
	     --i >= 0;
	     tmp++) {
		int n = NUM2INT(*tmp);
		if (likely(n < 128)) {
			if (unlikely(n == '"'))
				s_len += (sizeof("&quot;") - 2);
			if (unlikely(n == '&'))
				s_len += (sizeof("&amp;") - 2);
			if (unlikely(n == '>' || n == '<'))
				s_len += (sizeof("&gt;") - 2);
			continue;
		}

		CP_1252_ESCAPE(n);

		if (VALID_VALUE(n))
			s_len += bytes_for(n) - 1;
	}

	rv = rb_str_new(NULL, s_len);
        ASSOCIATE_INDEX(rv, rb_default_external_encoding());
	c = RSTRING_PTR(rv);

	for (tmp = RARRAY_PTR(array), i = RARRAY_LEN(array); --i >= 0; tmp++)
		c += escape(c, NUM2INT(*tmp));

	return rv;
}

void Init_fast_xs(void)
{
	assert(cp_1252[159 - 128] == 376); /* just in case I skipped a line */

	unpack_id = rb_intern("unpack");
	U_fmt = rb_str_new("U*", 2);
        ASSOCIATE_INDEX(U_fmt, rb_ascii8bit_encoding());
	C_fmt = rb_str_new("C*", 2);
        ASSOCIATE_INDEX(C_fmt, rb_ascii8bit_encoding());
	rb_global_variable(&U_fmt);
	rb_global_variable(&C_fmt);

	rb_define_method(rb_cString, "fast_xs", fast_xs, 0);
}
