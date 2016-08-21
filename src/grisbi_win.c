/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2016 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

/*START_INCLUDE*/
#include "grisbi_win.h"
#include "grisbi_app.h"
#include "accueil.h"
#include "gsb_data_account.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

#define GSB_NBRE_CHAR 15
#define GSB_NAMEFILE_TOO_LONG 45

/*START_EXTERN Variables externes PROVISOIRE*/

/* declared in parametres.c PROVISOIRE*/
extern struct gsb_etat_t etat;

/* declared in main.c PROVISOIRE*/
extern struct gsb_run_t run;
extern gchar *nom_fichier_comptes;
extern gchar *titre_fichier;

/*END_EXTERN*/

struct _GrisbiWin
{
  GtkApplicationWindow parent;
};

struct _GrisbiWinClass
{
  GtkApplicationWindowClass parent_class;
};

typedef struct _GrisbiWinPrivate GrisbiWinPrivate;

struct _GrisbiWinPrivate
{
	GtkBuilder 			*builder;

    /* box principale */
    GtkWidget           *main_box;

    /* page d'accueil affichée si pas de fichier chargé automatiquement */
    GtkWidget           *accueil_page;

    /* widget général si un fichier est chargé */
    GtkWidget           *vbox_general;
    GtkWidget           *hpaned_general;

    /* nom du fichier associé à la fenêtre */
    gchar               *filename;

    /* titre du fichier */
    gchar               *file_title;

    /* titre de la fenêtre */
    gchar               *window_title;

    /* Menus et barres d'outils */
    /* menu move-to-acc */
    GMenu               *menu;
    gboolean            init_move_to_acc;

    /* statusbar */
    GtkWidget           *statusbar;
    guint               context_id;
    guint               message_id;

    /* headings_bar */
    GtkWidget           *headings_eb;
	GtkWidget			*arrow_eb_left;
	GtkWidget			*arrow_eb_right;


    /* variables de configuration de la fenêtre */
/*	GrisbiWinEtat		*etat;
*/
    /* structure run */
/*    GrisbiWindowRun     *run;
*/
};

G_DEFINE_TYPE_WITH_PRIVATE(GrisbiWin, grisbi_win, GTK_TYPE_APPLICATION_WINDOW);

/* STATUS_BAR */
/**
 * initiate the GtkStatusBar to hold various status
 * information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_init_statusbar ( GrisbiWin *win )
{
    GtkWidget *statusbar;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    priv->context_id = gtk_statusbar_get_context_id ( GTK_STATUSBAR ( priv->statusbar ), "Grisbi" );
    priv->message_id = -1;
}


/* HEADINGS_EB */
/**
 * Trigger a callback functions only if button event that triggered it
 * was a simple click.
 *
 * \param button
 * \param event
 * \param callback function
 *
 * \return  TRUE.
 */
static gboolean grisbi_win_headings_simpleclick_event_run ( GtkWidget *button,
                        GdkEvent *button_event,
                        GCallback callback )
{
    if ( button_event -> type == GDK_BUTTON_PRESS )
    {
        callback ( );
    }

    return TRUE;
}

/**
 * initiate the headings_bar information.
 *
 * \param GrisbiWin *win
 *
 * \return
 */
static void grisbi_win_init_headings_eb ( GrisbiWin *win )
{
    GtkStyleContext *style;
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    style = gtk_widget_get_style_context ( priv->headings_eb );

	gtk_widget_override_background_color ( priv->arrow_eb_left, GTK_STATE_FLAG_ACTIVE, NULL );
    g_signal_connect ( G_OBJECT ( priv->arrow_eb_left ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_win_headings_simpleclick_event_run ),
                        gsb_gui_navigation_select_prev );

	gtk_widget_override_background_color ( priv->arrow_eb_right, GTK_STATE_FLAG_ACTIVE, NULL );
    g_signal_connect ( G_OBJECT ( priv->arrow_eb_right ),
                        "button-press-event",
                        G_CALLBACK ( grisbi_win_headings_simpleclick_event_run ),
                        gsb_gui_navigation_select_prev );

    gtk_widget_override_background_color ( priv->headings_eb, GTK_STATE_FLAG_ACTIVE, NULL );
}


/* WIN STATE */
/**
 * check on any change on the main window
 * for now, only to check if we set/unset the full-screen
 *
 * \param window
 * \param event
 * \param null
 *
 * \return FALSE
 * */
