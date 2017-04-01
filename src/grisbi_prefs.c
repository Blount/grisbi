/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_dirs.h"
#include "parametres.h"
#include "structures.h"
#include "utils.h"
#include "utils_buttons.h"
#include "utils_gtkbuilder.h"
#include "utils_prefs.h"
#include "prefs/prefs_page_archives.h"
#include "prefs/prefs_page_files.h"
#include "erreur.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/

struct _GrisbiPrefs
{
    GtkDialog dialog;
};

struct _GrisbiPrefsClass
{
    GtkDialogClass parent_class;
};

/* Private structure type */
typedef struct _GrisbiPrefsPrivate GrisbiPrefsPrivate;

struct _GrisbiPrefsPrivate
{
	GtkWidget           *prefs_paned;

    /* panel de gauche */
    GtkWidget *			left_sw;
    GtkWidget *			left_treeview;
    GtkTreeStore *		prefs_tree_model;
	GtkWidget *			togglebutton_expand_prefs;

    /* notebook de droite */
    GtkWidget *         notebook_prefs;
	GtkWidget *			vbox_files;
	GtkWidget *			vbox_archives;

    /* notebook import */
    GtkWidget *          notebook_import;
    GtkWidget *          spinbutton_valeur_echelle_recherche_date_import;
    GtkWidget *          checkbutton_get_fusion_import_transactions;
    GtkWidget *          checkbutton_get_categorie_for_payee;
    GtkWidget *          checkbutton_get_extract_number_for_check;
    GtkWidget *          radiobutton_get_fyear_by_value_date;
    GtkWidget *          treeview_associations;
    GtkWidget *          button_associations_add;
    GtkWidget *          button_associations_remove;
    GtkWidget *          hbox_associations_combo_payees;
    GtkWidget *          combo_associations_payees;
    GtkWidget *          entry_associations_search_str;
};


G_DEFINE_TYPE_WITH_PRIVATE (GrisbiPrefs, grisbi_prefs, GTK_TYPE_DIALOG)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * callback pour la fermeture des preferences
 *
 * \param prefs_dialog
 * \param result_id
 *
 * \return
 **/
static void grisbi_prefs_dialog_response  (GtkDialog *prefs,
										   gint result_id)
{
    if (!prefs)
	{
        return;
	}

	/* on récupère la dimension de la fenêtre */
	gtk_window_get_size (GTK_WINDOW (prefs), &conf.prefs_width, &conf.prefs_height);

	gtk_widget_destroy (GTK_WIDGET (prefs));
}

/**
 * récupère la largeur des préférences
 *
 * \param GtkWidget 		prefs
 * \param GtkAllocation 	allocation
 * \param gpointer 			null
 *
 * \return 					FALSE
 * */
static gboolean grisbi_prefs_size_allocate (GtkWidget *prefs,
											GtkAllocation *allocation,
											gpointer null)
{
    conf.prefs_width = allocation->width;

    return FALSE;
}

/**
 * save prefs hpahed width
 *
 * \param GtkWidget			hpaned
 * \param GtkAllocation 	allocation
 * \param gpointer			NULL
 *
 * \return FALSE
 */
static gboolean grisbi_prefs_paned_size_allocate (GtkWidget *prefs_hpaned,
												   GtkAllocation *allocation,
												   gpointer null)
{
    conf.prefs_panel_width = gtk_paned_get_position (GTK_PANED (prefs_hpaned));

	return FALSE;
}


/* RIGHT PANED */
/**
 * Création de la page de gestion des archives
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_setup_archives_page (GrisbiPrefs *prefs)
{
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

	priv->vbox_archives = GTK_WIDGET (prefs_page_archives_new (prefs));
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->vbox_archives, NULL);
}

/**
 * Création de la page de gestion des fichiers
 *
 * \param prefs
 *
 * \return
 */
