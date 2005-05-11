/*-- record_id.c --*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define MAXIDSIZE 104

/*--------------------------------------------------------------------*/
/*                   Memory allocation, initialization                */
/*--------------------------------------------------------------------*/

void rowids_init_null (datad *d)
{
  vectori_init_null (&d->rowid.id);
  vectori_init_null (&d->rowid.idv);
  d->rowid.maxId = -1;
}

void rowids_free (datad *d)
{
  vectori_free (&d->rowid.id);
}

void
rowids_alloc (datad *d) 
{
  vectori_alloc (&d->rowid.id, d->nrows);
}

void
rowidv_init (datad *d) 
{
  gint i, k;

  if (d->rowid.id.nels > 0) {

    /*
     * No longer assume sorting, but instead compute the maximum as we read
     * the id's use the maximum value of rowid.id.els given by d->rowid.maxId
     * as the dimension rowid.idv
     */
    gint nels = 1 + d->rowid.maxId; 

    vectori_alloc (&d->rowid.idv, nels);
    for (i=0; i<nels; i++)
      d->rowid.idv.els[i] = -1;

    /*
     *  example: 
     *   row.id.els = {1,3,5}
     *   row.idv.els = {-1,0,-1,1,-1,2}
    */  

    for (i=0; i<d->nrows; i++) {
      k = d->rowid.id.els[i];
      if(k >= nels)
        g_printerr("Invalid value (%d) for id; should be between 0 and %d\n",
          k, nels-1);
      else
        d->rowid.idv.els[k] = i;
    }
  }
}
