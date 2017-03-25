#include "shoes/types/color.h"
#include "shoes/types/native.h"
#include "shoes/types/shape.h"
#include "shoes/types/text.h"
#include "shoes/types/text_link.h"
#include "shoes/types/textblock.h"

// ruby
VALUE cTextBlock, cPara, cBanner, cTitle, cSubtitle, cTagline, cCaption, cInscription;

FUNC_M("+para", para, -1);
FUNC_M("+banner", banner, -1);
FUNC_M("+title", title, -1);
FUNC_M("+subtitle", subtitle, -1);
FUNC_M("+tagline", tagline, -1);
FUNC_M("+caption", caption, -1);
FUNC_M("+inscription", inscription, -1);

PLACE_COMMON(textblock);
CLASS_COMMON2(textblock);
REPLACE_COMMON(textblock);

MARKUP_DEF(para, BLOCK, cPara);
MARKUP_DEF(banner, BLOCK, cBanner);
MARKUP_DEF(title, BLOCK, cTitle);
MARKUP_DEF(subtitle, BLOCK, cSubtitle);
MARKUP_DEF(tagline, BLOCK, cTagline);
MARKUP_DEF(caption, BLOCK, cCaption);
MARKUP_DEF(inscription, BLOCK, cInscription);

static void shoes_textblock_iter_pango(VALUE texts, shoes_textblock *block, shoes_app *app);
static void shoes_textblock_make_pango(shoes_app *app, VALUE klass, shoes_textblock *block);
static void shoes_textblock_on_layout(shoes_app *app, VALUE klass, shoes_textblock *block);
static void shoes_app_style_for(shoes_textblock *block, shoes_app *app, VALUE klass, VALUE oattr, guint start_index, guint end_index);

void shoes_textblock_init() {
    cTextBlock = rb_define_class_under(cTypes, "TextBlock", rb_cObject);

    rb_define_alloc_func(cTextBlock, shoes_textblock_alloc);

    rb_define_method(cTextBlock, "app", CASTHOOK(shoes_canvas_get_app), 0);
    rb_define_method(cTextBlock, "contents", CASTHOOK(shoes_textblock_children), 0);
    rb_define_method(cTextBlock, "children", CASTHOOK(shoes_textblock_children), 0);
    rb_define_method(cTextBlock, "parent", CASTHOOK(shoes_textblock_get_parent), 0);
    rb_define_method(cTextBlock, "displace", CASTHOOK(shoes_textblock_displace), 2);
    rb_define_method(cTextBlock, "draw", CASTHOOK(shoes_textblock_draw), 2);
    rb_define_method(cTextBlock, "cursor=", CASTHOOK(shoes_textblock_set_cursor), 1);
    rb_define_method(cTextBlock, "cursor", CASTHOOK(shoes_textblock_get_cursor), 0);
    rb_define_method(cTextBlock, "cursor_left", CASTHOOK(shoes_textblock_cursorx), 0);
    rb_define_method(cTextBlock, "cursor_top", CASTHOOK(shoes_textblock_cursory), 0);
    rb_define_method(cTextBlock, "highlight", CASTHOOK(shoes_textblock_get_highlight), 0);
    rb_define_method(cTextBlock, "hit", CASTHOOK(shoes_textblock_hit), 2);
    rb_define_method(cTextBlock, "marker=", CASTHOOK(shoes_textblock_set_marker), 1);
    rb_define_method(cTextBlock, "marker", CASTHOOK(shoes_textblock_get_marker), 0);
    rb_define_method(cTextBlock, "move", CASTHOOK(shoes_textblock_move), 2);
    rb_define_method(cTextBlock, "top", CASTHOOK(shoes_textblock_get_top), 0);
    rb_define_method(cTextBlock, "left", CASTHOOK(shoes_textblock_get_left), 0);
    rb_define_method(cTextBlock, "width", CASTHOOK(shoes_textblock_get_width), 0);
    rb_define_method(cTextBlock, "height", CASTHOOK(shoes_textblock_get_height), 0);
    rb_define_method(cTextBlock, "remove", CASTHOOK(shoes_basic_remove), 0);
    rb_define_method(cTextBlock, "to_s", CASTHOOK(shoes_textblock_string), 0);
    rb_define_method(cTextBlock, "text", CASTHOOK(shoes_textblock_string), 0);
    rb_define_method(cTextBlock, "text=", CASTHOOK(shoes_textblock_replace), -1);
    rb_define_method(cTextBlock, "replace", CASTHOOK(shoes_textblock_replace), -1);
    rb_define_method(cTextBlock, "style", CASTHOOK(shoes_textblock_style_m), -1);
    rb_define_method(cTextBlock, "hide", CASTHOOK(shoes_textblock_hide), 0);
    rb_define_method(cTextBlock, "show", CASTHOOK(shoes_textblock_show), 0);
    rb_define_method(cTextBlock, "toggle", CASTHOOK(shoes_textblock_toggle), 0);
    rb_define_method(cTextBlock, "click", CASTHOOK(shoes_textblock_click), -1);
    rb_define_method(cTextBlock, "release", CASTHOOK(shoes_textblock_release), -1);
    rb_define_method(cTextBlock, "hover", CASTHOOK(shoes_textblock_hover), -1);
    rb_define_method(cTextBlock, "leave", CASTHOOK(shoes_textblock_leave), -1);

    cPara = rb_define_class_under(cTypes, "Para", cTextBlock);
    cBanner = rb_define_class_under(cTypes, "Banner", cTextBlock);
    cTitle = rb_define_class_under(cTypes, "Title", cTextBlock);
    cSubtitle = rb_define_class_under(cTypes, "Subtitle", cTextBlock);
    cTagline = rb_define_class_under(cTypes, "Tagline", cTextBlock);
    cCaption = rb_define_class_under(cTypes, "Caption", cTextBlock);
    cInscription = rb_define_class_under(cTypes, "Inscription", cTextBlock);

    RUBY_M("+para", para, -1);
    RUBY_M("+banner", banner, -1);
    RUBY_M("+title", title, -1);
    RUBY_M("+subtitle", subtitle, -1);
    RUBY_M("+tagline", tagline, -1);
    RUBY_M("+caption", caption, -1);
    RUBY_M("+inscription", inscription, -1);
}