static void grisbi_prefs_setup_files_page (GrisbiPrefs *prefs)
{
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

	priv->vbox_files = GTK_WIDGET (prefs_page_files_new (prefs));
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_prefs), priv->vbox_files, NULL);
}

/* LEFT PANED */
 /**
 * remplit le model pour la configuration des états
 *
 * \param GtkTreeStore		model
 * \param GrisbiPrefs		prefs
 *
 * \return
 * */
static void grisbi_prefs_left_panel_populate_tree_model (GtkTreeStore *tree_model,
														 GrisbiPrefs *prefs)
{
    GtkTreeIter iter;
    gint page = 0;

    /* append group page "Main" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Main"), -1);

    /* append page Fichiers */
    grisbi_prefs_setup_files_page (prefs);
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Files"), page);
    page++;

     /* append page Archives */
    grisbi_prefs_setup_archives_page (prefs);
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Archives"), page);
    page++;

     /* append page Import */
    //~ grisbi_prefs_setup_import_page (prefs);
    //~ utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Import"), page);
    //~ page++;

   /* append group page "Display" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Display"), -1);
    page++;

    /* append group page "Transactions" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Transactions"), -1);
    page++;

    /* append group page "Transaction form" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Transaction form"), -1);
    page++;

    /* append group page "Resources" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Resources"), -1);
    page++;

    /* append group page "Balance estimate" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Balance estimate"), -1);
    page++;

    /* append group page "Graphiques" */
    utils_prefs_left_panel_add_line (tree_model, &iter, NULL, NULL, _("Graphs"), -1);
    page++;

    //~ if (grisbi_app_get_active_filename () == NULL)
        //~ grisbi_prefs_sensitive_etat_widgets (prefs, FALSE);
    //~ else
        //~ grisbi_prefs_sensitive_etat_widgets (prefs, TRUE);

    /* return */
}

/**
 * création du tree_view qui liste les onglets de la fenêtre de dialogue
 *
 *
 *\return tree_view or NULL;
 * */
static void grisbi_prefs_left_tree_view_setup (GrisbiPrefs *prefs)
{
    GtkWidget *tree_view = NULL;
    GtkTreeStore *model = NULL;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);

    /* Création du model */
    model = gtk_tree_store_new (LEFT_PANEL_TREE_NUM_COLUMNS,
								G_TYPE_STRING,  				/* LEFT_PANEL_TREE_TEXT_COLUMN */
								G_TYPE_INT,     				/* LEFT_PANEL_TREE_PAGE_COLUMN */
								G_TYPE_INT,     				/* LEFT_PANEL_TREE_BOLD_COLUMN */
								G_TYPE_INT);    				/* LEFT_PANEL_TREE_ITALIC_COLUMN */

    /* Create treeView */
    tree_view = priv->left_treeview;
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));
    g_object_unref (G_OBJECT (model));

    /* set the color of selected row */
    utils_set_tree_view_selection_and_text_color (tree_view);

    /* make column */
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Categories",
													   cell,
													   "text", LEFT_PANEL_TREE_TEXT_COLUMN,
													   "weight", LEFT_PANEL_TREE_BOLD_COLUMN,
													   "style", LEFT_PANEL_TREE_ITALIC_COLUMN,
													   NULL);

    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));

    /* Handle select */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    g_signal_connect (selection,
					  "changed",
					  G_CALLBACK (utils_prefs_left_panel_tree_view_selection_changed),
					  priv->notebook_prefs);

    /* Choose which entries will be selectable */
    gtk_tree_selection_set_select_function (selection,
											utils_prefs_left_panel_tree_view_selectable_func,
											NULL,
											NULL);

    /* expand all rows after the treeview widget has been realized */
    g_signal_connect (tree_view,
					  "realize",
					  G_CALLBACK (utils_tree_view_set_expand_all_and_select_path_realize),
					  "0:0");

    /* remplissage du paned gauche */
    grisbi_prefs_left_panel_populate_tree_model (model, prefs);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void grisbi_prefs_init (GrisbiPrefs *prefs)
{
	GrisbiPrefsPrivate *priv;

	devel_debug (NULL);

	priv = grisbi_prefs_get_instance_private (prefs);
	gtk_widget_init_template (GTK_WIDGET (prefs));

    gtk_dialog_add_buttons (GTK_DIALOG (prefs),
                        GTK_STOCK_CLOSE,
                        GTK_RESPONSE_CLOSE,
                        NULL);

    gtk_window_set_destroy_with_parent (GTK_WINDOW (prefs), TRUE);

    /* set the default size */
    if (conf.prefs_width && conf.prefs_width > 830)
        gtk_window_set_default_size (GTK_WINDOW (prefs), conf.prefs_width, conf.prefs_height);
    else
        gtk_window_set_default_size (GTK_WINDOW (prefs), 830, conf.prefs_height);

	if (conf.prefs_panel_width > 200)
        gtk_paned_set_position (GTK_PANED (priv->prefs_paned), conf.prefs_panel_width);
    else
		gtk_paned_set_position (GTK_PANED (priv->prefs_paned), 200);

	/* initialise left_tree_view */
	grisbi_prefs_left_tree_view_setup (prefs);

	gtk_widget_show_all (GTK_WIDGET (prefs));
}
/**
 * finalise GrisbiPrefs
 *
 * \param object
 *
 * \return
 */
