/* tour2d.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "vars.h"
#include "externs.h"

#include "tour_pp_ui.h"
#include "tour2d_pp.h"
#include "projection-indices.h"


#define T2DON true
#define T2DOFF false

static void tour2d_speed_set_display(gdouble slidepos, displayd *dsp);
void tour2d_write_video(GGobiSession *gg);

void
display_tour2d_init_null (displayd *dsp, GGobiSession *gg)
{
  arrayd_init_null(&dsp->t2d.Fa);
  arrayd_init_null(&dsp->t2d.Fz);
  arrayd_init_null(&dsp->t2d.F);

  arrayd_init_null(&dsp->t2d.Ga);
  arrayd_init_null(&dsp->t2d.Gz);
  arrayd_init_null(&dsp->t2d.G);

  arrayd_init_null(&dsp->t2d.Va);
  arrayd_init_null(&dsp->t2d.Vz);

  arrayd_init_null(&dsp->t2d.tv);

  vectori_init_null(&dsp->t2d.subset_vars);
  vectorb_init_null(&dsp->t2d.subset_vars_p);
  vectori_init_null(&dsp->t2d.active_vars);
  vectorb_init_null(&dsp->t2d.active_vars_p);

  vectord_init_null(&dsp->t2d.lambda);
  vectord_init_null(&dsp->t2d.tau);
  vectord_init_null(&dsp->t2d.tinc);

  /* manipulation variables */
  arrayd_init_null(&dsp->t2d_Rmat1);
  arrayd_init_null(&dsp->t2d_Rmat2);
  arrayd_init_null(&dsp->t2d_mvar_3dbasis);
  arrayd_init_null(&dsp->t2d_manbasis);
}

void
alloc_tour2d (displayd *dsp, GGobiSession *gg)
{
  GGobiStage *d = dsp->d;
  gint nc = d->n_cols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayd_alloc(&dsp->t2d.Fa, 2, nc);
  arrayd_alloc(&dsp->t2d.Fz, 2, nc);
  arrayd_alloc(&dsp->t2d.F, 2, nc);

  arrayd_alloc(&dsp->t2d.Ga, 2, nc);
  arrayd_alloc(&dsp->t2d.Gz, 2, nc);
  arrayd_alloc(&dsp->t2d.G, 2, nc);

  arrayd_alloc(&dsp->t2d.Va, 2, nc);
  arrayd_alloc(&dsp->t2d.Vz, 2, nc);

  arrayd_alloc(&dsp->t2d.tv, 2, nc);

  vectori_alloc(&dsp->t2d.subset_vars, nc);
  vectorb_alloc_zero(&dsp->t2d.subset_vars_p, nc);
  vectori_alloc(&dsp->t2d.active_vars, nc);
  vectorb_alloc_zero(&dsp->t2d.active_vars_p, nc);

  vectord_alloc(&dsp->t2d.lambda, nc);
  vectord_alloc_zero(&dsp->t2d.tau, nc);
  vectord_alloc(&dsp->t2d.tinc, nc);

  /* manipulation variables */
  arrayd_alloc(&dsp->t2d_Rmat1, 3, 3);
  arrayd_alloc(&dsp->t2d_Rmat2, 3, 3);
  arrayd_alloc(&dsp->t2d_mvar_3dbasis, 3, 3);
  arrayd_alloc(&dsp->t2d_manbasis, 3, nc);
}

/*-- eliminate the nc columns contained in *cols --*/
void
tour2d_realloc_down (GSList *cols, GGobiStage *d, GGobiSession *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->d == d) {
      arrayd_delete_cols (&dsp->t2d.Fa, cols);
      arrayd_delete_cols (&dsp->t2d.Fz, cols);
      arrayd_delete_cols (&dsp->t2d.F, cols);
      arrayd_delete_cols (&dsp->t2d.Ga, cols);
      arrayd_delete_cols (&dsp->t2d.Gz, cols);
      arrayd_delete_cols (&dsp->t2d.G, cols);
      arrayd_delete_cols (&dsp->t2d.Va, cols);
      arrayd_delete_cols (&dsp->t2d.Vz, cols);
      arrayd_delete_cols (&dsp->t2d.tv, cols);

      vectori_delete_els (&dsp->t2d.subset_vars, cols);
      vectorb_delete_els (&dsp->t2d.subset_vars_p, cols);
      vectori_delete_els (&dsp->t2d.active_vars, cols);
      vectorb_delete_els (&dsp->t2d.active_vars_p, cols);

      vectord_delete_els (&dsp->t2d.lambda, cols);
      vectord_delete_els (&dsp->t2d.tau, cols);
      vectord_delete_els (&dsp->t2d.tinc, cols);

      arrayd_delete_cols (&dsp->t2d_manbasis, cols);
    }
  }
}

void
free_tour2d(displayd *dsp)
{
  vectori_free(&dsp->t2d.subset_vars);
  vectorb_free(&dsp->t2d.subset_vars_p);
  vectori_free(&dsp->t2d.active_vars);
  vectorb_free(&dsp->t2d.active_vars_p);

  vectord_free(&dsp->t2d.lambda);
  vectord_free(&dsp->t2d.tau);
  vectord_free(&dsp->t2d.tinc);

  arrayd_free (&dsp->t2d.Fa); 
  arrayd_free (&dsp->t2d.Fz); 
  arrayd_free (&dsp->t2d.F); 

  arrayd_free (&dsp->t2d.Ga); 
  arrayd_free (&dsp->t2d.Gz); 
  arrayd_free (&dsp->t2d.G); 

  arrayd_free (&dsp->t2d.Va); 
  arrayd_free (&dsp->t2d.Vz); 
  arrayd_free (&dsp->t2d.tv); 

  arrayd_free (&dsp->t2d_Rmat1); 
  arrayd_free (&dsp->t2d_Rmat2); 
  arrayd_free (&dsp->t2d_mvar_3dbasis); 
  arrayd_free (&dsp->t2d_manbasis); 
}

