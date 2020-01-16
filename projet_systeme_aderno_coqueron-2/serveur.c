#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define PORT 8080
#define MAX_BUFFER 1000
#define MAX_MOT 1000
#define MAX_CLIENTS 10
#define MAXCHAR 1000

const char *EXIT = "exit";
int nbmots;


int randNb (void)
{
  static int premier = 0;
  if (premier == 0){
     srand (time (NULL));
     premier = 1;
  }
   return (rand());
}



int checkListecar(char car,char listecar[]){
  int reponse=0;
  for(int i=0;i<strlen(listecar);i++){
    if(car == listecar[i])
      reponse=1;
  }
  if(reponse == 0)
    listecar[strlen(listecar)]=car;
  return reponse;
}

int checkLetter(char mot[],char motHidden[],char lettre,int *nbcorrect,char listecar[]){

  printf("je check une lettre\n");
  int reponse=0;

if(checkListecar(lettre,listecar)==0){
  for(int i=0;i<strlen(mot);i++){
    if(mot[i]==lettre){
      motHidden[i]=lettre;
      *nbcorrect=*nbcorrect+1;
      reponse=1;
      }
    }
  }else
    reponse=2;

  return reponse;
}

int testMot(char tampon[],char mot[]){
  printf("test du mot en cours\n");
  int reponse=1;
  if(strlen(tampon) == strlen(mot)-1){
    for(int i = 0 ; i < strlen(tampon) ; i++){
      printf("tampon %c,mot %c\n",tampon[i],mot[i]);
      if(tampon[i] != mot[i]){
        reponse=0;
        break;
      }
    }
  }else
    reponse=0;
  return reponse;
}

void  accueilMessage(int fdSocketCommunication,char taillemot[]){
  char endmsg[100]=" lettres";
  char startMsg[100]="JEU DU PENDU ! \nVotre mot : ";
  strcat(startMsg,taillemot);
  strcat(startMsg,endmsg);
  send(fdSocketCommunication, startMsg, strlen(startMsg), 0);
}

void ecouteReponse(int fdSocketCommunication,char tampon[]){
  bzero(tampon,strlen(tampon));
  printf("ecoute du message en cours\n");
  int nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
}

char* itoa(int i, char b[]){
    char const digit[] = "0123456789";
    char* p = b;
    if(i<0){
        *p++ = '-';
        i *= -1;
    }
    int shifter = i;
    do{ //Move to where representation ends
        ++p;
        shifter = shifter/10;
    }while(shifter);
    *p = '\0';
    do{ //Move back, inserting digits as u go
        *--p = digit[i%10];
        i = i/10;
    }while(i);
    return b;
}


void RecupMot(char listeMots[][MAX_MOT]){
    FILE *fp;
    char str[MAXCHAR];
    char* nomfichier = "mots.txt";
    int i=0;

    fp = fopen(nomfichier, "r");
    if (fp == NULL){
        printf("erreur à l'ouverture du fichier %s",nomfichier);
    }
    while (fgets(str, MAXCHAR, fp) != NULL){
        strcpy(listeMots[i],str);
        i++;
        nbmots=i;
      }
    fclose(fp);
}

char* choixMot(char listeMots[][MAX_MOT]){
  int i=randNb()%nbmots-1;
  return listeMots[i];
}

int testQuitter(char tampon[]) {
    return strcmp(tampon, EXIT) == 0;
}