static void grisbi_prefs_finalize (GObject *object)
{
/*     GrisbiPrefs *prefs = GRISBI_PREFS (object);  */

    devel_debug (NULL);

    /* libération de l'objet prefs */
    G_OBJECT_CLASS (grisbi_prefs_parent_class)->finalize (object);
}

/**
 * Initialise GrisbiPrefsClass
 *
 * \param
 *
 * \return
 */
static void grisbi_prefs_class_init (GrisbiPrefsClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grisbi_prefs_finalize;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/grisbi_prefs.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, prefs_paned);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, left_treeview);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, togglebutton_expand_prefs);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), GrisbiPrefs, notebook_prefs);

    /* signaux */
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_prefs_dialog_response);
    gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_prefs_size_allocate);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), grisbi_prefs_paned_size_allocate);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
GrisbiPrefs *grisbi_prefs_new (GrisbiWin *win)
{
  return g_object_new (GRISBI_PREFS_TYPE, "transient-for", win, NULL);
}



//~ /**
 //~ * sensitive a prefs
 //~ *
 //~ * \param object the object wich receive the signal, not used so can be NULL
 //~ * \param widget the widget to sensitive
 //~ *
 //~ * \return FALSE
 //~ * */
//~ static void grisbi_prefs_sensitive_etat_widgets (GrisbiPrefs *prefs,
                        //~ gboolean sensitive)
//~ {
    //~ gtk_widget_set_sensitive (prefs->priv->checkbutton_crypt_file, sensitive);
    //~ gtk_widget_set_sensitive (prefs->priv->notebook_import, sensitive);

//~ }

//~ /* RIGHT_PANEL : CALLBACKS */

//~ /**
 //~ * Set a boolean integer to the value of a checkbutton.  Normally called
 //~ * via a GTK "toggled" signal handler.
 //~ *
 //~ * \param checkbutton a pointer to a checkbutton widget.
 //~ * \param value to change
 //~ */
//~ static void grisbi_prefs_etat_checkbutton_changed (GtkToggleButton *checkbutton,
                        //~ gboolean *value)
//~ {

    //~ if (value)
    //~ {
        //~ grisbi_window_etat_mutex_lock ();
        //~ *value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
        //~ grisbi_window_etat_mutex_unlock ();
    //~ }
//~ }

//~ /* RIGHT_PANEL : ARCHIVES PAGE */
//~ /**
 //~ * Création de la page de gestion des archives
 //~ *
 //~ * \param prefs
 //~ *
 //~ * \return
 //~ */
