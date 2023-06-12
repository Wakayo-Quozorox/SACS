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
else
{

}
}

void add_log_input (string evenement)
{
    BSP_RTC_GetTime(&now);
    log = freopen("SACS_log.log", "w");
    if (log != NULL)
    {
        fprintf(log,"%02d:%02d:%02d %s \r", now.hours, now.minutes, now.seconds, evenement);
        fclose(log);
    }
    
}