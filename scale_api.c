/* scale_api.c: some api scaling routines */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
   I didn't do these right, I don't think, because pan takes
   incremental steps, while zoom resets the scale value in an
   absolute way.  Maybe the way I did the zooming is the way one
   would like.  If that's so, it could be renamed splot_zoom_set
   and left as it is.  splot_pan could be renamed splot_pan_set
   and reworked.
*/

void
splot_pan (splotd *sp, gint xstep, gint ystep, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  displayd *display = gg->current_display;

  if (cpanel->scale_pan_opt == P_OBLIQUE ||
      cpanel->scale_pan_opt == P_HORIZ)  /* pan horizontally */
  {
    sp->ishift.x += xstep;
  }

  if (cpanel->scale_pan_opt == P_OBLIQUE ||
      cpanel->scale_pan_opt == P_VERT)  /* pan vertically */
  {
    sp->ishift.y += ystep;
  }

  /*-- redisplay this plot --*/
  splot_plane_to_screen (display, cpanel, sp, gg);
  ruler_ranges_set (gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}

/*-- Set the current scale:  xsc, ysc are on (SCALE_MIN, ...) */
void
splot_zoom (splotd *sp, gfloat xsc, gfloat ysc, ggobid *gg) {
  displayd *display = gg->current_display;
  gint projection = projection_get (gg);
  icoords mid;
  gfloat *scale_x = (projection == TOUR2D) ? &sp->tour_scale.x : &sp->scale.x;
  gfloat *scale_y = (projection == TOUR2D) ? &sp->tour_scale.y : &sp->scale.y;
  gfloat scalefac_x = xsc / *scale_x;
  gfloat scalefac_y = ysc / *scale_y;

  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;

  if (xsc > SCALE_MIN && *scale_x * scalefac_x >= SCALE_MIN) {
    sp->ishift.x = mid.x + (gint) (scalefac_x * (gfloat) (sp->ishift.x - mid.x));
    *scale_x = xsc;
  }
  if (scalefac_y > SCALE_MIN && *scale_y * scalefac_y >= SCALE_MIN) {
    sp->ishift.y = mid.y + (gint) (scalefac_y * (gfloat) (sp->ishift.y - mid.y));
    *scale_y = ysc;
  }

  /*-- redisplay this plot --*/
  splot_plane_to_screen (display, &display->cpanel, sp, gg);
  ruler_ranges_set (gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}
