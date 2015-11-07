#ifndef FAULT_H
#define FAULT_H

#include <sys/types.h>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string> 
#include <ifaddrs.h>
#include <unistd.h>
#include "constant.h"
#include "filetransmit.h"


using namespace std;

vector<string> read_from_log(string log_file);

bool write_to_log(string log_file, vector<string> data,vector<Node> group, string sdfsfilename);

//return true if current machine is the closest.
bool closest(vector<Node> members, string machine_fail_ip, string my_ip);

//main function to relicate.
int replica(string machine_fail_ip, string my_ip, vector<Node> members, string log_file, vector<Node> group);


bool putFileRequest(string localfilename, string sdfsfilename, vector<Node> group);

void putFileHelper(string localfilename, string sdfsfilename, string desc);

bool getFileRequest( string sdfsfilename, string localfilename);

bool deleteFileRequest( string sdfsfilename);

#endif