// ruby
VALUE shoes_textblock_draw(VALUE self, VALUE c, VALUE actual) {
    double crx = 0., cry = 0.;
    int px, py, pd, li, ld;
    cairo_t *cr;
    shoes_canvas *canvas;
    PangoLayoutLine *last;
    PangoRectangle crect, lrect;

    VALUE ck = rb_obj_class(c);
    GET_STRUCT(textblock, self_t);
    Data_Get_Struct(c, shoes_canvas, canvas);
    cr = CCR(canvas);

    if (!NIL_P(self_t->attr) && ATTR(self_t->attr, hidden) == Qtrue)
        return self;

    ATTR_MARGINS(self_t->attr, 4, canvas);
    if (NIL_P(ATTR(self_t->attr, margin)) && NIL_P(ATTR(self_t->attr, margin_bottom)))
        bmargin = 12;
    self_t->place.flags = REL_CANVAS;
    self_t->place.flags |= NIL_P(ATTR(self_t->attr, left)) && NIL_P(ATTR(self_t->attr, right)) ? 0 : FLAG_ABSX;
    self_t->place.flags |= NIL_P(ATTR(self_t->attr, top)) && NIL_P(ATTR(self_t->attr, bottom)) ? 0 : FLAG_ABSY;
    self_t->place.x = ATTR2(int, self_t->attr, left, canvas->cx);
    self_t->place.y = ATTR2(int, self_t->attr, top, canvas->cy);
    if (!ORIGIN(canvas->place)) {
        self_t->place.dx = canvas->place.dx;
        self_t->place.dy = canvas->place.dy;
    } else {
        self_t->place.dx = 0;
        self_t->place.dy = 0;
    }
    self_t->place.dx += PXN(self_t->attr, displace_left, 0, CPW(canvas));
    self_t->place.dy += PXN(self_t->attr, displace_top, 0, CPH(canvas));
    self_t->place.w = ATTR2(int, self_t->attr, width, canvas->place.iw - (canvas->cx - self_t->place.x));
    self_t->place.iw = self_t->place.w - (lmargin + rmargin);
    ld = ATTR2(int, self_t->attr, leading, 4);

    if (self_t->layout != NULL)
        g_object_unref(self_t->layout);

    self_t->layout = pango_cairo_create_layout(cr);
    pd = 0;
    if (!ABSX(self_t->place) && self_t->place.x == canvas->cx) {
        if (self_t->place.x - CPX(canvas) > self_t->place.w) {
            self_t->place.x = CPX(canvas);
            canvas->cy = self_t->place.y = canvas->endy;
        } else {
            if (self_t->place.x > CPX(canvas)) {
                pd = self_t->place.x - CPX(canvas);
                pango_layout_set_indent(self_t->layout, pd * PANGO_SCALE);
                self_t->place.x = CPX(canvas);
            }
        }
    }

    pango_layout_set_width(self_t->layout, self_t->place.iw * PANGO_SCALE);
    pango_layout_set_spacing(self_t->layout, ld * PANGO_SCALE);
    shoes_textblock_on_layout(canvas->app, rb_obj_class(self), self_t);
    pango_layout_set_font_description(self_t->layout, shoes_world->default_font);

    //
    // Line up the first line with the y-cursor
    //
    if (!ABSX(self_t->place) && !ABSY(self_t->place) && pd) {
        last = pango_layout_get_line(self_t->layout, 0);
        pango_layout_line_get_pixel_extents(last, NULL, &lrect);
        if (lrect.width > self_t->place.iw - pd) {
            pango_layout_set_indent(self_t->layout, 0);
            self_t->place.x = CPX(canvas);
            canvas->cy = self_t->place.y = canvas->endy;
            pd = 0;
        }
    }
    self_t->place.ix = self_t->place.x + lmargin;
    self_t->place.iy = self_t->place.y + tmargin;

    li = pango_layout_get_line_count(self_t->layout) - 1;
    last = pango_layout_get_line(self_t->layout, li);
    pango_layout_line_get_pixel_extents(last, NULL, &lrect);
    pango_layout_get_pixel_size(self_t->layout, &px, &py);

    if (self_t->cursor != NULL && self_t->cursor->pos != INT_MAX) {
        int cursor = self_t->cursor->pos;
        if (cursor < 0) cursor += self_t->text->len + 1;
        pango_layout_index_to_pos(self_t->layout, cursor, &crect);
        crx = (self_t->place.ix + self_t->place.dx) + (crect.x / PANGO_SCALE);
        cry = (self_t->place.iy + self_t->place.dy) + (crect.y / PANGO_SCALE);
        self_t->cursor->x = (int)crx;
        self_t->cursor->y = (int)cry;
    }

    if (RTEST(actual)) {
        shoes_apply_transformation(cr, self_t->st, &self_t->place, 0);
        if (shoes_shape_check(cr, &self_t->place)) {
            cairo_move_to(cr, self_t->place.ix + self_t->place.dx, self_t->place.iy + self_t->place.dy);
            cairo_set_source_rgb(cr, 0., 0., 0.);
            pango_cairo_update_layout(cr, self_t->layout);
            pango_cairo_show_layout(cr, self_t->layout);

            if (self_t->cursor != NULL && self_t->cursor->pos != INT_MAX) {
                cairo_save(cr);
                cairo_new_path(cr);
                cairo_move_to(cr, crx, cry);
                cairo_line_to(cr, crx, cry + (crect.height / PANGO_SCALE));
                cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
                cairo_set_source_rgb(cr, 0., 0., 0.);
                cairo_set_line_width(cr, 1.);
                cairo_stroke(cr);
                cairo_restore(cr);
            }
        }

        shoes_undo_transformation(cr, self_t->st, &self_t->place, 0);
    }

    if (li == 0) {
        self_t->place.iw = px;
        self_t->place.w = px + lmargin + rmargin;
    }
    self_t->place.ih = py;
    self_t->place.h = py + tmargin + bmargin;
    INFO("TEXT: %d, %d (%d, %d) / (%d: %d, %d: %d) %d, %d [%d]\n", canvas->cx, canvas->cy,
         canvas->place.w, canvas->height, self_t->place.x, self_t->place.ix,
         self_t->place.y, self_t->place.iy, self_t->place.w, self_t->place.h, pd);

    if (!ABSY(self_t->place)) {
        // newlines have an empty size
        if (ck != cStack) {
            if (li == 0) {
                canvas->cx = self_t->place.x + lrect.x + lrect.width + rmargin + pd;
            } else {
                canvas->cy = self_t->place.y + py - lrect.height;
                if (lrect.width == 0) {
                    canvas->cx = self_t->place.x + lrect.x;
                } else {
                    canvas->cx = self_t->place.x + lrect.width + rmargin;
                }
            }
        }

        canvas->endy = max(self_t->place.y + self_t->place.h, canvas->endy);
        canvas->endx = canvas->cx;

        if (ck == cStack || canvas->cx - CPX(canvas) > canvas->width) {
            canvas->cx = CPX(canvas);
            canvas->cy = canvas->endy;
        }
        if (NIL_P(ATTR(self_t->attr, margin)) && NIL_P(ATTR(self_t->attr, margin_top)))
            bmargin = lrect.height;

        INFO("CX: (%d, %d) / LRECT: (%d, %d) / END: (%d, %d)\n",
             canvas->cx, canvas->cy,
             lrect.x, lrect.width,
             canvas->endx, canvas->endy);
    }
    return self;
}

