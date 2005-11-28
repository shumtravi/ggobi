/* scatmatClass.c */
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

#include "scatmatClass.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <math.h>
#include <string.h>

#include "externs.h"
#include "write_state.h"

static gboolean
cpanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{
  cpanel_scatmat_set (dpy, cpanel, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);
  return(true);
}

static void
movePointsMotionCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventMotion *event, ggobid *gg)
{
  if(sp->p1dvar == -1)
     scatterplotMovePointsMotionCb(display, sp, w, event, gg);
}

static void
movePointsButtonCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  if(sp->p1dvar == -1)
    scatterplotMovePointsButtonCb(display, sp, w, event, gg);
}

/* XXX duncan and dfs: you need to sort this out
static void
worldToRaw(displayd *display, splotd *sp, gint pt, datad *d, ggobid *gg)
{
 if (sp->p1dvar == -1) {
    world_to_raw_by_var (pt, sp->xyvars.x, display, d, gg);
    world_to_raw_by_var (pt, sp->xyvars.y, display, d, gg);
  }
}
*/


static gint 
variablePlottedP(displayd *display, gint *cols, gint ncols, datad *d)
{
	GList *l;
	gint j;
	splotd *sp;
        for (l = display->splots; l; l = l->next) {
          sp = (splotd *) l->data;

          for (j=0; j<ncols; j++) {
            if (sp->p1dvar == -1) {
              if (sp->xyvars.x == cols[j]) {
                return(sp->xyvars.x);
              }
              if (sp->xyvars.y == cols[j]) {
                return(sp->xyvars.y);
              }
            } else if (sp->p1dvar == cols[j]) {
              return(sp->p1dvar);
            }
          }
        }
	return(-1);
}

static gboolean
variableSelect(GtkWidget *w, displayd *dpy, splotd *sp, gint jvar, gint toggle, gint mouse, cpaneld *cpanel, ggobid *gg)
{
  gint jvar_prev;
  return(scatmat_varsel_simple (cpanel, sp, jvar, &jvar_prev, gg));
}

static void 
varpanelRefresh(displayd *display, splotd *sp, datad *d)
{
  gint j;
  GList *l;

  for (j=0; j<d->ncols; j++) {
    varpanel_toggle_set_active (VARSEL_X, j, false, d);

    varpanel_toggle_set_active (VARSEL_Y, j, false, d);
    varpanel_widget_set_visible (VARSEL_Y, j, false, d);
    varpanel_toggle_set_active (VARSEL_Z, j, false, d);
    varpanel_widget_set_visible (VARSEL_Z, j, false, d);
  }
  l = display->scatmat_cols;  /*-- assume rows = cols --*/
  while (l) {
    j = GPOINTER_TO_INT (l->data);
    varpanel_toggle_set_active (VARSEL_X, j, true, d);
    l = l->next;
  }
}


static void
varpanelTooltipsSet(displayd *display, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label)
{
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
    "Select to replace/insert/append a variable, or to delete it",
    NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
    "Click to replace/insert/append a variable, or to delete it",
    NULL);
}

static gint
plottedVarsGet(displayd *display, gint *cols, datad *d, ggobid *gg)
{
      GList *l;
      splotd *s;
      gint ncols = 0;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (s->p1dvar == -1) {
          if (!array_contains (cols, ncols, s->xyvars.x))
            cols[ncols++] = s->xyvars.x;
          if (!array_contains (cols, ncols, s->xyvars.y))
            cols[ncols++] = s->xyvars.y;
        } else {
          if (!array_contains (cols, ncols, s->p1dvar))
            cols[ncols++] = s->p1dvar;
        }
      }
      return(ncols);
}

displayd *
createWithVars(gboolean missing_p, gint nvars, gint *vars, datad *d, ggobid *gg)
{
   return(GGOBI(newScatmat)(vars, vars, nvars, nvars, d, gg));
}


void
add_xml_scatmat_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot = plots->data;
  int n, n1, i;

  n1 = g_list_length(plots);
  n = sqrt(n1);

  for(i = 0; i < n1 ; i+=n) {
      plot = (splotd *) g_list_nth_data(plots, i);
      XML_addVariable(node, plot->xyvars.x, dpy->d);
  }
}

