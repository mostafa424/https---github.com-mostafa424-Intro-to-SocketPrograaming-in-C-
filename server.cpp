#include <bits/stdc++.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include<thread>
#include <fstream>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h> 
#define PORTNUMBER "3490"
#define MAX_CLIENTS 10
#define TIMEOUT 10
using namespace std;
vector<struct pollfd> clientpolls ;
void handle_command(pair<int,string> connection,int clients);
void handle_get(int descriptor, string path, string params);
void handle_post(int descriptor,string body);
pair<int,string> accept_connection(int socket_descriptor);
int build_socket();
int main() {
    int clients_ctr =0;
    int socket_descriptor; pair<int,string>socket_to_command;int new_file_socket;
    socket_descriptor = build_socket();
    while(1){
        if(clientpolls.size()<MAX_CLIENTS){
            new_file_socket = accept_connection(socket_descriptor);
            struct pollfd newfd; newfd.fd = new_file_socket;
            newfd.events = POLL_IN;
            clientpolls.push_back(newfd);
            thread command_handler(handle_command,new_file_socket,clientpolls.size());
            command_handler.detach();
        }
        else{
            struct pollfd copyfds [clientpolls.size()];
            copy(clientpolls.begin(),clientpolls.end(),copyfds);
            int poll_count = poll(copyfds,clientpolls.size()+1,-1);
        }
    }
    return 0;
}
void establish_persistent_connection(int socket,int clients){
    float heuristic_timeout = TIMEOUT * (1 - (float)((float)clients / (float)MAX_CLIENTS))
    struct timeval tv;
        tv.tv_sec = heuristic_timeout;
        tv.tv_usec = 0;
        setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

int accept_connection(int socket_descriptor){
    int addrinfoList;int y=1;int new_file_descriptor;
    pair<int,string>res;
    struct sockaddr_storage client_addr;
    __socklen_t input_size ; 
    input_size = sizeof (client_addr); 
    new_file_descriptor = accept(socket_descriptor,(sockaddr*)&client_addr,&input_size);
    if(new_file_descriptor==-1){
        cout<<"failed to connect";exit(-1);
    }
    return new_file_descriptor;
}
int build_socket(){
        struct addrinfo details, *servinfo, *i;
        int socket_descriptor;int y=1;
        memset(&details,0,sizeof (details));
        details.ai_family = AF_INET;
        details.ai_socktype = SOCK_STREAM;
        details.ai_flags = AI_PASSIVE;
        
        if((getaddrinfo("localhost",PORTNUMBER,&details,&servinfo))!=0){
            cout<<"addrinfo returned an error";
            return -1;
        }

        for(i = servinfo;i!=NULL;i=i->ai_next){
            socket_descriptor = socket(i->ai_family,i->ai_socktype,i->ai_protocol);
            if(socket_descriptor==-1){
                continue;
            }
            if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &y,sizeof(int)) == -1) {
                perror("setsockopt");
                exit(1);
            }
            int msg = bind(socket_descriptor,i->ai_addr,i->ai_addrlen);
            if(msg==-1){
                close(socket_descriptor);
                continue;
            }
            break;
        }
        freeaddrinfo(servinfo);
        if(i==NULL){
            cout<<"failed to bind ";
            return -1;
        }
        int l = listen(socket_descriptor,20);
        if(l==-1){
            cout<<"listen failed"<<endl;
        }
        else{
            cout<<"Server ready"<<endl;
        }
        return socket_descriptor;
}
void handle_command(int descriptor,int clients){
    char new_command[1024];
    while(1){
        int new_commnum = recv(descriptor,new_command,1023,0);
        command[new_commnum] = '\0';
        string command = new_command;
        cout<<command<<" request received"<<endl;
        vector<string> args;
        establish_persistent_connection(descriptor,clients);
        args = parse_request(command);
        if(args[0]=="GET"){
            handle_get(descriptor,args.at(1),args.at(2));
        }
        else if(args[0] == "POST"){
            if(send(descriptor,"ok status:200",13,0)==-1){
                cout<<"unexpected disconnection!!"<<endl;return ;
            }
            handle_post(descriptor,command);
        }
        for(int i=0;i<clientpolls.size();i++){
            if(clientpolls.at(i).fd == descriptor){
                clientpolls.erase(clientpolls.begin()+i);
            }
        }
    }
    close(descriptor); 
}
void handle_get(int descriptor,string path,string params){
    ifstream fin(path, ifstream::binary);
    char text_to_be_sent[1024];
    while(!fin.eof()) {
        fin.read(text_to_be_sent,1023);
        int sig = send(descriptor,text_to_be_sent,1024,0);
        if(sig == -1){cout <<"Error during sending file";return;}
    }
}
void handle_post(int descriptor,string body){
    char buffer[1024]; 
    int readbytes;
    int ctr=1;
    string path = get_current_dir_name();
    string actual_path = path +"/files/Post"+to_string(ctr)+".txt";ctr++;
    ofstream bodyfile(actual_path);
    bodyfile.write(body.c_str(),body.size());
    while((readbytes = recv(descriptor,buffer,1023,0))!=-1){
        buffer[readbytes]='\0';
        body = buffer;
        bodyfile.write(body.c_str(),body.size());
    } 
    bodyfile.close();
}
vector<string> parse_request(string command){
        vector<string>args;
        int pos =0;
        string delim =" ";
        pos=command.find(" ");
        args.push_back(command.substr(0,pos)) ;
        command.erase(0, pos + delim.length());
        if(args[0] == "GET"){
            while((pos=command.find(" ")) != string::npos){
                args.push_back(command.substr(0,pos)) ;
                command.erase(0, pos + delim.length());
        }
            if(command.length()!=0){args.push_back(command.substr(0,command.length()));}
            
    }
    return args;
}