static void shoes_app_style_for(shoes_textblock *block, shoes_app *app, VALUE klass, VALUE oattr, guint start_index, guint end_index) {
    VALUE str = Qnil;
    VALUE hsh = rb_hash_aref(app->styles, klass);
    if (NIL_P(hsh) && NIL_P(oattr)) return;

    PangoAttribute *attr = NULL;

    APPLY_STYLE_COLOR(stroke, foreground);
    APPLY_STYLE_COLOR(fill, background);
    APPLY_STYLE_COLOR(strikecolor, strikethrough_color);
    APPLY_STYLE_COLOR(undercolor, underline_color);

    GET_STYLE(font);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            attr = pango_attr_font_desc_new(pango_font_description_from_string(RSTRING_PTR(str)));
        }
        APPLY_ATTR();
    }

    GET_STYLE(size);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "xx-small", 8) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_XX_SMALL);
            else if (strncmp(RSTRING_PTR(str), "x-small", 7) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_X_SMALL);
            else if (strncmp(RSTRING_PTR(str), "small", 5) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_SMALL);
            else if (strncmp(RSTRING_PTR(str), "medium", 6) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_MEDIUM);
            else if (strncmp(RSTRING_PTR(str), "large", 5) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_LARGE);
            else if (strncmp(RSTRING_PTR(str), "x-large", 7) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_X_LARGE);
            else if (strncmp(RSTRING_PTR(str), "xx-large", 8) == 0)
                attr = pango_attr_scale_new(PANGO_SCALE_XX_LARGE);
            else
                str = rb_funcall(str, s_to_i, 0);
        }
        if (TYPE(str) == T_FIXNUM) {
            int i = NUM2INT(str);
            if (i > 0)
                attr = pango_attr_size_new_absolute(ROUND(i * PANGO_SCALE * (96./72.)));
        }
        APPLY_ATTR();
    }

    GET_STYLE(family);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            attr = pango_attr_family_new(RSTRING_PTR(str));
        }
        APPLY_ATTR();
    }

    GET_STYLE(weight);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "ultralight", 10) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_ULTRALIGHT);
            else if (strncmp(RSTRING_PTR(str), "light", 5) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_LIGHT);
            else if (strncmp(RSTRING_PTR(str), "normal", 6) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
            else if (strncmp(RSTRING_PTR(str), "semibold", 8) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_SEMIBOLD);
            else if (strncmp(RSTRING_PTR(str), "bold", 4) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
            else if (strncmp(RSTRING_PTR(str), "ultrabold", 9) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_ULTRABOLD);
            else if (strncmp(RSTRING_PTR(str), "heavy", 5) == 0)
                attr = pango_attr_weight_new(PANGO_WEIGHT_HEAVY);
        } else if (TYPE(str) == T_FIXNUM) {
            int i = NUM2INT(str);
            if (i >= 100 && i <= 900)
                attr = pango_attr_weight_new((PangoWeight)i);
        }
        APPLY_ATTR();
    }

    GET_STYLE(rise);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING)
            str = rb_funcall(str, s_to_i, 0);
        if (TYPE(str) == T_FIXNUM) {
            int i = NUM2INT(str);
            attr = pango_attr_rise_new(i * PANGO_SCALE);
        }
        APPLY_ATTR();
    }

    GET_STYLE(kerning);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING)
            str = rb_funcall(str, s_to_i, 0);
        if (TYPE(str) == T_FIXNUM) {
            int i = NUM2INT(str);
            attr = pango_attr_letter_spacing_new(i * PANGO_SCALE);
        }
        APPLY_ATTR();
    }

    GET_STYLE(emphasis);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "normal", 6) == 0)
                attr = pango_attr_style_new(PANGO_STYLE_NORMAL);
            else if (strncmp(RSTRING_PTR(str), "oblique", 7) == 0)
                attr = pango_attr_style_new(PANGO_STYLE_OBLIQUE);
            else if (strncmp(RSTRING_PTR(str), "italic", 6) == 0)
                attr = pango_attr_style_new(PANGO_STYLE_ITALIC);
        }
        APPLY_ATTR();
    }

    GET_STYLE(strikethrough);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "none", 4) == 0)
                attr = pango_attr_strikethrough_new(FALSE);
            else if (strncmp(RSTRING_PTR(str), "single", 6) == 0)
                attr = pango_attr_strikethrough_new(TRUE);
        }
        APPLY_ATTR();
    }

    GET_STYLE(stretch);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "condensed", 9) == 0)
                attr = pango_attr_stretch_new(PANGO_STRETCH_CONDENSED);
            else if (strncmp(RSTRING_PTR(str), "normal", 6) == 0)
                attr = pango_attr_stretch_new(PANGO_STRETCH_NORMAL);
            else if (strncmp(RSTRING_PTR(str), "expanded", 8) == 0)
                attr = pango_attr_stretch_new(PANGO_STRETCH_EXPANDED);
        }
        APPLY_ATTR();
    }

    GET_STYLE(underline);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "none", 4) == 0)
                attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
            else if (strncmp(RSTRING_PTR(str), "single", 6) == 0)
                attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
            else if (strncmp(RSTRING_PTR(str), "double", 6) == 0)
                attr = pango_attr_underline_new(PANGO_UNDERLINE_DOUBLE);
            else if (strncmp(RSTRING_PTR(str), "low", 3) == 0)
                attr = pango_attr_underline_new(PANGO_UNDERLINE_LOW);
            else if (strncmp(RSTRING_PTR(str), "error", 5) == 0)
                attr = pango_attr_underline_new(PANGO_UNDERLINE_ERROR);
        }
        APPLY_ATTR();
    }

    GET_STYLE(variant);
    if (!NIL_P(str)) {
        if (TYPE(str) == T_STRING) {
            if (strncmp(RSTRING_PTR(str), "normal", 6) == 0)
                attr = pango_attr_variant_new(PANGO_VARIANT_NORMAL);
            else if (strncmp(RSTRING_PTR(str), "smallcaps", 9) == 0)
                attr = pango_attr_variant_new(PANGO_VARIANT_SMALL_CAPS);
        }
        APPLY_ATTR();
    }
}

