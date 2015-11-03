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

void putFile(int sockfd, std::string filename, std::string sdfsfilename, std::string& add, int port, char* buf, uint32_t len);




#endif