static gboolean grisbi_win_change_state_window ( GtkWidget *window,
                        GdkEventWindowState *event,
                        gpointer null )
{
    gboolean show;

    if ( event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED )
    {
        show = !( event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED );

/*        gtk_window_set_has_resize_grip ( GTK_WINDOW ( window ), show );
*/        conf.maximize_screen = !show;
    }

    /* return value */
    return FALSE;
}

/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_finalize ( GObject *object )
{
    GrisbiWinPrivate *priv;

    printf ("grisbi_win_finalize\n");
    devel_debug (NULL);
    priv = grisbi_win_get_instance_private ( GRISBI_WIN ( object ) );

    g_free ( priv->filename );
    priv->filename = NULL;

    g_free ( priv->file_title );
    priv->file_title = NULL;

    g_free ( priv->window_title );
    priv->window_title = NULL;

    G_OBJECT_CLASS (grisbi_win_parent_class)->finalize ( object );
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_dispose ( GObject *object )
{
    GrisbiWin *win = GRISBI_WIN ( object );
    GrisbiWinPrivate *priv;

    printf ("grisbi_win_dispose\n");
    priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    g_clear_object ( &priv->builder );
    g_clear_object ( &priv->menu );

    G_OBJECT_CLASS (grisbi_win_parent_class)->dispose ( object );
}

/**
 *
 *
 * \param GrisbiWin *win
 *
 * \return
 **/
static void grisbi_win_init ( GrisbiWin *win )
{
	GrisbiWinPrivate *priv;
	GtkWidget *statusbar;
    GtkWidget *headings_eb;

	printf ("grisbi_win_init\n");
	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    priv->filename = NULL;
    priv->file_title = NULL;
    priv->window_title = NULL;

	gtk_widget_init_template ( GTK_WIDGET ( win ) );

	/* initialisation de la barre d'état */
/*	grisbi_win_init_statusbar ( GRISBI_WIN ( win ) );
*/
	/* initialisation de headings_eb */
/*	grisbi_win_init_headings_eb ( GRISBI_WIN ( win ) );
*/
	run.window = GTK_WIDGET ( win );
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void grisbi_win_class_init ( GrisbiWinClass *klass )
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->dispose = grisbi_win_dispose;
    object_class->finalize = grisbi_win_finalize;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
                                               "/org/gtk/grisbi/ui/grisbi_win.ui");

	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( klass ), GrisbiWin, main_box );
/*	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( klass ), GrisbiWin, headings_eb );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( klass ), GrisbiWin, arrow_eb_left );
	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( klass ), GrisbiWin, arrow_eb_right );
*/	gtk_widget_class_bind_template_child_private ( GTK_WIDGET_CLASS ( klass ), GrisbiWin, statusbar );

    /* signaux */
    gtk_widget_class_bind_template_callback ( GTK_WIDGET_CLASS ( klass ), grisbi_win_change_state_window );
}

/*******************************************************************************
 * Public Methods
 ******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *grisbi_win_get_filename ( GrisbiWin *win )
{
	GrisbiWinPrivate *priv;

	if ( win == NULL )
		win = grisbi_app_get_active_window ( NULL );

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

	return priv->filename;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_set_filename ( GrisbiWin *win,
						const gchar *filename )
{
	GrisbiWinPrivate *priv;

	if ( !win )
        win = grisbi_app_get_active_window ( NULL );

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );
	priv->filename = g_strdup ( filename );
}

/**
 * retourne main_box
 *
 * \param GrisbiWin *win
 *
 * \return main_box
 **/
GtkWidget *grisbi_win_get_main_box ( GrisbiWin *win )
{
	GrisbiWinPrivate *priv;

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );
    return priv->main_box;
}

/* WIN_MENU */
/**
 *
 *
 * \param GrisbiWin 	win
 * \param GMenuModel  	menubar
 *
 * \return
 **/