VALUE shoes_textblock_string(VALUE self) {
    shoes_canvas *canvas;
    GET_STRUCT(textblock, self_t);
    Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
    if (!self_t->cached || self_t->pattr == NULL)
        shoes_textblock_make_pango(canvas->app, rb_obj_class(self), self_t);
    return rb_str_new(self_t->text->str, self_t->text->len);
}

static void shoes_textblock_iter_pango(VALUE texts, shoes_textblock *block, shoes_app *app) {
    VALUE v;
    long i;

    if (NIL_P(texts))
        return;

    for (i = 0; i < RARRAY_LEN(texts); i++) {
        v = rb_ary_entry(texts, i);
        if (rb_obj_is_kind_of(v, cTextClass)) {
            VALUE tklass = rb_obj_class(v);
            guint start;
            shoes_text *text;
            Data_Get_Struct(v, shoes_text, text);

            start = block->len;
            shoes_textblock_iter_pango(text->texts, block, app);
            if ((text->hover & HOVER_MOTION) && tklass == cLink)
                tklass = cLinkHover;
            shoes_app_style_for(block, app, tklass, text->attr, start, block->len);
            if (!block->cached && rb_obj_is_kind_of(v, cLink) && !NIL_P(text->attr)) {
                rb_ary_push(block->links, shoes_link_new(v, start, block->len));
            }
        } else if (rb_obj_is_kind_of(v, rb_cArray)) {
            shoes_textblock_iter_pango(v, block, app);
        } else {
            char *start, *end;
            v = rb_funcall(v, s_to_s, 0);
            block->len += (guint)RSTRING_LEN(v);
            if (!block->cached) {
                start = RSTRING_PTR(v);
                if (!g_utf8_validate(start, RSTRING_LEN(v), (const gchar **)&end))
                    shoes_error("not a valid UTF-8 string: %.*s", end - start, start);
                if (end > start)
                    g_string_append_len(block->text, start, end - start);
            }
        }
    }
}

