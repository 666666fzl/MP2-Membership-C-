#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <stdlib.h>
#include "fault_replica.h"
#include "connections.h"

extern int port;
extern vector<Node> members;  //store members in the group
extern int putFileSocket;
extern int listenFileSocket;
extern int getFileSocket;
extern int deleteFileSocket;
extern string my_ip_str;

bool write_to_log(string log_file, vector<string> data,vector<Node> group, string sdfsfilename){
    /*
    File* f;
    f = fopen(log_file,"w+");
    if(f != NULL){
        string info;
        for(int i = 0; i < group.size(); i++){
            info = group[i].ip_str + " " + sdfsilename + "\n";
            fputs(info,f);
        }
        for(int i = 0; i < data.size(); i++){
            fputs(data[i],f);
        }
    }
    */
    
    ofstream f (log_file);//flag
    if(f.is_open()){
        string info;
        for(int i = 0; i < group.size(); i++){
            info = group[i].ip_str + " " + sdfsfilename + "\n";
            f << info;
        }
        for(int i = 0; i < data.size(); i++){
            f << data[i] << endl;
        }

    }
    else{
        f.close();
        return false;
    }
    f.close();
    return true;

}

vector<string> read_from_log(string log_file){
    vector<string> Addr_File;
    string temp;
    ifstream f(log_file);
    if(f.is_open()){
        while(!f.eof()){
            temp="";
            getline(f,temp);
            Addr_File.push_back(temp);
        }
        f.close();
    }
    f.close();
    return Addr_File;
}




/*
These two functions deal with get file
*/
bool getFileRequest( string sdfsfilename, string localfilename)
{
    vector<string> data;
    vector<Node>group;
    for(int i = 0; i < members.size(); i ++)
    {
        if(members[i].ip_str==my_ip_str)
        {
            group.push_back(members[i]);
            break;
        }
    }
    data = read_from_log("file_location_log.txt");
    write_to_log("file_location_log.txt", data, group, sdfsfilename);

    char buf[1024];
    for(int i=0; i < members.size(); i++)
    {
        bzero(buf, 1024);
        int connectionFd;
        connect_to_server(members[i].ip_str.c_str(), port+3, &connectionFd);
       // getFileSocket = listen_socket(getFileSocket);
        getFile(connectionFd, sdfsfilename, localfilename, buf, 1024);
        if(members[i].ip_str!=my_ip_str)
        {
            putFileHelper("file_location_log.txt", "file_location_log.txt", members[i].ip_str.c_str());
        }
    }

    return true;
}

//////




/*
These two functions deal with put file
*/
void putFileHelper(string localfilename, string sdfsfilename, string desc)
{
    int connectionFd;
    connect_to_server(desc.c_str(), port+2, &connectionFd);//members
    putFile(connectionFd, localfilename, sdfsfilename, desc, port+2);//members
    cout<<"success put"<<endl;
}

bool putFileRequest(string localfilename, string sdfsfilename, vector<Node> group)
{

    vector<string> data;
    data = read_from_log("file_location_log.txt");
    write_to_log("file_location_log.txt", data, group, sdfsfilename);

    for(int i=0; i < members.size(); i++)//members
    {
        if(members[i].ip_str!=my_ip_str)
            putFileHelper("file_location_log.txt", "file_location_log.txt", members[i].ip_str.c_str());
    }

    for(int i=0; i < group.size(); i++)//members
    {
        putFileHelper(localfilename, sdfsfilename, group[i].ip_str.c_str());
    }
    return true;
}

/*
These two functions deal with delete file
*/
bool deleteFileRequest( string sdfsfilename)
{
    char buf[1024];
    for(int i=0; i < members.size(); i++)
    {
        bzero(buf, 1024);
        int connectionFd;
        connect_to_server(members[i].ip_str.c_str(), port+4, &connectionFd);
       // getFileSocket = listen_socket(getFileSocket);
        deleteFile(connectionFd, sdfsfilename);
    }
    return true;
}

bool closest(vector<Node> members, string machine_fail_ip, string my_ip){
	vector<string> machine_names;
	for(int i=0;i<members.size();i++){
		machine_names.push_back(members[i].ip_str);
	}
	std::sort(machine_names.begin(),machine_names.end());
	for(int i=0;i<machine_names.size();i++){
		if((machine_names[i]==machine_fail_ip) && (i != machine_names.size() -1) ){
			return (machine_names[i+1]==my_ip); 
		}
		else if((machine_names[i]==machine_fail_ip) && (i == machine_names.size() -1)){
			return (machine_names[0]==my_ip); 
		}
	}
	return false;//true if it is closest using the member list.
}

//members need to contain fail machine for now.
int replica(string machine_fail_ip, string my_ip, vector<Node> members, string log_file, vector<Node> group) {
	bool is_right_machine = closest(members, machine_fail_ip, my_ip);
	cout<<"got here"<<endl;
	if(!is_right_machine){
		return 0;
	}
	cout<<"got here"<<endl;
	//check the document to extract all the document into a vector.
	vector<string> file_to_replicate;
	vector<string> new_file;
	string temp;
	ifstream f(log_file);
	if(f.is_open()){
		while(!f.eof()){
			getline(f,temp);
			vector<string> doc;//every line
			std::istringstream buf(temp);
			std::istream_iterator<std::string> beg(buf), end;
			std::vector<std::string> tokens(beg, end); // done!
			for(auto& s: tokens){
				doc.push_back(s);
				//std::cout <<  s << '\n';
			}
			if(doc[0]==machine_fail_ip){
				file_to_replicate.push_back(doc[1]);//assume no duplicate
			}
			else{//if not fail machine
				new_file.push_back(temp);
			}
		}
		f.close();
	}
	
	//update log first
	//FILE* s;
	
	ofstream s (log_file);
	//s = fopen(log_file,"w+");
	if(s.is_open()){
		for(int i = 0; i < new_file.size(); i++){
			//fputs(new_file[i] + '\n',f);
			s << new_file[i] + '\n';
		}
	}

	//get these file from other machines, put them in random.
	for(int i=0; i< file_to_replicate.size();i++ ){
		getFileRequest(file_to_replicate[i], file_to_replicate[i]);
		//put file and write to log file;
		putFileRequest(file_to_replicate[i], file_to_replicate[i], group);//group?
		
		
		//ask file from each of the server.
		
		
		
		
		
	}
	return 1;
}
