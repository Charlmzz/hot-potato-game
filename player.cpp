#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>

#include "potato.hpp"

using namespace std;
class Player{
public:
    vector<int> fdLists;
    int myID;
    int playerNum;

    Player(): myID(-1), playerNum(0){
        fdLists.push_back(0);
    }

    int connectMaster(const char *hn, const char *p){
        int status;
        int socket_fd;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        const char *hostname = hn;
        const char *port = p;
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0){
            
            cout << "getaddrinfo error 1" << endl;
            exit(1);
        }

        socket_fd = socket(host_info_list->ai_family,
                host_info_list->ai_socktype,
                host_info_list->ai_protocol);
        if (socket_fd == -1){
            cout << "Error: cannot creat socket" << endl;
            exit(1);
        }

        status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status==-1){
            cout << "Error: cannot connect socket" << endl;
            exit(1);
        }
        freeaddrinfo(host_info_list);
        return socket_fd;
    }

    int connectClient(const char* hostname){
        int port = 66666+myID;
        //cout << "my port:";
        //cout << port;
        string s = to_string(port);
        const char *port_right = s.c_str();
        int status_right;
        int socket_fd;
        struct addrinfo host_info_right;
        struct addrinfo *host_info_list_right;
        memset(&host_info_right,0,sizeof(host_info_right));

        host_info_right.ai_family = AF_UNSPEC;
        host_info_right.ai_socktype = SOCK_STREAM;
        host_info_right.ai_flags = AI_PASSIVE;
        const char *hostname_right = NULL;
        status_right = getaddrinfo(hostname_right,port_right,&host_info_right,&host_info_list_right);
        if (status_right != 0){
            cout << "getaddrinfo error 2" << endl;
            exit(1);
        }
        socket_fd = socket(host_info_list_right->ai_family,
                host_info_list_right->ai_socktype,
                host_info_list_right->ai_protocol);
        if (socket_fd == -1){
            cout << "Error: cannot create socket" <<endl;
            exit(1);
        }

        int yes = 1;
        status_right = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status_right = bind(socket_fd, host_info_list_right->ai_addr, host_info_list_right->ai_addrlen);

        if (status_right == -1){
            cout << "Error: cannot bind socket" << endl;
            exit(1);
        }

        status_right = listen(socket_fd, 100);
        if (status_right == -1){
            cout << "Error: cannot listen on socket" << endl;
            exit(1);
        }

        /*
        NextInfo ni;
        recv(fdLists[0], &ni,sizeof(NextInfo),MSG_WAITALL);
        */
        int sig = 0;
        char hm[512];
        recv(fdLists[0],&sig,sizeof(int),MSG_WAITALL);
        recv(fdLists[0], hm, sizeof(hm),MSG_WAITALL);
        int leftPlayerPort = (myID==0)? playerNum-1:myID-1;
        leftPlayerPort+=66666; 
        //cout << "left player port: ";
        //cout << leftPlayerPort <<endl;
        string s1 = to_string(leftPlayerPort);

        //cout << "connecting another player" << endl;
        string str(hm);
        int fd2 = connectMaster(str.c_str(),s1.c_str()); //connect to left as a client
        //NextInfo info = connectMaster(str.c_str(),s1.c_str());
        fdLists[1] = fd2;
        struct sockaddr_storage socket_addr; //accept right
        socklen_t socket_addr_len = sizeof(socket_addr);
        int socket_fd_right = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (socket_fd_right==-1){
            cout << "Error: cannot accept connection on socket" << endl;
            exit(1);
        }
        fdLists[2] = socket_fd_right;
        freeaddrinfo(host_info_list_right);
        return socket_fd_right;
    }

    void initializeConnection(const char *hostname, const char *port){
        int fd1 = connectMaster(hostname,port);
        fdLists[0] = fd1;
        //cout << "1--------" <<endl;
        FirstInfo fi;
        int r = recv(fdLists[0], &fi, sizeof(FirstInfo),MSG_WAITALL);
        //cout << "2--------" <<endl;
        //cout << r << endl;
        //cout << "master fd: " << to_string(fdLists[0]) <<endl;
        myID = fi.myID;
        playerNum = fi.playerNum;
        cout << "Connected as player " << to_string(myID) << " out of "<< to_string(playerNum) << " total players" <<endl;
        char hm[512];
        hm[511] = '\0';
        gethostname(hm, 511);

        /*
        NextInfo ni;
        int nextPort = 66666 + myID;
        ni.leftPlayerPort = nextPort;
        string ip(hostname);
        ni.leftPlayerIP = ip;
        cout << ni.leftPlayerIP <<endl;
        cout << ni.leftPlayerPort <<endl;
        int s = send(fdLists[0], &ni, sizeof(NextInfo),0);
        */
        int signal = 1;
        int s = send(fdLists[0], &signal, sizeof(int),0);
        send(fdLists[0],hm,sizeof(hm),0); //
        //cout << s << endl;
        //cout << "3--------" <<endl;

        string str(hostname);
        int fd3 = connectClient(str.c_str()); //connect to right as host
        //fdLists[2] = fd3;
        //int signal2 = 1;
        playGame();
        
        for (int i=0;i<3;i++){
            close(fdLists[i]);
        }

        //cout << "closed successfully" <<endl;
         //aaaaaaaaaaaa
        //handle free!!!
    }

    void playGame(){
        fd_set readfds;
        bool flag = false;
        Potato potato;
        srand((unsigned int)time(NULL));
        while (!flag){
            FD_ZERO(&readfds);
            int numfds = 0;
            for (int i=0;i<3;i++){
                FD_SET(fdLists[i], &readfds);
                numfds = fdLists[i]>numfds? fdLists[i]:numfds;
            }
            if (select(numfds+1, &readfds, NULL, NULL, NULL)==-1){
                cout << "Select error" << endl;
                exit(1);
            }
            for (int i=0;i<3;i++){
                if (FD_ISSET(fdLists[i], &readfds)){
                    int received = recv(fdLists[i],&potato,sizeof(Potato),MSG_WAITALL);
                    if (potato.next==-1){
                        flag = true;
                        break;
                    }else{
                        potato.sequence[potato.index] = myID;
                        potato.index++;
                        if (potato.currNum==0){
                            potato.next = -1;
                            cout << "I'm it" <<endl;
                            send(fdLists[0],&potato,sizeof(Potato),0);
                        }else{
                            potato.currNum -=1;
                            int random = rand() %2;
                            if (random ==0){
                                potato.next = (myID ==0)? potato.playerNum-1:myID-1;
                                cout << "Sending potato to " << to_string(potato.next) << endl;
                                int s = send(fdLists[1],&potato,sizeof(Potato),0);
                                if (s==-1){
                                    cout << "sending error" << endl;
                                    exit(1);
                                }
                            }else{
                                potato.next = (myID == playerNum -1) ? 0: myID +1;
                                cout << "Sending potato to " << to_string(potato.next) << endl;
                                int s = send(fdLists[2], &potato,sizeof(Potato),0);
                                if (s==-1){
                                    cout << "Sending error" << endl;
                                    exit(1);
                                }
                            }
                        }
                    }
                }

            }
        }
    }
};

int main(int argc, char *argv[]){
    if (argc<3){
        cout << "wrong number of arguments" << endl;
        exit(EXIT_FAILURE);
    }
    Player player;
    player.fdLists.resize(3,0);
    player.initializeConnection(argv[1],argv[2]);
    return 0;
}