/* varpanel_ui.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#include "noop-checkbutton.h"

#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varpanel_checkbutton_set_active (gint jvar, gboolean active, datad *d)
{
  gboolean active_prev;

  if (jvar >= 0 && jvar < d->ncols) {
    GtkWidget *w = GTK_WIDGET (d->varpanel_ui.label[jvar]);
    if (GTK_WIDGET_REALIZED (d->varpanel_ui.label[jvar])) {

      active_prev = GTK_TOGGLE_BUTTON (w)->active;
      GTK_TOGGLE_BUTTON (w)->active = active;

      if (active != active_prev)
        gtk_widget_queue_draw (w);
    }
  }
}


void
varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint btn,
  gint alt_mod, gint ctrl_mod, gint shift_mod, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw;
  gint jvar_prev = -1;
  extern void tour2d_varsel (ggobid *, gint, gint);

  if (display == NULL || !GTK_IS_WIDGET (display->window)) {
    g_printerr ("Bug?  I see no active display\n");
    return ;
  }
  
  switch (display->displaytype) {

    case parcoords:
      redraw = parcoords_varsel (cpanel, sp, jvar, &jvar_prev, alt_mod, gg);
    break;

    case scatmat:
      redraw = scatmat_varsel_simple (cpanel, sp, jvar, &jvar_prev,
        btn, alt_mod, gg);
    break;

    case scatterplot:
      switch (cpanel->projection) {
        case P1PLOT:
          redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
        break;
        case XYPLOT:
          redraw = xyplot_varsel (sp, jvar, &jvar_prev, btn);
        break;
        case TOUR2D:
/*
          tour2d_varsel (gg, jvar, btn);
*/
        break;
        default:
        break;
    }
    break;
  }

  /*-- overkill for scatmat: could redraw one row, one column --*/
  /*-- overkill for parcoords: need to redraw at most 3 plots --*/
/* this is redrawing before it has the new window sizes, so the
 * lines aren't right */
  if (redraw) {
    display_tailpipe (display, gg);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Variable menus                                      */
/*-------------------------------------------------------------------------*/


static void
varsel_from_menu (GtkWidget *w, gpointer data)
{
  varseldatad *vdata = (varseldatad *) data;
  ggobid *gg = vdata->gg;
  displayd *display = gg->current_display;
  datad *d = display->d;
  cpaneld *cpanel = &display->cpanel;

  /*-- I think the menu should be destroyed here. --*/
  gtk_widget_destroy (w->parent);

  varsel (cpanel, gg->current_splot, vdata->jvar, vdata->btn,
    vdata->alt_mod, vdata->ctrl_mod, vdata->shift_mod, d, gg);
}

GtkWidget *
p1d_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->p1d_menu.vdata0.sp = gg->p1d_menu.vdata1.sp = gg->current_splot;
  gg->p1d_menu.vdata0.jvar = gg->p1d_menu.vdata1.jvar = jvar;
  gg->p1d_menu.vdata0.alt_mod = gg->p1d_menu.vdata1.alt_mod = false;

  gg->p1d_menu.vdata0.btn = 1;
  gg->p1d_menu.vdata1.btn = 2;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &(gg->p1d_menu.vdata0), gg);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &(gg->p1d_menu.vdata1), gg);

  return menu;
}

GtkWidget *
xyplot_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->xyplot_menu.vdata0.sp = gg->xyplot_menu.vdata1.sp = gg->current_splot;
  gg->xyplot_menu.vdata0.jvar = gg->xyplot_menu.vdata1.jvar = jvar;
  gg->xyplot_menu.vdata0.alt_mod = gg->xyplot_menu.vdata1.alt_mod = false;

  gg->xyplot_menu.vdata0.btn = 1;
  gg->xyplot_menu.vdata1.btn = 2;

  gg->xyplot_menu.vdata0.gg = gg;
  gg->xyplot_menu.vdata1.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->xyplot_menu.vdata0, gg);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->xyplot_menu.vdata1, gg);

  return menu;
}

GtkWidget *
rotation_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;
  
  gg->rotation_menu.vdata0.sp = gg->rotation_menu.vdata1.sp =
    gg->rotation_menu.vdata2.sp = gg->current_splot;
  gg->rotation_menu.vdata0.jvar = gg->rotation_menu.vdata1.jvar =
    gg->rotation_menu.vdata2.jvar = jvar;
  gg->rotation_menu.vdata0.alt_mod = gg->rotation_menu.vdata1.alt_mod =
    gg->rotation_menu.vdata2.alt_mod = false;

  gg->rotation_menu.vdata0.btn = 1;
  gg->rotation_menu.vdata1.btn = 2;
  gg->rotation_menu.vdata2.btn = 3;

  gg->rotation_menu.vdata2.gg = gg->rotation_menu.vdata1.gg =
    gg->rotation_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select X  L",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->rotation_menu.vdata0, gg);
  CreateMenuItem (menu, "Select Y  M",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->rotation_menu.vdata1, gg);
  CreateMenuItem (menu, "Select Z  R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->rotation_menu.vdata2, gg);

  return menu;
}

