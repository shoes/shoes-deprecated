#include "shoes/types/pattern.h"
#include "shoes/types/color.h"

// ruby
VALUE cColor, cColors;

void shoes_color_init() {
    cColor   = rb_define_class_under(cTypes, "Color", rb_cObject);

    rb_define_alloc_func(cColor, shoes_color_alloc);
    rb_define_method(rb_mKernel, "rgb", CASTHOOK(shoes_color_rgb), -1);
    rb_define_method(rb_mKernel, "gray", CASTHOOK(shoes_color_gray), -1);
    rb_define_singleton_method(cColor, "rgb", CASTHOOK(shoes_color_rgb), -1);
    rb_define_singleton_method(cColor, "gray", CASTHOOK(shoes_color_gray), -1);
    rb_define_singleton_method(cColor, "parse", CASTHOOK(shoes_color_parse), 1);

    rb_include_module(cColor, rb_mComparable);
    rb_define_method(cColor, "<=>", CASTHOOK(shoes_color_spaceship), 1);
    rb_define_method(cColor, "==", CASTHOOK(shoes_color_equal), 1);
    rb_define_method(cColor, "eql?", CASTHOOK(shoes_color_equal), 1);
    rb_define_method(cColor, "red", CASTHOOK(shoes_color_get_red), 0);
    rb_define_method(cColor, "green", CASTHOOK(shoes_color_get_green), 0);
    rb_define_method(cColor, "blue", CASTHOOK(shoes_color_get_blue), 0);
    rb_define_method(cColor, "alpha", CASTHOOK(shoes_color_get_alpha), 0);
    rb_define_method(cColor, "black?", CASTHOOK(shoes_color_is_black), 0);
    rb_define_method(cColor, "dark?", CASTHOOK(shoes_color_is_dark), 0);
    rb_define_method(cColor, "inspect", CASTHOOK(shoes_color_to_s), 0);
    rb_define_method(cColor, "invert", CASTHOOK(shoes_color_invert), 0);
    rb_define_method(cColor, "light?", CASTHOOK(shoes_color_is_light), 0);
    rb_define_method(cColor, "opaque?", CASTHOOK(shoes_color_is_opaque), 0);
    rb_define_method(cColor, "to_s", CASTHOOK(shoes_color_to_s), 0);
    rb_define_method(cColor, "to_pattern", CASTHOOK(shoes_color_to_pattern), 0);
    rb_define_method(cColor, "transparent?", CASTHOOK(shoes_color_is_transparent), 0);
    rb_define_method(cColor, "white?", CASTHOOK(shoes_color_is_white), 0);
    
    rb_define_method(cCanvas, "method_missing", CASTHOOK(shoes_color_method_missing), -1);
    
    rb_define_method(rb_mKernel, "rgb", CASTHOOK(shoes_color_rgb), -1);
    rb_define_method(rb_mKernel, "gray", CASTHOOK(shoes_color_gray), -1);
    rb_define_method(rb_mKernel, "gradient", CASTHOOK(shoes_color_gradient), -1);

    rb_const_set(cTypes, rb_intern("ALL_NAMED_COLORS"), rb_hash_new());
    cColors = rb_const_get(cTypes, rb_intern("ALL_NAMED_COLORS"));
    rb_const_set(cTypes, rb_intern("COLORS"), cColors);
    DEF_COLOR(aliceblue, 240, 248, 255);
    DEF_COLOR(antiquewhite, 250, 235, 215);
    DEF_COLOR(aqua, 0, 255, 255);
    DEF_COLOR(aquamarine, 127, 255, 212);
    DEF_COLOR(azure, 240, 255, 255);
    DEF_COLOR(beige, 245, 245, 220);
    DEF_COLOR(bisque, 255, 228, 196);
    DEF_COLOR(black, 0, 0, 0);
    DEF_COLOR(blanchedalmond, 255, 235, 205);
    DEF_COLOR(blue, 0, 0, 255);
    DEF_COLOR(blueviolet, 138, 43, 226);
    DEF_COLOR(brown, 165, 42, 42);
    DEF_COLOR(burlywood, 222, 184, 135);
    DEF_COLOR(cadetblue, 95, 158, 160);
    DEF_COLOR(chartreuse, 127, 255, 0);
    DEF_COLOR(chocolate, 210, 105, 30);
    DEF_COLOR(coral, 255, 127, 80);
    DEF_COLOR(cornflowerblue, 100, 149, 237);
    DEF_COLOR(cornsilk, 255, 248, 220);
    DEF_COLOR(crimson, 220, 20, 60);
    DEF_COLOR(cyan, 0, 255, 255);
    DEF_COLOR(darkblue, 0, 0, 139);
    DEF_COLOR(darkcyan, 0, 139, 139);
    DEF_COLOR(darkgoldenrod, 184, 134, 11);
    DEF_COLOR(darkgray, 169, 169, 169);
    DEF_COLOR(darkgreen, 0, 100, 0);
    DEF_COLOR(darkkhaki, 189, 183, 107);
    DEF_COLOR(darkmagenta, 139, 0, 139);
    DEF_COLOR(darkolivegreen, 85, 107, 47);
    DEF_COLOR(darkorange, 255, 140, 0);
    DEF_COLOR(darkorchid, 153, 50, 204);
    DEF_COLOR(darkred, 139, 0, 0);
    DEF_COLOR(darksalmon, 233, 150, 122);
    DEF_COLOR(darkseagreen, 143, 188, 143);
    DEF_COLOR(darkslateblue, 72, 61, 139);
    DEF_COLOR(darkslategray, 47, 79, 79);
    DEF_COLOR(darkturquoise, 0, 206, 209);
    DEF_COLOR(darkviolet, 148, 0, 211);
    DEF_COLOR(deeppink, 255, 20, 147);
    DEF_COLOR(deepskyblue, 0, 191, 255);
    DEF_COLOR(dimgray, 105, 105, 105);
    DEF_COLOR(dodgerblue, 30, 144, 255);
    DEF_COLOR(firebrick, 178, 34, 34);
    DEF_COLOR(floralwhite, 255, 250, 240);
    DEF_COLOR(forestgreen, 34, 139, 34);
    DEF_COLOR(fuchsia, 255, 0, 255);
    DEF_COLOR(gainsboro, 220, 220, 220);
    DEF_COLOR(ghostwhite, 248, 248, 255);
    DEF_COLOR(gold, 255, 215, 0);
    DEF_COLOR(goldenrod, 218, 165, 32);
    DEF_COLOR(gray, 128, 128, 128);
    DEF_COLOR(green, 0, 128, 0);
    DEF_COLOR(greenyellow, 173, 255, 47);
    DEF_COLOR(honeydew, 240, 255, 240);
    DEF_COLOR(hotpink, 255, 105, 180);
    DEF_COLOR(indianred, 205, 92, 92);
    DEF_COLOR(indigo, 75, 0, 130);
    DEF_COLOR(ivory, 255, 255, 240);
    DEF_COLOR(khaki, 240, 230, 140);
    DEF_COLOR(lavender, 230, 230, 250);
    DEF_COLOR(lavenderblush, 255, 240, 245);
    DEF_COLOR(lawngreen, 124, 252, 0);
    DEF_COLOR(lemonchiffon, 255, 250, 205);
    DEF_COLOR(lightblue, 173, 216, 230);
    DEF_COLOR(lightcoral, 240, 128, 128);
    DEF_COLOR(lightcyan, 224, 255, 255);
    DEF_COLOR(lightgoldenrodyellow, 250, 250, 210);
    DEF_COLOR(lightgreen, 144, 238, 144);
    DEF_COLOR(lightgrey, 211, 211, 211);
    DEF_COLOR(lightpink, 255, 182, 193);
    DEF_COLOR(lightsalmon, 255, 160, 122);
    DEF_COLOR(lightseagreen, 32, 178, 170);
    DEF_COLOR(lightskyblue, 135, 206, 250);
    DEF_COLOR(lightslategray, 119, 136, 153);
    DEF_COLOR(lightsteelblue, 176, 196, 222);
    DEF_COLOR(lightyellow, 255, 255, 224);
    DEF_COLOR(lime, 0, 255, 0);
    DEF_COLOR(limegreen, 50, 205, 50);
    DEF_COLOR(linen, 250, 240, 230);
    DEF_COLOR(magenta, 255, 0, 255);
    DEF_COLOR(maroon, 128, 0, 0);
    DEF_COLOR(mediumaquamarine, 102, 205, 170);
    DEF_COLOR(mediumblue, 0, 0, 205);
    DEF_COLOR(mediumorchid, 186, 85, 211);
    DEF_COLOR(mediumpurple, 147, 112, 219);
    DEF_COLOR(mediumseagreen, 60, 179, 113);
    DEF_COLOR(mediumslateblue, 123, 104, 238);
    DEF_COLOR(mediumspringgreen, 0, 250, 154);
    DEF_COLOR(mediumturquoise, 72, 209, 204);
    DEF_COLOR(mediumvioletred, 199, 21, 133);
    DEF_COLOR(midnightblue, 25, 25, 112);
    DEF_COLOR(mintcream, 245, 255, 250);
    DEF_COLOR(mistyrose, 255, 228, 225);
    DEF_COLOR(moccasin, 255, 228, 181);
    DEF_COLOR(navajowhite, 255, 222, 173);
    DEF_COLOR(navy, 0, 0, 128);
    DEF_COLOR(oldlace, 253, 245, 230);
    DEF_COLOR(olive, 128, 128, 0);
    DEF_COLOR(olivedrab, 107, 142, 35);
    DEF_COLOR(orange, 255, 165, 0);
    DEF_COLOR(orangered, 255, 69, 0);
    DEF_COLOR(orchid, 218, 112, 214);
    DEF_COLOR(palegoldenrod, 238, 232, 170);
    DEF_COLOR(palegreen, 152, 251, 152);
    DEF_COLOR(paleturquoise, 175, 238, 238);
    DEF_COLOR(palevioletred, 219, 112, 147);
    DEF_COLOR(papayawhip, 255, 239, 213);
    DEF_COLOR(peachpuff, 255, 218, 185);
    DEF_COLOR(peru, 205, 133, 63);
    DEF_COLOR(pink, 255, 192, 203);
    DEF_COLOR(plum, 221, 160, 221);
    DEF_COLOR(powderblue, 176, 224, 230);
    DEF_COLOR(purple, 128, 0, 128);
    DEF_COLOR(red, 255, 0, 0);
    DEF_COLOR(rosybrown, 188, 143, 143);
    DEF_COLOR(royalblue, 65, 105, 225);
    DEF_COLOR(saddlebrown, 139, 69, 19);
    DEF_COLOR(salmon, 250, 128, 114);
    DEF_COLOR(sandybrown, 244, 164, 96);
    DEF_COLOR(seagreen, 46, 139, 87);
    DEF_COLOR(seashell, 255, 245, 238);
    DEF_COLOR(sienna, 160, 82, 45);
    DEF_COLOR(silver, 192, 192, 192);
    DEF_COLOR(skyblue, 135, 206, 235);
    DEF_COLOR(slateblue, 106, 90, 205);
    DEF_COLOR(slategray, 112, 128, 144);
    DEF_COLOR(snow, 255, 250, 250);
    DEF_COLOR(springgreen, 0, 255, 127);
    DEF_COLOR(steelblue, 70, 130, 180);
    DEF_COLOR(tan, 210, 180, 140);
    DEF_COLOR(teal, 0, 128, 128);
    DEF_COLOR(thistle, 216, 191, 216);
    DEF_COLOR(tomato, 255, 99, 71);
    DEF_COLOR(turquoise, 64, 224, 208);
    DEF_COLOR(violet, 238, 130, 238);
    DEF_COLOR(wheat, 245, 222, 179);
    DEF_COLOR(white, 255, 255, 255);
    DEF_COLOR(whitesmoke, 245, 245, 245);
    DEF_COLOR(yellow, 255, 255, 0);
    DEF_COLOR(yellowgreen, 154, 205, 50);
}

