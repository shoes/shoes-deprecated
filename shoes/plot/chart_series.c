/*
 * shoes_chart_series class
 * encapsulates the data and unique presentation values of them
 * not really Shoes api user visible (yet)
*/
#include "shoes/types/color.h"
#include "shoes/plot/plot.h"


// forward declare
static VALUE shoes_chart_series_parse_points(VALUE);

void shoes_chart_series_mark(shoes_chart_series *self_t) {
    rb_gc_mark_maybe(self_t->values);
    rb_gc_mark_maybe(self_t->labels);
    rb_gc_mark_maybe(self_t->minv);
    rb_gc_mark_maybe(self_t->maxv);
    rb_gc_mark_maybe(self_t->name);
    rb_gc_mark_maybe(self_t->desc);
    rb_gc_mark_maybe(self_t->strokes);
    rb_gc_mark_maybe(self_t->point_type);
    rb_gc_mark_maybe(self_t->color);
}

void shoes_chart_series_free(shoes_chart_series *self_t) {
    RUBY_CRITICAL(SHOE_FREE(self_t));
}

VALUE shoes_chart_series_alloc(VALUE klass) {
    VALUE obj;
    shoes_chart_series *ser = SHOE_ALLOC(shoes_chart_series);
    SHOE_MEMZERO(ser, shoes_chart_series, 1);
    obj = Data_Wrap_Struct(klass, shoes_chart_series_mark, shoes_chart_series_free, ser);
    ser->values = rb_ary_new();
    ser->labels = rb_ary_new();
    ser->minv = Qnil;
    ser->maxv = Qnil;
    ser->name = Qnil;
    ser->desc = Qnil;
    ser->strokes = Qnil;
    ser->point_type = Qnil;
    ser->color = Qnil;
    return obj;
}

// This is called from plot.c shoes_plot_add()
void shoes_chart_series_Cinit(shoes_chart_series *self_t, VALUE rbvals, VALUE rblabels,
                              VALUE rbmax, VALUE rbmin, VALUE rbname, VALUE rbdesc, VALUE  rbstroke,
                              VALUE rbpoint_type, VALUE color_wrapped) {
    self_t->values = rbvals;
    self_t->labels = rblabels;
    self_t->maxv = rbmax;
    self_t->minv = rbmin;
    self_t->name = rbname;
    self_t->desc = rbdesc;
    if (NIL_P(rbdesc)) {
        //printf("fixme: no long name\n");
        self_t->desc = rbname;
    }
    self_t->strokes = rbstroke;
    self_t->point_type = rbpoint_type;
    self_t->color = color_wrapped;
}