GtkWidget *
tour2d_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->tour2d_menu.vdata0.sp = gg->tour2d_menu.vdata1.sp =
    gg->tour2d_menu.vdata2.sp = gg->current_splot;
  gg->tour2d_menu.vdata0.jvar = gg->tour2d_menu.vdata1.jvar =
    gg->tour2d_menu.vdata2.jvar = jvar;
  gg->tour2d_menu.vdata0.alt_mod = gg->tour2d_menu.vdata1.alt_mod =
    gg->tour2d_menu.vdata2.alt_mod = false;
  gg->tour2d_menu.vdata0.shift_mod = gg->tour2d_menu.vdata2.shift_mod = false;
  gg->tour2d_menu.vdata1.shift_mod = true;
  gg->tour2d_menu.vdata0.ctrl_mod = gg->tour2d_menu.vdata1.ctrl_mod = false;
  gg->tour2d_menu.vdata2.ctrl_mod = true;

  gg->tour2d_menu.vdata2.gg = gg->tour2d_menu.vdata1.gg =  
    gg->tour2d_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Tour   L,M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->tour2d_menu.vdata0, gg);
  CreateMenuItem (menu, "Manip  <Shift> L,M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->tour2d_menu.vdata1, gg);
  CreateMenuItem (menu, "Freeze <Ctrl> L,M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->tour2d_menu.vdata2, gg);

  return menu;
}

/*
  corr_tour_menu = gtk_menu_new ();
  CreateMenuItem (corr_tour_menu, "Tour X    L",         NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Tour Y    M",         NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Manip X   <Shift> L", NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Manip Y   <Shift> M", NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Freeze X  <Ctrl> L",  NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Freeze Y  <Ctrl> M",  NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
*/

GtkWidget *
parcoords_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->parcoords_menu.vdata0.sp = gg->parcoords_menu.vdata1.sp =
    gg->current_splot;
  gg->parcoords_menu.vdata0.jvar = gg->parcoords_menu.vdata1.jvar = jvar;
  gg->parcoords_menu.vdata0.alt_mod = false;
  gg->parcoords_menu.vdata1.alt_mod = true;

  gg->parcoords_menu.vdata1.gg = gg->parcoords_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select Y      M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->parcoords_menu.vdata0, gg);
  CreateMenuItem (menu, "Delete Y <alt>M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->parcoords_menu.vdata1, gg);

  return menu;
}