// ruby
void shoes_color_mark(shoes_color *color) {
}

void shoes_color_free(shoes_color *color) {
    RUBY_CRITICAL(free(color));
}

VALUE shoes_color_new(int r, int g, int b, int a) {
    shoes_color *color;
    VALUE obj = shoes_color_alloc(cColor);
    Data_Get_Struct(obj, shoes_color, color);
    color->r = r;
    color->g = g;
    color->b = b;
    color->a = a;
    return obj;
}

VALUE shoes_color_alloc(VALUE klass) {
    VALUE obj;
    shoes_color *color = SHOE_ALLOC(shoes_color);
    SHOE_MEMZERO(color, shoes_color, 1);
    obj = Data_Wrap_Struct(klass, shoes_color_mark, shoes_color_free, color);
    color->a = SHOES_COLOR_OPAQUE;
    color->on = TRUE;
    return obj;
}

VALUE shoes_color_rgb(int argc, VALUE *argv, VALUE self) {
    int a;
    VALUE _r, _g, _b, _a;
    rb_scan_args(argc, argv, "31", &_r, &_g, &_b, &_a);

    a = SHOES_COLOR_OPAQUE;
    if (!NIL_P(_a)) a = NUM2RGBINT(_a);
    return shoes_color_new(NUM2RGBINT(_r), NUM2RGBINT(_g), NUM2RGBINT(_b), a);
}

