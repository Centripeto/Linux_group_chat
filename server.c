#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/select.h>

#include "logger.h"
#include "utility_functions.h"

#define SERVER_PORT 8080

struct AcceptedSocket {
  int acceptedSocketFD;
  struct sockaddr_in address;
  char name[32];
  int error;
  bool acceptedSuccessfully;
};

int serverSocketFD;
struct AcceptedSocket acceptedSockets[10];
int atLeastOneClientConnected = 0;
int acceptedSocketsCount = 0;

void listenForNewConnectionsAndAccept();

struct AcceptedSocket * acceptIncomingConnection(struct sockaddr_in clientAddress, int clientSocketFD);

void createNewThreadForClient(struct AcceptedSocket *pSocket);

void receiveAndPrintIncomingData(int clientSocketFD);

char* greetNewClient(int clientSocketFD);

char* getClientName(char* input);

void sendMessageToOtherClients(char *buffer, int socketFD);

void disposeClient(int clientSocketFD);

int main() {

  // Creazione socket TCP
  serverSocketFD = createTCPIpv4Socket();
  if(errno != 0) {
    printf("An error occured while trying to create the connection socket.\n");
    eventLogger("An error occured while trying to create the connection socket of the Server.");
    exit(1);
  } else {
    printf("TCP Server socket created successfully.\n");
    eventLogger("TCP Server socket created successfully.");
  }

  // Configurazione indirizzo e porta del Server attraverso il metodo preso dalle utils
  struct sockaddr_in *serverAddress = createIPv4Address("", SERVER_PORT);
  if(errno != 0) {
    printf("An error occured while trying to configurate Server's address and port.\n");
    eventLogger("An error occured while trying to configurate Server's address and port.");
    exit(1);
  } else {
    printf("Server's address and port successfully configurated.\n");
    eventLogger("Server's address and port successfully configurated.");
  }

  // Assegnazione indirizzo al socket
  int bindResult = bind(serverSocketFD, (struct sockaddr *) serverAddress, sizeof(*serverAddress));
  if(bindResult != 0) {
    printf("An error occured while trying to configurate Server's address and port.\n");
    eventLogger("An error occured while trying to configurate Server's address and port.");
    exit(1);
  } else {
    printf("Server is binded successfully.\n");
    eventLogger("Server is binded successfully.");
  }

  // Messa in ascolto su porta
  int listenResult = listen(serverSocketFD, 10);
  if(listenResult != 0) {
    printf("An error occured while trying to listen for new connections.\n");
    eventLogger("An error occured while the Server was trying to listen for new connections.");
    exit(1);
  } else {
    printf("Server is listening correctly for new connections.\n\n");
    eventLogger("Server is listening correctly for new connections.");
  }

  // Una volta messo in ascolto il Server, crea un thread separato
  // per poter accettare nuove connessioni da parte dei nuovi Clients senza bloccare il sistema
  pthread_t id;
  pthread_create(&id, NULL, listenForNewConnectionsAndAccept, NULL);

  // Attesa della chiusura del thread per l'ascolto e chiusura della connessione al Server
  pthread_join(id, NULL);

  // Chiusura del Server
  printf("Server shutting down. No Clients are connected with Server.\n");
  eventLogger("Server shutting down. No Clients are connected with Server.");
  shutdown(serverSocketFD, SHUT_RDWR);

  return 0;

}

void listenForNewConnectionsAndAccept() {
  struct sockaddr_in clientAddress;
  int clientAddressSize = sizeof(struct sockaddr_in);
  while(1) {
    fd_set set;
    struct timeval timeout = {
      .tv_sec = 5,
      .tv_usec = 0
    };

    FD_ZERO(&set);
    FD_SET(serverSocketFD, &set);

    if(select(serverSocketFD+1, &set, NULL, NULL, &timeout) > 0) {
      int clientSocketFD = accept(serverSocketFD, (struct sockaddr *) &clientAddress, &clientAddressSize);
      if(clientSocketFD < 0) {
        if((errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
        errno == ENONET || errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH)) {
          sleep(1);
        } else {
          printf("An error occured while trying to accept a new Client's connection.\n");
          eventLogger("Server has got an error while trying to accept a new Client's connection."); 
        }
      } else {
        struct AcceptedSocket* clientSocket = acceptIncomingConnection(clientAddress, clientSocketFD);
        createNewThreadForClient(clientSocket);
      }
    }
    // Se almeno un Client si è collegato alla chat e non ne rimane più nessuno, procedi alla chiusura del Server
    //printf("%d, %d\n", atLeastOneClientConnected, acceptedSocketsCount);
    if(atLeastOneClientConnected == 1 && acceptedSocketsCount == 0) break;
  }
}