GtkWidget *
scatmat_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->scatmat_menu.vdata0.sp =
    gg->scatmat_menu.vdata1.sp =
    gg->scatmat_menu.vdata2.sp =
    gg->scatmat_menu.vdata3.sp =
    gg->current_splot;
  gg->scatmat_menu.vdata0.jvar =
    gg->scatmat_menu.vdata1.jvar =
    gg->scatmat_menu.vdata2.jvar =
    gg->scatmat_menu.vdata3.jvar = jvar;
  gg->scatmat_menu.vdata0.alt_mod = gg->scatmat_menu.vdata1.alt_mod = false;
  gg->scatmat_menu.vdata2.alt_mod = gg->scatmat_menu.vdata3.alt_mod = 3;

  gg->scatmat_menu.vdata0.btn = gg->scatmat_menu.vdata2.btn = 1;
  gg->scatmat_menu.vdata1.btn = gg->scatmat_menu.vdata3.btn = 2;

  gg->scatmat_menu.vdata3.gg =
    gg->scatmat_menu.vdata2.gg =
    gg->scatmat_menu.vdata1.gg =
    gg->scatmat_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select row  L",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->scatmat_menu.vdata0, gg);
  CreateMenuItem (menu, "Select col  M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->scatmat_menu.vdata1, gg);
  CreateMenuItem (menu, "Delete row  <alt>L",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC
    (varsel_from_menu), (gpointer) &gg->scatmat_menu.vdata2, gg);
  CreateMenuItem (menu, "Delete col  <alt>M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->scatmat_menu.vdata3, gg);

  return menu;
}

static gint
popup_varmenu (GtkWidget *w, GdkEvent *event, gpointer cbd) 
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel;
  gint jvar = GPOINTER_TO_INT (cbd);
  GtkWidget *p1d_menu, *xyplot_menu;
  GtkWidget *rotation_menu, *tour_menu;
  GtkWidget *parcoords_menu, *scatmat_menu;
  /*-- w  is the variable label --*/
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (w), "datad");

  if (display == NULL)
    return false;
  if (display->d != d)  /*-- only select for the current plot --*/
    return false;

  cpanel = &display->cpanel;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button == 1) {
      gint projection = projection_get (gg);

      switch (display->displaytype) {

        case scatterplot:
          switch (projection) {
            case P1PLOT:
              p1d_menu = p1d_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (p1d_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
            case XYPLOT:
              xyplot_menu = xyplot_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (xyplot_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
            case ROTATE:
              rotation_menu = rotation_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (rotation_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
            case TOUR2D:
            case COTOUR:
              tour_menu = tour2d_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
          }
          break;

        case scatmat:
          scatmat_menu = scatmat_menu_build (jvar, d, gg);
          gtk_menu_popup (GTK_MENU (scatmat_menu), NULL, NULL,
                position_popup_menu, NULL,
            bevent->button, bevent->time);
          break;

        case parcoords:
          parcoords_menu = parcoords_menu_build (jvar, d, gg);
          gtk_menu_popup (GTK_MENU (parcoords_menu), NULL, NULL,
                position_popup_menu, NULL,
            bevent->button, bevent->time);
          break;
      }
      return true;
    }
  }
  return false;
}

/*-------------------------------------------------------------------------*/
/*                   variable cloning                                      */
/*-------------------------------------------------------------------------*/

void
variable_clone (gint jvar, const gchar *newName, gboolean update,
  datad *d, ggobid *gg) 
{
  gint nc = d->ncols + 1;
  gint j;
  
  /*-- set a view of the data values before adding the new label --*/
  vartable_row_append (d->ncols-1, d, gg);
  vartable_realloc (nc, d, gg);
  d->vartable[nc-1].collab =
    g_strdup (newName && newName[0] ? newName : d->vartable[jvar].collab);
  d->vartable[nc-1].collab_tform =
    g_strdup (newName && newName[0] ? newName : d->vartable[jvar].collab);

  for (j=0; j<d->ncols; j++) {
    d->varpanel_ui.label = (GtkWidget **)
      g_realloc (d->varpanel_ui.label, nc * sizeof (GtkWidget *));
  }

  /*-- now the rest of the variables --*/
  d->vartable[nc-1].groupid = d->vartable[nc-1].groupid_ori =
    d->vartable[d->ncols-1].groupid + 1; 
  d->vartable[nc-1].jitter_factor = d->vartable[jvar].jitter_factor;
  d->vartable[nc-1].nmissing = d->vartable[jvar].nmissing;

  if(update) {
    updateAddedColumn (nc, jvar, d, gg);
  }

  gtk_widget_show_all (gg->varpanel_ui.varpanel);
}


gboolean
updateAddedColumn (gint nc, gint jvar, datad *d, ggobid *gg)
{
  if(jvar > -1) {
    d->vartable[nc-1].mean = d->vartable[jvar].mean;
    d->vartable[nc-1].median = d->vartable[jvar].median;
    d->vartable[nc-1].lim.min =
      d->vartable[nc-1].lim_raw.min = d->vartable[nc-1].lim_raw_gp.min =
      d->vartable[nc-1].lim_tform.min = d->vartable[nc-1].lim_tform_gp.min =
      d->vartable[jvar].lim_raw.min;
    d->vartable[nc-1].lim.max =
      d->vartable[nc-1].lim_raw.max = d->vartable[nc-1].lim_raw_gp.max =
      d->vartable[nc-1].lim_tform.max = d->vartable[nc-1].lim_tform_gp.max =
      d->vartable[jvar].lim_raw.max;
   } 

  transform_values_init (nc-1, d, gg);

  pipeline_arrays_add_column (jvar, d, gg);  /* reallocate and copy */
  missing_arrays_add_column (jvar, d, gg);

  d->ncols++;
  tform_to_world (d, gg); /*-- need this only for the new variable --*/

  return (true);
}

/*-------------------------------------------------------------------------*/


/*-- here's where we'd reset what's selected according to the current mode --*/
void
varpanel_refresh (ggobid *gg) {
  gint j;
  displayd *display = gg->current_display;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;
  gint nd = g_slist_length (gg->d);
  gint k;

  if (nd > 0 && sp != NULL) {

    for (k=0; k<nd; k++) {
      datad *d = (datad*) g_slist_nth_data (gg->d, k);
      if (display->d != d)
        ;  /*-- we will only deal with the current datad --*/
      else {

        switch (display->displaytype) {

          case parcoords:
          {
            GList *l;
            for (j=0; j<d->ncols; j++)
              varpanel_checkbutton_set_active (j, false, d);

            l = display->splots;
            while (l) {
              j = ((splotd *) l->data)->p1dvar;
              varpanel_checkbutton_set_active (j, true, d);
              l = l->next;
            }
          }
          break;

          case scatmat:
          {
            GList *l;
            for (j=0; j<d->ncols; j++)
              varpanel_checkbutton_set_active (j, false, d);
            l = display->scatmat_cols;  /*-- assume rows = cols --*/
            while (l) {
              j = GPOINTER_TO_INT (l->data);
              varpanel_checkbutton_set_active (j, true, d);
              l = l->next;
            }
          }
          break;

          case scatterplot:
            switch (cpanel->projection) {
              case P1PLOT:
                for (j=0; j<d->ncols; j++)
                  varpanel_checkbutton_set_active (j, (j == sp->p1dvar), d);
              break;
              case XYPLOT:
                for (j=0; j<d->ncols; j++)
                  varpanel_checkbutton_set_active (j,
                    (j == sp->xyvars.x || j == sp->xyvars.y), d);
              break;
              default:
              break;
          }
          break;
        }
      }
    }
  }
}

/*-- responds to a button_press_event --*/
static gint
varsel_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  splotd *sp = gg->current_splot;

  if (d != display->d)
    return true;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint button = bevent->button;
    gboolean alt_mod, shift_mod, ctrl_mod;
    gint j, jvar;

    /*-- respond only to button 1 and button 2 --*/
    if (button != 1 && button != 2)
      return false;

    jvar = -1;
    for (j=0; j<d->ncols; j++) {
      if (d->varpanel_ui.label[j] == w) {
        jvar = j;
        break;
      }
    }

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    if (ctrl_mod) {
      variable_clone (jvar, NULL, true, d, gg);
      return (true);
    }
    
    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, d, gg);
    varpanel_refresh (gg);
    return true;
  }

  return false;
}