VALUE shoes_color_gradient(int argc, VALUE *argv, VALUE self) {
    shoes_pattern *pattern;
    VALUE obj, r1, r2;
    VALUE attr = Qnil;
    rb_scan_args(argc, argv, "21", &r1, &r2, &attr);
    CHECK_HASH(attr);

    obj = shoes_pattern_alloc(cPattern);
    Data_Get_Struct(obj, shoes_pattern, pattern);
    pattern->source = Qnil;
    shoes_pattern_gradient(pattern, r1, r2, attr);
    return obj;
}

VALUE shoes_color_gray(int argc, VALUE *argv, VALUE self) {
    VALUE _g, _a;
    int g, a;
    rb_scan_args(argc, argv, "02", &_g, &_a);

    a = SHOES_COLOR_OPAQUE;
    g = 128;
    if (!NIL_P(_g)) g = NUM2RGBINT(_g);
    if (!NIL_P(_a)) a = NUM2RGBINT(_a);
    return shoes_color_new(g, g, g, a);
}

cairo_pattern_t *shoes_color_pattern(VALUE self) {
    GET_STRUCT(color, color);
    if (color->a == 255)
        return cairo_pattern_create_rgb(color->r / 255., color->g / 255., color->b / 255.);
    else
        return cairo_pattern_create_rgba(color->r / 255., color->g / 255., color->b / 255., color->a / 255.);
}

