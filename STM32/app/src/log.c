#include <stdio.h>
 
void create_log_file ()
{
FILE* log = NULL;
log = fopen("SACS_log.log", "w");
if (log != NULL)
{
fputs("Génération du fichier log pour le protocole SACS", log);
fclose(log);
}
}

void add_log_input (string evenement)
{
    
}