void 
display_tour2d_init (displayd *dsp, GGobiSession *gg) {
  gint i, j;
  GGobiStage *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->n_cols;

  if (nc < MIN_NVARS_FOR_TOUR2D)
    return;

  alloc_tour2d(dsp, gg);
 
    /* Initialize starting subset of active variables */
  if (nc < 8) {
    dsp->t2d.nsubset = dsp->t2d.nactive = nc;
    for (j=0; j<nc; j++) {
      dsp->t2d.subset_vars.els[j] = dsp->t2d.active_vars.els[j] = j;
      dsp->t2d.subset_vars_p.els[j] = dsp->t2d.active_vars_p.els[j] = true;
    }
  }
  else {
    dsp->t2d.nsubset = dsp->t2d.nactive = 3;
    for (j=0; j<3; j++) {
      dsp->t2d.subset_vars.els[j] = dsp->t2d.active_vars.els[j] = j;
      dsp->t2d.subset_vars_p.els[j] = dsp->t2d.active_vars_p.els[j] = true;
    }
    for (j=3; j<nc; j++) {
      dsp->t2d.subset_vars.els[j] = dsp->t2d.active_vars.els[j] = 0;
      dsp->t2d.subset_vars_p.els[j] = dsp->t2d.active_vars_p.els[j] = false;
    }
  }

  /* declare starting base as first p chosen variables */
  arrayd_zero (&dsp->t2d.Fa);
  arrayd_zero (&dsp->t2d.Fz);
  arrayd_zero (&dsp->t2d.F);
  arrayd_zero (&dsp->t2d.Ga);
  arrayd_zero (&dsp->t2d.Gz);
/*
  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d.Fa.vals[i][j] = dsp->t2d.Fz.vals[i][j] = 
        dsp->t2d.F.vals[i][j] = dsp->t2d.Ga.vals[i][j] = 
        dsp->t2d.Gz.vals[i][j] = 0.0;
*/

  for (i=0; i<2; i++) {
    dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Fa.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Ga.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.Gz.vals[i][dsp->t2d.active_vars.els[i]] = 1.0;
  }

  dsp->t2d.dist_az = 0.0;
  dsp->t2d.delta = cpanel->t2d.step*M_PI_2/10.0;
  dsp->t2d.tang = 0.0;

  dsp->t2d.idled = 0;
  dsp->t2d.get_new_target = true;

  dsp->t2d_video = false;

  /* manip */
  dsp->t2d_manip_var = 0;

  /* pp */
  dsp->t2d.target_selection_method = TOUR_RANDOM;
  dsp->t2d_ppda = NULL;
  dsp->t2d_axes = true;
  dsp->t2d_pp_op.temp_start = 1.0;
  dsp->t2d_pp_op.cooling = 0.99;

  tour2d_speed_set_display(sessionOptions->defaultTourSpeed, dsp);
}

void
tour2d_fade_vars (gboolean fade, GGobiSession *gg) 
{
  gg->tour2d.fade_vars = fade;
}

void
tour2d_all_vars (displayd *dsp) 
{
  GGobiSession *gg = dsp->ggobi;
  GGobiStage *d = dsp->d;
  gint j;

  //gg->tour2d.all_vars = !gg->tour2d.all_vars;

  //if (gg->tour2d.all_vars)
  //{
    for (j=0; j<d->n_cols; j++) {
      dsp->t2d.subset_vars.els[j] = j;
      dsp->t2d.active_vars.els[j] = j;
      dsp->t2d.subset_vars_p.els[j] = true;
      dsp->t2d.active_vars_p.els[j] = true;
    }
    dsp->t2d.nsubset = d->n_cols;
    dsp->t2d.nactive = d->n_cols;
    dsp->t2d.get_new_target = true;
    zero_tau(dsp->t2d.tau, 2);
    varcircles_visibility_set (dsp, gg);
    varpanel_refresh (dsp, gg);

    if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
      free_optimize0_p(&dsp->t2d_pp_op);
      alloc_optimize0_p(&dsp->t2d_pp_op, d->n_rows, dsp->t2d.nactive, 
        2);
      t2d_pp_reinit(dsp, gg);
    }  
  //}
}

void tour2d_speed_set(gdouble slidepos, GGobiSession *gg) {

  displayd *dsp = gg->current_display;
  tour2d_speed_set_display(slidepos, dsp);
}

static void tour2d_speed_set_display(gdouble slidepos, displayd *dsp) 
{
  cpaneld *cpanel;
  if (dsp) {
    cpanel = &dsp->cpanel;
    if (cpanel) {
      cpanel->t2d.slidepos = slidepos;
      speed_set(slidepos, &cpanel->t2d.step, &dsp->t2d.delta);
    }
  }
}

void tour2d_pause (cpaneld *cpanel, gboolean state, displayd *dsp, GGobiSession *gg) 
{
  gboolean pausedp = cpanel->t2d.paused;

  if (dsp == NULL)
    return;

  cpanel->t2d.paused = state;

  /* This condition is experimental and is used to avoid the case
      where we have an XY plot in paused tour mode and we create a new
      plot.  When that happens, the initialization of that new plot in
      cpanel_tour2d_set sets the pause button which triggers this
      routine to be invoked as part of the callback.  Since the paused
      state is 0, we end up calling tour2d_func with state = 1 which
      means turn it on.  And so the tour is active for that new
      display!  And we consume CPU cycles galore.  DTL.  */

  /*  if(state == 0 && dsp->t2d.idled == 0) */
  if(pausedp == 0 && state == 0 && dsp->t2d.idled == 0)
      return;

  tour2d_func (!cpanel->t2d.paused, dsp, gg);

  if (cpanel->t2d.paused) {
    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (dsp, FULL, gg);
  }
}

/*-- add/remove jvar to/from the subset of variables that <may> be active --*/
gboolean
tour2d_subset_var_set (gint jvar, GGobiStage *d, displayd *dsp, GGobiSession *gg)
{
  gboolean in_subset = dsp->t2d.subset_vars_p.els[jvar];
  gint j, k;
  gboolean changed = false;

  /*
   * require 3 variables in the subset, though only 2 are
   *   required in active_vars
  */
  if (in_subset) {
    if (dsp->t2d.nsubset > MIN_NVARS_FOR_TOUR2D) {
      dsp->t2d.subset_vars_p.els[jvar] = false;
      dsp->t2d.nsubset -= 1;
      changed = true;
    }
  } else {
    dsp->t2d.subset_vars_p.els[jvar] = true;
    dsp->t2d.nsubset += 1;
    changed = true;
  }

  /*-- reset subset_vars based on subset_vars_p --*/
  if (changed) {
    dsp->t2d_manipvar_inc = false;
    for (j=0, k=0; j<d->n_cols; j++)
      if (dsp->t2d.subset_vars_p.els[j]) {
        dsp->t2d.subset_vars.els[k++] = j;
        if (j == dsp->t2d_manip_var)
          dsp->t2d_manipvar_inc = true;
      }
    /*-- Manip var needs to be one of the active vars --*/
    if (!dsp->t2d_manipvar_inc) {
      dsp->t2d_manip_var = dsp->t2d.subset_vars.els[0];
    }
      
    zero_tau(dsp->t2d.tau, 2);
    dsp->t2d.get_new_target = true;

  }

  return changed;
}

