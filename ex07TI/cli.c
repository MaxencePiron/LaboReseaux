/*---------------------------------------------------------------
   Vanstapel Herman
   cphex\cli.c

 Le client dit bonjour et
 le serveur fait de même
------------------------------------------------------------------*/
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>  /* ces deux include pour le getpid */
#include <string.h>
#include <errno.h>
#include "../udplib/udplib.h"
#include "structure.h"

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char *argv[])
{
 int rc ;
 int Desc ;
 int tm ; 
 int Port, Ip ;
 char Tampon[80] ;
 int returned_from_longjump ;
 unsigned int time_interval  ; 
 int ret ;
 int Compteur = 0 ;
 struct timeval tv;
 
 struct sockaddr_in sthis ; /* this ce programme */
 struct sockaddr_in sos ; /* s = serveur */
 struct sockaddr_in sor ; /* r = remote */
 u_long  IpSocket , IpServer;
 u_short PortSocket, PortServer ; 
 
 struct Requete UneRequeteE ;
 struct Requete UneRequeteR ;

 memset(&sthis,0,sizeof(struct sockaddr_in)) ;
 memset(&sos,0,sizeof(struct sockaddr_in)) ; 
 memset(&sor,0,sizeof(struct sockaddr_in)) ; 
 
 if (argc!=5)
{
    printf("cli client portc serveur ports\n") ;
    exit(1) ;
 }

 /* Récupération IP & port   */
 IpSocket= inet_addr(argv[1]);
 PortSocket = atoi(argv[2]);
 
 IpServer = inet_addr(argv[3]) ;
 PortServer = atoi(argv[4]);
 
 // Desc = CreateSockets(&psoo,&psos,,atoi(argv[2]),argv[3],atoi(argv[4])) ;
 Desc=creer_socket(SOCK_DGRAM,&IpSocket,PortSocket,&sthis);
 
 if ( Desc == -1 )
    die("CreateSockets:") ;
 else
    fprintf(stderr,"CreateSockets %d\n",Desc) ;
  tv.tv_sec = 10;
     tv.tv_usec = 0;
     if (setsockopt( Desc, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      		perror("Error");
  	}
 
 Compteur = 0 ;
 while(1)
 {
   sos.sin_family = AF_INET ;
   sos.sin_addr.s_addr= IpServer ;
   sos.sin_port = htons(PortServer) ;
   
   
   UneRequeteE.Compteur = htonl(Compteur) ;
   UneRequeteE.Type = htonl(Question) ; 
   strncpy(UneRequeteE.Message , "Hello Multiclient" , sizeof(UneRequeteE.Message)) ;

   redo:
   rc = SendDatagram(Desc,&UneRequeteE,sizeof(struct Requete) ,&sos ) ;

   if ( rc == -1 )
      perror("SendDatagram") ;
   else
      fprintf(stderr,"Envoi du message %d avec %d bytes\n",ntohl(UneRequeteE.Compteur), rc ) ;
 
   while(1) /* on boucle tant que l'on ne reçoit pas le bon message */
   {
     memset(&UneRequeteR,0,sizeof(struct Requete)) ;
     tm = sizeof(struct Requete) ;
     rc = ReceiveDatagram( Desc, &UneRequeteR,tm, &sor ) ;
     if (rc<0)
     {
        fprintf(stderr,"rc %d errno:%d \n",rc,errno) ;
        perror("receive:") ;
	if ( errno==EAGAIN )
	   goto redo ;
	else
	   exit(0) ;
     }		
     else 
       if (ntohl(UneRequeteR.Compteur) != Compteur ) 
          printf("Message %d doublon !!!!!\n",ntohl(UneRequeteR.Compteur)) ;
     else
        break ;
   }
   fprintf(stderr,"Reçu bytes:%d:Compteur %d:%s\n",rc,ntohl(UneRequeteR.Compteur),UneRequeteR.Message ) ;
   Compteur++ ;
   sleep(5) ;
 }

 close(Desc) ;
}