gboolean
scatmatEventHandlersToggle(displayd * dpy, splotd * sp, gboolean state,
                            ProjectionMode pmode, InteractionMode imode)
{
  switch (imode) {
  case DEFAULT_IMODE:
      switch (sp->p1dvar) {
        case -1:
          xyplot_event_handlers_toggle (sp, state);
        break;
        default:
          p1d_event_handlers_toggle (sp, state);
      }
  break;
  case SCALE:
      scale_event_handlers_toggle (sp, state);
  break;
  case BRUSH:
      brush_event_handlers_toggle (sp, state);
  break;
  case IDENT:
      identify_event_handlers_toggle (sp, state);
  break;
  default:
  break;
  }

  return (false);
}

static gboolean
scatmatKeyEventHandled(GtkWidget *w, displayd *display, splotd * sp, GdkEventKey *event, ggobid *gg)
{
  gboolean ok = true;
  ProjectionMode pmode = NULL_PMODE;
  InteractionMode imode = DEFAULT_IMODE;

  if (event->state == 0 || event->state == GDK_CONTROL_MASK) {

  switch (event->keyval) {
    case GDK_h:
    case GDK_H:
      pmode = EXTENDED_DISPLAY_PMODE;
    break;

    case GDK_s:
    case GDK_S:
      imode = SCALE;
    break;
    case GDK_b:
    case GDK_B:
      imode = BRUSH;
    break;
    case GDK_i:
    case GDK_I:
      imode = IDENT;
    break;

    default:
      ok = false;
    break;
  }

  if (ok) {
    GGOBI(full_viewmode_set)(pmode, imode, gg);
  }
  } else { ok = false; }

  return ok;
}

static void
displaySet(displayd *display, ggobid *gg)
{
/*  GtkWidget *imode_menu;

  imode_menu = scatmat_imode_menu_make (gg->imode_accel_group,
    G_CALLBACK(imode_set_cb), gg, true);
  gg->imode_item = submenu_make ("_Interaction", 'I',
    gg->main_accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->imode_item),
    imode_menu); 
  submenu_insert (gg->imode_item, gg->main_menubar, 2);*/
}

static gboolean
handlesInteraction(displayd *display, InteractionMode v)
{
  return(v == SCALE || v == BRUSH || v == IDENT || /*v == MOVEPTS ||*/
          v == DEFAULT_IMODE);
}

static GtkWidget *
scatmatCPanelWidget(displayd *dpy, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w = cpanel_scatmat_make(gg);
  }
  *modeName = "Scatterplot Matrix";
  return(w);
}

static void
splotScreenToTform(cpaneld *cpanel, splotd *sp, icoords *scr,
		   fcoords *tfd, ggobid *gg)
{
  gcoords planar, world;
  greal precis = (greal) PRECISION1;
  greal ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gfloat scale_x, scale_y;
  vartabled *vt, *vtx, *vty;

  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

/*
 * screen to plane 
*/
  planar.x = (scr->x - sp->max.x/2) * precis / sp->iscale.x ;
  planar.x += sp->pmid.x;
  planar.y = (scr->y - sp->max.y/2) * precis / sp->iscale.y ;
  planar.y += sp->pmid.y;

/*
 * plane to world
*/

  if (sp->p1dvar != -1) {

      vt = vartable_element_get (sp->p1dvar, d);
      max = vt->lim.max;
      min = vt->lim.min;
      rdiff = max - min;

      if (display->p1d_orientation == HORIZONTAL) {
        /* x */
        world.x = planar.x;
        ftmp = world.x / precis;
        tfd->x = (ftmp + 1.0) * .5 * rdiff;
        tfd->x += min;
      } else {
        /* y */
        world.y = planar.y;
        ftmp = world.y / precis;
        tfd->y = (ftmp + 1.0) * .5 * rdiff;
        tfd->y += min;
      }
  } else {

      /* x */
      vtx = vartable_element_get (sp->xyvars.x, d);
      max = vtx->lim.max;
      min = vtx->lim.min;
      rdiff = max - min;
      world.x = planar.x;
      ftmp = world.x / precis;
      tfd->x = (ftmp + 1.0) * .5 * rdiff;
      tfd->x += min;

      /* y */
      vty = vartable_element_get (sp->xyvars.y, d);
      max = vty->lim.max;
      min = vty->lim.min;
      rdiff = max - min;
      world.y = planar.y;
      ftmp = world.y / precis;
      tfd->y = (ftmp + 1.0) * .5 * rdiff;
      tfd->y += min;
  }
}

