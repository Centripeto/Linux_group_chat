#include <unistd.h>
#include <pthread.h>

#include "logger.h"
#include "utility_functions.h"

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080

char* clientName = NULL;
int clientSocketFD;

void createClientName();

void greetServer(struct sockaddr_in *serverAddress);

void startChat();

void listenForIncomingMessages();

int main() {

  // Generazione del nome per il Client
  createClientName();

  // Creazione del socket del Client attraverso il metodo preso dalle utils
  clientSocketFD = createTCPIpv4Socket();
  if(errno != 0) {
    printf("An error occured while trying to create the connection socket.\n");
    eventLogger("An error occured while Client %s was trying to create the connection socket.", clientName);
    exit(1);
  } else {
    printf("TCP Client socket created successfully.\n");
    eventLogger("TCP Client socket created successfully for %s.", clientName);
  }

  // Configurazione indirizzo e porta del Server attraverso il metodo preso dalle utils
  struct sockaddr_in *serverAddress = createIPv4Address(SERVER_ADDRESS, SERVER_PORT);
  if(errno != 0) {
    printf("An error occured while trying to configurate Server's address and port.\n");
    eventLogger("An error occured while Client %s was trying to configurate Server's address and port.", clientName);
    exit(1);
  } else {
    printf("Server's address and port successfully configurated.\n");
    eventLogger("Server's address and port successfully configurated for Client %s.", clientName);
  }

  // Connessione al server predefinito
  int connection_status = connect(clientSocketFD, (struct sockaddr *) serverAddress, sizeof(*serverAddress));
  if(connection_status != 0) {
    printf("An error occured while trying to connect to the server.\n");
    eventLogger("An error occured while %s was trying to connect to the server.", clientName);
    exit(1);
  } else {
    printf("Connection correctly established.\n\n");
    eventLogger("Client %s's attempt to connect to the server was successful.", clientName);
  }

  // Saluto e presentazione al Server
  greetServer(serverAddress);

  // Una volta effettuata la handshake, viene creato un thread di ascolto dedicato
  // per poter ascoltare in tempo reale senza bloccare il sistema
  pthread_t id;
  pthread_create(&id, NULL, listenForIncomingMessages, NULL);

  // Una volta creato il thread dedicato, il Client potrà iniziare ad inviare messaggi al Server
  startChat();

  // Attesa della chiusura del thread per l'ascolto e chiusura della connessione al Server
  pthread_join(id, NULL);
  close(clientSocketFD);

  return 0;

}

void createClientName() {
  size_t nameSize = 0;
  printf("Please enter your name:\n");
  ssize_t nameCount = getline(&clientName, &nameSize, stdin);
  clientName[nameCount-1] = 0;
  printf("\n");
}

void greetServer(struct sockaddr_in *serverAddress) {
  int responseNumber;
  char bufferForServer[1024];
  char bufferFromServer[1024];

  sprintf(bufferForServer, "%s: Hello Server!, my IP address is: %s\n", clientName, inet_ntoa(serverAddress->sin_addr));
  send(clientSocketFD, bufferForServer, strlen(bufferForServer), 0);
  printf("Client name: %s - Client address: %s\n\n", clientName, inet_ntoa(serverAddress->sin_addr));
  eventLogger("Client %s with IP address: %s has entered the chat.", clientName, inet_ntoa(serverAddress->sin_addr));

  // Attesa ricezione saluto e info server
  while(1) {
    responseNumber = recv(clientSocketFD, bufferFromServer, sizeof(bufferFromServer), 0);
    if(responseNumber > 0) {
      printf("Welcome message from Server: %s\n\n", bufferFromServer);
      eventLogger("The Server greeted %s with the following message: %s", clientName, bufferFromServer);
      break;
    }
  }
}

void listenForIncomingMessages() {
  char listenBuffer[1024];
  while (1) {
    ssize_t amountReceived = recv(clientSocketFD, listenBuffer, sizeof(listenBuffer), 0);
    if(amountReceived > 0) {
      listenBuffer[amountReceived] = 0;
      if(strcmp(listenBuffer, "Server: See you soon!") == 0) {
        printf("%s\n", listenBuffer);
        eventLogger("Server said goodbye to Client %s.", clientName);
        break;
      }
      printf("%s\n", listenBuffer);
      eventLogger("%s", listenBuffer);
    }
    if(amountReceived == 0) break;
  }
}

void startChat() {
  char *line = NULL;
  size_t lineSize = 0;
  printf("Type exit whenever you want to leave the chat.\n");

  char chatBuffer[1024];

  while(1) {
    ssize_t charCount = getline(&line, &lineSize, stdin);
    line[charCount-1] = 0;

    // Se è stato inviato un messaggio non vuoto
    if(charCount > 0) {
      // Se è stato scritto precisamente "exit", si lascia la chat salutando il Server e ricevendo infine la sua risposta
      if(strcmp(line, "exit") == 0) {
        sprintf(chatBuffer, "Bye Server! I'm going.");
        ssize_t amountSent = send(clientSocketFD, chatBuffer, strlen(chatBuffer), 0);
        eventLogger("%s: %s", clientName, chatBuffer);
        break;
      } else {
        sprintf(chatBuffer, "%s: %s", clientName, line);
        ssize_t amountSent = send(clientSocketFD, chatBuffer, strlen(chatBuffer), 0);
      }
    }
  }
}