/*-------------------------------------------------------------------------*/
/*                  initialize and populate the var panel                  */
/*-------------------------------------------------------------------------*/

/*-- build the scrolled window and vbox; the d-specific parts follow --*/
void
varpanel_make (GtkWidget *parent, ggobid *gg) {

  gg->selvarfg_GC = NULL;

  gg->varpanel_ui.varpanel_accel_group = gtk_accel_group_new ();

  /*-- create a scrolled window --*/
  gg->varpanel_ui.scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent),
    gg->varpanel_ui.scrolled_window, true, true, 2);
  gtk_widget_show (gg->varpanel_ui.scrolled_window);

  /*-- create varpanel, a vbox, and add it to the scrolled window --*/
  gg->varpanel_ui.varpanel = gtk_vbox_new (false, 10);
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    gg->varpanel_ui.varpanel);

  gtk_widget_show_all (gg->varpanel_ui.scrolled_window);
}


/*-- create a column of check buttons? --*/
void varpanel_populate (datad *d, ggobid *gg)
{
  gint j;
  GtkWidget *ebox;
  GtkWidget *frame = gtk_frame_new (NULL);

  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (gg->varpanel_ui.varpanel),
    frame, false, false, 2);

  /*-- add an ebox to the frame --*/
  ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (frame), ebox);
  
  /*-- add a vbox to the ebox --*/
  d->varpanel_ui.vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (ebox), d->varpanel_ui.vbox);

  gtk_widget_show_all (frame);
  gdk_flush ();

  d->varpanel_ui.label = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));
  for (j=0; j<d->ncols; j++) {
    d->varpanel_ui.label[j] =
      gtk_noop_check_button_new_with_label (d->vartable[j].collab);
    GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.label[j]), gg, true);

    gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.label[j]),
      "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);

    gtk_box_pack_start (GTK_BOX (d->varpanel_ui.vbox),
      d->varpanel_ui.label[j], true, true, 0);
    gtk_widget_show (d->varpanel_ui.label[j]);
  }
    
}


void
varlabel_set (gint j, datad *d, ggobid *gg) {
  gtk_label_set_text (GTK_LABEL (GTK_BIN (d->varpanel_ui.label[j])->child),
    d->vartable[j].collab_tform);
}

/*-------------------------------------------------------------------------*/
/*                          API; not used                                  */
/*-------------------------------------------------------------------------*/

void
GGOBI(selectScatterplotX) (gint jvar, ggobid *gg) 
{
  displayd *display = gg->current_display;
  if (display->displaytype != scatterplot)
    return;
  else {

    datad *d = display->d;
    splotd *sp = (splotd *) display->splots->data;
    cpaneld *cpanel = &display->cpanel;

    varsel (cpanel, sp, jvar, 1, false, false, false, d, gg);
  }
}

