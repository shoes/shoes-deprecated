#include "shoes/types/color.h"
#include "shoes/types/image.h"
#include "shoes/types/pattern.h"
#include "shoes/types/shape.h"

// ruby
VALUE cShape;

FUNC_M("+shape", shape, -1);

PLACE_COMMON(shape);
CLASS_COMMON2(shape);
TRANS_COMMON(shape, 1);

void shoes_shape_init() {
    cShape    = rb_define_class_under(cTypes, "Shape", rb_cObject);
    
    rb_define_alloc_func(cShape, shoes_shape_alloc);
    
    rb_define_method(cShape, "app", CASTHOOK(shoes_canvas_get_app), 0);
    rb_define_method(cShape, "displace", CASTHOOK(shoes_shape_displace), 2);
    rb_define_method(cShape, "draw", CASTHOOK(shoes_shape_draw), 2);
    rb_define_method(cShape, "move", CASTHOOK(shoes_shape_move), 2);
    rb_define_method(cShape, "parent", CASTHOOK(shoes_shape_get_parent), 0);
    rb_define_method(cShape, "top", CASTHOOK(shoes_shape_get_top), 0);
    rb_define_method(cShape, "left", CASTHOOK(shoes_shape_get_left), 0);
    rb_define_method(cShape, "width", CASTHOOK(shoes_shape_get_width), 0);
    rb_define_method(cShape, "height", CASTHOOK(shoes_shape_get_height), 0);
    rb_define_method(cShape, "remove", CASTHOOK(shoes_basic_remove), 0);
    rb_define_method(cShape, "style", CASTHOOK(shoes_shape_style), -1);
    rb_define_method(cShape, "hide", CASTHOOK(shoes_shape_hide), 0);
    rb_define_method(cShape, "show", CASTHOOK(shoes_shape_show), 0);
    rb_define_method(cShape, "toggle", CASTHOOK(shoes_shape_toggle), 0);
    rb_define_method(cShape, "click", CASTHOOK(shoes_shape_click), -1);
    rb_define_method(cShape, "release", CASTHOOK(shoes_shape_release), -1);
    rb_define_method(cShape, "hover", CASTHOOK(shoes_shape_hover), -1);
    rb_define_method(cShape, "leave", CASTHOOK(shoes_shape_leave), -1);

    RUBY_M("+shape", shape, -1);
}

// ruby
VALUE shoes_shape_draw(VALUE self, VALUE c, VALUE actual) {
    shoes_place place;
    shoes_canvas *canvas;
    GET_STRUCT(shape, self_t);
    if (ATTR(self_t->attr, hidden) == Qtrue) return self;
    Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
    shoes_place_exact(&place, self_t->attr, CPX(canvas), CPY(canvas));

    if (RTEST(actual))
        shoes_shape_sketch(CCR(canvas), self_t->name, &place, self_t->st, self_t->attr, self_t->line, 1);

    self_t->place = place;
    return self;
}

void shoes_shape_mark(shoes_shape *path) {
    rb_gc_mark_maybe(path->parent);
    rb_gc_mark_maybe(path->attr);
}

void shoes_shape_free(shoes_shape *path) {
    shoes_transform_release(path->st);
    if (path->line != NULL) cairo_path_destroy(path->line);
    RUBY_CRITICAL(free(path));
}

VALUE shoes_shape_attr(int argc, VALUE *argv, int syms, ...) {
    int i;
    va_list args;
    VALUE hsh = Qnil;
    va_start(args, syms);
    if (argc < 1) hsh = rb_hash_new();
    else if (rb_obj_is_kind_of(argv[argc - 1], rb_cHash)) hsh = argv[argc - 1];
    for (i = 0; i < syms; i++) {
        ID sym = va_arg(args, ID);
        if (argc > i && !rb_obj_is_kind_of(argv[i], rb_cHash))
            hsh = shoes_hash_set(hsh, sym, argv[i]);
    }
    va_end(args);
    return hsh;
}

