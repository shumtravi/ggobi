/*-- utils_gdk.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <string.h>

#include <gtk/gtk.h>
#include <vars.h>
#include "externs.h"

GdkColor *
NewColor (glong red, glong green, glong blue) {
  gboolean writeable = false, best_match = true;
  GdkColor *c = (GdkColor *) g_malloc (sizeof (GdkColor));

  c->red = red;
  c->green = green;
  c->blue = blue;

  if (gdk_colormap_alloc_color(gdk_colormap_get_system (),
    c, writeable, best_match) == false)
  {
    g_printerr("Unable to allocate color\n");
    c = NULL;
  }

  return (c);
}

/*
 * The plotted glyph is actually 2*size + 1 on a side, so the
 * size progression is  5, 7, 9, 11, 13, ...     That's
 * because it seems necessary to have glyphs that have odd
 * sizes in order to make sure the point is at the center of
 * the glyph.  That may be overly fastidious for large glyphs,
 * but it's neceessary for the small ones.
*/
void
draw_glyph (GdkDrawable *drawable, glyphd *gl, icoords *xypos, gint jpos, ggobid *gg)
{
  gushort size = gl->size + 1;

  switch (gl->type) {

    case PLUS:
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x - size, xypos[jpos].y,
        xypos[jpos].x + size, xypos[jpos].y);
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x, xypos[jpos].y - size,
        xypos[jpos].x, xypos[jpos].y + size);
    break;
    case X:
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x - size, xypos[jpos].y - size,
        xypos[jpos].x + size, xypos[jpos].y + size);
      gdk_draw_line (drawable, gg->plot_GC,
        xypos[jpos].x + size, xypos[jpos].y - size,
        xypos[jpos].x - size, xypos[jpos].y + size);
    break;
    case OR:
      gdk_draw_rectangle (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size);
    break;
    case FR:
      gdk_draw_rectangle (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size);
      gdk_draw_rectangle (drawable, gg->plot_GC, true,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size);
    break;
    case OC:
      gdk_draw_arc (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size, 0, (gshort) 23040);
    break;
    case FC:
      gdk_draw_arc (drawable, gg->plot_GC, false,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size, 0, (gshort) 23040);
      gdk_draw_arc (drawable, gg->plot_GC, true,
        xypos[jpos].x - size, xypos[jpos].y - size,
        2*size, 2*size, 0, (gshort) 23040);
    break;
    case DOT_GLYPH:
      gdk_draw_point (drawable, gg->plot_GC, xypos[jpos].x, xypos[jpos].y);
    break;
    case UNKNOWN_GLYPH:
    default:
      g_printerr ("build_glyph: impossible glyph type %d\n", gl->type);
  }
}

void
splot_text_extents (gchar *text, GtkStyle *style,
  gint *lbearing, gint *rbearing, gint *width, gint *ascent, gint *descent)
{
  gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    text, strlen(text),
    lbearing, rbearing, width, ascent, descent);
}

void
splot_draw_string (gchar *text, gint xpos, gint ypos,
  GtkStyle *style, GdkDrawable *drawable, ggobid *gg)
{
  gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    gg->plot_GC, xpos, ypos, text);
}

void
mousepos_get_pressed (GtkWidget *w, GdkEventButton *event,
                      gboolean *btn1_down_p, gboolean *btn2_down_p, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  gint grab_ok;
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y, &state);

  grab_ok = gdk_pointer_grab (sp->da->window,
    false,
    (GdkEventMask) (GDK_POINTER_MOTION_MASK|GDK_BUTTON_RELEASE_MASK),
    (GdkWindow *) NULL,
    (GdkCursor *) NULL,
    event->time);

  if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    *btn1_down_p = true;
  else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    *btn2_down_p = true;
  else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    *btn2_down_p = true;

  if (*btn1_down_p) gg->buttondown = 1;
  else if (*btn2_down_p) gg->buttondown = 2;
}

void
mousepos_get_motion (GtkWidget *w, GdkEventMotion *event,
                     gboolean *btn1_down_p, gboolean *btn2_down_p, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  /*-- that is, if using motion hints --*/
/*
  if (event->is_hint) {
*/

    gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
      &state);
    if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;

/*
  } else {

    sp->mousepos.x = (gint) event->x;
    sp->mousepos.y = (gint) event->y;
    if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;
  }
*/

  if (*btn1_down_p) gg->buttondown = 1;
  else if (*btn2_down_p) gg->buttondown = 2;
}

gboolean
mouseinwindow (splotd *sp) {
  return (0 < sp->mousepos.x && sp->mousepos.x < sp->max.x &&
          0 < sp->mousepos.y && sp->mousepos.y < sp->max.y) ;

}

/*--------------------------------------------------------------------*/
/*              Drawing 3D sliders                                    */
/*--------------------------------------------------------------------*/

/* (x,y) is the center of the rectangle */
void
draw_3drectangle (GdkDrawable *drawable, gint x, gint y,
  gint width, gint height, ggobid *gg)
{
  GdkPoint points[7];
  gint w = width/2;
  gint h = height/2;

  /*-- draw the rectangles --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->mediumgray);
  gdk_draw_rectangle (drawable, gg->plot_GC, TRUE, x-w, y-h, width, height);

  /*-- draw the dark shadows --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->darkgray);
  points [0].x = x - w;
  points [0].y = y + h;
  points [1].x = x + w;
  points [1].y = y + h;
  points [2].x = x + w;
  points [2].y = y - h;

  points [3].x = points[2].x - 1;
  points [3].y = points[2].y + 1;
  points [4].x = points[1].x - 1;
  points [4].y = points[1].y - 1;
  points [5].x = points[0].x + 1;
  points [5].y = points[0].y - 1;

  points [6].x = x - w;
  points [6].y = y + h;
  gdk_draw_polygon (drawable, gg->plot_GC, TRUE, points, 7);
  gdk_draw_line (drawable, gg->plot_GC, x-1, y-(h-1), x-1, y+(h-2));

  /*-- draw the light shadows --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->lightgray);
  points [0].x = x - w;  /*-- lower left --*/
  points [0].y = y + (h-1);
  points [1].x = x - w;  /*-- upper left --*/
  points [1].y = y - h;
  points [2].x = x + (w-1);  /*-- upper right --*/
  points [2].y = y - h;

  points [3].x = points[2].x - 1;
  points [3].y = points[2].y + 1;
  points [4].x = points[1].x + 1;
  points [4].y = points[1].y + 1;
  points [5].x = points[0].x + 1;
  points [5].y = points[0].y - 1;

  points [6].x = points[0].x;
  points [6].y = points[0].y;
  gdk_draw_polygon (drawable, gg->plot_GC, TRUE, points, 7);
  gdk_draw_line (drawable, gg->plot_GC, x, y-(h-1), x, y+(h-2));
}