static void shoes_textblock_make_pango(shoes_app *app, VALUE klass, shoes_textblock *block) {
    if (!block->cached) {
        block->text = g_string_new(NULL);
        block->links = rb_ary_new();
    }
    block->len = 0;
    block->pattr = pango_attr_list_new();

    shoes_textblock_iter_pango(block->texts, block, app);
    shoes_app_style_for(block, app, klass, block->attr, 0, block->len);

    if (block->cursor != NULL && block->cursor->pos != INT_MAX &&
            block->cursor->hi != INT_MAX && block->cursor->pos != block->cursor->hi) {
        PangoAttribute *attr = pango_attr_background_new(255 * 255, 255 * 255, 0);
        attr->start_index = min(block->cursor->pos, block->cursor->hi);
        attr->end_index = max(block->cursor->pos, block->cursor->hi);
        pango_attr_list_insert(block->pattr, attr);
    }

    block->cached = 1;
}

static void shoes_textblock_on_layout(shoes_app *app, VALUE klass, shoes_textblock *block) {
    char *attr = NULL;
    VALUE str = Qnil, hsh = Qnil, oattr = Qnil;
    g_return_if_fail(block != NULL);
    g_return_if_fail(PANGO_IS_LAYOUT(block->layout));

    oattr = block->attr;
    hsh = rb_hash_aref(app->styles, klass);

    if (!block->cached || block->pattr == NULL)
        shoes_textblock_make_pango(app, klass, block);
    pango_layout_set_text(block->layout, block->text->str, -1);
    pango_layout_set_attributes(block->layout, block->pattr);

    GET_STYLE(justify);
    if (!NIL_P(str))
        pango_layout_set_justify(block->layout, RTEST(str));

    GET_STYLE(align);
    if (TYPE(str) == T_STRING) {
        if (strncmp(RSTRING_PTR(str), "left", 4) == 0)
            pango_layout_set_alignment(block->layout, PANGO_ALIGN_LEFT);
        else if (strncmp(RSTRING_PTR(str), "center", 6) == 0)
            pango_layout_set_alignment(block->layout, PANGO_ALIGN_CENTER);
        else if (strncmp(RSTRING_PTR(str), "right", 5) == 0)
            pango_layout_set_alignment(block->layout, PANGO_ALIGN_RIGHT);
    }

    GET_STYLE(wrap);
    if (TYPE(str) == T_STRING) {
        if (strncmp(RSTRING_PTR(str), "word", 4) == 0)
            pango_layout_set_wrap(block->layout, PANGO_WRAP_WORD);
        else if (strncmp(RSTRING_PTR(str), "char", 4) == 0)
            pango_layout_set_wrap(block->layout, PANGO_WRAP_CHAR);
        else if (strncmp(RSTRING_PTR(str), "trim", 4) == 0)
            pango_layout_set_ellipsize(block->layout, PANGO_ELLIPSIZE_END);
    }
}

