#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <sys/stat.h>
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
    ofstream f (log_file);//flag
    if(f.is_open()){
        string info;
        for(int i = 0; i < group.size(); i++){
            info = group[i].ip_str + " " + sdfsfilename;
            auto it = find(data.begin(), data.end(), info);
            if(it!=data.end())
                continue;
            f << info << endl;
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
        while(getline(f,temp)){
            Addr_File.push_back(temp);
        }
        f.close();
    }
    f.close();
    return Addr_File;
}

bool delete_from_log(string log_file, vector<string> data, string sdfsfilename)
{
    for(auto it=data.begin(); it!=data.end();)
    {
        if(it->find(sdfsfilename)!=string::npos)
        {
            it = data.erase(it);
        }
        else
            it++;
    }

    ofstream f (log_file);//flag
    if(f.is_open()){
        for(int i = 0; i < data.size(); i++){
            f << data[i] << endl;
        }
    }

}


/*
These two functions deal with get file
*/
bool getFileRequest( string sdfsfilename, string localfilename)
{
    char buf[1024];
    for(int i=0; i < members.size(); i++)
    {
        bzero(buf, 1024);
        int connectionFd;
        connect_to_server(members[i].ip_str.c_str(), port+3, &connectionFd);
       // getFileSocket = listen_socket(getFileSocket);
        getFile(connectionFd, sdfsfilename, localfilename, buf, 1024);
    }

    struct stat file_stat;
    bool changeLog = true;
    if(stat (localfilename.c_str(), &file_stat) != 0)
    {
        printf("FILE %s does not exist\n", sdfsfilename.c_str());
        changeLog = false;
    }
    if(changeLog)
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
        for(int i = 0; i < members.size(); i ++)
        {
            if(members[i].ip_str!=my_ip_str)
            {
                putFileHelper("file_location_log.txt", "file_location_log.txt", members[i].ip_str.c_str());
            }
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
    cout<<"success put "<<sdfsfilename<<endl;
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
    vector<string>data = read_from_log("file_location_log.txt");
    delete_from_log("file_location_log.txt", data, sdfsfilename);

    for(int i=0; i < members.size(); i++)//members
    {
        if(members[i].ip_str!=my_ip_str)
            putFileHelper("file_location_log.txt", "file_location_log.txt", members[i].ip_str.c_str());
    }

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
        cout<<"currently in members "<<members[i].ip_str<<endl;
		machine_names.push_back(members[i].ip_str);
	}
    machine_names.push_back(machine_fail_ip);
	std::sort(machine_names.begin(),machine_names.end());
    for(int i=0;i<machine_names.size();i++){
        cout<<"sorted members are "<<machine_names[i]<<endl;
    }
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

vector<Node> nodeWithoutFile(vector<Node> members, string sdfsfilename, vector<string> data)
{
    vector<Node> group(members.begin(), members.end());
    for(int i = 0; i < data.size(); i ++)
    {
        vector<string> tokens;//every line
        stringstream ss(data[i]); // Insert the string into a stream
        string temp_buf;
        while (ss >> temp_buf)
            tokens.push_back(temp_buf);
        if(tokens[1]==sdfsfilename)
        {
            for(auto it = group.begin(); it!=group.end(); it++)
            {
                if(it->ip_str==tokens[0])
                {
                    group.erase(it);
                    break;
                }
            }
        }
    }

    return group;
}

//members need to contain fail machine for now.
int replica(string machine_fail_ip, string my_ip, vector<Node> members, string log_file, vector<Node> group) {
	bool is_right_machine = closest(members, machine_fail_ip, my_ip);
	cout<<"got here1"<<endl;
	if(!is_right_machine){
		return 0;
	}
    vector<string>data = read_from_log(log_file);
	cout<<"got here2"<<endl;
	//check the document to extract all the document into a vector.
	vector<string> file_to_replicate;
	vector<string> new_file;
	string temp;
	ifstream f(log_file);
	if(f.is_open()){
		while(getline(f,temp)){
            cout<<"temp is "<<temp<<endl;
            temp = temp.substr(temp.find_first_not_of(" "));
            if(temp!="")
            {
    			vector<string> doc;//every line
                stringstream ss(temp); // Insert the string into a stream
                string temp_buf;
                while (ss >> temp_buf)
                    doc.push_back(temp_buf);

    			if(doc[0]==machine_fail_ip){
    				file_to_replicate.push_back(doc[1]);//assume no duplicate
    			}
    			else{//if not fail machine
                    cout<<"after token is "<<temp<<endl;
    				new_file.push_back(temp);
    			}
            }
		}
		f.close();
	}
	cout<<"got here3"<<endl;
	//update log first
	//FILE* s;
	
	ofstream s (log_file);
	//s = fopen(log_file,"w+");
	if(s.is_open()){
		for(int i = 0; i < new_file.size(); i++){
			//fputs(new_file[i] + '\n',f);
			s << new_file[i] << endl;
		}
	}


    
	//get these file from other machines, put them in random.
	for(int i=0; i< file_to_replicate.size();i++ )
    {
        cout<<"replicate file "<<file_to_replicate[i]<<endl;
        file_to_replicate[i] = file_to_replicate[i].substr(file_to_replicate[i].find_first_not_of(" "));
        bool needToPut = false;
        for(int j = 0; j < data.size(); j ++)
        {
            data[j] = data[j].substr(data[j].find_first_not_of(" "));
            if(data[j]!="" && file_to_replicate[i]!="")
            {
                vector<string> tokens;//every line
                stringstream ss(data[j]); // Insert the string into a stream
                string temp_buf;
                while (ss >> temp_buf)
                    tokens.push_back(temp_buf);
                if(tokens[1]==file_to_replicate[i])
                {
                    if(tokens[0]==my_ip_str)
                    {
                        needToPut = true;
                        break;
                    }

                }
            }

        }

        //put file and write to log file;
        if(needToPut)
        {
            cout<<"lonnnnnnnnnngg doinggggg a PUT"<<endl;
            vector<Node> candidates = nodeWithoutFile(members, file_to_replicate[i], data);
            int randIdx = rand()% candidates.size();
            vector<Node> ret;
            ret.push_back(candidates[randIdx]);
            putFileRequest(file_to_replicate[i], file_to_replicate[i], ret);//group?
        }
        else
        {
            cout<<"lonnnnnnnnnngg doinggggg a GET"<<endl;
            getFileRequest(file_to_replicate[i], file_to_replicate[i]);
        }
	
	}
	return 1;
}
