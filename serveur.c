#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <poll.h>
#include <assert.h>
#define NB_CLIENT 3 //Supporte au maximum 2 joueurs + le serveur
#define NAME_MAX_LEN 32 //Un maximum de 32 lettres

typedef struct Serveur{
    int nb_client;
    struct pollfd liste [NB_CLIENT];
}Serveur;

//Création du serveur
Serveur* Nouveau_Serveur (const char* port) {
	Serveur* serv = NULL;
  int socket_srv;
  struct sockaddr_in addr;

  socket_srv = socket(AF_INET, SOCK_STREAM, 0); //vérification d'erreurs
      if (socket_srv < 0) {
          perror("socket");
          exit(1);
      }

  //port = atoi(argv[1]);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(atoi(port)); //htons(port)
                
  if (bind (socket_srv, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
          perror("bind");
          exit(1);   
  }

  if (listen (socket_srv, 8) < 0){
      perror("listen");
      exit(1);      
  }

  serv = malloc (sizeof(Serveur)); //(sizeof(*serv)) = same
  assert(serv);

  serv->nb_client = 0;
  bzero(serv->liste, sizeof(*serv->liste) * (NB_CLIENT));
  serv->liste[0].fd = socket_srv;
  serv->liste[0].events = POLLIN;

	return serv;
}

//Supprimer le serveur
void Supprimer_Serveur (Serveur* serveur) {
  for (int i = 0; i <= serveur->nb_client; ++i) {
    close(serveur->liste[i].fd);
  }
  free(serveur);
}

//Envoie des données dans le serveur
void Envoyer_donnees (int fd, const char *buf, int len) {
  int mot = 0;
  int n;

  for (mot = 0; mot < len; mot += n) {
  	n = write (fd, buf + mot, len - mot);
  	if (n < 0)
  		break;
  }

  if (n == -1) {
    fprintf(stderr, "Données partiellement envoyées\n");
  }
}

int Ajout_client (Serveur *serveur) {
  int fd; //file descriptor
  if ((fd = accept(serveur->liste[0].fd, NULL, NULL)) < 0) {
    if (errno == EWOULDBLOCK) {
      perror("accept");
      return -1;
    }
  }

  if (serveur->nb_client == (NB_CLIENT - 1)) {
    write(fd, "Too much players connected\n", 26);
    close(fd);
    return -1;
  }

  serveur->nb_client += 1;
  serveur->liste[serveur->nb_client].fd = fd;
  serveur->liste[serveur->nb_client].events = POLLIN;
  Envoyer_donnees(serveur->liste[serveur->nb_client].fd , "connect", 7);

  return fd;
}

void Grillej1(int grille[]){
  grille[1*15+1] = 3;
  grille[1*15+2] = 3;
  grille[1*15+3] = 3;

  grille[2*15+7] = 2;
  grille[2*15+8] = 2;

  grille[5*15+3] = 4;
  grille[6*15+3] = 4;
  grille[7*15+3] = 4;
  grille[8*15+3] = 4;

  grille[6*15+11] = 1;

  grille[11*15+5] = 5;
  grille[11*15+6] = 5;
}

void Grillej2(int grille[]){
  grille[4*15+7] = 3;
  grille[5*15+7] = 3;
  grille[6*15+7] = 3;

  grille[2*15+4] = 2;
  grille[3*15+4] = 2;

  grille[8*15+6] = 4;
  grille[8*15+7] = 4;
  grille[8*15+8] = 4;
  grille[8*15+9] = 4;

  grille[1*15+11] = 1;

  grille[11*15+3] = 5;
  grille[12*15+3] = 5;
}

void Transfomer_grille(char* buf, int grille[]){
  int i; 
  for (i = 0; i < 225; ++i) {
    if(grille[i] == 0)
      buf[i] = 126;
    else
      buf[i] = 120;
  }
}

void BoucleJeu(int grillej1[], int grillej2[], Serveur* client) {
  /* Savoir quel joueur joue */
  struct pollfd sockj1[1];
  struct pollfd sockj2[1];

  sockj1[0] = client->liste[1];
  sockj2[0] = client->liste[2];

  int joueuractif = 0; 
  int attaque;
  char buf[512];
  int vieBateauxj1[] = {1,2,3,4,2};
  int vieBateauxj2[] = {1,2,3,4,2};

  /* Initialisation des vies */
  int viej1 = 12; 
  int viej2 = 12;

  while(viej1 > 0 && viej2 > 0) {
    if(joueuractif == 0){
      /* affiche l'historique des coups joués par le joueur 1 */
      Envoyer_donnees(sockj1[0].fd, "attack", 6);
      poll(sockj1, 1, 900000);
      read(sockj1[0].fd, buf, sizeof(buf));
      attaque = atoi(buf);

      if(grillej2[attaque] != 0) {
        viej2--;
        vieBateauxj2[grillej2[attaque]-1]--;
        /* verifie si le bateau n'a plus de vie */
        if(vieBateauxj2[grillej2[attaque]] == 0){
          Envoyer_donnees(sockj1[0].fd, "coule", 5);
        }

        else {
          Envoyer_donnees(sockj1[0].fd, "touche", 6);
        }

        grillej2[attaque] = 0;
      }

      else {
        Envoyer_donnees(sockj1[0].fd, "rate", 4);
      }
    }

    else {  
      Envoyer_donnees(sockj2[0].fd, "attack", 6);
      poll(sockj2, 1, 50000);
      read(sockj2[0].fd, buf, sizeof(buf));
      attaque = atoi(buf);

      if(grillej1[attaque] != 0) {
        viej2--;
        vieBateauxj1[grillej2[attaque]-1]--;
        /* vérifie si le bateau n'a plus de vie */

        if(vieBateauxj2[grillej2[attaque]] == 0){
          Envoyer_donnees(sockj2[0].fd, "coule", 5);
        }

        else {
          Envoyer_donnees(sockj2[0].fd, "touche", 6);
        }

        grillej2[attaque] = 0;
        /* met une indication que le joueur a touché un bateau sur la mémoire */
      }

      else {
        Envoyer_donnees(sockj2[0].fd, "rate", 4);
      }           
    }

    joueuractif = 1 - joueuractif;
  }

  if(joueuractif == 1){
     Envoyer_donnees(sockj1[0].fd, "Victory 2", 9);
     Envoyer_donnees(sockj2[0].fd, "Victory 1", 9);
  }

  else {
     Envoyer_donnees(sockj2[0].fd, "Victory 2", 9);
     Envoyer_donnees(sockj1[0].fd, "Victory 1", 9);
  }
}

int main(int argc, char* argv[]){

  Serveur* client;
  int grillej1[225];
  int grillej2[225];
  bzero(grillej1, 225);
  bzero(grillej2, 225);
  char buffer[512];

  if(argc != 2){
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  if((client = Nouveau_Serveur(argv[1])) == NULL) {
    fprintf(stderr, "Le serveur ne peut pas etre lance.\n");
    exit(1);
  }

  Ajout_client(client);
  Ajout_client(client);
  Grillej1(grillej1);
  Grillej2(grillej2);
  Transfomer_grille(buffer, grillej1);
  Envoyer_donnees(client->liste[1].fd, buffer, 225);
  Transfomer_grille(buffer, grillej2);    
  Envoyer_donnees(client->liste[2].fd, buffer, 225);
  sleep(2);
  BoucleJeu(grillej1, grillej2, client);
  Supprimer_Serveur(client);
  return 0;
}