void shoes_color_grad_stop(cairo_pattern_t *pattern, double stop, VALUE self) {
    GET_STRUCT(color, color);
    if (color->a == 255)
        return cairo_pattern_add_color_stop_rgb(pattern, stop, color->r / 255., color->g / 255., color->b / 255.);
    else
        return cairo_pattern_add_color_stop_rgba(pattern, stop, color->r / 255., color->g / 255., color->b / 255., color->a / 255.);
}

VALUE shoes_color_args(int argc, VALUE *argv, VALUE self) {
    VALUE _r, _g, _b, _a, _color;
    argc = rb_scan_args(argc, argv, "13", &_r, &_g, &_b, &_a);

    if (argc == 1 && rb_obj_is_kind_of(_r, cColor))
        _color = _r;
    else if (argc == 1 && rb_obj_is_kind_of(_r, rb_cString))
        _color = shoes_color_parse(cColor, _r);
    else if (argc == 1 || argc == 2)
        _color = shoes_color_gray(argc, argv, cColor);
    else
        _color = shoes_color_rgb(argc, argv, cColor);

    return _color;
}

VALUE shoes_color_parse(VALUE self, VALUE source) {
    VALUE reg;

    reg = rb_funcall(source, s_match, 1, reHEX3_SOURCE);
    if (!NIL_P(reg)) {
        NEW_COLOR(color, obj);
        color->r = NUM2INT(rb_str2inum(rb_reg_nth_match(1, reg), 16)) * 17;
        color->g = NUM2INT(rb_str2inum(rb_reg_nth_match(2, reg), 16)) * 17;
        color->b = NUM2INT(rb_str2inum(rb_reg_nth_match(3, reg), 16)) * 17;
        return obj;
    }

    reg = rb_funcall(source, s_match, 1, reHEX_SOURCE);
    if (!NIL_P(reg)) {
        NEW_COLOR(color, obj);
        color->r = NUM2INT(rb_str2inum(rb_reg_nth_match(1, reg), 16));
        color->g = NUM2INT(rb_str2inum(rb_reg_nth_match(2, reg), 16));
        color->b = NUM2INT(rb_str2inum(rb_reg_nth_match(3, reg), 16));
        return obj;
    }

    reg = rb_funcall(source, s_match, 1, reRGB_SOURCE);
    if (!NIL_P(reg)) {
        NEW_COLOR(color, obj);
        color->r = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
        color->g = NUM2INT(rb_Integer(rb_reg_nth_match(2, reg)));
        color->b = NUM2INT(rb_Integer(rb_reg_nth_match(3, reg)));
        return obj;
    }

    reg = rb_funcall(source, s_match, 1, reRGBA_SOURCE);
    if (!NIL_P(reg)) {
        NEW_COLOR(color, obj);
        color->r = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
        color->g = NUM2INT(rb_Integer(rb_reg_nth_match(2, reg)));
        color->b = NUM2INT(rb_Integer(rb_reg_nth_match(3, reg)));
        color->a = NUM2INT(rb_Integer(rb_reg_nth_match(4, reg)));
        return obj;
    }

    reg = rb_funcall(source, s_match, 1, reGRAY_SOURCE);
    if (!NIL_P(reg)) {
        NEW_COLOR(color, obj);
        color->r = color->g = color->b = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
        return obj;
    }

    reg = rb_funcall(source, s_match, 1, reGRAYA_SOURCE);
    if (!NIL_P(reg)) {
        NEW_COLOR(color, obj);
        color->r = color->g = color->b = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
        color->a = NUM2INT(rb_Integer(rb_reg_nth_match(2, reg)));
        return obj;
    }

    return Qnil;
}