void
scatmatDisplayClassInit(GGobiScatmatDisplayClass *klass)
{
	klass->parent_class.show_edges_p = true;
	klass->parent_class.treeLabel = klass->parent_class.titleLabel = "Scatterplot Matrix";

	klass->parent_class.cpanel_set = cpanelSet;
        klass->parent_class.imode_control_box = scatmatCPanelWidget;

	klass->parent_class.xml_describe = add_xml_scatmat_variables;
	klass->parent_class.move_points_motion_cb = movePointsMotionCb;
	klass->parent_class.move_points_button_cb = movePointsButtonCb;
/* XXX duncan and dfs: you need to sort this out
	klass->parent_class.world_to_raw = worldToRaw;
*/
	klass->parent_class.variable_plotted_p = variablePlottedP;
	klass->parent_class.variable_select = variableSelect;
	klass->parent_class.varpanel_refresh = varpanelRefresh;
	klass->parent_class.varpanel_tooltips_set = varpanelTooltipsSet;
	klass->parent_class.plotted_vars_get = plottedVarsGet;
	klass->parent_class.createWithVars = createWithVars;
	klass->parent_class.display_set = displaySet;
	klass->parent_class.mode_ui_get = scatmat_mode_ui_get;
	klass->parent_class.handles_interaction = handlesInteraction;

  klass->parent_class.event_handlers_toggle = scatmatEventHandlersToggle;
  klass->parent_class.splot_key_event_handled = scatmatKeyEventHandled;
}


/* */
static gchar *
treeLabel(splotd *splot, datad *d, ggobid *gg)
{
   gint n;
   vartabled *vtx, *vty;
   gchar *buf;
      vtx = vartable_element_get (splot->xyvars.x, d);
      vty = vartable_element_get (splot->xyvars.y, d);

      n = strlen (vtx->collab) + strlen (vty->collab) + 5;
      buf = (gchar*) g_malloc (n * sizeof (gchar*));
      sprintf (buf, "%s v %s", vtx->collab, vty->collab);
      return(buf);
}


static void
worldToPlane(splotd *sp, datad *d, ggobid *gg)
{
      if (sp->p1dvar == -1)
        xy_reproject (sp, d->world.vals, d, gg);
      else
        p1d_reproject (sp, d->world.vals, d, gg);
}

gboolean
drawEdgeP(splotd *sp, gint m, datad *d, datad *e, ggobid *gg)
{
	gboolean draw_edge = true;
        if (sp->p1dvar != -1) {
          if (e->missing.vals[m][sp->p1dvar])
            draw_edge = false;
        } else {
          if (e->missing.vals[m][sp->xyvars.x] ||
              e->missing.vals[m][sp->xyvars.y])
          {
            draw_edge = false;
          }
        }
	return(draw_edge);
}

gboolean
drawCaseP(splotd *sp, gint m, datad *d, ggobid *gg)
{
	gboolean draw_case = true;
        if (sp->p1dvar != -1) {
          if (d->missing.vals[m][sp->p1dvar])
            draw_case = false;
        } else {
          if (d->missing.vals[m][sp->xyvars.x] ||
              d->missing.vals[m][sp->xyvars.y])
          {
            draw_case = false;
          }
        }
	return(draw_case);
}

void
addPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
    if (sp->p1dvar == -1)
      scatterXYAddPlotLabels(sp, drawable, gg->plot_GC);
    else {
           /*-- 1dplot: center the label --*/
      scatter1DAddPlotLabels(sp, drawable, gg->plot_GC);
    }
}


static gint
splotVariablesGet(splotd *sp, gint *cols, datad *d)
{
	if(sp->p1dvar > -1) {
   	   cols[0] = sp->p1dvar;
	   return(1);
	} else {
    	   cols[0] = sp->xyvars.x;
     	   cols[1] = sp->xyvars.y;
   	   return(2);
	}
}


void
scatmatSPlotClassInit(GGobiScatmatSPlotClass *klass) 
{
  klass->parent_class.tree_label = treeLabel;

  /* reverse pipeline */ 
  klass->parent_class.screen_to_tform = splotScreenToTform;
  klass->parent_class.world_to_plane = worldToPlane;

  klass->parent_class.draw_case_p = drawCaseP;
  klass->parent_class.draw_edge_p = drawEdgeP;
  klass->parent_class.add_plot_labels = addPlotLabels;

  klass->parent_class.plotted_vars_get = splotVariablesGet;
}