VALUE shoes_textblock_style_m(int argc, VALUE *argv, VALUE self) {
    GET_STRUCT(textblock, self_t);
    VALUE obj = shoes_textblock_style(argc, argv, self);
    shoes_textblock_uncache(self_t, FALSE);
    return obj;
}

void shoes_textblock_mark(shoes_textblock *text) {
    rb_gc_mark_maybe(text->texts);
    rb_gc_mark_maybe(text->links);
    rb_gc_mark_maybe(text->attr);
    rb_gc_mark_maybe(text->parent);
}

//
// Frees the pango attribute list.
// The `all` flag means the string has changed as well.
//
void shoes_textblock_uncache(shoes_textblock *text, unsigned char all) {
    if (text->pattr != NULL)
        pango_attr_list_unref(text->pattr);
    text->pattr = NULL;
    if (all) {
        if (text->text != NULL)
            g_string_free(text->text, TRUE);
        text->text = NULL;
        text->cached = 0;
    }
}

void shoes_textblock_free(shoes_textblock *text) {
    shoes_transform_release(text->st);
    shoes_textblock_uncache(text, TRUE);
    if (text->cursor != NULL)
        SHOE_FREE(text->cursor);
    if (text->layout != NULL)
        g_object_unref(text->layout);
    RUBY_CRITICAL(free(text));
}

