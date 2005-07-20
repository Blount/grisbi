#ifndef _ECHEANCIER_LISTE_H
#define _ECHEANCIER_LISTE_H (1)

#define COL_NB_DATE 0
#define COL_NB_ACCOUNT 1
#define COL_NB_PARTY 2
#define COL_NB_FREQUENCY 3
#define COL_NB_MODE 4
#define COL_NB_NOTES 5
#define COL_NB_AMOUNT 6		/* doit Ãªtre le dernier de la liste
				   Ã  cause de plusieurs boucles for */
#define NB_COLS_SCHEDULER 7

/* define the columns in the store
 * as the data are filled above, the number here
 * begin at NB_COLS_SCHEDULER */

#define SCHEDULER_COL_NB_BACKGROUND 8		/*< color of the background */
#define SCHEDULER_COL_NB_SAVE_BACKGROUND 9	/*< when selection, save of the normal color of background */
#define SCHEDULER_COL_NB_AMOUNT_COLOR 10 	/*< color of the amount */
#define SCHEDULER_COL_NB_TRANSACTION_ADDRESS 11
#define SCHEDULER_COL_NB_FONT 12		/*< PangoFontDescription if used */

#define SCHEDULER_COL_NB_TOTAL 13 


enum scheduler_periodicity {
    SCHEDULER_PERIODICITY_ONCE_VIEW,
    SCHEDULER_PERIODICITY_WEEK_VIEW,
    SCHEDULER_PERIODICITY_MONTH_VIEW,
    SCHEDULER_PERIODICITY_TWO_MONTHS_VIEW,
    SCHEDULER_PERIODICITY_TRIMESTER_VIEW,
    SCHEDULER_PERIODICITY_YEAR_VIEW,
    SCHEDULER_PERIODICITY_CUSTOM_VIEW,
    SCHEDULER_PERIODICITY_NB_CHOICES,
};

enum periodicity_units {
    PERIODICITY_DAYS,
    PERIODICITY_WEEKS,
    PERIODICITY_MONTHS,
    PERIODICITY_YEARS,
};

/* START_INCLUDE_H */
#include "echeancier_liste.h"
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean affichage_traits_liste_echeances ( void );
void affiche_cache_commentaire_echeancier( void );
void click_sur_saisir_echeance ( void );
GtkWidget *creation_liste_echeances ( void );
void edition_echeance ( void );
void gsb_scheduler_check_scheduled_transactions_time_limit ( void );
gboolean gsb_scheduler_delete_scheduled_transaction ( struct operation_echeance *scheduled_transaction );
gboolean gsb_gui_change_scheduler_view ( enum scheduler_periodicity periodicity );
void new_scheduled_transaction ( void );
void remplissage_liste_echeance ( void );
void selectionne_echeance ( struct operation_echeance *echeance );
void supprime_echeance ( struct operation_echeance *echeance );
gboolean traitement_clavier_liste_echeances ( GtkWidget *tree_view_liste_echeances,
					      GdkEventKey *evenement );
/* END_DECLARATION */
#endif
