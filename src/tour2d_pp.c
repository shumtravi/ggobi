/* tour2d_pp.c */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke and Eun-Kuung Lee

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be contacted at the following email addresses:
    dicook@iastate.edu    sigbert@wiwi.hu-berlin.de
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined __APPLE__ && !defined __GNUC__
#endif

#include "vars.h"
#include "externs.h"

#include "tour2d_pp.h"
#include "tour_pp.h"


typedef enum {HOLES, CENTRAL_MASS, LDA, CGINI, CENTROPY}  StandardPPIndexTypes;


#define EXPMINUS1 0.3678794411714423
#define ONEMINUSEXPMINUS1 0.63212056

void t2d_pptemp_set(gdouble slidepos, displayd *dsp, GGobiSession *gg) {
  dsp->t2d_pp_op.temp_start = slidepos;
}

void t2d_ppcool_set(gdouble slidepos, displayd *dsp, GGobiSession *gg) {
  dsp->t2d_pp_op.cooling = slidepos;
}

gdouble mean_fn2(gdouble *x1, gdouble *x2, gint n)
{
  gint i;
  gdouble tmean, tmpf1;
  gdouble mean1, mean2;

  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x1[i];
  mean1 = tmpf1 / (gdouble)n;
  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x2[i];
  mean2 = tmpf1 / (gdouble)n;
  tmean = 0.;
  for (i=0; i<n; i++) {
    tmean += ((x1[i]-mean1)*(x2[i]-mean2));
  }
  tmean /= ((gdouble)n);
  tmean += (mean1*mean2);

  return(tmean);
}

/*
void
alloc_holes_p(holes_param *hp, gint nrows)
{
  hp->h0 = (gdouble *) g_malloc(
    (guint) nrows*sizeof(gdouble *));
  hp->h1 = (gdouble *) g_malloc(
    (guint) nrows*sizeof(gdouble *));
}

void
free_holes_p(holes_param *hp)
{
  g_free(hp->h0);
  g_free(hp->h1);
}
*/
/********************************************************************
Index          : Holes
Transformation : needs sphered variables/principal components
Purpose        : computes a projection into a normal density fn
Note           : only works for 2d now, could be generalized
*********************************************************************/

/* gint holes(array_d *pdata, void *param, gdouble *val)
{
  holes_param *hp = (holes_param *) param;
  gint i, m;

  * Calculate coefficients *
  for (i=0; i<pdata->nrows; i++)
  {
    m = i;
    hp->h0[m] = exp(-pdata->vals[m][0]*pdata->vals[m][0]/2.) ;
    hp->h1[m] = exp(-pdata->vals[m][1]*pdata->vals[m][1]/2.) ;
  }

  * Calculate index *
  hp->acoefs = mean_fn2(hp->h0,hp->h1,pdata->nrows);
  *val = (1. - hp->acoefs)/(gdouble) ONEMINUSEXPMINUS1 ;
  return(0);
}
*/
/********************************************************************

Index          : Central Mass
Transformation : needs sphered variables/principal components
Purpose        : computes a neg projection into a normal density fn
Note           : only works for 2d now, could be generalized

*********************************************************************/

/* gint
central_mass(array_d *pdata, void *param, gdouble *val)
{
  holes_param *hp = (holes_param *) param;
  gint i, m;

  * Calculate coefficients *
  for (i=0; i<pdata->nrows; i++)
  {
    m = i;
    hp->h0[m] = exp(-pdata->vals[m][0]*pdata->vals[m][0]/2.) ;
    hp->h1[m] = exp(-pdata->vals[m][1]*pdata->vals[m][1]/2.) ;
  }

  * Calculate index *
  hp->acoefs = mean_fn2(hp->h0,hp->h1,pdata->nrows);
  *val = (hp->acoefs - (gdouble)EXPMINUS1)/(gdouble)ONEMINUSEXPMINUS1 ;
  return(0);
}
*/
/***************************************************/
/*  2D Holes index for raw data                    */
/*   holes_raw1                                    */
/*   holes_raw2 : use inverse function             */
/***************************************************/

