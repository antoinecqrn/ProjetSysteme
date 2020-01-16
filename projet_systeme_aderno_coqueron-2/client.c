#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER 1000

const char *EXIT = "exit";


void lireMessage(char tampon[]) {
    printf("Saisir un mot à envoyer :\n");
    fgets(tampon, MAX_BUFFER, stdin);
    strtok(tampon, "\n");
}

int testQuitter(char tampon[]) {
    if(strlen(tampon) > 20 && strlen(tampon) < 40)
      return 1;
    else
      return 0;
}




int main(int argc , char const *argv[]) {
    int fdSocket;
    int nbRecu;
    int err=0;

    struct sockaddr_in coordonneesServeur;
    int longueurAdresse;
    char tampon[MAX_BUFFER];

    fdSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (fdSocket < 0) {
        printf("socket incorrecte\n");
        exit(EXIT_FAILURE);
    }

    // On prépare les coordonnées du serveur
    longueurAdresse = sizeof(struct sockaddr_in);
    memset(&coordonneesServeur, 0x00, longueurAdresse);

    coordonneesServeur.sin_family = PF_INET;
    // adresse du serveur
    inet_aton("127.0.0.1", &coordonneesServeur.sin_addr);
    // toutes les interfaces locales disponibles
    coordonneesServeur.sin_port = htons(PORT);

    if (connect(fdSocket, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
        printf("connexion impossible\n");
        exit(EXIT_FAILURE);
    }

    printf("connexion ok\n");

    while (1) {
      printf("nouvelle partie \n\n");
      err=0;
      while(err == 0){
        fflush(stdin);
        bzero(tampon,strlen(tampon));
        nbRecu=recv(fdSocket, tampon, MAX_BUFFER, 0);
        if (nbRecu > 0) {
          tampon[nbRecu] = 0;
          printf("Recu : %s\n", tampon);
          if (testQuitter(tampon)==1) {
            err=1;// on quitte la boucle

            }
        }
        if(err != 1){
         lireMessage(tampon);
         send(fdSocket, tampon, strlen(tampon), 0);
       }

      }
      printf("Voulez vous réessayer ? Tapez 1 pour confirmer");
      char reponse[MAX_BUFFER];
      fgets(reponse, MAX_BUFFER, stdin);
      send(fdSocket, reponse, strlen(reponse), 0);
      if(reponse[0] != '1'){
        close(fdSocket);
        return EXIT_SUCCESS;
      }
  }
}