unsigned char shoes_shape_check(cairo_t *cr, shoes_place *place) {
    double ox1 = place->ix, oy1 = place->iy, ox2 = place->ix + place->iw, oy2 = place->iy + place->ih;
    double cx1, cy1, cx2, cy2;
    cairo_clip_extents(cr, &cx1, &cy1, &cx2, &cy2);
    if (place->iw < 0) ox1 = place->ix - (ox2 = -place->iw);
    if (place->ih < 0) oy1 = place->iy - (oy2 = -place->ih);
    if (cy2 - cy1 == 1.0 && cx2 - cx1 == 1.0) return 1;
    if ((ox1 < cx1 && ox2 < cx1) || (oy1 < cy1 && oy2 < cy1) ||
            (ox1 > cx2 && ox2 > cx2) || (oy1 > cy2 && oy2 > cy2)) return 0;
    return 1;
}

void shoes_shape_sketch(cairo_t *cr, ID name, shoes_place *place, shoes_transform *st, VALUE attr, cairo_path_t* line, unsigned char draw) {
    double sw = ATTR2(dbl, attr, strokewidth, 1.);
    if (name == s_oval && place->w > 0 && place->h > 0) {
        shoes_apply_transformation(cr, st, place, 1);
        if (!shoes_shape_check(cr, place))
            return shoes_undo_transformation(cr, st, place, 1);
        if (draw) cairo_new_path(cr);
        cairo_translate(cr, (place->x * 1.) + (place->w / 2.), (place->y * 1.) + (place->h / 2.));
        cairo_scale(cr, place->w / 2., place->h / 2.);
        cairo_arc(cr, 0., 0., 1., 0., SHOES_PIM2);
        cairo_close_path(cr);
        shoes_undo_transformation(cr, st, place, 1);
    } else if (name == s_arc && place->w > 0 && place->h > 0) {
        double a1 = ATTR2(dbl, attr, angle1, 0.);
        double a2 = ATTR2(dbl, attr, angle2, 0.);
        shoes_apply_transformation(cr, st, place, 0);
        if (!shoes_shape_check(cr, place))
            return shoes_undo_transformation(cr, st, place, 0);
        if (draw) cairo_new_path(cr);
        shoes_cairo_arc(cr, SWPOS(place->x), SWPOS(place->y),
                        place->w * 1., place->h * 1., a1, a2);
        shoes_undo_transformation(cr, st, place, 0);
    } else if (name == s_rect && place->w > 0 && place->h > 0) {
        double cv = ATTR2(dbl, attr, curve, 0.);
        shoes_apply_transformation(cr, st, place, 0);
        if (!shoes_shape_check(cr, place))
            return shoes_undo_transformation(cr, st, place, 0);
        if (draw) cairo_new_path(cr);
        shoes_cairo_rect(cr, SWPOS(place->x), SWPOS(place->y), place->w * 1., place->h * 1., cv);
        shoes_undo_transformation(cr, st, place, 0);
    } else if (name == s_line) {
        shoes_apply_transformation(cr, st, place, 0);
        if (!shoes_shape_check(cr, place))
            return shoes_undo_transformation(cr, st, place, 0);
        cairo_move_to(cr, SWPOS(place->ix), SWPOS(place->iy));
        cairo_line_to(cr, SWPOS(place->ix + place->iw), SWPOS(place->iy + place->ih));
        shoes_undo_transformation(cr, st, place, 0);
    } else if (name == s_arrow && place->w > 0) {
        double h, tip, x;
        x = place->x + (place->w / 2.);
        h = place->w * 0.8;
        place->h = ROUND(h);
        tip = place->w * 0.42;

        shoes_apply_transformation(cr, st, place, 0);
        if (!shoes_shape_check(cr, place))
            return shoes_undo_transformation(cr, st, place, 0);
        if (draw) cairo_new_path(cr);
        cairo_move_to(cr, SWPOS(x), SWPOS(place->y));
        cairo_rel_line_to(cr, -tip, +(h*0.5));
        cairo_rel_line_to(cr, 0, -(h*0.25));
        cairo_rel_line_to(cr, -(place->w-tip), 0);
        cairo_rel_line_to(cr, 0, -(h*0.5));
        cairo_rel_line_to(cr, +(place->w-tip), 0);
        cairo_rel_line_to(cr, 0, -(h*0.25));
        cairo_close_path(cr);
        shoes_undo_transformation(cr, st, place, 0);
    } else if (name == s_star) {
        int i, points;
        double outer, inner, angle, r;
        points = ATTR2(int, attr, points, 10);
        outer = ATTR2(dbl, attr, outer, 100.);
        inner = ATTR2(dbl, attr, inner, 50.);

        if (outer > 0) {
            place->w = place->h = ROUND(outer);
            shoes_apply_transformation(cr, st, place, 0);
            if (!shoes_shape_check(cr, place))
                return shoes_undo_transformation(cr, st, place, 0);
            if (draw) cairo_new_path(cr);
            cairo_move_to(cr, place->x * 1., (place->y * 1.) + outer);
            for (i = 1; i <= points * 2; i++) {
                angle = (i * SHOES_PI) / (points * 1.);
                r = (i % 2 == 0 ? outer : inner);
                cairo_line_to(cr, place->x + r * sin(angle), place->y + r * cos(angle));
            }
            cairo_close_path(cr);
            shoes_undo_transformation(cr, st, place, 0);
        }
    } else if (name == s_shape) {
        shoes_apply_transformation(cr, st, place, 0);
        if (!shoes_shape_check(cr, place))
            return shoes_undo_transformation(cr, st, place, 0);
        cairo_translate(cr, SWPOS(place->x), SWPOS(place->y));
        cairo_append_path(cr, line);
        shoes_undo_transformation(cr, st, place, 0);
    } else return;

    if (draw) {
        ID cap = s_rect;
        if (!NIL_P(ATTR(attr, cap))) cap = SYM2ID(ATTR(attr, cap));
        ID dash = s_nodot;
        if (!NIL_P(ATTR(attr, dash))) dash = SYM2ID(ATTR(attr, dash));
        PATH_OUT(cr, attr, *place, sw, cap, dash, fill, cairo_fill_preserve);
        PATH_OUT(cr, attr, *place, sw, cap, dash, stroke, cairo_stroke);
    }
}