//~ static void grisbi_prefs_setup_archives_page (GrisbiPrefs *prefs)
//~ {
    //~ GrisbiAppConf *conf;
    //~ GrisbiWindowEtat *etat;

    //~ conf = grisbi_app_get_conf ();
    //~ etat = grisbi_window_get_struct_etat ();

//~ }


//~ /* RIGHT_PANEL : IMPORT PAGE */
//~ /**
 //~ * Création de la page de gestion de l'importation des fichiers
 //~ *
 //~ * \param prefs
 //~ *
 //~ * \return
 //~ */
//~ static void grisbi_prefs_setup_import_page (GrisbiPrefs *prefs)
//~ {
    //~ GrisbiAppConf *conf;
    //~ GrisbiWindowEtat *etat;
    //~ GtkWidget *box;
    //~ GtkWidget *child;
    //~ GdkPixbuf *pixbuf;

    //~ conf = grisbi_app_get_conf ();
    //~ etat = grisbi_window_get_struct_etat ();

    //~ /* set the icon for settings tab */
    //~ box = GTK_WIDGET (gtk_builder_get_object (grisbi_prefs_builder, "hbox_import_settings"));
    //~ child = gtk_image_new_from_file (g_build_filename (gsb_dirs_get_pixmaps_dir (),
                        //~ "import.png", NULL));
    //~ gtk_box_pack_start (GTK_BOX (box), child, FALSE, FALSE, 0);
    //~ gtk_box_reorder_child (GTK_BOX (box), child, 0);
    //~ gtk_widget_show (child);

    //~ /* set the variables for settings tab */
    //~ gtk_spin_button_set_value (GTK_SPIN_BUTTON (prefs->priv->spinbutton_valeur_echelle_recherche_date_import),
                        //~ (gdouble) etat->valeur_echelle_recherche_date_import);
    //~ gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->priv->checkbutton_get_fusion_import_transactions),
                        //~ etat->get_fusion_import_transactions);
    //~ gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->priv->checkbutton_get_categorie_for_payee),
                        //~ etat->get_categorie_for_payee);
    //~ gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prefs->priv->checkbutton_get_extract_number_for_check),
                        //~ etat->get_extract_number_for_check);
    //~ utils_radiobutton_set_active_index (prefs->priv->radiobutton_get_fyear_by_value_date,
                        //~ etat->get_fyear_by_value_date);

    //~ /* Connect signal */
    //~ g_signal_connect (G_OBJECT (prefs->priv->spinbutton_valeur_echelle_recherche_date_import),
                        //~ "value-changed",
                        //~ G_CALLBACK (grisbi_prefs_spinbutton_changed),
                        //~ &etat->valeur_echelle_recherche_date_import);

    //~ g_signal_connect (prefs->priv->checkbutton_get_fusion_import_transactions,
                        //~ "toggled",
                        //~ G_CALLBACK (grisbi_prefs_etat_checkbutton_changed),
                        //~ &etat->get_fusion_import_transactions);

    //~ g_signal_connect (prefs->priv->checkbutton_get_categorie_for_payee,
                        //~ "toggled",
                        //~ G_CALLBACK (grisbi_prefs_etat_checkbutton_changed),
                        //~ &etat->get_categorie_for_payee);

    //~ g_signal_connect (prefs->priv->checkbutton_get_extract_number_for_check,
                        //~ "toggled",
                        //~ G_CALLBACK (grisbi_prefs_etat_checkbutton_changed),
                        //~ &etat->get_extract_number_for_check);

    //~ g_signal_connect (prefs->priv->radiobutton_get_fyear_by_value_date,
                        //~ "toggled",
                        //~ G_CALLBACK (grisbi_prefs_etat_checkbutton_changed),
                        //~ &etat->get_fyear_by_value_date);


    //~ /* set the icon for import_associations tab */
    //~ box = GTK_WIDGET (gtk_builder_get_object (grisbi_prefs_builder, "hbox_import_associations"));
    //~ pixbuf = gdk_pixbuf_new_from_file_at_size (g_build_filename (gsb_dirs_get_pixmaps_dir (),
                        //~ "payees.png", NULL),
                        //~ 24,
                        //~ 24,
                        //~ NULL);
    //~ if (pixbuf)
    //~ {
        //~ child = gtk_image_new_from_pixbuf (pixbuf);
        //~ gtk_box_pack_start (GTK_BOX (box), child, FALSE, FALSE, 0);
        //~ gtk_box_reorder_child (GTK_BOX (box), child, 0);
        //~ gtk_widget_show (child);
    //~ }

    //~ /* set the variables for import_associations tab */
    //~ parametres_import_associations_init_treeview (prefs->priv->treeview_associations);
    //~ prefs->priv->combo_associations_payees = parametres_import_associations_get_combo_payees (etat);
    //~ gtk_box_pack_start (GTK_BOX (prefs->priv->hbox_associations_combo_payees),
                        //~ prefs->priv->combo_associations_payees,
                        //~ TRUE,
                        //~ TRUE,
                        //~ 0);
    //~ parametres_import_associations_init_callback ();