VALUE shoes_textblock_new(VALUE klass, VALUE texts, VALUE attr, VALUE parent, shoes_transform *st) {
    shoes_canvas *canvas;
    shoes_textblock *text;
    VALUE obj = shoes_textblock_alloc(klass);
    Data_Get_Struct(obj, shoes_textblock, text);
    Data_Get_Struct(parent, shoes_canvas, canvas);
    text->texts = shoes_text_check(texts, obj);
    text->attr = attr;
    text->parent = parent;
    text->st = shoes_transform_touch(st);
    return obj;
}

VALUE shoes_textblock_alloc(VALUE klass) {
    VALUE obj;
    shoes_textblock *text = SHOE_ALLOC(shoes_textblock);
    SHOE_MEMZERO(text, shoes_textblock, 1);
    obj = Data_Wrap_Struct(klass, shoes_textblock_mark, shoes_textblock_free, text);
    text->texts = Qnil;
    text->links = Qnil;
    text->attr = Qnil;
    text->parent = Qnil;
    text->cursor = NULL;
    return obj;
}

VALUE shoes_textblock_children(VALUE self) {
    GET_STRUCT(textblock, text);
    return text->texts;
}

static void shoes_textcursor_reset(shoes_textcursor *c) {
    c->pos = c->hi = c->x = c->y = INT_MAX;
}

VALUE shoes_textblock_set_cursor(VALUE self, VALUE pos) {
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL) {
        if (NIL_P(pos)) return Qnil;
        else            shoes_textcursor_reset(self_t->cursor = SHOE_ALLOC(shoes_textcursor));
    }

    if (NIL_P(pos)) shoes_textcursor_reset(self_t->cursor);
    else if (pos == ID2SYM(s_marker)) {
        if (self_t->cursor->hi != INT_MAX) {
            self_t->cursor->pos = min(self_t->cursor->pos, self_t->cursor->hi);
            self_t->cursor->hi = INT_MAX;
        }
    } else            self_t->cursor->pos = NUM2INT(pos);
    shoes_textblock_uncache(self_t, FALSE);
    shoes_canvas_repaint_all(self_t->parent);
    return pos;
}

VALUE shoes_textblock_get_cursor(VALUE self) {
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL || self_t->cursor->pos == INT_MAX) return Qnil;
    return INT2NUM(self_t->cursor->pos);
}

VALUE shoes_textblock_cursorx(VALUE self) {
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL || self_t->cursor->x == INT_MAX) return Qnil;
    return INT2NUM(self_t->cursor->x);
}

VALUE shoes_textblock_cursory(VALUE self) {
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL || self_t->cursor->y == INT_MAX) return Qnil;
    return INT2NUM(self_t->cursor->y);
}

VALUE shoes_textblock_set_marker(VALUE self, VALUE pos) {
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL) {
        if (NIL_P(pos)) return Qnil;
        else            shoes_textcursor_reset(self_t->cursor = SHOE_ALLOC(shoes_textcursor));
    }

    if (NIL_P(pos)) self_t->cursor->hi = INT_MAX;
    else            self_t->cursor->hi = NUM2INT(pos);
    shoes_textblock_uncache(self_t, FALSE);
    shoes_canvas_repaint_all(self_t->parent);
    return pos;
}