/* gint holes_raw1(array_d *pdata, void *param, gdouble *val)
{ 
*
   holes_param *hp = (holes_param *) param;
*
   gint i, p=pdata->ncols, n=pdata->nrows;
   gdouble m1, m2, x1, x2, temp;
   gdouble *cov;
   gdouble det,acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) 
     cov[i] = 0;
   m1=0; 
   m2=0;
   for (i=0; i<n; i++) { 
     m1 += pdata->vals[i][0];
     m2 += pdata->vals[i][1];
   }      
   m1 /= n;
   m2 /= n;

   for (i=0; i<n; i++) {  
     cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
     cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
     cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
   }

   cov[2] = cov[1];
   det = cov[0]*cov[3]-cov[1]*cov[1];
   cov[0] /= det;
   cov[1] /= det;
   cov[2] /= det;
   cov[3] /= det;
   acoefs=0.;

   for(i=0; i<n; i++) {  
     x1 = pdata->vals[i][0]-m1; 
     x2 = pdata->vals[i][1]-m2;
     temp = (cov[3]*x1*x1-2*cov[1]*x1*x2+cov[0]*x2*x2);
     acoefs += exp(-temp/2);
   }

   *val = (1.-acoefs/n)/(gdouble) ONEMINUSEXPMINUS1;

   g_free(cov);
   return(0);
}

gint holes_raw2( array_d *pdata, void *param, gdouble *val)
{ 
   gint i, p=pdata->ncols, n=pdata->nrows;
   gdouble m1, m2,x1,x2,temp;
   gdouble *cov;
*
   holes_param *hp = (holes_param *) param;
   gdouble det;
*
   gdouble acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0]/n;
     m2 += pdata->vals[i][1]/n;
   }      
   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }
   cov[2]= cov[1];
   inverse(cov,p);
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= cov[0]*x1*x1+(cov[1]+cov[2])*x1*x2+cov[3]*x2*x2;
        acoefs +=exp(-temp/2);
   }

   *val = (1.-acoefs/n)/(gdouble) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);
}
*/

/**********************************************************/
/*  2D Central Mass index for raw data                    */
/*   central_mass_raw1                                    */
/*   central_mass_raw2 : use inverse function             */
/**********************************************************/

/* gint central_mass_raw1(array_d *pdata, void *param, gdouble *val)
{
*
   holes_param *hp = (holes_param *) param;
*
   gint i, p=pdata->ncols, n=pdata->nrows;
   gdouble m1, m2,x1,x2,temp;
   gdouble *cov;
   gdouble det,acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0];
     m2 += pdata->vals[i][1];
   }      
   m1 /= n;
   m2 /= n;

   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }

   cov[2]= cov[1];
   det = cov[0]*cov[3]-cov[1]*cov[1];
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= (cov[3]*x1*x1-2*cov[1]*x1*x2+cov[0]*x2*x2)/det;
        acoefs +=exp(-temp/2);
   }

   *val = (acoefs/n-(gdouble)EXPMINUS1)/(gdouble) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);

}

gint central_mass_raw2(array_d *pdata, void *param, gdouble *val)
{
   *holes_param *hp = (holes_param *) param;*
   gint i, p=pdata->ncols, n=pdata->nrows;
   gdouble m1, m2,x1,x2,temp;
   gdouble *cov;
   gdouble acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0]/n;
     m2 += pdata->vals[i][1]/n;
   }      
   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }
   cov[2]= cov[1];
   inverse(cov,p);
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= cov[0]*x1*x1-(cov[1]+cov[2])*x1*x2+cov[3]*x2*x2;
        acoefs +=exp(-temp/2);
   }
   *val = (acoefs/n-(gdouble)EXPMINUS1)/(gdouble) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);

}
*/
/*void
holes_deriv(holes_param *hp, gdouble **data, gdouble **pdata)
{
  gint i, k, m;
  gdouble tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<hp->ncols; k++)
      hp->derivs[i][k] = 0.;

* alpha *
  for (k=0; k<hp->ncols; k++)
  {
    tmpf = 0.;
    for (i=0; i<hp->nrows; i++)
    {
      m = i;
      tmpf += (pdata[0][m]*hp->h0[m]*hp->h1[m]*
        (data[m][k] -
        hp->alpha[k]*pdata[0][m] -
        hp->beta[k]*pdata[1][m]));
    }
    tmpf /= ((gdouble)hp->nrows);
    hp->derivs[0][k] = tmpf;
  }

* beta *
  for (k=0; k<hp->ncols; k++)
  {
    tmpf = 0.;
    for (i=0; i<hp->nrows; i++)
    {
      m = i;
      tmpf += (pdata[1][m]*hp->h0[m]*hp->h1[m]*
        (data[m][k] -
        hp->alpha[k]*pdata[0][m] -
        hp->beta[k]*pdata[1][m]));
    }
    tmpf /= ((gdouble)hp->nrows);
    hp->derivs[1][k] = tmpf;
  }

}
*/