//~ }



//~ /**
 //~ * initialise le bouton expand collapse all
 //~ *
 //~ * \param suffixe name
 //~ * \param tree_view
 //~ *
 //~ * \return
 //~ */
//~ static void grisbi_prefs_left_panel_init_button_expand (GtkWidget *tree_view)
//~ {
    //~ GtkWidget *button;

    //~ button = GTK_WIDGET (gtk_builder_get_object (grisbi_prefs_builder, "togglebutton_expand_prefs"));

    //~ g_object_set_data (G_OBJECT (button), "hbox_expand",
                        //~ gtk_builder_get_object (grisbi_prefs_builder, "hbox_toggle_expand_prefs"));

    //~ g_object_set_data (G_OBJECT (button), "hbox_collapse",
                        //~ gtk_builder_get_object (grisbi_prefs_builder, "hbox_toggle_collapse_prefs"));

    //~ g_signal_connect (G_OBJECT (button),
                        //~ "clicked",
                        //~ G_CALLBACK (utils_togglebutton_collapse_expand_all_rows),
                        //~ tree_view);
//~ }


//~ /* CREATE OBJECT */
//~ /**
 //~ * Initialise GrisbiPrefs
 //~ *
 //~ * \param prefs
 //~ *
 //~ * \return
 //~ */
//~ static void grisbi_prefs_init (GrisbiPrefs *prefs)
//~ {
    //~ GrisbiAppConf *conf;

    //~ devel_debug (NULL);
    //~ conf = grisbi_app_get_conf ();

    //~ prefs->priv = GRISBI_PREFS_GET_PRIVATE (prefs);

    //~ if (!grisbi_prefs_initialise_builder (prefs))
        //~ exit (1);

    //~ /* Attache prefs au gtk_builder permet de retrouver les widgets créés en interne */
    //~ g_object_set_data (G_OBJECT (grisbi_prefs_builder), "prefs", prefs);

    //~ gtk_dialog_add_buttons (GTK_DIALOG (prefs),
                        //~ GTK_STOCK_CLOSE,
                        //~ GTK_RESPONSE_CLOSE,
                        //~ GTK_STOCK_HELP,
                        //~ GTK_RESPONSE_HELP,
                        //~ NULL);

    //~ gtk_window_set_title (GTK_WINDOW (prefs), _("Grisbi preferences"));
    //~ gtk_window_set_destroy_with_parent (GTK_WINDOW (prefs), TRUE);

    //~ gtk_container_set_border_width (GTK_CONTAINER (prefs), 5);
    //~ gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (prefs))), 2);
    //~ gtk_container_set_border_width (GTK_CONTAINER (gtk_dialog_get_action_area (GTK_DIALOG (prefs))), 5);
    //~ gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_action_area (GTK_DIALOG (prefs))), 6);

    //~ /* set the default size */
    //~ if (conf->prefs_width)
        //~ gtk_window_set_default_size (GTK_WINDOW (prefs),
                        //~ conf->prefs_width, conf->prefs_height);

    //~ /* create the tree_view */
    //~ prefs->priv->treeview_left_panel = grisbi_prefs_left_panel_setup_tree_view (prefs);

    //~ /* initialise le bouton expand all */
    //~ grisbi_prefs_left_panel_init_button_expand (prefs->priv->treeview_left_panel);

    //~ /* connect the signals */
    //~ g_signal_connect (G_OBJECT (prefs->priv->hpaned),
                        //~ "size_allocate",
                        //~ G_CALLBACK (grisbi_prefs_hpaned_size_allocate),
                        //~ NULL);

    //~ g_signal_connect (prefs,
                        //~ "response",
                        //~ G_CALLBACK (grisbi_prefs_dialog_response),
                        //~ NULL);

    //~ gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (prefs))),
                        //~ prefs->priv->hpaned, TRUE, TRUE, 0);
    //~ g_object_unref (prefs->priv->hpaned);

    //~ gtk_widget_show_all (prefs->priv->hpaned);

    //~ /* return */