VALUE shoes_textblock_get_marker(VALUE self) {
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL || self_t->cursor->hi == INT_MAX) return Qnil;
    return INT2NUM(self_t->cursor->hi);
}

VALUE shoes_textblock_get_highlight(VALUE self) {
    int marker, start, len;
    GET_STRUCT(textblock, self_t);
    if (self_t->cursor == NULL || self_t->cursor->pos == INT_MAX) return Qnil;
    marker = self_t->cursor->hi;
    if (marker == INT_MAX) marker = self_t->cursor->pos;
    start = min(self_t->cursor->pos, marker);
    len = max(self_t->cursor->pos, marker) - start;
    return rb_ary_new3(2, INT2NUM(start), INT2NUM(len));
}

VALUE shoes_find_textblock(VALUE self) {
    while (!NIL_P(self) && !rb_obj_is_kind_of(self, cTextBlock)) {
        SETUP_BASIC();
        self = basic->parent;
    }
    return self;
}

VALUE shoes_textblock_send_hover(VALUE self, int x, int y, VALUE *clicked, char *t) {
    VALUE url = Qnil;
    int index, trailing, i, hover;
    GET_STRUCT(textblock, self_t);
    if (self_t->layout == NULL || NIL_P(self_t->links)) return Qnil;
    if (!NIL_P(self_t->attr) && ATTR(self_t->attr, hidden) == Qtrue) return Qnil;

    x -= self_t->place.ix + self_t->place.dx;
    y -= self_t->place.iy + self_t->place.dy;
    hover = pango_layout_xy_to_index(self_t->layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing);
    if (hover != (self_t->hover & HOVER_MOTION)) {
        shoes_textblock_uncache(self_t, FALSE);
        INFO("HOVER (%d, %d) OVER (%d, %d)\n", x, y, self_t->place.ix + self_t->place.dx, self_t->place.iy + self_t->place.dy);
    }
    for (i = 0; i < RARRAY_LEN(self_t->links); i++) {
        VALUE urll = shoes_link_at(self_t, rb_ary_entry(self_t->links, i), index, hover, clicked, t);
        if (NIL_P(url)) url = urll;
    }

    return url;
}

VALUE shoes_textblock_motion(VALUE self, int x, int y, char *t) {
    VALUE url = shoes_textblock_send_hover(self, x, y, NULL, t);
    if (!NIL_P(url)) {
        shoes_canvas *canvas;
        GET_STRUCT(textblock, self_t);
        Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
        shoes_app_cursor(canvas->app, s_link);
    }
    return url;
}

VALUE shoes_textblock_hit(VALUE self, VALUE _x, VALUE _y) {
    int x = NUM2INT(_x), y = NUM2INT(_y), index, trailing;
    GET_STRUCT(textblock, self_t);
    x -= self_t->place.ix + self_t->place.dx;
    y -= self_t->place.iy + self_t->place.dy;
    if (x < 0 || x > self_t->place.iw || y < 0 || y > self_t->place.ih)
        return Qnil;
    pango_layout_xy_to_index(self_t->layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing);
    return INT2NUM(index);
}

VALUE shoes_textblock_send_click(VALUE self, int button, int x, int y, VALUE *clicked) {
    VALUE v = Qnil;

    if (button > 0) {
        GET_STRUCT(textblock, self_t);
        v = shoes_textblock_send_hover(self, x, y, clicked, NULL);
        if (self_t->hover & HOVER_MOTION)
            self_t->hover = HOVER_MOTION | HOVER_CLICK;
    }

    return v;
}

void shoes_textblock_send_release(VALUE self, int button, int x, int y) {
    GET_STRUCT(textblock, self_t);
    if (button > 0 && (self_t->hover & HOVER_CLICK)) {
        VALUE proc = ATTR(self_t->attr, release);
        self_t->hover ^= HOVER_CLICK;
        if (!NIL_P(proc))
            shoes_safe_block(self, proc, rb_ary_new3(1, self));
    }
}