int main(int argc, char const *argv[]) {
    int fdSocketAttente;
    int fdSocketCommunication;;
    int nbcorrect=0;
    int nbfaute=0;
    int gagne=0;
    struct sockaddr_in coordonneesServeur;
    struct sockaddr_in coordonneesAppelant;
    char tampon[MAX_BUFFER];
    int nbRecu;
    int longueurAdresse;
    int pid;
    char listecar[MAX_MOT];
    char listeMots[MAX_MOT][MAX_MOT];
    char mot[MAX_MOT];
    char motHidden[MAX_MOT];
    char msg[MAX_MOT];

    RecupMot(listeMots);

    fdSocketAttente = socket(PF_INET, SOCK_STREAM, 0);

    if (fdSocketAttente < 0) {
        printf("socket incorrecte\n");
        exit(EXIT_FAILURE);
    }

    // On prépare l’adresse d’attachement locale
    longueurAdresse = sizeof(struct sockaddr_in);
    memset(&coordonneesServeur, 0x00, longueurAdresse);

    coordonneesServeur.sin_family = PF_INET;
    // toutes les interfaces locales disponibles
    coordonneesServeur.sin_addr.s_addr = htonl(INADDR_ANY);
    // toutes les interfaces locales disponibles
    coordonneesServeur.sin_port = htons(PORT);

    if (bind(fdSocketAttente, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
        printf("erreur de bind\n");
        exit(EXIT_FAILURE);
    }

    if (listen(fdSocketAttente, 5) == -1) {
        printf("erreur de listen\n");
        exit(EXIT_FAILURE);
    }
    socklen_t tailleCoord = sizeof(coordonneesAppelant);
    int nbClients = 0;






    while (nbClients < MAX_CLIENTS) {
        if ((fdSocketCommunication = accept(fdSocketAttente, (struct sockaddr *) &coordonneesAppelant,
                                            &tailleCoord)) == -1) {
            printf("erreur de accept\n");
            exit(EXIT_FAILURE);
        }

        printf("Client connecté - %s:%d\n",
               inet_ntoa(coordonneesAppelant.sin_addr),
               ntohs(coordonneesAppelant.sin_port));


    if ((pid = fork()) == 0) {
            close(fdSocketAttente);
            while (1) {
                  printf("Nouvelle partie \n");
                  strcpy(mot,choixMot(listeMots));
                  mot[0]=tolower(mot[0]);
                  for(int i = 0 ; i < strlen(mot)-1 ; i++){
                    motHidden[i]='*';
                  }
                printf("mot choisi : %s",mot);
                accueilMessage(fdSocketCommunication,motHidden);
                while(gagne == 0 && nbfaute < 10 && nbcorrect != strlen(mot)-1){
                    printf("nbcorrect check :%d\n",nbcorrect);
                    fflush(stdout);
                    ecouteReponse(fdSocketCommunication,tampon);
                    printf("mot recu :%s\n",tampon);
                    if(strlen(tampon) > 1){
                      if(testMot(tampon,mot) == 1){
                        printf("mot correct\n");
                        gagne=1;
                      }
                      else{
                        printf("mot incorrect\n");
                        nbfaute++;}
                    }else{
                      if(checkLetter(mot,motHidden,tampon[0],&nbcorrect,listecar) == 0){
                        nbfaute++;
                      }
                    }
                    if(nbcorrect == strlen(mot)-1)
                      gagne=1;
                    if(nbfaute>9)
                      gagne=-1;
                    if(gagne != 1 && gagne != -1){
                      printf("envoi du mot caché\n");
                      send(fdSocketCommunication,motHidden,strlen(motHidden),0);
                    }
                    printf("nb faute : %d\n",nbfaute);

                   }
                printf("fin de partie\n");
                if(gagne == -1){
                  strcpy(msg,"nombre d'erreurs maximum atteint");
                }else{
                  strcpy(msg,"partie gagné avec ");
                  char nbfauteChar[MAX_MOT];
                  strcat(msg,itoa(nbfaute,nbfauteChar));
                  strcat(msg," fautes");
                }
                send(fdSocketCommunication,msg,strlen(msg),0);
                printf("ecoute du tampon \n");
                recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
                if(tampon[0] != '1'){
                  exit(EXIT_SUCCESS);
                  break;
                }else{
                  bzero(tampon,strlen(tampon));
                  bzero(mot,strlen(mot));
                  bzero(motHidden,strlen(motHidden));
                  bzero(listecar,strlen(listecar));
                  nbfaute=0;
                  nbcorrect=0;
                  gagne=0;
                }
              }
              }
      nbClients++;
    }

    close(fdSocketCommunication);
    close(fdSocketAttente);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        wait(NULL);
    }

    printf("Fin du programme.\n");
    return EXIT_SUCCESS;
}
