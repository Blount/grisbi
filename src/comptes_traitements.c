/* ************************************************************************** */
/* Fichier qui s'occupe des manipulations de comptes                          */
/*                                                                            */
/*                         comptes_traitements.c                              */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (aportal@univ-monpt.fr)	      */
/*			http://www.grisbi.org				      */
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
#include "structures.h"
#include "variables-extern.c"
#include "comptes_traitements.h"
#include "constants.h"



#include "accueil.h"
#include "categories_onglet.h"
#include "comptes_gestion.h"
#include "comptes_onglet.h"
#include "dialog.h"
#include "echeancier_liste.h"
#include "etats_config.h"
#include "fichiers_gestion.h"
#include "imputation_budgetaire.h"
#include "operations_comptes.h"
#include "operations_liste.h"
#include "search_glist.h"
#include "tiers_onglet.h"
#include "traitement_variables.h"
#include "type_operations.h"


extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GSList *gsliste_echeances;
extern struct operation_echeance *echeance_selectionnnee;
extern gint nb_echeances;
gint noname_account_number = 0;


/* ************************************************************************** */
/* Routine appelée lorsque l'on crée un nouveau compte                        */
/* ************************************************************************** */
void  nouveau_compte ( void )
{
    gint type_de_compte;
    gint no_compte;

    if ( !nb_comptes )
    {
	nouveau_fichier ();
	return;
    }

    type_de_compte = demande_type_nouveau_compte ();

    if ( type_de_compte == -1 )
	return;

    no_compte = initialisation_nouveau_compte ( type_de_compte );

    /* si la création s'est mal placée, on se barre */

    if ( no_compte == -1 )
	return;

    /* on crée la liste des opés */

    ajoute_nouvelle_liste_operation( NO_COMPTE );

    /* on recrée les combofix des catégories */

    mise_a_jour_categ();

    /* on met à jour l'option menu des formulaires des échéances et des opés */

    update_options_menus_comptes ();

    /* mise à jour de l'accueil */

    update_liste_comptes_accueil ();

    remplissage_liste_comptes_etats ();
    selectionne_liste_comptes_etat_courant ();

    gtk_widget_set_sensitive ( bouton_supprimer_compte,
			       TRUE );

    /* crée le nouveau bouton du compte et l'ajoute à la liste des comptes */

    reaffiche_liste_comptes ();

    /* on crée le nouveau compte dans les propriétés des comptes */

    compte_courant_onglet = nb_comptes - 1;
    reaffiche_liste_comptes_onglet ();

    /* on se met sur l'onglet de propriétés du compte */

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
			    3 );

    modification_fichier ( TRUE );
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Cette fonction crée un nouveau compte, l'initialise, l'ajoute aux comptes  */
/* et renvoie le no du compte créé                                            */
/* renvoie -1 s'il y a un pb                                                  */
/* ************************************************************************** */
gint initialisation_nouveau_compte ( gint type_de_compte )
{

    if  (!(p_tab_nom_de_compte = realloc ( p_tab_nom_de_compte, ( nb_comptes + 1 )* sizeof ( gpointer ) )))
    {
	dialogue ( _("Cannot allocate memory, bad things will happen soon") );
	return (-1);
    };


    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + nb_comptes;

    if  (!(*p_tab_nom_de_compte_variable = calloc ( 1,
						    sizeof (struct donnees_compte) )) )
    {
	dialogue ( _("Cannot allocate memory, bad things will happen soon") );
	return (-1);
    };

    p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;

    noname_account_number++;

    /* insère ses paramètres ( comme c'est un appel à calloc, tout ce qui est à 0 est déjà initialisé )*/

    NOM_DU_COMPTE = g_strdup_printf ( _("No name %d"), noname_account_number );
    OPERATION_SELECTIONNEE = GINT_TO_POINTER ( -1 );
    DEVISE = 1;
    MISE_A_JOUR = 1;
    NO_COMPTE = nb_comptes;
    AFFICHAGE_R = 0;
    NB_LIGNES_OPE = 3;

    TYPE_DE_COMPTE = type_de_compte;

    nb_comptes++;


    /* on crée les types par défaut */

    creation_types_par_defaut ( NO_COMPTE,
				0);

    /* on met le compte à la fin dans le classement des comptes */

    ordre_comptes = g_slist_append ( ordre_comptes,
				     GINT_TO_POINTER ( NO_COMPTE ) );

    return (NO_COMPTE);
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Fonction affichant une boite de dialogue contenant une liste des comptes   */
/* pour en supprimer un                                                       */
/* ************************************************************************** */
void supprimer_compte ( void )
{
    short actualise = 0, i;
    GSList *pointeur_liste;
    gint compte_modifie;
    gchar *nom_compte_supprime;
    gint page_en_cours;

    compte_modifie = compte_courant_onglet;
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_modifie;

    if ( !question_yes_no_hint ( g_strdup_printf (_("Delete account \"%s\"?"),
						  NOM_DU_COMPTE),
				 _("This will irreversibly remove this account and all operations that were previously contained.  There is no undo for this.") ))
	return;

    /* on commence ici la suppression du compte */

    /* si qu'un compte, on fermer le fichier */

    if ( nb_comptes == 1 )
    {
	etat.modification_fichier = 0;
	fermer_fichier ();
	return;
    }

    /* supprime l'onglet du compte */
    gtk_notebook_remove_page ( GTK_NOTEBOOK ( notebook_listes_operations ),
			       compte_modifie + 1 );

    /*       suppression des échéances */
    while ( ( pointeur_liste = 
	      g_slist_find_custom ( gsliste_echeances,
				    GINT_TO_POINTER ( compte_modifie ),
				    (GCompareFunc)cherche_compte_dans_echeances )))
    {
	if ( echeance_selectionnnee == ECHEANCE_COURANTE )
	{
	    /* 	    met use of cast expressions as lvalues is deprecatedi */
	    /* 		pour ECHEANCE_COURANTE = GINT_TO_POINTER (-1); */
	    /* comprends pas, donc fait les trucs differemment... */

	    pointeur_liste -> data = GINT_TO_POINTER (-1);
	}

	gsliste_echeances = g_slist_remove ( gsliste_echeances, ECHEANCE_COURANTE );
	nb_echeances--;
    }


    /* supprime le compte de la liste de l'ordre des comptes */
    ordre_comptes = g_slist_remove ( ordre_comptes,
				     GINT_TO_POINTER ( compte_modifie ));


    /* modifie les numéros des comptes supérieurs au compte supprimé
       dans l'ordre des comptes */
    pointeur_liste = ordre_comptes;

    do
    {
	if ( GPOINTER_TO_INT ( pointeur_liste -> data ) > compte_modifie )
	    pointeur_liste -> data--;
    }
    while ( ( pointeur_liste = pointeur_liste -> next ) );


    if ( compte_courant == compte_modifie )
    {
	actualise = 1;
	compte_courant = 0;
    }
    else
	if ( compte_courant > compte_modifie )
	{
	    compte_courant--;
	    actualise = 0;
	}


    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_modifie;
    p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;
    //  nb_comptes--;

    g_slist_free ( LISTE_OPERATIONS );
    nom_compte_supprime = g_strdup ( NOM_DU_COMPTE );

    /* on décale en mémoire les comptes situés après */
    for ( i = compte_modifie ; i < nb_comptes ; i++ )
    {
	NO_COMPTE = NO_COMPTE -1;
	*p_tab_nom_de_compte_variable = *(p_tab_nom_de_compte_variable + 1);
	p_tab_nom_de_compte_variable++;
    }

    nb_comptes--;

    /* recherche les échéances pour les comptes plaçés après le compe supprimé */
    /* pour leur diminuer leur numéro de compte de 1 */
    pointeur_liste = gsliste_echeances;

    while ( pointeur_liste )
    {
	if ( ECHEANCE_COURANTE -> compte > compte_modifie )
	    ECHEANCE_COURANTE -> compte--;
	if ( ECHEANCE_COURANTE -> compte_virement > compte_modifie )
	    ECHEANCE_COURANTE -> compte_virement--;

	pointeur_liste = pointeur_liste -> next;
    }

    /*   fait le tour des opés de tous les comptes, */
    /*     pour les opés des comptes > à celui supprimé, on descend le
	   no de compte */
    /*     pour les virements vers le compte supprimé, met
	   relation_no_compte à -1 */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i < nb_comptes ; i++ )
    {
	GSList *pointeur_tmp;

	pointeur_tmp = LISTE_OPERATIONS;

	while ( pointeur_tmp )
	{
	    struct structure_operation *operation;

	    operation = pointeur_tmp -> data;

	    if ( operation -> no_compte > compte_modifie )
		operation -> no_compte--;

	    if ( operation -> relation_no_operation )
	    {
		if ( operation -> relation_no_compte == compte_modifie )
		{
		    operation -> relation_no_compte = -1;
		    MISE_A_JOUR = 1;
		}
		else
		    if ( operation -> relation_no_compte > compte_modifie )
			operation -> relation_no_compte--;
	    }
	    pointeur_tmp = pointeur_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }

    /* le compte courant de l'onglet de comptes est diminué de 1 ou reste */
    /* à 0 s'il l'était */

    if ( compte_courant_onglet )
	compte_courant_onglet--;

    /* retire le bouton du compte dans la liste des comptes */
    /*   pour cela, on efface vbox_liste_comptes et on le recrée */

    reaffiche_liste_comptes();
    reaffiche_liste_comptes_onglet ();

    /* on recrée les combofix des tiers et des catégories */

    mise_a_jour_tiers();
    mise_a_jour_categ();
    mise_a_jour_imputation();

    /* on met à jour l'option menu du formulaire des échéances */

    update_options_menus_comptes ();

    /* réaffiche la liste si necessaire */

    page_en_cours = gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook_general));

    changement_compte ( GINT_TO_POINTER ( compte_courant ));

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), page_en_cours );

    remplissage_liste_echeance ();
    update_liste_echeances_manuelles_accueil();
    update_liste_comptes_accueil ();
    mise_a_jour_soldes_minimaux ();
    mise_a_jour_fin_comptes_passifs();

    remplissage_liste_comptes_etats ();
    selectionne_liste_comptes_etat_courant ();

    modification_fichier( TRUE ); 

}
/* ************************************************************************** */