VALUE shoes_color_spaceship(VALUE self, VALUE c2) {
    int v1, v2;
    shoes_color *color2;
    GET_STRUCT(color, color);
    if (!rb_obj_is_kind_of(c2, cColor)) return Qnil;
    Data_Get_Struct(c2, shoes_color, color2);
    v1 = color->r + color->g + color->b;
    v2 = color2->r + color2->g + color2->b;
    if (v1 == v2) return INT2FIX(0);
    if (v1 > v2)  return INT2FIX(1);
    else          return INT2FIX(-1);
}

VALUE shoes_color_equal(VALUE self, VALUE c2) {
    if (!rb_obj_is_kind_of(c2, cColor)) return Qnil;

    GET_STRUCT(color, color);
    shoes_color *color2;
    Data_Get_Struct(c2, shoes_color, color2);
    if (color->r == color2->r && color->g == color2->g &&
            color->b == color2->b && color->a == color2->a ) {
        return Qtrue;
    } else return Qfalse;
}

VALUE shoes_color_get_red(VALUE self) {
    GET_STRUCT(color, color);
    return INT2NUM(color->r);
}

VALUE shoes_color_get_green(VALUE self) {
    GET_STRUCT(color, color);
    return INT2NUM(color->g);
}

VALUE shoes_color_get_blue(VALUE self) {
    GET_STRUCT(color, color);
    return INT2NUM(color->b);
}