/*-- add or remove jvar from the set of active variables --*/
void 
tour2d_active_var_set (gint jvar, GGobiStage *d, displayd *dsp, GGobiSession *gg)
{
  gint j, jtmp, k;
  gboolean in_subset = dsp->t2d.subset_vars_p.els[jvar];
  gboolean active = dsp->t2d.active_vars_p.els[jvar];

  /*
   * This covers the case where we've just removed a variable
   * from the subset and then called tour2d_active_var_set ..
   * but the variable is already inactive, so we don't need to
   * do anything.
  */
  if (!active && !in_subset)
/**/return;

  /* deselect var if t2d.nactive > 2 */
  if (active) {
    if (dsp->t2d.nactive > 2) {
      for (j=0; j<dsp->t2d.nactive; j++) {
        if (jvar == dsp->t2d.active_vars.els[j]) 
          break;
      }
      if (j<dsp->t2d.nactive-1) {
        for (k=j; k<dsp->t2d.nactive-1; k++) {
          dsp->t2d.active_vars.els[k] = dsp->t2d.active_vars.els[k+1];
        }
      }
      dsp->t2d.nactive--;
 
      if (!gg->tour2d.fade_vars) /* set current position without sel var */
      {
        gt_basis(dsp->t2d.Fa, dsp->t2d.nactive, dsp->t2d.active_vars, 
          d->n_cols, (gint) 2);
        arrayd_copy(&dsp->t2d.Fa, &dsp->t2d.F);
        zero_tau(dsp->t2d.tau, 2);
      }

      dsp->t2d.active_vars_p.els[jvar] = false;
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->t2d.active_vars.els[dsp->t2d.nactive-1]) {
      dsp->t2d.active_vars.els[dsp->t2d.nactive] = jvar;
    }
    else if (jvar < dsp->t2d.active_vars.els[0]) {
      for (j=dsp->t2d.nactive; j>0; j--) {
        dsp->t2d.active_vars.els[j] = dsp->t2d.active_vars.els[j-1];
      }
      dsp->t2d.active_vars.els[0] = jvar;
    }
    else {
      jtmp = dsp->t2d.nactive;
      for (j=0; j<dsp->t2d.nactive-1; j++) {
        if (jvar > dsp->t2d.active_vars.els[j] &&
            jvar < dsp->t2d.active_vars.els[j+1])
        {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->t2d.nactive-1;j>=jtmp; j--) 
        dsp->t2d.active_vars.els[j+1] = dsp->t2d.active_vars.els[j];
      dsp->t2d.active_vars.els[jtmp] = jvar;
    }
    dsp->t2d.nactive++;
    dsp->t2d.active_vars_p.els[jvar] = true;
  }

  dsp->t2d.get_new_target = true;

  /* Check if pp indices are being calculated, if so re-allocate
     and re-initialize as necessary */
  if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
    free_optimize0_p(&dsp->t2d_pp_op);
    alloc_optimize0_p(&dsp->t2d_pp_op, d->n_rows, dsp->t2d.nactive, 2);
    t2d_pp_reinit(dsp, gg);
  }
}

static void
tour2d_manip_var_set (gint j, GGobiSession *gg)
{
  displayd *dsp = gg->current_display;

  dsp->t2d_manip_var = j;    
}

gboolean
tour2d_varsel (GtkWidget *w, gint jvar, gint toggle, gint mouse,
  GGobiStage *d, GGobiSession *gg)
{
  displayd *dsp = gg->current_display;
  gboolean changed = true;

  if (GTK_IS_TOGGLE_BUTTON(w) || GTK_IS_BUTTON(w)) {
    /*
     * add/remove jvar to/from the subset of variables that <may> be active
    */
    gboolean fade = gg->tour2d.fade_vars;

    changed = tour2d_subset_var_set(jvar, d, dsp, gg);
    if (changed) {
      varcircles_visibility_set (dsp, gg);

      /*-- Add/remove the variable to/from the active set, too. --*/
      gg->tour2d.fade_vars = false;
      tour2d_active_var_set (jvar, d, dsp, gg);
      gg->tour2d.fade_vars = fade;
    }

  } else if (GTK_IS_DRAWING_AREA(w)) {
    
    /*-- we don't care which button it is --*/
    if (d->vcirc_ui.jcursor == GDK_HAND2) {
      tour2d_manip_var_set (jvar, gg);
      varcircles_cursor_set_default (d);

    } else {
      /*-- add or remove from set of active variables --*/
      tour2d_active_var_set (jvar, d, dsp, gg);
      /*    if (dsp->t2d.target_selection_method == TOUR_PP)*/
    }
  }

  return changed;
}

void
tour2d_projdata(splotd *sp, gdouble **world_data, GGobiStage *d, GGobiSession *gg) {
  gint j, m;
  displayd *dsp = (displayd *) sp->displayptr;
  gdouble tmpf, maxx, maxy;

  if (sp->tour2d.initmax) {
    sp->tour2d.maxscreen = 1;
    sp->tour2d.initmax = false;
  }

  tmpf = 1/sp->tour2d.maxscreen;
  maxx = sp->tour2d.maxscreen;
  maxy = sp->tour2d.maxscreen;
  for (m=0; m<d->n_rows; m++)
  {
    sp->planar[m].x = 0;
    sp->planar[m].y = 0;
    for (j=0; j<d->n_cols; j++)
    {
      sp->planar[m].x += (gdouble)(dsp->t2d.F.vals[0][j]*world_data[m][j]);
      sp->planar[m].y += (gdouble)(dsp->t2d.F.vals[1][j]*world_data[m][j]);
    }
    sp->planar[m].x *= tmpf;
    sp->planar[m].y *= tmpf;
    if (fabs(sp->planar[m].x) > maxx)
      maxx = fabs(sp->planar[m].x);
    if (fabs(sp->planar[m].y) > maxy)
      maxy = fabs(sp->planar[m].y);
  }

  if ((maxx > 1) || (maxy > 1)) {
    sp->tour2d.maxscreen = (maxx > maxy) ? maxx : maxy;
    tmpf = 1/tmpf;
  }
}

