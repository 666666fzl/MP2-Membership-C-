#include <iostream>
#include <string> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "filetransmit.h"
#include "constant.h"


int receivePutRequest(int sockfd, char* buf, uint32_t len, std::string& sender)
{
    struct sockaddr addr;
    socklen_t fromlen = sizeof addr;

    int byte_count = 0;

    FILE * filew;
    filew=fopen("acopy.txt","wb");
    int numw = 0;
    while ((byte_count = recvfrom(sockfd, buf, len, 0, &addr, &fromlen))!=0)
    {
        while ((numw=fwrite(buf+numw,1,byte_count-numw,filew)) != 0);
    }
    fclose(filew);

    if (byte_count == -1)
    {
        printf("ERROR RECEIVING!!!\n");
        exit(-1);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *) &addr;

    sender = inet_ntoa(sin->sin_addr);

    return byte_count;
}

void putFile(int out_fd, std::string filename, std::string& add, int port, char* buf, uint32_t len)
{
    struct sockaddr_in servaddr,cliaddr;
    struct hostent *server;
    struct stat stat_buf;      /* argument to fstat */
    int rc;                    /* holds return code of system calls */

    server = gethostbyname(add.c_str());

    if(server == NULL)
    {
        std::cout << "Host does not exist" << std::endl;
        exit(1);
    }

    memset((char *) &servaddr,0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    memcpy((char *) &servaddr.sin_addr.s_addr,(char *) server -> h_addr, server -> h_length);
    servaddr.sin_port = htons(port);

    /* open the file to be sent */
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "unable to open '%s': %s\n", filename.c_str(), strerror(errno));
      exit(1);
    }

    /* get the size of the file to be sent */
    fstat(fd, &stat_buf);

    /* copy file using sendfile */
    off_t offset = 0;
    rc = sendfile (fd, out_fd, offset, &(stat_buf.st_size), NULL, 0);
    if (rc == -1) {
      fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
      exit(1);
    }
    if (rc != stat_buf.st_size) {
      fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
              rc,
              (int)stat_buf.st_size);
      exit(1);
    }
}