/**
 *  Create an option menu with the list of accounts.  This list is
 *  clickable and activates func if specified.
 *
 * \param func Function to call when a line is selected
 * \param activate_currrent If set to TRUE, does not mark as
 *        unsensitive current account
 *
 * \return A newly created option menu
 */
GtkWidget * creation_option_menu_comptes ( GtkSignalFunc func, 
					   gboolean activate_currrent )
{
    GtkWidget *menu;
    GtkWidget *item;
    GSList *ordre_comptes_variable;

    menu = gtk_menu_new ();

    ordre_comptes_variable = ordre_comptes;

    do
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable -> data );

	item = gtk_menu_item_new_with_label ( NOM_DU_COMPTE );
	gtk_object_set_data ( GTK_OBJECT ( item ),
			      "no_compte",
			      GINT_TO_POINTER ( p_tab_nom_de_compte_variable - p_tab_nom_de_compte ));
	if ( func )
	    gtk_signal_connect ( GTK_OBJECT ( item ), "activate", GTK_SIGNAL_FUNC(func), NULL );
	gtk_menu_append ( GTK_MENU ( menu ), item );

	if ( !activate_currrent && 
	     p_tab_nom_de_compte_courant == p_tab_nom_de_compte_variable )
	{
	    gtk_widget_set_sensitive ( item, FALSE );
	}      

	gtk_widget_show ( item );
    }
    while ( (  ordre_comptes_variable = ordre_comptes_variable -> next ) );

    return ( menu );
}
/* ************************************************************************** */