void tour2d_scramble(GGobiSession *gg)
{
  displayd *dsp = gg->current_display;
  GGobiStage *d = dsp->d;

  arrayd_zero (&dsp->t2d.Fa);
  arrayd_zero (&dsp->t2d.Fz);
  arrayd_zero (&dsp->t2d.F);
  arrayd_zero (&dsp->t2d.Ga);
  arrayd_zero (&dsp->t2d.Gz);

  gt_basis(dsp->t2d.Fa, dsp->t2d.nactive, dsp->t2d.active_vars, 
    d->n_cols, (gint) 2);
  arrayd_copy(&dsp->t2d.Fa, &dsp->t2d.F);

  dsp->t2d.tau.els[0] = 0.0;
  dsp->t2d.tau.els[1] = 0.0;

  dsp->t2d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

void tour2d_snap(GGobiSession *gg)
{
  displayd *dsp = gg->current_display;
  splotd *sp = gg->current_splot;
  GGobiStage *d = dsp->d;
  gint j;
  gdouble rnge;

  for (j=0; j<d->n_cols; j++) {
    rnge = ggobi_variable_get_range(ggobi_stage_get_variable(d, j));
    fprintf(stdout,"%f %f %f %f \n", dsp->t2d.F.vals[0][j], 
      dsp->t2d.F.vals[1][j],dsp->t2d.F.vals[0][j]/rnge*sp->scale.x,
      dsp->t2d.F.vals[1][j]/rnge*sp->scale.y);
  }
}

void tour2d_video(GGobiSession *gg)
{
  displayd *dsp = gg->current_display;
  if (dsp == NULL)
    return;

  dsp->t2d_video = !dsp->t2d_video;
}

void tour2d_write_video(GGobiSession *gg) 
{
  displayd *dsp = gg->current_display;
  splotd *sp = gg->current_splot;
  GGobiStage *d = dsp->d;
  gint j;
  gdouble rnge;

  /*  g_printerr("%f %f\n",sp->scale.x, sp->scale.y);*/
  for (j=0; j<d->n_cols; j++) {
    rnge = ggobi_variable_get_range(ggobi_stage_get_variable(d, j));
    fprintf(stdout,"%f %f %f %f\n", dsp->t2d.F.vals[0][j], 
      dsp->t2d.F.vals[1][j], dsp->t2d.F.vals[0][j]/rnge*sp->scale.x,
      dsp->t2d.F.vals[1][j]/rnge*sp->scale.y);
  }
}

void
tour2d_run(displayd *dsp, GGobiSession *gg)
{
  GGobiStage *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint i, j, nv;
  /*  static gint count = 0;*/
  gboolean revert_random = false;
  gint k;
  gboolean chosen;
  gdouble eps = .01;
  gint pathprob = 0;
  extern void t2d_ppdraw_think(displayd *,GGobiSession *);

  /* Need to see why ppval goes down even though optimize is on. */
  /* Controls interpolation steps */
  if (!dsp->t2d.get_new_target && 
      !reached_target(dsp->t2d.tang, dsp->t2d.dist_az,
       dsp->t2d.target_selection_method, &dsp->t2d.ppval, &dsp->t2d.oppval)) {

    increment_tour(dsp->t2d.tinc, dsp->t2d.tau, dsp->t2d.dist_az, 
      dsp->t2d.delta, &dsp->t2d.tang, (gint) 2);
    tour_reproject(dsp->t2d.tinc, dsp->t2d.G, dsp->t2d.Ga, dsp->t2d.Gz, 
      dsp->t2d.F, dsp->t2d.Va, d->n_cols, (gint) 2);

    /* plot pp indx */
    if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
      /*    if (dsp->t2d_ppda != NULL) {*/

      dsp->t2d.oppval = dsp->t2d.ppval;
      revert_random = t2d_switch_index(cpanel->t2d, 0, dsp, gg);
      t2d_ppdraw(dsp->t2d.ppval, dsp, gg);
    }
  }
  else { /* we're at the target plane */
    if (dsp->t2d.get_new_target) { /* store the pp parameters */
      if (dsp->t2d.target_selection_method == TOUR_PP)
      {
	/*        dsp->t2d_pp_op.index_best = dsp->t2d.ppval;
        for (i=0; i<2; i++)
          for (j=0; j<dsp->t2d.nactive; j++)
            dsp->t2d_pp_op.proj_best.vals[i][j] = 
	    dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];*/
      }
    }
    else 
    {/* make sure the ending projection is the same as the target */
      if (dsp->t2d.target_selection_method == TOUR_RANDOM)
      {
        if (dsp->t2d.tau.els[0] > 0.0 || dsp->t2d.tau.els[1] > 0.0) {
          do_last_increment(dsp->t2d.tinc, dsp->t2d.tau, 
            dsp->t2d.dist_az, (gint) 2);
          tour_reproject(dsp->t2d.tinc, dsp->t2d.G, dsp->t2d.Ga, dsp->t2d.Gz,
            dsp->t2d.F, dsp->t2d.Va, d->n_cols, (gint) 2);
        }
      }
    }
    nv = 0;
    for (i=0; i<d->n_cols; i++) {
      chosen = false;
      for (k=0; k<dsp->t2d.nactive; k++) {
        if (dsp->t2d.active_vars.els[k] == i) {
          chosen = true;
          break;
        }
      }
      if (!chosen) {
        if (fabs(dsp->t2d.F.vals[0][i]) < eps && 
          fabs(dsp->t2d.F.vals[1][i]) < eps)
          dsp->t2d.F.vals[0][i] = dsp->t2d.F.vals[1][i] = 0.0;
        if (fabs(dsp->t2d.F.vals[0][i]) > eps || 
          fabs(dsp->t2d.F.vals[1][i]) > eps)
        {
          nv++;
        }
      }
    }
    /* now cleanup: store the current basis into the starting basis */
    arrayd_copy(&dsp->t2d.F, &dsp->t2d.Fa);
    if (nv == 0 && dsp->t2d.nactive <= 2) /* only generate new dir if num of
                                           active/used variables is > 2 -
                                           this code allows for motion to
                                           continue while a variable is 
                                           fading out. */
      dsp->t2d.get_new_target = true;
    else {
      if (dsp->t2d.target_selection_method == TOUR_RANDOM) {
        gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
          d->n_cols, (gint) 2);
      }
      else if (dsp->t2d.target_selection_method == TOUR_PP) {
        /* pp guided tour  */

        for (j=0; j<2; j++)
          for (i=0; i<d->n_cols; i++)
            dsp->t2d.Fz.vals[j][i] = 0.0;
        dsp->t2d.Fz.vals[0][dsp->t2d.active_vars.els[0]]=1.0;
        dsp->t2d.Fz.vals[1][dsp->t2d.active_vars.els[1]]=1.0;

        dsp->t2d.oppval = -1.0;
        t2d_ppdraw_think(dsp, gg);
/*XX*/  gdk_flush ();
        revert_random = t2d_switch_index(cpanel->t2d, 
          dsp->t2d.target_selection_method, dsp, gg);

        if (!revert_random) {
          for (i=0; i<2; i++)
            for (j=0; j<dsp->t2d.nactive; j++) {
              if (isfinite((gdouble)dsp->t2d_pp_op.proj_best.vals[i][j]) != 0)
                dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[j]] = 
                  dsp->t2d_pp_op.proj_best.vals[i][j];
	    }
	  /*            dsp->t2d_pp_op.index_best = 0.0;*/
	    /*g_printerr ("tour_run:index_best %f temp %f \n", dsp->t2d_pp_op.index_best, dsp->t2d_pp_op.temp);
g_printerr ("proj: ");
for (i=0; i<dsp->t2d_pp_op.proj_best.ncols; i++) g_printerr ("%f ", dsp->t2d_pp_op.proj_best.vals[0][i]);
g_printerr ("\n");
	    */
          /* if the best projection is the same as the previous one, switch 
              to a random projection */
	  /*          if (!checkequiv(dsp->t2d.Fa.vals, dsp->t2d.Fz.vals, d->n_cols, 2)) 
          {
            gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
              d->n_cols, (gint) 2);
            for (i=0; i<2; i++)
              for (j=0; j<dsp->t2d.nactive; j++)
                dsp->t2d_pp_op.proj_best.vals[i][j] = 
                  dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[j]];
            revert_random = t2d_switch_index(cpanel->t2d, 
              dsp->t2d.target_selection_method, dsp, gg);
	      }*/
  /*          t2d_ppdraw(dsp->t2d.ppval, dsp, gg);*/
  /*          count = 0;*/
          ggobi_sleep(0);  
        }
        else
        {
	  /*          gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
		      d->n_cols, (gint) 2);*/
        }
        
      }
      pathprob = tour_path(dsp->t2d.Fa, dsp->t2d.Fz, dsp->t2d.F, d->n_cols, 
        (gint) 2, dsp->t2d.Ga,
        dsp->t2d.Gz, dsp->t2d.G, dsp->t2d.lambda, dsp->t2d.tv, dsp->t2d.Va,
        dsp->t2d.Vz, dsp->t2d.tau, dsp->t2d.tinc, 
        &dsp->t2d.dist_az, &dsp->t2d.tang);
      if (pathprob == 0) 
        dsp->t2d.get_new_target = false;
      else if (pathprob == 1) { /* problems with Fa so need to force a jump */
        tour2d_scramble(gg);
        pathprob = tour_path(dsp->t2d.Fa, dsp->t2d.Fz, dsp->t2d.F, d->n_cols, 
          (gint) 2, dsp->t2d.Ga,
          dsp->t2d.Gz, dsp->t2d.G, dsp->t2d.lambda, dsp->t2d.tv, dsp->t2d.Va,
          dsp->t2d.Vz, dsp->t2d.tau, dsp->t2d.tinc, 
          &dsp->t2d.dist_az, &dsp->t2d.tang);
      }
      else if (pathprob == 2 || pathprob == 3) { /* problems with Fz,
                                    so will force a new choice of Fz */
        dsp->t2d.get_new_target = true;
      }
    }
  }
  
  display_tailpipe (dsp, FULL_1PIXMAP, gg);
  varcircles_refresh (d, gg);
  if (dsp->t2d_video) tour2d_write_video(gg);
}

