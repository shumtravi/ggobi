/* impute_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


gboolean
impute_fixed (gint impute_type, gint nvars, gint *vars, datad *d, ggobid *gg)
{
  gint i, j, k, m;
  gfloat maxval, minval, range, val, impval;
  gchar *val_str;
  gboolean ok = true;
  vartabled *vt;
  GtkWidget *w;

  if (d->missing.nrows == 0) {
    quick_message ("There are no missings.\n", false);
    return false;
  }

  g_assert (d->missing.nrows == d->nrows);
  g_assert (d->missing.ncols == d->ncols);

  if (impute_type == IMP_ABOVE || impute_type == IMP_BELOW) {

    if (impute_type == IMP_ABOVE) {
      w = widget_find_by_name (gg->impute.window, "IMPUTE:entry_above");
      val_str = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
    }  else if (impute_type == IMP_BELOW) {
      w = widget_find_by_name (gg->impute.window, "IMPUTE:entry_below");
      val_str = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
    }

    if (strlen (val_str) == 0) {
      gchar *message = g_strdup_printf (
        "You selected '%% over or under' but didn't specify a percentage.\n");
      quick_message (message, false);
      g_free (message);
      ok = false;
      return ok;
    }

    val = (gfloat) atof (val_str);
    g_free (val_str);
    if (val < 0 || val > 100) {
      gchar *message = g_strdup_printf (
        "You specified %f%%; please specify a percentage between 0 and 100.\n",
        val);
      quick_message (message, false);
      g_free (message);
      ok = false;
      return ok;
    }

    for (k=0; k<nvars; k++) {
       j = vars[k];
       vt = vartable_element_get (j, d);

      /* Use find the limits of the non-missing data */
      minval = vt->lim_display.min;
      maxval = vt->lim_display.max;
      range = maxval - minval;

      /* Then fill it in */
      if (impute_type == IMP_ABOVE)
        impval = maxval + (val/100.) * range;
      else if (impute_type == IMP_BELOW)
        impval = minval - (val/100.) * range;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        if (d->missing.vals[m][j]) {
          d->raw.vals[m][j] = d->tform.vals[m][j] = impval;
        }
      }
    }
  }
  else if (impute_type == IMP_FIXED) {
    w = widget_find_by_name (gg->impute.window, "IMPUTE:entry_val");
    val_str = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
    if (strlen (val_str) == 0) {
      quick_message (
        "You've selected 'Specify' but haven't specified a value.\n",
         false);
      ok = false;
      return ok;
    }
    else {
      impval = (gfloat) atof (val_str);
      g_free (val_str);
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        for (k=0; k<nvars; k++) {
          j = vars[k];
          if (d->missing.vals[m][j]) {
            d->raw.vals[m][j] = d->tform.vals[m][j] = impval;
          }
        }
      }
    }
  }

  return ok;
}

gboolean
impute_mean_or_median (gint type, gint nvars, gint *vars, 
   datad *d, ggobid *gg)
{
  gint i, j, k, m;
  vartabled *vt;
  gboolean redraw = false;

  if (d->nmissing == 0)
/**/return false;

  for (m=0; m<nvars; m++) {
    j = vars[m];
    vt = vartable_element_get (j, d);
    for (i=0; i<d->nrows_in_plot; i++) {
      k = d->rows_in_plot.els[i];
      if (!d->hidden_now.els[k]) {   /* ignore erased values altogether */
        if (d->missing.vals[k][j]) {
          d->raw.vals[k][j] = d->tform.vals[k][j] = (type == IMP_MEAN) ?
            vt->mean : vt->median;
          redraw = true;
        }
      }
    }
  }
  return redraw;
}

static void
impute_single (gint *missv, gint nmissing, gint *presv, gint npresent,
  gint col, datad *d, ggobid *gg)
{
  gint i, k;
  gfloat rrand;

  /*
   * Then loop over the missing values, plugging in some value
   * drawn from the present values.
  */
  for (i=0; i<nmissing; i++) {
    for (k=0; k<npresent; k++) {
      rrand = (gfloat) randvalue();

      if ( ((npresent - k) * rrand) < 1.0 ) {
        d->raw.vals[missv[i]][col] = d->raw.vals[presv[k]][col];
        /*
         * This is the default -- transformations will be applied
         * later to those that need it.
        */
        d->tform.vals[missv[i]][col] = d->tform.vals[presv[k]][col];
        break;
      }
    }
  }
}

void
impute_random (datad *d, gint nvars, gint *vars, ggobid *gg)
{
/* Perform single random imputation */

  gint i, j, k, n, m, npresent, *presv, nmissing, *missv;

  if (d->nmissing == 0)
/**/return;

  presv = (gint *) g_malloc (d->nrows_in_plot * sizeof (gint));
  missv = (gint *) g_malloc (d->nrows_in_plot * sizeof (gint));

  if (gg->impute.bgroup_p && d->nclusters > 1) {

    /* Loop over the number of brushing groups */
    for (n=0; n<d->nclusters; n++) {

      /* Then loop over the number of columns */
      for (m=0; m<nvars; m++) {
        npresent = nmissing = 0;
        j = vars[m];

        /*
         * And finally over the rows, including only those rows
         * which belong to the current cluster
        */
        for (i=0; i<d->nrows_in_plot; i++) {
          k = d->rows_in_plot.els[i];
          if (d->clusterid.els[k] == n) { 
            if (!d->hidden_now.els[k]) {   /* ignore erased values altogether */
              if (d->missing.vals[k][j])
                missv[nmissing++] = k;
              else
                presv[npresent++] = k;
            }
          }
        }
        impute_single (missv, nmissing, presv, npresent, j, d, gg);
      }
    }
  }

  else {
    for (m=0; m<nvars; m++) {
      npresent = nmissing = 0;
      j = vars[m];
      /*
       * Build the vector of indices of present values that can be used
       * to draw from.
      */
      for (i=0; i<d->nrows_in_plot; i++) {
        k = d->rows_in_plot.els[i];
        if (!d->hidden_now.els[k]) {   /* ignore erased values altogether */
          if (d->missing.vals[k][j])
            missv[nmissing++] = k;
          else
            presv[npresent++] = k;
        }
      }
      impute_single (missv, nmissing, presv, npresent, j, d, gg);
    }
  }

  g_free (presv);
  g_free (missv);
}