/*void
central_mass_deriv(gdouble **data, gdouble **proj_data, gdouble *alpha, gdouble *beta,
gdouble **derivs, int n, int *rows_in_plot, int p, int nactive, int *active_vars)
{
  int i, k, m;
  gdouble tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf -= (proj_data[0][m]*hp->h0[m]*hp->h1[m]*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((gdouble)n);
    derivs[0][active_vars[k]] = tmpf;
  }

  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf -= (proj_data[1][m]*hp->h0[m]*hp->h1[m]*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((gdouble)n);
    derivs[1][active_vars[k]] = tmpf;
  }

}*/

/* This function interacts with control  buttons in ggobi */
void t2d_optimz(gint optimz_on, gboolean *nt, gint *bm, displayd *dsp) {
  gboolean new_target = *nt;
  gint bas_meth = *bm;
  gint i, j;

  if (optimz_on) 
  {
    for (i=0; i<2; i++)
      for (j=0; j<dsp->t2d.nactive; j++)
        dsp->t2d_pp_op.proj_best.vals[i][j] = 
          dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];
    /*    dsp->t2d.ppval = dsp->t2d_indx_min;*/
    dsp->t2d_pp_op.index_best = dsp->t2d.ppval;
    bas_meth = 1;
  }
  else
    bas_meth = 0;

  new_target = true;

  *nt = new_target;
  *bm = bas_meth;
}

void t2d_clear_pppixmap(displayd *dsp, GGobiSession *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (dsp->t2d_pp_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_line (dsp->t2d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (dsp->t2d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);
}

void t2d_clear_ppda(displayd *dsp, GGobiSession *gg)
{
  gint i;

  /* clear the ppindx matrix */
  dsp->t2d_ppindx_count = 0;
  dsp->t2d_indx_min=1000.;
  dsp->t2d_indx_max=-1000.;
  for (i=0; i<100; i++) 
  {
    dsp->t2d_ppindx_mat[i] = 0.0;
  }

  t2d_clear_pppixmap(dsp, gg);
}

void t2d_ppdraw_all(gint wid, gint hgt, gint margin, displayd *dsp, GGobiSession *gg)
{
  GdkPoint pptrace[100];
  gint i;

  t2d_clear_pppixmap(dsp, gg);

  for (i=0; i<dsp->t2d_ppindx_count; i++) 
  {
    pptrace[i].x = margin+i*2;
    pptrace[i].y = hgt-margin-(gint)((gdouble)((dsp->t2d_ppindx_mat[i]-
      dsp->t2d_indx_min)/(gdouble) (dsp->t2d_indx_max-dsp->t2d_indx_min)) * 
      (gdouble) (hgt - 2*margin));
  }
  gdk_draw_lines (dsp->t2d_pp_pixmap, gg->plot_GC,
    pptrace, dsp->t2d_ppindx_count);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);

}

/* This is writes text to the pp window to inform the
user that optimize is finding a new maximum */ 
void t2d_ppdraw_think(displayd *dsp, GGobiSession *gg)
{
  splotd *sp = (splotd *) g_list_nth_data (dsp->splots, 0);
  colorschemed *scheme = gg->activeColorScheme;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;
  PangoLayout *layout = gtk_widget_create_pango_layout(sp->da, "Thinking...");
  
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_layout(dsp->t2d_pp_pixmap, gg->plot_GC, 10, 10, layout);
  g_object_unref(G_OBJECT(layout));
  /*gdk_text_extents (
    gtk_style_get_font (style),
    varlab, strlen (varlab),
    &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (dsp->t2d_pp_pixmap,
    gtk_style_get_font (style),
     gg->plot_GC, 10, 10, varlab);*/
  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);
}

