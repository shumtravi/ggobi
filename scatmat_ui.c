/* scatmat_ui.c */

#include <strings.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/

static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append"};
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  cpaneld *cpanel = &current_display->cpanel;

  switch (indx) {
    case 0:
      cpanel->scatmat_selection_mode = VAR_REPLACE;
      break;
    case 1:
      cpanel->scatmat_selection_mode = VAR_INSERT;
      break;
    case 2:
      cpanel->scatmat_selection_mode = VAR_APPEND;
      break;
  }
}

/*------------------------------------------------------------------------*/
/*                         Control panel                                  */
/*------------------------------------------------------------------------*/

void
cpanel_scatmat_make () {
  GtkWidget *vb, *lbl, *opt;
  
  control_panel[SCATMAT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[SCATMAT]), 5);

/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[SCATMAT]), vb, false, false, 0);

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Selecting a variable either replaces the variable in the current plot (swapping if appropriate), inserts a new plot before the current plot, or appends a new plot after it",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
                        sizeof (selection_mode_lbl) / sizeof (gchar *),
                        selection_mode_cb);

  gtk_widget_show_all (control_panel[SCATMAT]);
}


/*------------------------------------------------------------------------*/
/*                       Resetting the main menubar                       */
/*------------------------------------------------------------------------*/


void
scatmat_main_menus_make (GtkAccelGroup *accel_group, GtkSignalFunc func) {

/*
 * I/O menu
*/
  scatmat_mode_menu = gtk_menu_new ();

  CreateMenuItem (scatmat_mode_menu, "Scatterplot Matrix",
    "^x", "", NULL, accel_group, func, GINT_TO_POINTER (SCATMAT));

  /* Add a separator */
  CreateMenuItem (scatmat_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (scatmat_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func, GINT_TO_POINTER (SCALE));
  CreateMenuItem (scatmat_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, GINT_TO_POINTER (BRUSH));
  CreateMenuItem (scatmat_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, GINT_TO_POINTER (IDENT));
  CreateMenuItem (scatmat_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, GINT_TO_POINTER (MOVEPTS));

  gtk_widget_show (scatmat_mode_menu);
}