struct AcceptedSocket * acceptIncomingConnection(struct sockaddr_in clientAddress, int clientSocketFD) {

  struct AcceptedSocket* acceptedSocket = malloc(sizeof (struct AcceptedSocket));

  acceptedSocket->address = clientAddress;
  acceptedSocket->acceptedSocketFD = clientSocketFD;
  acceptedSocket->acceptedSuccessfully = true;

  printf("A new Client has been accepted to communicate.\n");
  eventLogger("Server has accepted successfully a new Client into connection pool.");

  acceptedSockets[acceptedSocketsCount++] = *acceptedSocket;
  atLeastOneClientConnected = 1;

  if(acceptedSocketsCount > 1) {
    printf("Clients connected right now with Server are %d.\n", acceptedSocketsCount);
    eventLogger("Clients connected right now with Server are %d", acceptedSocketsCount);
  } else {
    printf("Only %d Client is connected right now with Server.\n", acceptedSocketsCount);
    eventLogger("Only %d Client is connected right now with Server.", acceptedSocketsCount);
  }

  return acceptedSocket;
}

void createNewThreadForClient(struct AcceptedSocket *pSocket) {
  pthread_t id;
  pthread_create(&id, NULL, receiveAndPrintIncomingData, pSocket->acceptedSocketFD);
}

void receiveAndPrintIncomingData(int clientSocketFD) {
  char buffer[1024];
  char* clientName = greetNewClient(clientSocketFD);

  while (1) {
    ssize_t amountReceived = recv(clientSocketFD, buffer, sizeof(buffer), 0);

    // Reindirizzamento messaggio di un Client verso tutti gli altri
    if(amountReceived > 0) {
      buffer[amountReceived] = 0;
      // Il Client ha fatto richiesta di disconnessione dalla chat?
      if(strcmp(buffer, "Bye Server! I'm going.") == 0) {
        // Saluta il Client che se ne sta andando
        sprintf(buffer, "Server: See you soon!");
        send(clientSocketFD, buffer, strlen(buffer), 0);
        printf("Client %s has left the chat.\n", clientName);
        eventLogger("Client %s has left the chat.", clientName);
        if(acceptedSocketsCount > 0) {
          // Notifica a tutti gli altri Client collegati dell'avvenuta disconnessione dalla chat di uno di essi
          sprintf(buffer, "Client %s has left the chat.", clientName);
          sendMessageToOtherClients(buffer, clientSocketFD);
        }
        break;
      }
      // Inoltro del messaggio ricevuto da un Client verso tutti gli altri ancora collegati
      printf("%s\n", buffer);
      eventLogger("%s", buffer);
      sendMessageToOtherClients(buffer, clientSocketFD);
    }
  }
  disposeClient(clientSocketFD);
}

char* greetNewClient(int clientSocketFD) {
  char buffer[1024];
  int responseNumber;
  char* clientName = malloc(sizeof(char*));

  // Attesa di ricezione del saluto e delle info del nuovo Client
  while(1) {
    responseNumber = recv(clientSocketFD, buffer, sizeof(buffer), 0);
    if(responseNumber > 0) {
      printf("%s", buffer);
      clientName = getClientName(buffer);
      eventLogger("The Server recieved greetings from Client %s.", clientName);
      break;
    }
  }
  // Invio saluto di benvenuto al nuovo Client
  sprintf(buffer, "Welcome %s, have fun!", clientName);
  send(clientSocketFD, buffer, strlen(buffer), 0);
  printf("Welcome %s, have fun!\n", clientName);
  eventLogger("The Server sent a welcome text to the Client %s.", clientName);

  return clientName;
}

char* getClientName(char* input) {
  char* initial = malloc(strlen(input) + 1);  
  char* src = input;
  char* dst = initial;

  while(*src != ':' && *src != '\0') {
    *dst = *src;
    src++;
    dst++;
  }

  *dst = '\0';

  return initial;
}

void sendMessageToOtherClients(char *buffer, int ignoredSocketFD) {
  for(int i = 0 ; i < acceptedSocketsCount ; i++)
    if(acceptedSockets[i].acceptedSocketFD != ignoredSocketFD) {
      send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
    }
}

void disposeClient(int clientSocketFD) {
  int toRemove = -1;

  for(int i = 0; i < acceptedSocketsCount; i++) {
    if(acceptedSockets[i].acceptedSocketFD == clientSocketFD) {
        toRemove = i;
    }
  }
  for (int i = toRemove; i < acceptedSocketsCount - 1; i++) {
    acceptedSockets[i] = acceptedSockets[i+1];
  }

  acceptedSocketsCount--;
  close(clientSocketFD);
}