void
tour2d_do_step(displayd *dsp, GGobiSession *gg)
{
  tour2d_run(dsp, gg);
}

gint
tour2d_idle_func (displayd *dsp)
{
  GGobiSession *gg = GGobiFromDisplay (dsp);
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->t2d.paused;
  if (doit) {
    tour2d_run (dsp, gg);
    gdk_flush (); 
  }

  return (doit);
}

void tour2d_func (gboolean state, displayd *dsp, GGobiSession *gg)
{
  /* 
   * Since the tour variables are stored at the display level,
   * assume for the time being that a display with a tour must
   * be running in the first and only splot.
   */
  splotd *sp = (splotd *) g_list_nth_data (dsp->splots, 0);

  if (state) {
    if (dsp->t2d.idled == 0) {
      dsp->t2d.idled = g_idle_add_full (G_PRIORITY_LOW,
                                   (GtkFunction) tour2d_idle_func, dsp, NULL);

      gg->tour2d.idled = 1;
    }
  } else {
    if (dsp->t2d.idled != 0) {
      g_source_remove (dsp->t2d.idled);
      dsp->t2d.idled = 0;
    }
    gg->tour2d.idled = 0;
  }

  splot_connect_expose_handler (dsp->t2d.idled, sp);
}

void tour2d_reinit(GGobiSession *gg)
{
  gint i;
  displayd *dsp = gg->current_display;
  GGobiStage *d = dsp->d;
  splotd *sp = gg->current_splot;

  arrayd_zero (&dsp->t2d.Fa);
  arrayd_zero (&dsp->t2d.Fz);
  arrayd_zero (&dsp->t2d.F);
  arrayd_zero (&dsp->t2d.Ga);
  arrayd_zero (&dsp->t2d.Gz);

  for (i=0; i<2; i++)
  {
    dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Fa.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Ga.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.Gz.vals[i][dsp->t2d.active_vars.els[i]] = 1.0;
  }
  /*  for (i=0; i<2; i++) {
    for (j=0; j<d->n_cols; j++) {
      dsp->t2d.Fa.vals[i][j] = 0.;
      dsp->t2d.F.vals[i][j] = 0.;
    }
    dsp->t2d.Fa.vals[i][dsp->t2d.active_vars.els[i]] = 1.;
    dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[i]] = 1.;
    }*/

  dsp->t2d.tau.els[0] = 0.0;
  dsp->t2d.tau.els[1] = 0.0;

  dsp->t2d.get_new_target = true;

  sp->tour2d.initmax = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);

  if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) 
    t2d_pp_reinit(dsp, gg);
}

