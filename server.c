#include "server.h"

/* Include all kinds of socket & networking headers ... */
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/stat.h>
/* ... and good old unistd.h ... */
#include <unistd.h>
#include <sys/unistd.h>
/* ... and good old stdio and string headers */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* OK, finally we're done with all those includes */

#define DOCROOT "/srv/aim/"
#define SEND_BOUNDARY send(cs, boundary, strlen(boundary), 0);

int main(int argc, char **argv, char **env) {
  struct sockaddr *mysockaddr;
  struct sockaddr *client_addr;
  struct sockaddr_in myaddr_in;
  int sfd;
  /* Create a client socket file descriptor. */
  int csfd;
  int errval, so_reuseaddr;

  /* Initialize structs with 0 */
  bzero((char *) &mysockaddr, sizeof(mysockaddr));
  bzero((char *) &client_addr, sizeof(client_addr));

  socklen_t client_addr_size = sizeof(struct sockaddr);

  sfd = socket(PF_INET, SOCK_STREAM, 0);
  /* Fill in information about our new socket. */
  bzero((char *) &myaddr_in, sizeof(myaddr_in));
  myaddr_in.sin_family = AF_INET;
  myaddr_in.sin_port = htons(4779);
  myaddr_in.sin_addr.s_addr = inet_addr("0.0.0.0");
  

  if (bind(sfd, (struct sockaddr *)&myaddr_in, sizeof myaddr_in) == -1) {
    errval = errno;
    printf("Bind failed.\n%s\n", strerror(errval));
    return 1;
  }
  setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof so_reuseaddr);
  /* Listen for incoming connections. */
  bzero((char *) &mysockaddr, sizeof(mysockaddr));
  bzero((char *) &client_addr, sizeof(client_addr));
  while (1) {
  
    listen(sfd, 20);
    fprintf(stderr, "listening on sfd  for csfd's...\n");
    /* Accept incoming connections detected with listen() and put them on csfd. */
    csfd = accept(sfd, client_addr, &client_addr_size);
    if (csfd == -1) { errval = errno; fprintf(stderr, "Red Reera! accept() failed\n%s\n", strerror(errval)); }
    fprintf(stderr, "... csfd (%d) accepted\n", csfd);
    /* 
    char read_buf[1024];
    int n = 0;
    bzero(read_buf, 1024);
    n = read(csfd, read_buf, 1024);
    if (n > 0) printf("read: %s\n", read_buf);
    */
    attp_impl(sfd, csfd);
    fprintf(stderr, "...Done with (%d)\n", csfd);
    if (shutdown(csfd, SHUT_RDWR) == -1) fprintf(stderr, "shutdown error.\n");
    if (close(csfd) == -1) fprintf(stderr, "close(csfd) error.\n");
  }

  if (close(sfd) == -1) fprintf(stderr, "close(sfd) error.\n");
  return 1;
  fprintf(stderr, "csfd (%d) accepted\n", csfd);
}