/* This is the pp index plot drawing routine */ 
void t2d_ppdraw(gdouble pp_indx_val, displayd *dsp, GGobiSession *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;
  gint j;
  static gboolean init = true;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  if (init) {
    t2d_clear_ppda(dsp, gg);
    init = false;
  }

  dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count] = pp_indx_val;

  if (dsp->t2d_indx_min > pp_indx_val)
      dsp->t2d_indx_min = pp_indx_val;
  if (dsp->t2d_indx_max < pp_indx_val)
    dsp->t2d_indx_max = pp_indx_val;

  if (dsp->t2d_indx_min == dsp->t2d_indx_max) dsp->t2d_indx_min *= 0.9999;

  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
    dsp->t2d_indx_min, dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count], dsp->t2d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  if (dsp->t2d_ppindx_count == 0) 
  {
    dsp->t2d_ppindx_count++;
  }
  else if (dsp->t2d_ppindx_count > 0 && dsp->t2d_ppindx_count < 80) {
    t2d_ppdraw_all(wid, hgt, margin, dsp, gg);
    dsp->t2d_ppindx_count++;
  }
  else if (dsp->t2d_ppindx_count >= 80) 
  {
    /* cycle values back into array */
    for (j=0; j<=dsp->t2d_ppindx_count; j++)
      dsp->t2d_ppindx_mat[j] = dsp->t2d_ppindx_mat[j+1];
    t2d_ppdraw_all(wid, hgt, margin, dsp, gg);
  }

  g_free (label);
}

void t2d_pp_reinit(displayd *dsp, GGobiSession *gg)
{
  gint i, j;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  for (i=0; i<dsp->t2d_pp_op.proj_best.nrows; i++)
    for (j=0; j<dsp->t2d_pp_op.proj_best.ncols; j++)
      dsp->t2d_pp_op.proj_best.vals[i][j] = 
	  dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];
  dsp->t2d.ppval = 0.0;
  dsp->t2d.oppval = -1.0;
  dsp->t2d_pp_op.index_best = 0.0;
  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
			   dsp->t2d_indx_min, dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count], 
			   dsp->t2d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  t2d_clear_ppda(dsp, gg);
  g_free (label);
}

/********************************************************************

                         INDEX CALCULATION

The index function has to be defined as

     gint index (array_d *pdata, void *param, gdouble *val)

with   

Input:  pdata   projected data
        param   additional parameters for the index 
                (will not be touched by the optimization routine)

Output: val     the index-value
        
This function should simply calculate the index value for a provided
projection.

*********************************************************************/

gdouble t2d_calc_indx (array_d pd,
                Tour_PPIndex_f index,
                void *param)
{ 
  gdouble indexval;

  index (&pd, param, &indexval, NULL);

  return(indexval);
}