void grisbi_win_init_menubar ( GrisbiWin *win,
						gpointer app )
{
	GrisbiWinPrivate *priv;
	GAction *action;
    gchar * items[] = {
        "save",
        "save-as",
        "export-accounts",
        "create-archive",
        "export-archive",
        "debug-acc-file",
        "obf-acc-file",
        "debug-mode",
        "file-close",
        "edit-ope",
        "new-ope",
        "remove-ope",
        "template-ope",
        "clone-ope",
        "convert-ope",
        "new-acc",
        "remove-acc",
        "show-form",
        "show-reconciled",
        "show-archived",
        "show-closed-acc",
        "show-ope",
        "reset-width-col",
        NULL
    };
    gchar **tmp = items;

	printf ("grisbi_win_init_menubar\n");
	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

	/* initialisations sub menus */
	action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "show-form");
    g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( conf.formulaire_toujours_affiche ) );
	action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "show-closed-acc");
    g_action_change_state ( G_ACTION ( action ), g_variant_new_boolean ( conf.show_closed_accounts ) );

	/* disabled menus */
    while ( *tmp )
    {
        gsb_menu_gui_sensitive_win_menu_item ( *tmp, FALSE );

        tmp++;
    }

    /* sensibilise le menu preferences */
    action = grisbi_app_get_prefs_action ();
    g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), FALSE );
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_delete ( void )
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;
    GMenu *menu;
    GSList *tmp_list;

    printf ("grisbi_win_menu_move_to_acc_delete\n");

    win = grisbi_app_get_active_window ( NULL );
    priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    if ( priv->init_move_to_acc == FALSE )
        return;

    tmp_list = gsb_data_account_get_list_accounts ();
    while ( tmp_list )
    {
        gint i;

        i = gsb_data_account_get_no_account ( tmp_list -> data );

        if ( !gsb_data_account_get_closed_account ( i ) )
        {
            gchar *tmp_name;

            tmp_name = g_strdup_printf ( "move-to-acc%d", i );
            g_action_map_remove_action ( G_ACTION_MAP ( win ), tmp_name );

            g_free ( tmp_name );
        }
        tmp_list = tmp_list -> next;
    }

    menu = grisbi_app_get_menu_edit ();

    g_menu_remove ( menu, 3 );
    priv->init_move_to_acc = FALSE;
}

/**
 *
 *
 * \param GrisbiWin 	win
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_new ( void )
{
    GrisbiWin *win;
    GrisbiWinPrivate *priv;
    GAction *action;
    GMenu *menu;
    GMenu *submenu;
    GMenuItem *menu_item;
    GSList *tmp_list;
    gchar *label;
    gchar *action_name;
    printf ("grisbi_win_menu_move_to_acc_new\n");

    win = grisbi_app_get_active_window ( NULL );
    priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    menu = grisbi_app_get_menu_edit ();

    submenu = g_menu_new ();

    tmp_list = gsb_data_account_get_list_accounts ();
    while ( tmp_list )
    {
        gint i;

        i = gsb_data_account_get_no_account ( tmp_list -> data );

        if ( !gsb_data_account_get_closed_account ( i ) )
        {
            gchar *tmp_name;
            gchar *account_name;
            gchar *action_name;

            tmp_name = g_strdup_printf ( "move-to-acc%d", i );
            account_name = gsb_data_account_get_name ( i );
            if ( !account_name )
                account_name = _("Unnamed account");

            action = (GAction *) g_simple_action_new ( tmp_name, NULL );
            g_signal_connect ( action,
                        "activate",
                        G_CALLBACK ( grisbi_cmd_move_to_account_menu ),
                        GINT_TO_POINTER ( i ) );
            g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), FALSE );

            g_action_map_add_action ( G_ACTION_MAP ( grisbi_app_get_active_window ( NULL ) ), action );
            g_object_unref ( G_OBJECT ( action ) );

            action_name = g_strconcat ("win.", tmp_name, NULL);
            g_menu_append ( submenu, account_name, action_name );

            g_free ( tmp_name );
            g_free ( action_name );
        }

        tmp_list = tmp_list -> next;
    }

    menu_item = g_menu_item_new_submenu ( _("Move transaction to another account"), (GMenuModel*) submenu );
    g_menu_item_set_detailed_action ( menu_item, "win.move-to-acc" );
    action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), "move-to-acc" );
    g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), FALSE );

    g_menu_insert_item ( G_MENU ( menu ), 3, menu_item );
    g_object_unref ( menu_item );
    g_object_unref ( submenu );
    priv->init_move_to_acc = TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void grisbi_win_menu_move_to_acc_update ( gboolean active )
{
    GrisbiWin *win;
    GAction *action;
    GSList *tmp_list;
    gint current_account;
    static gboolean flag_active = FALSE;

    printf ("grisbi_win_menu_move_to_acc_update : active = %d\n", active);

    if ( flag_active == active )
        return;

    win = grisbi_app_get_active_window ( NULL );

    tmp_list = gsb_data_account_get_list_accounts ();
    while ( tmp_list )
    {
        gint i;

        i = gsb_data_account_get_no_account ( tmp_list -> data );

        if ( !gsb_data_account_get_closed_account ( i ) )
        {
            gchar *tmp_name;

            tmp_name = g_strdup_printf ( "move-to-acc%d", i );
            action = g_action_map_lookup_action ( G_ACTION_MAP ( win ), tmp_name );

            if ( gsb_gui_navigation_get_current_account () == i )
            {
                g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), FALSE );
                tmp_list = tmp_list -> next;
                continue;
            }
            if ( active )
                g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), TRUE );
            else
                g_simple_action_set_enabled ( G_SIMPLE_ACTION ( action ), FALSE );

            g_free ( tmp_name );
        }
        tmp_list = tmp_list -> next;
    }
    flag_active = active;
}

/* MAIN WINDOW */
/**
 * set the title of the window
 *
 * \param gint 		account_number
 *
 * \return			TRUE if OK, FALSE otherwise
 * */