VALUE shoes_shape_new(VALUE parent, ID name, VALUE attr, shoes_transform *st, cairo_path_t *line) {
    shoes_shape *path;
    shoes_canvas *canvas;
    VALUE obj = shoes_shape_alloc(cShape);
    Data_Get_Struct(obj, shoes_shape, path);
    Data_Get_Struct(parent, shoes_canvas, canvas);
    path->parent = parent;
    path->attr = attr;
    path->name = name;
    path->st = shoes_transform_touch(st);
    path->line = line;
    COPY_PENS(path->attr, canvas->attr);
    return obj;
}

VALUE shoes_shape_alloc(VALUE klass) {
    VALUE obj;
    shoes_shape *shape = SHOE_ALLOC(shoes_shape);
    SHOE_MEMZERO(shape, shoes_shape, 1);
    obj = Data_Wrap_Struct(klass, shoes_shape_mark, shoes_shape_free, shape);
    shape->attr = Qnil;
    shape->parent = Qnil;
    shape->line = NULL;
    return obj;
}

VALUE shoes_shape_motion(VALUE self, int x, int y, char *touch) {
    char h = 0;
    VALUE click;
    GET_STRUCT(shape, self_t);

    click = ATTR(self_t->attr, click);

    if (IS_INSIDE(self_t, x, y)) {
        cairo_bool_t in_shape;
        cairo_t *cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
        // if (self_t->line != NULL)
        // {
        //   cairo_new_path(cr);
        //   cairo_append_path(cr, self_t->line);
        // }
        // else
        shoes_shape_sketch(cr, self_t->name, &self_t->place, self_t->st, self_t->attr, self_t->line, 0);
        in_shape = cairo_in_fill(cr, x, y);
        cairo_destroy(cr);

        if (in_shape) {
            if (!NIL_P(click)) {
                shoes_canvas *canvas;
                Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
                shoes_app_cursor(canvas->app, s_link);
            }
            h = 1;
        }
    }

    CHECK_HOVER(self_t, h, touch);

    return h ? click : Qnil;
    return Qnil;
}

