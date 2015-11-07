#include <iostream>
#include <string> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "filetransmit.h"
#include "constant.h"

/*
    Dealing with Put file and its request
*/
int receivePutRequest(int sockfd, char* buf, uint32_t len, std::string& sender)
{
    struct sockaddr addr;
    socklen_t fromlen = sizeof addr;
    cout<<"receiving"<<endl;
    int byte_count = 0;
    bzero(buf, len);
    FILE * filew;
    //filew=fopen("acopy.txt","wb");
    int numw = 0;
    bool findFileName = false;
    while ((byte_count = recvfrom(sockfd, buf, len, 0, &addr, &fromlen))!=0)
    {
        if(!findFileName)
        {
            string temp(buf);
            string createFileName = "";
            for(int i = 0; i < temp.size(); i ++)
            {
                if(temp[i]==':')
                {
                    findFileName = true;
                    filew=fopen(createFileName.c_str(),"wb");
                    fwrite(buf+i+1,1,byte_count,filew);
                    break;

                }
                else
                {
                    createFileName+=temp[i];
                }
            }
        }
        else
        {
            fwrite(buf,1,byte_count,filew);
        }
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

void putFile(int out_fd, std::string localfilename, std::string sdfsfilename, std::string& add, int port)
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


    sdfsfilename+=":";
    int filename_len = write(out_fd,sdfsfilename.c_str(), strlen(sdfsfilename.c_str()));
    if(filename_len<0) printf("Error: sending filename\n");

    /* open the file to be sent */
    int fd = open(localfilename.c_str(), O_RDONLY);
    if (fd == -1) {
      fprintf(stderr, "unable to open '%s': %s\n", localfilename.c_str(), strerror(errno));
      exit(1);
    }
    /* get the size of the file to be sent */
    fstat(fd, &stat_buf);
    /* copy file using sendfile */
    off_t offset = 0;
    rc = sendfile (out_fd, fd, &offset, stat_buf.st_size);
    close(out_fd);
    close(fd);
    
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

//--------------------------------------------------------------------------------------------------------
/*
    Dealing with Get file and its request
*/
string receiveGetRequest(int sockfd, char* buf, uint32_t len, std::string& sender)
{
    struct sockaddr addr;
    socklen_t fromlen = sizeof addr;
    cout<<"receiving get"<<endl;
    int byte_count = 0;

    string sdfsfilename = "";
    bool findFileName = false;
    while (!findFileName && (byte_count = recvfrom(sockfd, buf, len, 0, &addr, &fromlen))!=0)
    {
        printf("get file %s\n", buf);
        string temp(buf);
        for(int i = 0; i < temp.size(); i ++)
        {
            if(temp[i]==':')
            {
                findFileName = true;
            }
            else
            {
                sdfsfilename+=temp[i];
            }
        }
    }
    if (byte_count == -1)
    {
        printf("ERROR RECEIVING!!!\n");
        exit(-1);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *) &addr;

    sender = inet_ntoa(sin->sin_addr);

    return sdfsfilename;
}

void getFile(int sock_fd, std::string sdfsfilename, std::string localfilename, char* buf, uint32_t len)
{
    struct sockaddr addr;
    socklen_t fromlen = sizeof addr;
    int byte_count = 0;
    sdfsfilename+=":";
    bzero(buf, len);
    int filename_len = write(sock_fd,sdfsfilename.c_str(), strlen(sdfsfilename.c_str()));

    if(filename_len<0) 
        printf("Error: sending filename\n");

    FILE *filew = fopen(localfilename.c_str(), "wb");

    while ((byte_count = recvfrom(sock_fd, buf, len, 0, &addr, &fromlen))!=0)
    {
        printf("%s\n", buf);
        fwrite(buf,1,byte_count,filew);
    }
    close(sock_fd);
    fclose(filew);
}

void replyGetRequest(int sockfd, string sdfsfilename)
{
    int fd = open(sdfsfilename.c_str(), O_RDWR);
    if(fd==-1)
    {
        printf("FILE %s does not exist\n", sdfsfilename.c_str());
    }

    struct stat stat_buf;      /* argument to fstat */
    int rc;                    /* holds return code of system calls */
    /* get the size of the file to be sent */
    fstat(fd, &stat_buf);
    /* copy file using sendfile */
    off_t offset = 0;
    rc = sendfile (sockfd, fd, &offset, stat_buf.st_size);
    close(fd);
    close(sockfd);

}

void deleteFile(int sock_fd, std::string sdfsfilename)
{
    int filename_len = write(sock_fd,sdfsfilename.c_str(), strlen(sdfsfilename.c_str()));

    if(filename_len<0) 
        printf("Error: sending filename\n");

    close(sock_fd);
}

void receiveDeleteRequest(int sockfd)
{
    struct sockaddr addr;
    socklen_t fromlen = sizeof addr;
    cout<<"receiving get"<<endl;
    int byte_count = 0;
    char buf[1024];
    bool findFileName = false;
    while ((byte_count = recv(sockfd, buf, sizeof(buf), 0))!=0)
    {
        
    }
    string sdfsfilename(buf); 
    cout<<sdfsfilename<<endl;
    if (byte_count == -1)
    {
        printf("ERROR RECEIVING!!!\n");
        exit(-1);
    }

    remove(sdfsfilename.c_str());
}