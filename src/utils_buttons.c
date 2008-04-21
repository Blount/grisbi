/* ************************************************************************** */
/*                                  utils_buttons.c			      */
/*                                                                            */
/*     Copyright (C)	2000-2003 C�dric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#include "include.h"

/*START_INCLUDE*/
#include "utils_buttons.h"
#include "./utils.h"
#include "./utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *window ;
/*END_EXTERN*/




/* **************************************************************************************************************************** */
GtkWidget *cree_bouton_url ( const gchar *adr,
			     const gchar *inscription )
{
    GtkWidget *bouton;

    bouton = gtk_button_new_with_mnemonic ( inscription );
    gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			    GTK_RELIEF_NONE );
    g_signal_connect_data ( G_OBJECT ( bouton ),
			    "clicked",
			    G_CALLBACK ( lance_navigateur_web ),
			    my_strdup ( adr),
			    NULL,
			    G_CONNECT_SWAPPED );
    return ( bouton );

}
/* **************************************************************************************************************************** */




GtkWidget * new_stock_image_label ( GsbButtonStyle style, const gchar * stock_id, const gchar * name )
{
    GtkWidget * vbox, * label, * image;

    vbox = gtk_vbox_new ( FALSE, 0 );

    /* Define image */
    if ( style == GSB_BUTTON_ICON || style == GSB_BUTTON_BOTH )
    {
	image = gtk_image_new_from_stock ( stock_id, GTK_ICON_SIZE_LARGE_TOOLBAR );
	gtk_box_pack_start ( GTK_BOX(vbox), image, TRUE, TRUE, 0 );
    }

    /* Define label */
    if ( style == GSB_BUTTON_TEXT || style == GSB_BUTTON_BOTH )
    {
	label = gtk_label_new ( name );
	gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_CENTER );
	gtk_box_pack_start ( GTK_BOX(vbox), label, TRUE, TRUE, 0 );
    }
    
    return vbox;
}




GtkWidget * new_image_label ( GsbButtonStyle style, const gchar * image_name, const gchar * name )
{
    GtkWidget * vbox, * label, * image;

    vbox = gtk_vbox_new ( FALSE, 0 );

    /* Define image */
    if ( style == GSB_BUTTON_ICON || style == GSB_BUTTON_BOTH )
    {
	image = gtk_image_new_from_file (g_build_filename (PIXMAPS_DIR,
							   image_name, NULL));
	gtk_box_pack_start ( GTK_BOX(vbox), image, TRUE, TRUE, 0 );
    }

    /* Define label */
    if ( style == GSB_BUTTON_TEXT || style == GSB_BUTTON_BOTH )
    {
	label = gtk_label_new ( name );
	gtk_label_set_text_with_mnemonic ( GTK_LABEL(label), name );
	gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );
	gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_CENTER );
	gtk_box_pack_start ( GTK_BOX(vbox), label, TRUE, TRUE, 0 );
    }

    gtk_widget_show_all ( vbox );

    return vbox;
}



/**
 * TODO: document
 * Borrowed from the Gimp Toolkit and modified.
 *
 */
void set_popup_position (GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
  GtkWidget *widget;
  GtkRequisition requisition;
  gint screen_width, menu_xpos, menu_ypos, menu_width;

  widget = GTK_WIDGET (user_data);

  gtk_widget_get_child_requisition (GTK_WIDGET (menu), &requisition);
  menu_width = requisition.width;

  gdk_window_get_origin (widget->window, &menu_xpos, &menu_ypos);

  menu_xpos += widget->allocation.x;
  menu_ypos += widget->allocation.y + widget->allocation.height - 2;

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    menu_xpos = menu_xpos + widget->allocation.width - menu_width;

  /* Clamp the position on screen */
  screen_width = gdk_screen_get_width (gtk_widget_get_screen (widget));
  
  if (menu_xpos < 0)
    menu_xpos = 0;
  else if ((menu_xpos + menu_width) > screen_width)
    menu_xpos -= ((menu_xpos + menu_width) - screen_width);

  *x = menu_xpos;
  *y = menu_ypos;
  *push_in = TRUE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