VALUE shoes_shape_send_click(VALUE self, int button, int x, int y) {
    VALUE v = Qnil;

    if (button > 0) {
        GET_STRUCT(shape, self_t);
        v = shoes_shape_motion(self, x, y, NULL);
        if (self_t->hover & HOVER_MOTION)
            self_t->hover = HOVER_MOTION | HOVER_CLICK;
    }

    return v;
}

void shoes_shape_send_release(VALUE self, int button, int x, int y) {
    GET_STRUCT(shape, self_t);
    if (button > 0 && (self_t->hover & HOVER_CLICK)) {
        VALUE proc = ATTR(self_t->attr, release);
        self_t->hover ^= HOVER_CLICK;
        if (!NIL_P(proc))
            shoes_safe_block(self, proc, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
    }
}

// canvas
VALUE shoes_canvas_shape(int argc, VALUE *argv, VALUE self) {
    int x;
    double x1, y1, x2, y2;
    cairo_t *shape = NULL;
    cairo_path_t *line = NULL;
    
    SETUP_SHAPE();

    shape = canvas->shape;
    VALUE attr = shoes_shape_attr(argc, argv, 2, s_left, s_top);
    canvas->shape = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
    cairo_move_to(canvas->shape, 0, 0);
    if (rb_block_given_p()) rb_funcall(rb_block_proc(), s_call, 0);

#if CAIRO_VERSION_MAJOR == 1 && CAIRO_VERSION_MINOR <= 4
    cairo_fill_extents(canvas->shape, &x1, &y1, &x2, &y2);
#else
    cairo_path_extents(canvas->shape, &x1, &y1, &x2, &y2);
#endif
    x = ROUND(x2 - x1);
    ATTRSET(attr, width, INT2NUM(x));
    x = ROUND(y2 - y1);
    ATTRSET(attr, height, INT2NUM(x));
    line = cairo_copy_path(canvas->shape);
    canvas->shape = shape;
    return shoes_add_shape(self, s_shape, attr, line);
}

VALUE shoes_add_shape(VALUE self, ID name, VALUE attr, cairo_path_t *line) {
    if (rb_obj_is_kind_of(self, cImage)) {
        SETUP_IMAGE();
        shoes_shape_sketch(image->cr, name, &place, NULL, attr, line, 1);
        return self;
    }

    SETUP_CANVAS();
    if (canvas->shape != NULL) {
        shoes_place place;
        shoes_place_exact(&place, attr, 0, 0);
        cairo_new_sub_path(canvas->shape);
        shoes_shape_sketch(canvas->shape, name, &place, canvas->st, attr, line, 0);
        return self;
    }

    return shoes_add_ele(canvas, shoes_shape_new(self, name, attr, canvas->st, line));
}

VALUE shoes_canvas_arc(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 6, s_left, s_top, s_width, s_height, s_angle1, s_angle2);
    return shoes_add_shape(self, s_arc, attr, NULL);
}

VALUE shoes_canvas_rect(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 5, s_left, s_top, s_width, s_height, s_curve);
    return shoes_add_shape(self, s_rect, attr, NULL);
}

VALUE shoes_canvas_oval(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 4, s_left, s_top, s_width, s_height);
    //VALUE attr = shoes_shape_attr(argc, argv, 3, s_left, s_top, s_radius);
    //rb_warn("shoes_canvas_oval: %s\n", RSTRING_PTR(rb_inspect(attr)));
    return shoes_add_shape(self, s_oval, attr, NULL);
}

VALUE shoes_canvas_line(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 4, s_left, s_top, s_right, s_bottom);
    return shoes_add_shape(self, s_line, attr, NULL);
}

VALUE shoes_canvas_arrow(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 3, s_left, s_top, s_width);
    return shoes_add_shape(self, s_arrow, attr, NULL);
}

VALUE shoes_canvas_star(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 5, s_left, s_top, s_points, s_outer, s_inner);
    return shoes_add_shape(self, s_star, attr, NULL);
}