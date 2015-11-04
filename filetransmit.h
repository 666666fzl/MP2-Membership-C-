#ifndef FILETRANSMIT_H
#define FILETRANSMIT_H

#include <sys/types.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string> 
#include <string.h> 
#include <ifaddrs.h>
#include <unistd.h>

using namespace std;


int receivePutRequest(int sockfd, char* buf, uint32_t len, std::string& sender);

void putFile(int out_fd, std::string localfilename, std::string sdfsfilename, std::string& add, int port, char* buf, uint32_t len);

string receiveGetRequest(int sockfd, char* buf, uint32_t len, std::string& sender);

void getFile(int sock_fd, std::string sdfsfilename, std::string localfilename, char* buf, uint32_t len);

void replyGetRequest(int sockfd, string sdfsfilename); 

void deleteFile(int sock_fd, string sdfsfilename);

void receiveDeleteRequest(int sockfd); 

#endif