/* Variable manipulation */
void
tour2d_manip_init(gint p1, gint p2, splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  GGobiStage *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  GGobiSession *gg = GGobiFromSPlot(sp);
  gint j, k;
  gint n1vars = dsp->t2d.nactive;
  /*gdouble ftmp;*/
  gdouble tol = 0.05; 
  gdouble dtmp1;

  /* need to turn off tour */
  if (!cpanel->t2d.paused)
    tour2d_func(T2DOFF, gg->current_display, gg);

  /* If de-selected variables are still fading out of the tour
     we will need to take them out before starting manipulation - 
     no we don't 8/5/02 */
  /*  for (j=0; j<d->n_cols; j++)
    if (dsp->t2d.active_vars_p.els[j] == false) {
       if (dsp->t2d.F.vals[0][j] > 0.0) 
         dsp->t2d.F.vals[0][j] = 0.0;
       if (dsp->t2d.F.vals[1][j] > 0.0)
         dsp->t2d.F.vals[1][j] = 0.0;
    }
  norm(dsp->t2d.F.vals[0],d->n_cols);
  norm(dsp->t2d.F.vals[1],d->n_cols);
  if (!gram_schmidt(dsp->t2d.F.vals[0], dsp->t2d.F.vals[1],
  d->n_cols))*/
#ifdef EXCEPTION_HANDLING
    g_printerr("");/*t2d.F[0] equivalent to t2d.F[1]\n");*/
#else
      ;
#endif
  
  dsp->t2d_manipvar_inc = false;
  dsp->t2d_pos1 = dsp->t2d_pos1_old = p1;
  dsp->t2d_pos2 = dsp->t2d_pos2_old = p2;
  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->t2d.nactive; j++)
    if (dsp->t2d.active_vars.els[j] == dsp->t2d_manip_var) {
      dsp->t2d_manipvar_inc = true;
      n1vars--;
    }

  /* here need to check if the manip var is wholly contained in u, and
     if so do some check */

  if (n1vars > 1)
  {
    /* make manip basis, from existing projection */
    /* 0,1 will be the remainder of the projection, and
       2 will be the indicator vector for the manip var */
    for (j=0; j<d->n_cols; j++) 
    {
      dsp->t2d_manbasis.vals[0][j] = dsp->t2d.F.vals[0][j];
      dsp->t2d_manbasis.vals[1][j] = dsp->t2d.F.vals[1][j];
      dsp->t2d_manbasis.vals[2][j] = 0.;
    }
    dsp->t2d_manbasis.vals[2][dsp->t2d_manip_var] = 1.;

    for (j=0; j<3; j++)
    {
      for (k=0; k<3; k++)
        dsp->t2d_mvar_3dbasis.vals[j][k] = 0.;
      dsp->t2d_mvar_3dbasis.vals[j][j] = 1.;
    }

    norm(dsp->t2d_manbasis.vals[0],d->n_cols); /* this is just in case */
    norm(dsp->t2d_manbasis.vals[1],d->n_cols); /* it seems to work ok */
    norm(dsp->t2d_manbasis.vals[2],d->n_cols); /* without normalizing here */

    /* Check if column 3 (2) of manbasis is effectively equal to 
       column 1 (0) or 2(1). If they are then we'll have to randomly
       generate a new column 3. If not then we orthonormalize column 3
       on the other two. */
    while (!gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
    {
       gt_basis(dsp->t2d.tv, dsp->t2d.nactive, dsp->t2d.active_vars, 
        d->n_cols, (gint) 1);
      for (j=0; j<d->n_cols; j++) 
        dsp->t2d_manbasis.vals[2][j] = dsp->t2d.tv.vals[0][j];
    }
    while (!gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
    {
       gt_basis(dsp->t2d.tv, dsp->t2d.nactive, dsp->t2d.active_vars, 
        d->n_cols, (gint) 1);
      for (j=0; j<d->n_cols; j++) 
        dsp->t2d_manbasis.vals[2][j] = dsp->t2d.tv.vals[0][j];
    }
    while (!gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[1],
        d->n_cols))
    {
       gt_basis(dsp->t2d.tv, dsp->t2d.nactive, dsp->t2d.active_vars, 
        d->n_cols, (gint) 1);
      for (j=0; j<d->n_cols; j++) 
        dsp->t2d_manbasis.vals[1][j] = dsp->t2d.tv.vals[0][j];
    }
    /* This is innocuous, if the vectors are orthnormal nothing gets changed.
       But it protects against the case when vectors 0,1 were not
       orthonormal and a new vector 1 was generated, it checks the o.n.
       of all 3 vectors again. */
    gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[1],
      d->n_cols);
    gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
      d->n_cols);
    gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
      d->n_cols);

    /*    ftmp = 0.0;
    while (ftmp < tol) {
    if ((fabs(inner_prod(dsp->t2d_manbasis.vals[0],dsp->t2d_manbasis.vals[2],
       d->n_cols))>1.0-tol) || 
       (fabs(inner_prod(dsp->t2d_manbasis.vals[1],
       dsp->t2d_manbasis.vals[2],d->n_cols))>1.0-tol))
    {
      gt_basis(dsp->t2d.tv, dsp->t2d.nactive, dsp->t2d.active_vars, 
        d->n_cols, (gint) 1);
      for (j=0; j<d->n_cols; j++) 
        dsp->t2d_manbasis.vals[2][j] = dsp->t2d.tv.vals[0][j];
      g_printerr("0 manbasis2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[2][i]);
      g_printerr("\n");
      if (!gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
        d->n_cols)) 
        g_printerr("t2d_manbasis[0] equivalent to t2d_manbasis[2]\n");
      if (!gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
        g_printerr("t2d_manbasis[1] equivalent to t2d_manbasis[2]\n");

        g_printerr("1 manbasis0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[0][i]);
        g_printerr("\n");
        g_printerr("1 manbasis1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[1][i]);
        g_printerr("\n");
        g_printerr("1 manbasis2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[2][i]);
        g_printerr("\n");
      ftmp = calc_norm (dsp->t2d_manbasis.vals[2], d->n_cols);
    }
    else if (fabs(inner_prod(dsp->t2d_manbasis.vals[0],
      dsp->t2d_manbasis.vals[1],d->n_cols))>1.0-tol) 
    {
      printf("1 = 0\n");
      gt_basis(dsp->t2d.tv, dsp->t2d.nactive, dsp->t2d.active_vars, 
        d->n_cols, (gint) 1);
      for (j=0; j<d->n_cols; j++) 
        dsp->t2d_manbasis.vals[1][j] = dsp->t2d.tv.vals[0][j];
      if (!gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[1],
		   d->n_cols))
        g_printerr("t2d_manbasis[0] equivalent to t2d_manbasis[1]\n"); * this might not be necessary *
      if (!gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
        g_printerr("t2d_manbasis[0] equivalent to t2d_manbasis[2]\n");
      if (!gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
        g_printerr("t2d_manbasis[1] equivalent to t2d_manbasis[2]\n");
      ftmp = calc_norm (dsp->t2d_manbasis.vals[1], d->n_cols);
    }      
    else {
      printf("ok\n");
      if (!gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
        g_printerr("t2d_manbasis[0] equivalent to t2d_manbasis[2]\n");
      if (!gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
        d->n_cols))
        g_printerr("t2d_manbasis[1] equivalent to t2d_manbasis[2]\n");
      ftmp = calc_norm (dsp->t2d_manbasis.vals[2], d->n_cols);
    }
    }*/

    /*    while (ftmp < tol) {
	  }*/

    dsp->t2d_no_dir_flag = false;
    if (cpanel->t2d.manip_mode == MANIP_RADIAL)
    { /* check if variable is currently visible in plot */
      if ((dsp->t2d.F.vals[0][dsp->t2d_manip_var]*
        dsp->t2d.F.vals[0][dsp->t2d_manip_var] +
        dsp->t2d.F.vals[1][dsp->t2d_manip_var]*
        dsp->t2d.F.vals[1][dsp->t2d_manip_var]) < tol)
        dsp->t2d_no_dir_flag = true; /* no */
      else
      { /* yes: set radial manip direction to be current direction
             of contribution */
        dsp->t2d_rx = (gdouble) dsp->t2d.F.vals[0][dsp->t2d_manip_var];
        dsp->t2d_ry = (gdouble) dsp->t2d.F.vals[1][dsp->t2d_manip_var];
        dtmp1 = sqrt(dsp->t2d_rx*dsp->t2d_rx+dsp->t2d_ry*dsp->t2d_ry);
        dsp->t2d_rx /= dtmp1;
        dsp->t2d_ry /= dtmp1;
      }
    }
  }

}