// This is not a visible Shoes widget but it is a Shoes class.
VALUE shoes_chart_series_new(int argc, VALUE *argv, VALUE self) {
    shoes_chart_series *self_t;
    VALUE newseries = Qnil;
    VALUE rbsz, rbvals, rblabels, rbmin, rbmax, rbname, rbdesc, rbcolor;
    VALUE rbstroke, rbpoint, rbpoint_type  = Qnil;
    VALUE color_wrapped = Qnil;
    rb_arg_list args;
    switch (rb_parse_args(argc, argv, "h", &args)) {
        case 1:
            newseries = args.a[0];
    }

    if (TYPE(newseries) == T_HASH) {
        rbvals = shoes_hash_get(newseries, rb_intern("values"));
        rblabels = shoes_hash_get(newseries, rb_intern("labels"));
        rbmin = shoes_hash_get(newseries, rb_intern("min"));
        rbmax = shoes_hash_get(newseries, rb_intern("max"));
        rbname = shoes_hash_get(newseries, rb_intern("name"));
        rbdesc = shoes_hash_get(newseries, rb_intern("desc"));
        rbcolor  = shoes_hash_get(newseries, rb_intern("color"));
        rbstroke = shoes_hash_get(newseries, rb_intern("strokewidth"));
        rbpoint = shoes_hash_get(newseries, rb_intern("points"));

        if ( NIL_P(rbvals) || TYPE(rbvals) != T_ARRAY ) {
            rb_raise(rb_eArgError, "plot.add: Missing an Array of values");
        }
        int valsz = RARRAY_LEN(rbvals);
        rbsz = INT2NUM(valsz);
        if (NIL_P(rbmin) || NIL_P(rbmax)) {
            rb_raise(rb_eArgError, "plot.add: Missing min: or max: option");
        }
        if ( NIL_P(rblabels)) {
            // we can fake it - poorly - TODO: call a user given proc ?
            int l = NUM2INT(rbsz);
            int i;
            rblabels = rb_ary_new2(l);
            for (i = 0; i < l; i++) {
                char t[8];
                sprintf(t, "%i", i+1);
                VALUE foostr = rb_str_new2(t);
                rb_ary_store(rblabels, i, foostr);
            }
        }

        if (NIL_P(rbname))
            rb_raise(rb_eArgError, "plot.add missing name:");
        if (NIL_P(rbdesc)) {
            rbdesc = rbname;
        }
        // handle colors
        if (! NIL_P(rbcolor)) {
            if (rb_obj_is_kind_of(rbcolor, cColor)) {
                color_wrapped = rbcolor;
            } else {
                if (TYPE(rbcolor) != T_STRING)
                    rb_raise(rb_eArgError, "plot.add color must be a string");
                char *cstr = RSTRING_PTR(rbcolor);
                color_wrapped = shoes_hash_get(cColors, rb_intern(cstr));
                if (NIL_P(color_wrapped))
                    rb_raise(rb_eArgError, "plot.add color: not a known color");
            }
        } else {
            // will have to accept the default colors of the chart
            color_wrapped = Qnil;
        }

        if (!NIL_P(rbstroke)) {
            if (TYPE(rbstroke) != T_FIXNUM)
                rb_raise(rb_eArgError, "plot.add strokewidth not an integer\n");
        } else {
            rbstroke = INT2NUM(1); // default
        }

        // This is weird. We handle :true, :false/nil and string.
        if (NIL_P(rbpoint)) {
            rbpoint_type = INT2NUM(NUB_NONE);
        } else {
            rbpoint_type = shoes_chart_series_parse_points(rbpoint);
        }
#if 0
        if (TYPE(rbpoint) == T_STRING) {
            char *req = RSTRING_PTR(rbpoint);
            if (!strcmp(req, "dot"))
                rbpoint_type = INT2NUM(NUB_DOT);
            else if (!strcmp(req, "circle"))
                rbpoint_type = INT2NUM(NUB_CIRCLE);
            else if (!strcmp(req, "box"))
                rbpoint_type = INT2NUM(NUB_BOX);
            else if (!strcmp(req, "rect"))
                rbpoint_type = INT2NUM(NUB_RECT);
            else
                rb_raise(rb_eArgError, "plot.add points: string does not match known types\n");
        } else if (TYPE(rbpoint) == T_TRUE) {
            rbpoint_type = INT2NUM(NUB_DOT);
        } else if (TYPE(rbpoint) == T_FALSE) {
            rbpoint_type = INT2NUM(NUB_NONE);
        }
#endif
    } else {
        rb_raise(rb_eArgError, "misssing something in plot.add \n");
    }
    VALUE obj = shoes_chart_series_alloc(cChartSeries);
    Data_Get_Struct(obj, shoes_chart_series, self_t);
    shoes_chart_series_Cinit(self_t, rbvals, rblabels, rbmax, rbmin, rbname, rbdesc,
                             rbstroke, rbpoint_type, color_wrapped);
    return obj;
}

// ugly
static VALUE shoes_chart_series_parse_points(VALUE rbpoint) {
    // This is weird. We handle :true, :false/nil and string.
    VALUE rbpoint_type;
    if (NIL_P(rbpoint)) {
        rbpoint_type = INT2NUM(NUB_NONE);
        return rbpoint_type;
    }
    if (TYPE(rbpoint) == T_STRING) {
        char *req = RSTRING_PTR(rbpoint);
        if (!strcmp(req, "dot"))
            rbpoint_type = INT2NUM(NUB_DOT);
        else if (!strcmp(req, "circle"))
            rbpoint_type = INT2NUM(NUB_CIRCLE);
        else if (!strcmp(req, "box"))
            rbpoint_type = INT2NUM(NUB_BOX);
        else if (!strcmp(req, "rect"))
            rbpoint_type = INT2NUM(NUB_RECT);
        else
            rb_raise(rb_eArgError, "plot.add points: string does not match known types\n");
    } else if (TYPE(rbpoint) == T_TRUE) {
        rbpoint_type = INT2NUM(NUB_DOT);
    } else if (TYPE(rbpoint) == T_FALSE) {
        rbpoint_type = INT2NUM(NUB_NONE);
    } else {
        rb_raise(rb_eArgError, "misssing something in plot.add \n");
    }
    return rbpoint_type;
}

