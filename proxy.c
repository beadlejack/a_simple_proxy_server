/*
 * Jackson Beadle
 * COEN 317 - Programming Assignment 1
 *
 * Socket programming adapted in part from the
 * SimpleEchoSocket example posted on Camino.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024


/*
 * error()
 */

void error(char *msg) {
  perror(msg);
  exit(1);
}


int main(int argc, char **argv) {

  int listenfd; /* listening socket */
  int connfd; /* connection socket */
  int destfd; /* dest socket */
  int portno;
  int clientlen;
  struct sockaddr_in serveraddr; /* server addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct sockaddr_in destaddr; /* dest addr */
  struct hostent *hostp; /* client host info */
  struct hostent *dest; /* dest host info  */
  char buf[BUFSIZE];
  //char temp[BUFSIZE];
  char out[BUFSIZE];
  char *hostaddrp;
  int optval;
  int n, m;
  char *document_root;


  /* check command line args */
  if (argc != 5) {
    fprintf(stderr, "usage: %s -document_root \"path/to/folder\" -port <portno>", argv[0]);
    exit(1);
  }

  /* read portno and document_root */
  if (strcmp("-port", argv[1]) == 0)  {
    portno = atoi(argv[2]);
    document_root = argv[4];
  } else {
    portno = atoi(argv[4]);
    document_root = argv[2];
  }

  /* printfs to make sure we're reading parameters correctly */
  printf("document_root: %s ", document_root);
  printf("portno: %d\n", portno);


  /* socket: create a socket */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0)
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets
  * us rerun the server immediately after we kill it;
  * otherwise we have to wait about 20 secs.
  * Eliminates "ERROR on binding: Address already in use" error.
  * Take from SimpleEchoServer example.
  */
  optval = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));


  /* build server's internet address */
  /* build the server's internet address */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET; /* we are using the Internet */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* accept reqs to any IP addr */
  serveraddr.sin_port = htons((unsigned short)portno); /* port to listen on */

  /* bind() */
  if (bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    error("Error on bind()");

  /* listen() - give it 5 requests*/
  if (listen(listenfd, 5) < 0)
    error("Error on listen()");


  /* main loop */
  clientlen = sizeof(clientaddr);
  while (1) {

    connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (connfd < 0)
      error("Error on accept()");

    /* gethostbyaddr to ID client */
    hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
        sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("Error on gethostbyaddr() (w/ client)");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("Error on inet_ntoa\n");
    printf("Server established with %s (IP: %s)\n", hostp->h_name, hostaddrp);


    /* now we are connected to the client */

    /* read() from client */
    bzero(buf, BUFSIZE);
    n = read(connfd, buf, BUFSIZE);
    if (n < 0)
      error("Error on read() from socket");
    printf("received %d bytes: %s", n, buf);

    char t1[20], t2[200], t3[15], t4[200];
    char *temp;
    sscanf(buf, "%s %s %s", t1, t2, t3);

    if (strncmp(t1, "GET", 3) == 0)
      printf("GET parsed successfully\n");
    else
      error("Error: malformed HTTP, no GET");

    if ((strncmp(t2, "http://", 7) == 0) || (strncmp(t2, "https://", 8) == 0)) {
      temp = strtok(t2, "//");
      temp = strtok(NULL, "/");

      sprintf(t2, "%s", temp);
      printf("host = %s\n", t2);

      dest = gethostbyname(t2);
      if (dest == NULL) {
        error("Error on gethostbyname() to dest");
      }

      temp = strtok(NULL, " ");
      if (temp != NULL)
        sprintf(t4, "/%s", temp);
      else
        sprintf(t4, "/index.html");
      printf("resource = %s\n" , t4);
    } else {
      error("Error: malformed HTTP, invalid resource");
    }

    if ((strncmp(t3, "HTTP/1.0", 8) == 0) || (strncmp(t3, "HTTP/1.1", 8) == 0)) {
      printf("HTTP type parsed successfully\n");
    } else {
      error("Error: malformed HTTP, no type spec");
    }

    /* okay we have parsed the GET line and we have a dest name */
    /* time to setup connection to actual server */
    destfd = socket(AF_INET, SOCK_STREAM, 0);
    if (destfd < 0)
      error("Error opening socket for dest");

    bzero((char *) &destaddr, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    bcopy((char *)dest->h_addr,
    (char *)&destaddr.sin_addr.s_addr, dest->h_length);
    destaddr.sin_port = htons(80);

    if (connect(destfd, &destaddr, sizeof(destaddr)) < 0)
      error("Error on connect() to dest");

    bzero(buf, BUFSIZE);
    sprintf(buf, "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",t4, t2);
    printf("buffer: %s\n", buf);

    /* write() to dest */
    m = write(destfd, buf, strlen(buf));
    if (m < 0)
      error("Error on write() to dest");

    /* read() from dest */
    do {
      bzero(buf, BUFSIZE);
      m = read(destfd, buf, BUFSIZE);
      if (m >= 0)
        write(connfd, buf, strlen(buf));
    } while (m > 0);

    /* write() to client */
    char *str = "we wrote\n";
    n = write(connfd, str, strlen(str));
    if (n < 0)
      error("Error on write() to socket");



    close(connfd);


  }

  return 0;
}
