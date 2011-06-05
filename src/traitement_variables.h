#ifndef _TRAITEMENT_VARIABLES_H
#define _TRAITEMENT_VARIABLES_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/*START_DECLARATION*/
void free_variables ( void );
void init_variables ( void );
void initialisation_couleurs_listes ( void );
void initialise_largeur_colonnes_tab_affichage_ope ( gint type_operation, const gchar *description );
gchar *gsb_variables_get_titre_colonne_liste_ope ( gint element );
void menus_sensitifs ( gboolean sensitif );
void menus_view_sensitifs ( gboolean sensitif );
void modification_fichier ( gboolean modif );
/*END_DECLARATION*/


#endif