/**
 *  Create an option menu with the list of unclosed accounts.  This list is
 *  clickable and activates func if specified.
 *
 * \param func Function to call when a line is selected
 * \param activate_currrent If set to TRUE, does not mark as
 *        unsensitive current account
 *
 * \return A newly created option menu
 */
GtkWidget * creation_option_menu_comptes_nonclos ( GtkSignalFunc func, 
						   gboolean activate_currrent )
{
    GtkWidget *menu;
    GtkWidget *item;
    GSList *ordre_comptes_variable;

    menu = gtk_menu_new ();

    ordre_comptes_variable = ordre_comptes;

    do
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable -> data );
	
	if ( !COMPTE_CLOTURE )
	{
	    item = gtk_menu_item_new_with_label ( NOM_DU_COMPTE );
	    gtk_object_set_data ( GTK_OBJECT ( item ),
				  "no_compte",
				  GINT_TO_POINTER ( p_tab_nom_de_compte_variable - p_tab_nom_de_compte ));
	    if ( func )
		gtk_signal_connect ( GTK_OBJECT ( item ), "activate", GTK_SIGNAL_FUNC(func), NULL );
	    gtk_menu_append ( GTK_MENU ( menu ), item );

	    if ( !activate_currrent && 
		 p_tab_nom_de_compte_courant == p_tab_nom_de_compte_variable )
	    {
		gtk_widget_set_sensitive ( item, FALSE );
	    }      

	    gtk_widget_show ( item );
	}
    }
    while ( (  ordre_comptes_variable = ordre_comptes_variable -> next ) );

    return ( menu );
}
/* ************************************************************************** */