VALUE shoes_color_get_alpha(VALUE self) {
    GET_STRUCT(color, color);
    return INT2NUM(color->a);
}

VALUE shoes_color_is_black(VALUE self) {
    GET_STRUCT(color, color);
    return (color->r + color->g + color->b == 0) ? Qtrue : Qfalse;
}

VALUE shoes_color_is_dark(VALUE self) {
    GET_STRUCT(color, color);
    return ((int)color->r + (int)color->g + (int)color->b < SHOES_COLOR_DARK) ? Qtrue : Qfalse;
}

VALUE shoes_color_is_light(VALUE self) {
    GET_STRUCT(color, color);
    return ((int)color->r + (int)color->g + (int)color->b > SHOES_COLOR_LIGHT) ? Qtrue : Qfalse;
}

VALUE shoes_color_is_opaque(VALUE self) {
    GET_STRUCT(color, color);
    return (color->a == SHOES_COLOR_OPAQUE) ? Qtrue : Qfalse;
}

VALUE shoes_color_is_transparent(VALUE self) {
    GET_STRUCT(color, color);
    return (color->a == SHOES_COLOR_TRANSPARENT) ? Qtrue : Qfalse;
}

VALUE shoes_color_is_white(VALUE self) {
    GET_STRUCT(color, color);
    return (color->r + color->g + color->b == 765) ? Qtrue : Qfalse;
}

VALUE shoes_color_invert(VALUE self) {
    GET_STRUCT(color, color);
    NEW_COLOR(color2, obj);
    color2->r = 255 - color->r;
    color2->g = 255 - color->g;
    color2->b = 255 - color->b;
    color2->a = color->a;
    return obj;
}

VALUE shoes_color_to_s(VALUE self) {
    GET_STRUCT(color, color);

    VALUE ary = rb_ary_new3(4,
                            INT2NUM(color->r), INT2NUM(color->g), INT2NUM(color->b),
                            rb_float_new((color->a * 1.) / 255.));

    if (color->a == 255)
        return rb_funcall(rb_str_new2("rgb(%d, %d, %d)"), s_perc, 1, ary);
    else
        return rb_funcall(rb_str_new2("rgb(%d, %d, %d, %0.1f)"), s_perc, 1, ary);
}

// TODO: deprecated code remove after 2017-10
VALUE shoes_color_to_pattern(VALUE self) {
    return shoes_pattern_method(cPattern, self);
}

VALUE shoes_color_method_missing(int argc, VALUE *argv, VALUE self) {
    VALUE alpha;
    VALUE cname = argv[0];
    VALUE c = Qnil;
    if (rb_obj_is_kind_of(cColors, rb_cHash))
        c = rb_hash_aref(cColors, cname);
    if (NIL_P(c)) {
        self = rb_inspect(self);
        rb_raise(rb_eNoMethodError, "undefined method `%s' for %s",
                 rb_id2name(SYM2ID(cname)), RSTRING_PTR(self));
    }

    rb_scan_args(argc, argv, "11", &cname, &alpha);
    if (!NIL_P(alpha)) {
        shoes_color *color;
        Data_Get_Struct(c, shoes_color, color);
        c = shoes_color_new(color->r, color->g, color->b, NUM2RGBINT(alpha));
    }

    return c;
}