void
tour2d_manip(gint p1, gint p2, splotd *sp, GGobiSession *gg) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  GGobiStage *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint actual_nvars = dsp->t2d.nactive;
  gboolean offscreen = false;
  gdouble phi, cosphi, sinphi, ca, sa, cosm, cospsi, sinpsi;
  gdouble distx, disty, x1, x2, y1, y2;
  gdouble denom = (gdouble) MIN(sp->max.x, sp->max.y)/2.;
  gdouble tol = 0.01;
  gdouble dtmp1, dtmp2;
  gdouble len_motion;
  gint i,j,k;
  gboolean pp_problem = false;

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 < 0)
    offscreen = true;

  if (dsp->t2d_manipvar_inc)
    actual_nvars = dsp->t2d.nactive-1;

  if (!offscreen) {
    dsp->t2d_pos1_old = dsp->t2d_pos1;
    dsp->t2d_pos2_old = dsp->t2d_pos2;
  
    dsp->t2d_pos1 = p1;
    dsp->t2d_pos2 = p2;

    if (actual_nvars > 1)
    {
      distx = disty = 0;
      if (cpanel->t2d.manip_mode != MANIP_ANGULAR)
      {
        if (cpanel->t2d.manip_mode == MANIP_OBLIQUE) 
        {
          distx = dsp->t2d_pos1 - dsp->t2d_pos1_old;
          disty = dsp->t2d_pos2_old - dsp->t2d_pos2;
        }
        else if (cpanel->t2d.manip_mode == MANIP_VERT) 
        {
          disty = dsp->t2d_pos2_old - dsp->t2d_pos2;
        }
        else if (cpanel->t2d.manip_mode == MANIP_HOR) 
        {
          distx = dsp->t2d_pos1 - dsp->t2d_pos1_old;
        }
        else if (cpanel->t2d.manip_mode == MANIP_RADIAL) 
        {
          if (dsp->t2d_no_dir_flag)
          {
            distx = dsp->t2d_pos1 - dsp->t2d_pos1_old;
            disty = dsp->t2d_pos2_old - dsp->t2d_pos2;
            dsp->t2d_rx = distx;
            dsp->t2d_ry = disty; 
            dtmp1 = sqrt(dsp->t2d_rx*dsp->t2d_rx+dsp->t2d_ry*dsp->t2d_ry);
            dsp->t2d_rx /= dtmp1;
            dsp->t2d_ry /= dtmp1;
            dsp->t2d_no_dir_flag = false;
          }
          distx = (dsp->t2d_rx*(dsp->t2d_pos1 - dsp->t2d_pos1_old) + 
            dsp->t2d_ry*(dsp->t2d_pos2_old - dsp->t2d_pos2))*dsp->t2d_rx;
          disty = (dsp->t2d_rx*(dsp->t2d_pos1 - dsp->t2d_pos1_old) + 
            dsp->t2d_ry*(dsp->t2d_pos2_old - dsp->t2d_pos2))*dsp->t2d_ry;
        }
        dtmp1 = (gdouble) (distx*distx+disty*disty);
        len_motion = (gdouble) sqrt(dtmp1);

        if (len_motion < tol) /* just in case, maybe not necessary */
        {
          dsp->t2d_Rmat2.vals[0][0] = 1.0;
          dsp->t2d_Rmat2.vals[0][1] = 0.0;
          dsp->t2d_Rmat2.vals[0][2] = 0.0;
          dsp->t2d_Rmat2.vals[1][0] = 0.0;
          dsp->t2d_Rmat2.vals[1][1] = 1.0;
          dsp->t2d_Rmat2.vals[1][2] = 0.0;
          dsp->t2d_Rmat2.vals[2][0] = 0.0;
          dsp->t2d_Rmat2.vals[2][1] = 0.0;
          dsp->t2d_Rmat2.vals[2][2] = 1.0;
        }
        else
        {
          phi = len_motion / denom;
     
          ca = distx/len_motion;
          sa = disty/len_motion;
      
          cosphi = (gdouble) cos((gdouble) phi);
          sinphi = (gdouble) sin((gdouble) phi);
          cosm = 1.0 - cosphi;
          dsp->t2d_Rmat2.vals[0][0] = ca*ca*cosphi + sa*sa;
          dsp->t2d_Rmat2.vals[0][1] = -cosm*ca*sa;
          dsp->t2d_Rmat2.vals[0][2] = sinphi*ca;
          dsp->t2d_Rmat2.vals[1][0] = -cosm*ca*sa;
          dsp->t2d_Rmat2.vals[1][1] = sa*sa*cosphi + ca*ca;
          dsp->t2d_Rmat2.vals[1][2] = sinphi*sa;
          dsp->t2d_Rmat2.vals[2][0] = -sinphi*ca;
          dsp->t2d_Rmat2.vals[2][1] = -sinphi*sa;
          dsp->t2d_Rmat2.vals[2][2] = cosphi;
        }
      }
      else 
      { /* angular constrained manipulation */
        if (dsp->t2d_pos1_old != sp->max.x/2 && 
          dsp->t2d_pos2_old != sp->max.y/2 &&
          dsp->t2d_pos1 != sp->max.x/2 && 
          dsp->t2d_pos2 != sp->max.y/2)
        {
          x1 = dsp->t2d_pos1_old - sp->max.x/2;
          y1 = dsp->t2d_pos2_old - sp->max.y/2;
          dtmp1 = sqrt(x1*x1+y1*y1);
          x1 /= dtmp1;
          y1 /= dtmp1;
          x2 = dsp->t2d_pos1 - sp->max.x/2;
          y2 = dsp->t2d_pos2 - sp->max.y/2;
          dtmp2 = sqrt(x2*x2+y2*y2);
          x2 /= dtmp2;
          y2 /= dtmp2;
          if (dtmp1 > tol && dtmp2 > tol)
          {
            cospsi = x1*x2+y1*y2;
            sinpsi = x1*y2-y1*x2;
          }
          else
          {
            cospsi = 1.;    
            sinpsi = 0.;
          }
        }
        else
        {
          cospsi = 1.;
          sinpsi = 0.;
        }
        dsp->t2d_Rmat2.vals[0][0] = cospsi;
        dsp->t2d_Rmat2.vals[0][1] = sinpsi;
        dsp->t2d_Rmat2.vals[0][2] = 0.;
        dsp->t2d_Rmat2.vals[1][0] = -sinpsi;
        dsp->t2d_Rmat2.vals[1][1] = cospsi;
        dsp->t2d_Rmat2.vals[1][2] = 0.;
        dsp->t2d_Rmat2.vals[2][0] = 0.;
        dsp->t2d_Rmat2.vals[2][1] = 0.;
        dsp->t2d_Rmat2.vals[2][2] = 1.;
      }

      /* Set up the rotation matrix in the 3D manip space */
      for (i=0; i<3; i++) 
        for (j=0; j<3; j++)
        {
          dtmp1 = 0.;
          for (k=0; k<3; k++)
            dtmp1 += (dsp->t2d_mvar_3dbasis.vals[i][k]*
              dsp->t2d_Rmat2.vals[k][j]);
          dsp->t2d_Rmat1.vals[i][j] = dtmp1;
        }
      arrayd_copy(&dsp->t2d_Rmat1, &dsp->t2d_mvar_3dbasis);

      norm(dsp->t2d_mvar_3dbasis.vals[0],3); /* just in case */
      norm(dsp->t2d_mvar_3dbasis.vals[1],3); /* seems to work ok without */
      norm(dsp->t2d_mvar_3dbasis.vals[2],3); /* this */
      if (!gram_schmidt(dsp->t2d_mvar_3dbasis.vals[0], 
        dsp->t2d_mvar_3dbasis.vals[1], 3))
#ifdef EXCEPTION_HANDLING
        g_printerr("");/*t2d_mvar[0] equivalent to t2d_mvar[1]\n");*/
#else
        ;
#endif
      if (!gram_schmidt(dsp->t2d_mvar_3dbasis.vals[0], 
        dsp->t2d_mvar_3dbasis.vals[2], 3))
#ifdef EXCEPTION_HANDLING
          g_printerr("");/*t2d_mvar[0] equivalent to t2d_mvar[2]\n");*/
#else
          ;
#endif
      if (!gram_schmidt(dsp->t2d_mvar_3dbasis.vals[1], 
        dsp->t2d_mvar_3dbasis.vals[2], 3))
#ifdef EXCEPTION_HANDLING
        g_printerr("");/*t2d_mvar[1] equivalent to t2d_mvar[2]\n");*/
#else
          ;
#endif

      /* Generate the projection of the data corresponding to 
         the 3D rotation in the manip space. */
      for (j=0; j<d->n_cols; j++)
      {
        dsp->t2d.F.vals[0][j] = 
          dsp->t2d_manbasis.vals[0][j]*dsp->t2d_mvar_3dbasis.vals[0][0] +
          dsp->t2d_manbasis.vals[1][j]*dsp->t2d_mvar_3dbasis.vals[0][1] +
          dsp->t2d_manbasis.vals[2][j]*dsp->t2d_mvar_3dbasis.vals[0][2];
        dsp->t2d.F.vals[1][j] = 
          dsp->t2d_manbasis.vals[0][j]*dsp->t2d_mvar_3dbasis.vals[1][0] +
          dsp->t2d_manbasis.vals[1][j]*dsp->t2d_mvar_3dbasis.vals[1][1] +
          dsp->t2d_manbasis.vals[2][j]*dsp->t2d_mvar_3dbasis.vals[1][2];
      }
      norm(dsp->t2d.F.vals[0], d->n_cols);
      norm(dsp->t2d.F.vals[1], d->n_cols);
      /*      if (calc_norm(dsp->t2d.F.vals[0], d->n_cols)>1.01) {
	g_printerr("1 F0 out of bounds\n");
        g_printerr("F0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d.F.vals[0][i]);
        g_printerr("\n");
        g_printerr("F1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d.F.vals[1][i]);
        g_printerr("\n");
        g_printerr("manbasis0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[0][i]);
        g_printerr("\n");
        g_printerr("manbasis1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[1][i]);
        g_printerr("\n");
        g_printerr("manbasis2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_manbasis.vals[2][i]);
        g_printerr("\n");
        g_printerr("m3dvar0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_mvar_3dbasis.vals[0][i]);
        g_printerr("\n");
        g_printerr("m3dvar1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_mvar_3dbasis.vals[1][i]);
        g_printerr("\n");
        g_printerr("m3dvar2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d_mvar_3dbasis.vals[2][i]);
        g_printerr("\n");
        g_printerr("distx %f disty %f\n",distx,disty);
      }
      if (calc_norm(dsp->t2d.F.vals[1], d->n_cols)>1.01) 
      g_printerr("1 F1 out of bounds\n");*/
      if (!gram_schmidt(dsp->t2d.F.vals[0], dsp->t2d.F.vals[1], d->n_cols))
#ifdef EXCEPTION_HANDLING
        g_printerr("");/*t2d.F[0] equivalent to t2d.F[2]\n");*/
#else
        ;
#endif

      /*      if (calc_norm(dsp->t2d.F.vals[0], d->n_cols)>1.0) 
	g_printerr("F0 out of bounds\n");
      if (calc_norm(dsp->t2d.F.vals[1], d->n_cols)>1.0) 
	g_printerr("F1 out of bounds\n");
      */
    }

    /* plot pp indx */
    if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
      /*    if (dsp->t2d_ppda != NULL) {*/

      dsp->t2d.oppval = dsp->t2d.ppval;
      pp_problem = t2d_switch_index(cpanel->t2d, 
        0, dsp, gg);
      t2d_ppdraw(dsp->t2d.ppval, dsp, gg);
    }

    display_tailpipe (dsp, FULL, gg);
    varcircles_refresh (d, gg);
  }
}

void
tour2d_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  GGobiSession *gg = GGobiFromSPlot(sp);

  disconnect_motion_signal (sp);

  arrayd_copy(&dsp->t2d.F, &dsp->t2d.Fa);
  zero_tau(dsp->t2d.tau, 2);
  dsp->t2d.get_new_target = true;

  /* need to turn on tour? */
  if (!cpanel->t2d.paused) {
    tour2d_func(T2DON, dsp, gg);

    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (dsp, FULL, gg);
  }
}

#undef T2DON
#undef T2DOFF