/* ************************************************************************** */
void changement_choix_compte_echeancier ( GtkWidget *item )
{
    GtkWidget *menu;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( item ),
												 "no_compte" ));

    if ( gtk_widget_get_style ( widget_formulaire_echeancier[SCHEDULER_FORM_CREDIT] ) == style_entree_formulaire[ENCLAIR] )
    {
	/*       il y a qque chose dans le crédit, on met le menu des types crédit */

	if ( (menu = creation_menu_types ( 2,
					   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( item),
										   "no_compte" )),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					  cherche_no_menu_type_echeancier ( TYPE_DEFAUT_CREDIT ) );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }
    else
    {
	/*       il y a qque chose dans le débit ou c'est par défaut, on met le menu des types débit */

	if ( (menu = creation_menu_types ( 1,
					   GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT (item ),
										   "no_compte" )),
					   1 )))
	{
	    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
				       menu );
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] ),
					  cherche_no_menu_type_echeancier ( TYPE_DEFAUT_DEBIT ) );
	    gtk_widget_show ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
	}
	else
	    gtk_widget_hide ( widget_formulaire_echeancier[SCHEDULER_FORM_TYPE] );
    }
}
/* ************************************************************************** */

/* ************************************************************************** */
void creation_types_par_defaut ( gint no_compte,
				 gulong dernier_cheque )
{
    gpointer **save_p_tab;

    save_p_tab = p_tab_nom_de_compte_variable;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;


    /* si des types d'opé existaient déjà, on les vire */

    if ( TYPES_OPES )
	g_slist_free ( TYPES_OPES );

    TYPES_OPES = NULL;
    TYPE_DEFAUT_DEBIT = 0;
    TYPE_DEFAUT_CREDIT = 0;

    if ( !TYPE_DE_COMPTE )
    {
	/* c'est un compte bancaire, on ajoute virement, prélèvement, chèque et cb */
	/* 	  modification par rapport à avant, les nouveaux n°: */
	/* 	    1=virement, 2=dépot, 3=cb, 4=prélèvement, 5=chèque */
	/* les modifs pour chaque opés se feront à leur chargement */

	struct struct_type_ope *type_ope;

	type_ope = malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 1;
	type_ope -> nom_type = g_strdup ( _("Transfer") );
	type_ope -> signe_type = 0;
	type_ope -> affiche_entree = 1;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	TYPES_OPES = g_slist_append ( TYPES_OPES,
				      type_ope );

	type_ope = malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 2;
	type_ope -> nom_type = g_strdup ( _("Deposit") );
	type_ope -> signe_type = 2;
	type_ope -> affiche_entree = 0;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	TYPES_OPES = g_slist_append ( TYPES_OPES,
				      type_ope );

	type_ope = malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 3;
	type_ope -> nom_type = g_strdup ( _("Credit card") );
	type_ope -> signe_type = 1;
	type_ope -> affiche_entree = 0;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	TYPES_OPES = g_slist_append ( TYPES_OPES,
				      type_ope );

	type_ope = malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 4;
	type_ope -> nom_type = g_strdup ( _("Direct debit") );
	type_ope -> signe_type = 1;
	type_ope -> affiche_entree = 0;
	type_ope -> numerotation_auto = 0;
	type_ope -> no_en_cours = 0;
	type_ope -> no_compte = no_compte;

	TYPES_OPES = g_slist_append ( TYPES_OPES,
				      type_ope );

	type_ope = malloc ( sizeof ( struct struct_type_ope ));
	type_ope -> no_type = 5;
	type_ope -> nom_type = g_strdup ( _("Cheque") );
	type_ope -> signe_type = 1;
	type_ope -> affiche_entree = 1;
	type_ope -> numerotation_auto = 1;
	type_ope -> no_en_cours = dernier_cheque;
	type_ope -> no_compte = no_compte;

	TYPES_OPES = g_slist_append ( TYPES_OPES,
				      type_ope );


	TYPE_DEFAUT_DEBIT = 3;
	TYPE_DEFAUT_CREDIT = 2;

	/* on crée le tri pour compte bancaire qui sera 1 2 3 4 5 */

	LISTE_TRI = g_slist_append ( LISTE_TRI,
				     GINT_TO_POINTER ( 1 ));
	LISTE_TRI = g_slist_append ( LISTE_TRI,
				     GINT_TO_POINTER ( 2 ));
	LISTE_TRI = g_slist_append ( LISTE_TRI,
				     GINT_TO_POINTER ( 3 ));
	LISTE_TRI = g_slist_append ( LISTE_TRI,
				     GINT_TO_POINTER ( 4 ));
	LISTE_TRI = g_slist_append ( LISTE_TRI,
				     GINT_TO_POINTER ( 5 ));
    }
    else
    {
	if ( TYPE_DE_COMPTE == 2 )
	{
	    /* c'est un compte de passif, on ne met que le virement */

	    struct struct_type_ope *type_ope;

	    type_ope = malloc ( sizeof ( struct struct_type_ope ));
	    type_ope -> no_type = 1;
	    type_ope -> nom_type = g_strdup ( _("Transfer") );
	    type_ope -> signe_type = 0;
	    type_ope -> affiche_entree = 1;
	    type_ope -> numerotation_auto = 0;
	    type_ope -> no_en_cours = 0;
	    type_ope -> no_compte = no_compte;

	    TYPES_OPES = g_slist_append ( TYPES_OPES,
					  type_ope );

	    TYPE_DEFAUT_DEBIT = 1;
	    TYPE_DEFAUT_CREDIT = 1;

	    /* on crée le tri pour compte passif qui sera 1 */

	    LISTE_TRI = g_slist_append ( LISTE_TRI,
					 GINT_TO_POINTER ( 1 ));
	}
    }

    p_tab_nom_de_compte_variable = save_p_tab;
}
/* ************************************************************************** */