// parse - Returns an mostly or entirely ruby  array of arrays
// used by radar charts called from shoes_plot_new() plot.c
VALUE shoes_plot_parse_column_settings(VALUE opts) {
    if (TYPE(opts) != T_ARRAY)
        rb_raise(rb_eArgError, "column_settings is not an array");
    int count = RARRAY_LEN(opts);
    int i;
    VALUE outer_ary = rb_ary_new_capa(count);
    for (i = 0; i < count; i++) {
        VALUE rbcol = rb_ary_entry(opts, i);
        if (TYPE(rbcol) == T_ARRAY) {
            rb_ary_store(outer_ary, i, rbcol);
        } else if (TYPE(rbcol) != T_HASH) {
            rb_raise(rb_eArgError, "one of column_settings[] is not an array or hash");
        } else {
            // we know its a hash.
            VALUE inner_ary = rb_ary_new();
            VALUE rblabel, rbmin, rbmax, rbfmt;
            rblabel = shoes_hash_get(rbcol, rb_intern("label"));
            rbmin = shoes_hash_get(rbcol, rb_intern("min"));
            rbmax = shoes_hash_get(rbcol, rb_intern("max"));
            rbfmt = shoes_hash_get(rbcol, rb_intern("format"));
            rb_ary_store(inner_ary, 0, rblabel);
            rb_ary_store(inner_ary, 1, rbmin);
            rb_ary_store(inner_ary, 2, rbmax);
            if (! NIL_P(rbfmt))
                rb_ary_store(inner_ary, 3, rbfmt);

            rb_ary_store(outer_ary, i, inner_ary);
        }
    }
    return outer_ary;
}

// Simple getter/setter  methods
VALUE shoes_chart_series_values(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->values;
}

VALUE shoes_chart_series_labels(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->labels;
}

VALUE shoes_chart_series_min(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->minv;
}

VALUE shoes_chart_series_min_set(VALUE self, VALUE val) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    cs->minv = val;
    return cs->minv;
}

VALUE shoes_chart_series_max(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->maxv;
}

VALUE shoes_chart_series_max_set(VALUE self, VALUE val) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    cs->maxv = val;
    return cs->maxv;
}

VALUE shoes_chart_series_name(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->name;
}

VALUE shoes_chart_series_desc(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->desc;
}

VALUE shoes_chart_series_desc_set(VALUE self, VALUE str) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    cs->desc = str;
    return cs->desc;
}
VALUE shoes_chart_series_color(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->color;
}

VALUE shoes_chart_series_color_set(VALUE self, VALUE clr) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    cs->color = clr;
    return cs->color;
}

VALUE shoes_chart_series_strokewidth(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->strokes;
}

VALUE shoes_chart_series_strokewidth_set(VALUE self, VALUE wid) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    cs->strokes = wid;
    return cs->strokes;
}

VALUE shoes_chart_series_points(VALUE self) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    return cs->point_type;
}

VALUE shoes_chart_series_points_set(VALUE self, VALUE pt) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    if (TYPE(pt) == T_FIXNUM)
        cs->point_type = pt;
    else
        cs->point_type =  shoes_chart_series_parse_points(pt);
    return cs->point_type;
}

// ---- more interesting methods ----

// Returns and an array[label, value] at idx
VALUE shoes_chart_series_get(VALUE self, VALUE idx) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    VALUE nary = Qnil;
    if (TYPE(idx) == T_FIXNUM) {
        nary = rb_ary_new_capa(2);
        long i  = NUM2INT(idx);
        rb_ary_store(nary, 0, rb_ary_entry(cs->labels, i));
        rb_ary_store(nary, 1, rb_ary_entry(cs->values, i));
    }
    return nary;
}

// Sets labels[idx] and values[idx]
VALUE shoes_chart_series_set(VALUE self, VALUE idx, VALUE ary) {
    shoes_chart_series *cs;
    Data_Get_Struct(self, shoes_chart_series, cs);
    if ((TYPE(idx) == T_FIXNUM) && (TYPE(ary) == T_ARRAY) && (RARRAY_LEN(ary) == 2)) {
        long i  = NUM2INT(idx);
        rb_ary_store(cs->labels, i, rb_ary_entry(ary, 0));
        rb_ary_store(cs->values, i, rb_ary_entry(ary, 1));
    } else
        rb_raise(rb_eArgError, "bad arguments to chart_series.set");
    return Qtrue;
}
