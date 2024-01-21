# Linux_group_chat
Chat comune in architettura Client-Server scritto in linguaggio C e compatibile con i sistemi Linux.

Il seguente progetto in C è stato realizzato attraverso l'uso delle librerie di Linux e rappresenta un sistema di Client e Server in locale che permette ai vari Client (max. 10 collegati contemporaneamente) di parlare all'interno di una chat comune.

Il Server è capace di:
- Ricevere richieste di connessione da parte dei Clients (massimo 10 contemporaneamente)
- Rispondere ai Client che si connettono al Server con un saluto personalizzato
- Ricevere del testo dai Clients e reinoltrarlo a tutti i Clients connessi
- Quando un Client chiede di disconnettersi dalla chat comune:
    - il Server lo saluta
    - avvisa gli altri Clients ancora collegati della disconnessione del Client
    - chiude la connessione con esso liberando le risorse impiegate
- Spegnersi autonomamente quando, dopo che almeno un Client si sia collegato alla chat, non rimane più nessuno online

Il Client è capace di:
- Instaurare una connessione con il Server effettuando una richiesta inviando le proprie info (Nome del Client, Indirizzo IP)
- Inviare del testo al Server di grandezza massima 1024 Bytes prendendo il testo digitato in console dall'utente
- Uscire correttamente dalla chat di gruppo salutando il Server e chiudere la connessione con esso

Tutte le informazioni rilevanti, ovvero errori riscontrati durante l'esecuzione del programma e le comunicazioni tra il Server e i vari Clients, sono documentate all'interno di un file di log unico indicando la data e l'orario del messaggio.

La gestione dei processi bloccanti come la funzione accept() e l'ascolto in generale del socket per ricevere messaggi sono stati gestiti attraverso i threads.

Facendo un elenco pratico dei possibili miglioramenti che si potrebbero implementare all'interno del progetto, si potrebbe realizzare:
- Una gestione più efficiente della memoria usando maggiormente la funzione select() anzichè aprire direttamente un thread dedicato
- Una funzione che avvisa gli altri Clients già collegati alla chat dell'avvenuta connessione di un nuovo ospite
- Una funzione che elenca i nomi dei Clients già connessi alla chat da fornire al nuovo ospite

# ATTENZIONE!!
Il codice contenuto all'interno di questo progetto della chat comune è stato realizzato a scopo accademico universitario per allenarmi all'uso del linguaggio C (e la sua enorme complessità per quanto il progetto non sia di per sè avanzato) su base Ubuntu Linux.
E' caldamente sconsigliato utilizzare il codice di questo progetto all'interno di un Web Server pubblico per via della sua fragilità ma soprattutto per le falle di sicurezza contenute. Da usare a vostro rischio e pericolo per dei server di produzione!

Non mi assumo alcuna responsabilità derivato dall'uso che viene fatto del codice sia in ambito privato che professionale.

# PER AVVIARE IL SISTEMA
In questo progetto, non essendo presenti i file CMake utili per poter compilare in velocità i file necessari al Server ed ai Clients, di seguito vengono forniti i comandi da avviare in console quando ci si trova nella folder dove saranno presenti i file di questo progetto:

gcc -o Server server.c logger.c utility_functions.c
    
gcc -o Client client.c logger.c utility_functions.c

Successivamente, per avviare il Server e i vari Clients, sarà sufficiente avviare i file binari dalla console attraverso i comandi ./Server e ./Client
Se si è curiosi di sapere come stanno andando i processi attivati dal Server e dai vari Clients sulla porta impegnata per le comunicazioni (di default per il progetto è la 8080), si può aprire una nuova console e digitare il comando: lsof -i :8080