/* ************************************************************************** */
/* Cette fonction est appelée lors de la création d'un nouveau compte.        */
/* elle renvoie le type demandé pour pouvoir mettre ensuite les types par     */
/* défaut.                                                                    */
/* ************************************************************************** */
gint demande_type_nouveau_compte ( void )
{
    GtkWidget *dialog;
    gint resultat;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *bouton;
    gint type_compte;

    dialog = dialogue_special_no_run ( GTK_MESSAGE_QUESTION,
				       GTK_BUTTONS_OK_CANCEL,
				       make_hint ( _("Choose account type"),
						   _("If you choose to continue, an account will be created with default payment methods chosen according to your choice.\nYou will be able to change account type later." ) ) );

    /* création de la ligne du type de compte */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), hbox,
			 FALSE, FALSE, 6 );
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Account type")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 12 );
    gtk_widget_show ( label );

    bouton = gtk_option_menu_new ();
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( bouton ),
			       creation_menu_type_compte() );
    gtk_box_pack_start ( GTK_BOX (hbox), bouton, TRUE, TRUE, 12 );
    gtk_widget_show ( bouton );

    resultat = gtk_dialog_run ( GTK_DIALOG(dialog) );

    if ( resultat != GTK_RESPONSE_OK )
    {
	gtk_widget_destroy ( dialog );
	return ( -1 );
    }

    type_compte = GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( bouton ) -> menu_item ),
							  "no_type_compte" ));

    gtk_widget_destroy ( dialog );

    return ( type_compte );
}
/* ************************************************************************** */



/* ************************************************************************** */
/* cette fonction est appelée pour mettre un option menu de compte sur le */
/* compte donné en argument */
/* elle renvoie le no à mettre dans history */
/* ************************************************************************** */
gint recherche_compte_dans_option_menu ( GtkWidget *option_menu,
					 gint no_compte )
{
    GList *liste_menu;
    GList *liste_tmp;

    liste_menu = GTK_MENU_SHELL ( gtk_option_menu_get_menu ( GTK_OPTION_MENU ( option_menu ))) -> children;
    liste_tmp = liste_menu;

    while ( liste_tmp )
    {
	gint *no;

	no = gtk_object_get_data ( GTK_OBJECT ( liste_tmp -> data ),
				   "no_compte" );
	if ( GPOINTER_TO_INT (no) == no_compte )
	    return g_list_position ( liste_menu,
				     liste_tmp );
	liste_tmp = liste_tmp -> next;
    }
    return 0;
}
/* ************************************************************************** */


/* ************************************************************************** */
/* il y a eu un chgt dans les comptes, cette fonction modifie les */
/* options menus qui contiennent les noms de compte */
/* ************************************************************************** */

void update_options_menus_comptes ( void )
{
    /*     on met à jour l'option menu de l'échéancier */

    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( widget_formulaire_echeancier[SCHEDULER_FORM_ACCOUNT] ),
			       creation_option_menu_comptes_nonclos (GTK_SIGNAL_FUNC(changement_choix_compte_echeancier),
								     TRUE) );

}
/* ************************************************************************** */