/* 
 We could call this with the tour t2d object rather than the index type
 since we no longer need the indxtype 
*/
gboolean
t2d_switch_index(Tour2DCPanel controls, gint basismeth, displayd *dsp,
  GGobiSession *gg)
{
  GGobiStage *d = dsp->d;
  gint kout, nrows = d->n_rows;
  gdouble *gdata;
  gint i, j, k;

  if (d->n_rows == 1)  /* can't do pp on no data! */
    return(false);

  /* Copy data into pp opt'n data */
  for (i=0; i<d->n_rows; i++)
    for (j=0; j<dsp->t2d.nactive; j++)
      dsp->t2d_pp_op.data.vals[i][j] = 
        ggobi_stage_get_raw_value(d, i, dsp->t2d.active_vars.els[j]);

  /* Copy current projection into opt'n projection */
  for (i=0; i<2; i++)
    for (j=0; j<dsp->t2d.nactive; j++) 
      dsp->t2d_pp_op.proj_best.vals[i][j] = 
        dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];

  for (k=0; k<2; k++) 
    for (i=0; i<d->n_rows; i++) {
      dsp->t2d_pp_op.pdata.vals[i][k] = 
          (ggobi_stage_get_raw_value(d, i, dsp->t2d.active_vars.els[0])*
          dsp->t2d_pp_op.proj_best.vals[k][0]);
      for (j=1; j<dsp->t2d.nactive; j++)
        dsp->t2d_pp_op.pdata.vals[i][k] += 
          (ggobi_stage_get_raw_value(d, i, dsp->t2d.active_vars.els[j])*
          dsp->t2d_pp_op.proj_best.vals[k][j]);
    }

  GGOBI_STAGE_ATTR_INIT(d, cluster);
  gdata  = g_malloc (nrows*sizeof(gdouble));
  for (i = 0 ; i < nrows; i++)
  { 
    gdata[i] = GGOBI_STAGE_GET_ATTR_CLUSTER(d, i);
  }

  if(controls.ppindex.index_f) {
      if(controls.ppindex.checkGroups == false || 
           !compute_groups (dsp->t2d_pp_param.group, dsp->t2d_pp_param.ngroup, &dsp->t2d_pp_param.numgroups, nrows, gdata)) 
      { 
	  controls.ppindex.index_f(&dsp->t2d_pp_op.pdata, &dsp->t2d_pp_param,  &dsp->t2d.ppval, controls.ppindex.userData);
	  if(basismeth == 1) 
	      kout = optimize0 (&dsp->t2d_pp_op, controls.ppindex.index_f, &dsp->t2d_pp_param);
      }
  }

#if 0
  switch (indxtype)
  { 
    case HOLES: 
      dsp->t2d.ppval = t2d_calc_indx (dsp->t2d_pp_op.pdata, 
				      holes_raw, &dsp->t2d_pp_param);
      if (basismeth == 1) {
        kout = optimize0 (&dsp->t2d_pp_op, holes_raw, &dsp->t2d_pp_param);
      }
    break;
    case CENTRAL_MASS: 
      dsp->t2d.ppval = t2d_calc_indx (dsp->t2d_pp_op.pdata,
				      central_mass_raw, &dsp->t2d_pp_param);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t2d_pp_op, central_mass_raw, &dsp->t2d_pp_param);
    break;
    case LDA: 
      if (!compute_groups (dsp->t2d_pp_param.group, dsp->t2d_pp_param.ngroup, 
        &dsp->t2d_pp_param.numgroups, nrows, gdata)) {
        dsp->t2d.ppval = t2d_calc_indx (dsp->t2d_pp_op.pdata,
					discriminant, &dsp->t2d_pp_param);
        if (basismeth == 1)
          kout = optimize0 (&dsp->t2d_pp_op, discriminant, &dsp->t2d_pp_param);
      }
      break;
    case CGINI: 
      if (!compute_groups (dsp->t2d_pp_param.group, dsp->t2d_pp_param.ngroup, 
        &dsp->t2d_pp_param.numgroups, nrows, 
			   gdata)) {
        dsp->t2d.ppval = t2d_calc_indx (dsp->t2d_pp_op.pdata,
					cartgini, &dsp->t2d_pp_param);
        if (basismeth == 1)
          kout = optimize0 (&dsp->t2d_pp_op, cartgini, &dsp->t2d_pp_param);
      }
      break;
    case CENTROPY: 
      if (!compute_groups (dsp->t2d_pp_param.group, dsp->t2d_pp_param.ngroup, 
        &dsp->t2d_pp_param.numgroups, nrows, 
			   gdata)) {
        dsp->t2d.ppval = t2d_calc_indx (dsp->t2d_pp_op.pdata,
          cartentropy, &dsp->t2d_pp_param);
        if (basismeth == 1)
          kout = optimize0 (&dsp->t2d_pp_op, cartentropy, &dsp->t2d_pp_param);
      }
      break;
    break;
    default: 
/* Shouldn't we free gdata. */
      return(true);
    break;
  }
#endif

  g_free (gdata);
  return(false);
}


#undef ONEMINUSEXPMINUS1
#undef EXPMINUS1