int attp_impl(int ss, int cs) {
  /* Implement Connection Setup, Part 2 */
  /* Create a constant string. */
  const char *conest = "ATTP\\10 Connection Established\n";
  int conest_len = strlen(conest);
  send(cs, conest , conest_len, 0);
  /* Prepare to recv() in a client command with a buffer. */
  char cbuf[1024];
  /* Create a while (1) loop to keep from closing after a client command */
  while (1) {
    /* bzero() cbuf to make way for the Client!! */
    bzero(cbuf, 1024);
    /* recv() the client's input to the buffer created earlier. */
    recv(cs, cbuf, 1024, 0);
    /* strtok() cbuf with ' ' as delimiter */
    char cwords[4][1024];
    char *token;
    int token_count = 0;
    token = strtok(cbuf, " ");
    while (token != NULL) {
      bzero(cwords[token_count], 1024);
      strncpy(cwords[token_count], token, strlen(token));
      token_count++;
      token = strtok(NULL, " ");
    }
    const char *res_100 = "ATTP\\10 100 OK\n";
    const char *res_101 = "ATTP\\10 101 Is A Directory\n";
    const char *res_121 = "ATTP\\10 121 Server Busy\n";
    const char *res_122 = "ATTP\\10 122 File Busy\n";
    const char *res_123 = "ATTP\\10 123 Not Found\n";
    const char *res_124 = "ATTP\\10 124 Permission Denied\n";
    const char *res_125 = "ATTP\\10 125 Unknown Error\n";
    const char *res_126 = "ATTP\\10 126 Unknown Command\n";
    const char *res_190 = "ATTP\\10 190 Connection Closed\n";
    const char *pver = "ATTP\\10\n";
    if (strncmp(cwords[0], "FETCH", 5) == 0) {
      if (token_count != 3) {
        send(cs, res_126, strlen(res_126), 0);
        continue;
      }
      fprintf(stderr, "FETCH cword[1] = %s \n", cwords[1]);
      /* Check for malicious input: remove .. from string.  */
      char tmp_str[1024];
      int pos1 = 1;
      int pos2 = 1;
      strncpy(tmp_str, cwords[1], 1024);
      while (tmp_str[pos1] != '\0') {
        if (tmp_str[pos1] == '.' && tmp_str[pos1 - 1] == '.') {
          pos1++;
          pos2--;
        }
        cwords[1][pos2] = tmp_str[pos1];
        pos1++;
        pos2++;
      }
      cwords[1][pos2] = '\0';
      fprintf(stderr, "pos2: %d", pos2);
      
      FILE *fd;
      char *goodfile;
      goodfile = malloc(1024);
      snprintf(goodfile, 1024, "%s%s", DOCROOT, cwords[1]);
      fprintf(stderr, "Fetching this absolute path: %s\n", goodfile);
      fd = fopen(goodfile, "r");
      int i;
      char boundary[32];
      for (i=0; i < 30; i++) {
        boundary[i] = (rand() % 220) + 33;
      }
      boundary[30] = '\n';
      boundary[31] = '\0';

      if (fd == NULL) {
        fprintf(stderr, "E: %s errno: %d\n", strerror(errno), errno);
        if (errno == EACCES) send(cs, res_124, strlen(res_124), 0);
        if (errno == EMFILE) { send(cs, res_121, strlen(res_121), 0); close(cs); }
        if (errno == ETXTBSY) send(cs, res_122, strlen(res_122), 0);
        if (errno == ENOENT) send(cs, res_123, strlen(res_123), 0);
      }
      else if (isDirectory(fd)) {
        send(cs, res_101, strlen(res_101), 0);
        SEND_BOUNDARY
        
        DIR *dirp;
        char sendbuf[1024];
        struct dirent *dirconts;
        dirp = fdopendir(fileno(fd));
        dirconts = readdir(dirp);
        printf(".\n");
        while ((dirconts = readdir(dirp)) != NULL) {
          snprintf(sendbuf, 1024, "%s\n", dirconts->d_name);
          send(cs, sendbuf, strlen(sendbuf), 0);
        }

        SEND_BOUNDARY
      }
      else {
        send(cs, boundary, strlen(boundary), 0);
        int bytes_sent;
        while (0 < sendfile(cs, fileno(fd), NULL, 1024)) {}
        /* 
        while (!feof(fd)) {
          stuff_read = fread(buf, sizeof(char), 4096, fd);
          if (stuff_read) {
            send(cs, buf, strlen(buf), 0);
          
          sendfile(cs, fileno(fd), NULL, 1024);
        }
        */
        send(cs, boundary, strlen(boundary), 0);
      }
    }
    else if (strncmp(cwords[0], "EXIT", 4) == 0) {
      send(cs, res_190, strlen(res_190), 0);
      break;
    }
    else if (strncmp(cwords[0], "VERSION", 7) == 0) {
      send(cs, pver, strlen(pver), 0); 
    } else { 
      send(cs, res_126, strlen(res_126), 0);
    }
  }
  fprintf(stderr, "Returning from attp_impl,\n");
  return 0;
}

int isDirectory(FILE* fd) {
  int status;
  struct stat st_buf;
  status = fstat(fileno(fd), &st_buf);
  if (status != 0) {
    fprintf(stderr, "fstat error in isDirectory()");
    return 0;
  }
  if (S_ISDIR(st_buf.st_mode)) {
    return 1;
  } else {
    return 0;
  }
}

