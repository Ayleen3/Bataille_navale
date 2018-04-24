#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> 
#include <assert.h>
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <poll.h>
#include <stdarg.h>
#define NB_PLAYERS_MAX 2

void Client_send (int fd, const char *buf, int len) {
  int sent_len = 0;
  int n;
  
  while (sent_len < len) {
    n = write(fd, buf + sent_len, len - sent_len);
    if (n < 0) {
      break;
    }
    sent_len += n;
  }
  
  if (n == -1) {
    fprintf(stderr, "Données partiellement envoyées\n");
  }
}

void Client_sendf (int fd, const char *fmt, ...) {
  va_list va;
  char buf[4096];
  int len;
  
  va_start(va, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, va);
  va_end(va);
  
  if (len >= (int)sizeof(buf)) {
    fprintf(stderr, "String trop long, annulé\n");
  }
  Client_send(fd, buf, len);
}

void Affiche_grille (int* grille){
  int ligne = 0, colonne = 0; 
  for (; ligne < 15; ++ligne) {
    for (; colonne < 15; ++colonne){
      if(grille[ligne * 15 + colonne] == 0)
        printf(" . ");
      else
        printf(" %d ", grille[ligne * 15 + colonne]);
    }
    printf("\n");
  }
}

void Affiche_grille_c (char* buffer){
  int ligne = 0, colonne = 0; 
  for (; ligne < 15; ++ligne) {
    for (; colonne < 15; ++colonne){
      printf("%c", buffer [(ligne * 15 + colonne)]);
    }
    printf("\n");
  }
}

void reaction(char *buf, int sock, int memoirejoueur[], int marque){

	int attaque1 = 0;
  int attaque2 = 0;

	if(strncmp (buf, "attack", 6) == 0){
      Affiche_grille(memoirejoueur);
      printf("attente des coordonnées x \n");
      scanf("%d", &attaque1);
      printf("attente des coordonnées y \n");
      scanf("%d", &attaque2);
      attaque2 *= 15;
      attaque2 += attaque1;
      Client_sendf(sock,"%d",attaque2);
      marque = attaque2;
	}

	if(strncmp (buf, "~", 1) == 0){
    printf("Voici votre grille de bateau\n");
		Affiche_grille_c(buf);
	}

	if(strncmp (buf, "coule", 5) == 0){
    printf("coule \n");
		memoirejoueur[marque] = 1;
	}

	if(strncmp (buf, "touche", 6) == 0){
    printf("touche\n");
		memoirejoueur[marque] = 1;
  }

  if(strncmp (buf, "rate", 4) == 0){
  printf("rate \n");
	memoirejoueur[marque] = 2;
	}
}

int main(int argc,char* argv[]){
  struct pollfd fds[1];
	int sock = 0, marque = 0;
	struct sockaddr_in address;
	struct hostent *host;
	char buf[512];
	int memoirejoueur[225];
  bzero(memoirejoueur, 225);

  if(argc != 3){ 
      printf("Usage: %s <host> <port>\n", argv[0]);
      exit(1);
  }

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		exit(1);
	}

	host = gethostbyname(argv[1]);
  	if (host == NULL) {
   		perror("gethostbyname");
    	exit(1);
  	}

  	address.sin_family = AF_INET;
  	address.sin_addr.s_addr = *((uint32_t *) host->h_addr);
  	address.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
    		perror("connect");
    		exit(1);
  	}

  	fds[0].fd = sock;
  	fds[0].events = POLLIN;

    poll(fds, 1, 15000);
    read(fds[0].fd, buf, sizeof(buf));

    if (strncmp (buf, "connect", 7) == 0){
      while (strncmp (buf, "Victory", 7) != 0){
      	poll (fds, 1, 888000);
        read (fds[0].fd, buf, sizeof(buf));
      	reaction(buf,fds[0].fd, memoirejoueur, marque);
       }
    }

    if(strncmp (buf, "Victory 2",9) == 0){
      printf("Vous avez gagné\n");
    }

    else {
      printf("Vous avez perdu\n");
    }

    close(sock);
    return -1;
}