gboolean grisbi_win_set_grisbi_title ( gint account_number )
{
    gchar *titre_grisbi = NULL;
    gchar *titre = NULL;
    gint tmp_number;
    gboolean return_value;

    if ( nom_fichier_comptes == NULL )
    {
        titre_grisbi = g_strdup ( _("Grisbi") );
        return_value = TRUE;
    }
    else
    {
        switch ( conf.display_grisbi_title )
        {
            case GSB_ACCOUNTS_TITLE:
                if ( titre_fichier && strlen ( titre_fichier ) )
                    titre = g_strdup ( titre_fichier );
            break;
            case GSB_ACCOUNT_HOLDER:
            {
                if ( account_number == -1 )
                    tmp_number = gsb_data_account_first_number ();
                else
                    tmp_number = account_number;

                if ( tmp_number == -1 )
                {
                    if ( titre_fichier && strlen ( titre_fichier ) )
                        titre = g_strdup ( titre_fichier );
                }
                else
                {
                    titre = g_strdup ( gsb_data_account_get_holder_name ( tmp_number ) );

                    if ( titre == NULL )
                        titre = g_strdup ( gsb_data_account_get_name ( tmp_number ) );
                }
            break;
            }
            case GSB_ACCOUNTS_FILE:
                if ( nom_fichier_comptes && strlen ( nom_fichier_comptes ) )
                    titre = g_path_get_basename ( nom_fichier_comptes );
            break;
        }

        if ( titre && strlen ( titre ) > 0 )
        {
            titre_grisbi = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
            g_free ( titre );

            return_value = TRUE;
        }
        else
        {
            titre_grisbi = g_strconcat ( "<", _("unnamed"), ">", NULL );
            return_value = FALSE;
        }
    }
    gtk_window_set_title ( GTK_WINDOW ( run.window ), titre_grisbi );

    if ( titre_grisbi && strlen ( titre_grisbi ) > 0 )
    {
        gsb_main_page_update_homepage_title ( titre_grisbi );
        g_free ( titre_grisbi );
    }

    return return_value;
}

/**
 * set size and position of the main window of grisbi.
 *
 * \param GrisbiWin *win
 *
 * \return
 * */
void grisbi_win_set_size_and_position ( GtkWindow *win )
{
	GrisbiWinPrivate *priv;

	printf ("grisbi_win_set_size_and_position\n");

	priv = grisbi_win_get_instance_private ( GRISBI_WIN ( win ) );

    /* set the size of the window */
    if ( conf.main_width && conf.main_height )
        gtk_window_set_default_size ( GTK_WINDOW ( win ), conf.main_width, conf.main_height );
    else
        gtk_window_set_default_size ( GTK_WINDOW ( win ), 900, 600 );

    /* display window at position */
    gtk_window_move ( GTK_WINDOW ( win ), conf.x_position, conf.y_position );

    /* set the full screen if necessary */
    if ( conf.full_screen )
        gtk_window_fullscreen ( GTK_WINDOW ( win ) );

    /* put up the screen if necessary */
    if ( conf.maximize_screen )
        gtk_window_maximize ( GTK_WINDOW ( win ) );
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */