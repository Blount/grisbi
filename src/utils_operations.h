/*START_DECLARATION*/
struct structure_operation *operation_par_cheque ( gint no_cheque,
						   gint no_compte );
struct structure_operation *operation_par_id ( gchar *no_id,
					       gint no_compte );
struct structure_operation *operation_par_no ( gint no_operation,
					       gint no_compte );
void update_transaction_in_trees ( struct structure_operation * transaction );
/*END_DECLARATION*/