//~ }


//~ /**
 //~ * show the preferences dialog
 //~ *
 //~ * \param parent
 //~ *
 //~ * \return
 //~ **/
//~ void grisb_prefs_show_dialog (GrisbiWindow *parent)
//~ {
    //~ if (!GRISBI_IS_WINDOW (parent))
        //~ return;

    //~ if (grisbi_prefs_dialog == NULL)
    //~ {
        //~ grisbi_prefs_dialog = GTK_WIDGET (g_object_new (GRISBI_TYPE_PREFS, NULL));
        //~ g_signal_connect (grisbi_prefs_dialog,
                        //~ "destroy",
                        //~ G_CALLBACK (gtk_widget_destroyed),
                        //~ &grisbi_prefs_dialog);
    //~ }

    //~ if (GTK_WINDOW (parent) != gtk_window_get_transient_for (GTK_WINDOW (grisbi_prefs_dialog)))
    //~ {
        //~ gtk_window_set_transient_for (GTK_WINDOW (grisbi_prefs_dialog), GTK_WINDOW (parent));
    //~ }

    //~ gtk_window_present (GTK_WINDOW (grisbi_prefs_dialog));
//~ }


//~ /* FONCTIONS UTILITAIRES */
//~ /**
 //~ * retourne le widget demandé
 //~ *
 //~ * \param nom du widget
 //~ *
 //~ * \return le widget demandé
 //~ */
//~ GtkWidget *grisbi_prefs_get_widget_by_name (const gchar *name)
//~ {
    //~ GrisbiPrefs *prefs = g_object_get_data (G_OBJECT (grisbi_prefs_builder), "prefs");

    //~ if (strcmp (name, "combo_associations_payees") == 0)
        //~ return prefs->priv->combo_associations_payees;
    //~ else
        //~ return utils_gtkbuilder_get_widget_by_name (grisbi_prefs_builder, name, NULL);
//~ }


//~ /**
 //~ * remet à jour les préférences suite à chargement fichier
 //~ *
 //~ * \param
 //~ *
 //~ * \return
 //~ **/
//~ void grisbi_prefs_refresh_preferences (gboolean new_file)
//~ {
    //~ GrisbiPrefs *prefs;

    //~ if (grisbi_prefs_dialog == NULL)
        //~ return;

    //~ prefs = g_object_get_data (G_OBJECT (grisbi_prefs_builder), "prefs");

    //~ if (new_file)
        //~ grisbi_prefs_sensitive_etat_widgets (prefs, TRUE);
    //~ else
        //~ grisbi_prefs_sensitive_etat_widgets (prefs, FALSE);

    //~ grisbi_prefs_refresh_files_page (prefs);
    //~ grisbi_prefs_setup_import_page (prefs);
//~ }

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
