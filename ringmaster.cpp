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

class Ringmaster{
public:
    int playerNum;
    int hopNum;
    vector<int> client_connection_fd;
    //vector<int> sequence;//////
    //vector<string> leftPlayerIP;
    //vector<int> leftPlayerPort;

    
    void connectPlayer(const char *p, Potato potato){
        int status;
        int socket_fd;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        const char *hostname = NULL;
        const char *port = p;
        memset(&host_info,0,sizeof(host_info));
        host_info.ai_family = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags = AI_PASSIVE;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status!=0){
            cout << "getaddrinfo error" <<endl;
            exit(1);
        }

        socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
        if (socket_fd == -1){
            cout << "Error: cannot create socket" << endl;
            exit(1);
        }

        int yes = 1;
        status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(socket_fd,host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1){
            cout << "Error: cannot bind socket" << endl;
            exit(1);
        }

        status = listen(socket_fd,100);
        if (status == -1){
            cout << "Error: cannot listen on socket" << endl;
            exit(1);
        }

        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        client_connection_fd.resize(playerNum,0);
        char hm[playerNum][512];//
        //leftPlayerIP.resize(playerNum);
        //leftPlayerPort.resize(playerNum,0);
        for (int i=0; i<playerNum; i++){
            client_connection_fd[i] = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
            if (client_connection_fd[i]==-1){
                cout << "Error: cannot accept connection on socket" << endl;
                exit(1);
            }
            FirstInfo fi;
            fi.hopNum = hopNum;
            fi.myID = i;
            fi.playerNum = playerNum;
            send(client_connection_fd[i], &fi, sizeof(FirstInfo),0);
            //cout << "info sent" <<endl;
            /*
            int r = recv(client_connection_fd[i],&ni, sizeof(NextInfo),MSG_WAITALL);
            cout << r<< endl;
            cout << client_connection_fd[i] <<endl;
            cout << "info received" <<endl;
            cout << ni.leftPlayerIP << endl;
            cout << ni.leftPlayerPort << endl;
            leftPlayerIP[i] = ni.leftPlayerIP;
            leftPlayerPort[i] = ni.leftPlayerPort;
            */
           int signal = 0;
           recv(client_connection_fd[i], &signal, sizeof(int),MSG_WAITALL);
           recv(client_connection_fd[i],hm[i],sizeof(hm[i]),MSG_WAITALL);
            cout << "Player " << to_string(i) << " is ready to play" << endl;
            if (i>0){
                /*
                NextInfo ni1;
                ni1.leftPlayerIP = leftPlayerIP[i-1];
                ni1.leftPlayerPort = leftPlayerPort[i-1];
                send(client_connection_fd[i], &ni1, sizeof(NextInfo),0);
                */
                int sig = 1;
                send(client_connection_fd[i], &sig, sizeof(int),0);
                send(client_connection_fd[i], hm[i-1], sizeof(hm[i-1]),0); ///
                //cout << "sending next player info" << endl;
            }
            if (i==playerNum-1){
                /*
                NextInfo ni2;
                ni2.leftPlayerIP = leftPlayerIP[i];
                ni2.leftPlayerPort = leftPlayerPort[i];
                send(client_connection_fd[i],&ni2,sizeof(NextInfo),0);
                */
                int sig = 1;
                send(client_connection_fd[0], &sig, sizeof(int),0);
                send(client_connection_fd[0],hm[i],sizeof(hm[i]),0); ///
                //cout << "sending my info to first player" << endl;
            }
        }
        playGame(potato);
        freeaddrinfo(host_info_list);
        close(socket_fd);
    }

    void playGame(Potato potato){
        if (hopNum==0){
            cout << "hop num is 0. Closing the game." <<endl;
            potato.next = -1;
            closeGame(potato);
        }else{
            srand((unsigned int)time(NULL));
            int random = rand()%playerNum;
            potato.start = random;
            potato.next = random;
            potato.currNum -=1;
            potato.index = 0;
            //cout << random<<endl;
            cout << "Ready to start the game, sending potato to player " << random<< endl;
            //cout << potato.sequence.size()<<endl;
            //cout << client_connection_fd[random] <<endl;
            send(client_connection_fd[random], &potato,sizeof(Potato),0);
            Potato endPotato;
            endPotato = waitResponse();
            //closeGame(endPotato);
            //cout << "Im here" <<endl;
            if (endPotato.next!=-1){
                cout << "Potato Error" << endl;
                exit(1);
            }
            for (int i=0;i<playerNum;i++){
                //cout << "Im here 2" <<endl;
                send(client_connection_fd[i], &endPotato, sizeof(Potato),0);
            }
            cout << "Trace of potato:" << endl;
            //cout << endPotato.sequence.size() <<endl;
            //cout << endPotato.sequence[0] <<endl;
            //cout << endPotato.sequence[1] <<endl;
            //cout << endPotato.sequence[2] <<endl;
            for (int i=0; i<hopNum; i++){
                cout << endPotato.sequence[i];
                if (i<hopNum-1){
                    cout << ",";
                }
            }
            cout << "\n";
        }
        
    }

    Potato waitResponse(){
        fd_set readfds;
        int numfds = 0;
        Potato backPotato;
        FD_ZERO(&readfds);
        for (int i=0;i<playerNum;i++){
            FD_SET(client_connection_fd[i],&readfds);
            numfds = client_connection_fd[i]>numfds ? client_connection_fd[i]:numfds;
        }
        if (select(numfds+1, &readfds, NULL, NULL, NULL) ==-1){
            cout << "Select error" << endl;
            exit(EXIT_FAILURE);
        }
        for (int i=0;i<playerNum;i++){
            if (FD_ISSET(client_connection_fd[i], &readfds)){
                recv(client_connection_fd[i], &backPotato, sizeof(backPotato), MSG_WAITALL);
                //cout << "end potato received" <<endl;
                //cout << backPotato.next << endl;
            }
        }
        return backPotato;

    }

    void closeGame(Potato potato){
        if (potato.next!=-1){
            cout << "Potato Error" << endl;
            exit(1);
        }
        for (int i=0;i<playerNum;i++){
            send(client_connection_fd[i], &potato, sizeof(Potato),0);
        }
    }

};

int main(int argc, char*argv[]){
    if (argc!=4){
        std::cout << "wrong number of arguments\n" <<endl;
        exit(EXIT_FAILURE);
    }
    
    int playerNum = atoi(argv[2]);
    int hopNum = atoi(argv[3]);
    if (playerNum<=1 || hopNum<0 || hopNum>512){
        std::cout << "Wrong player or hop number\n" << endl;
        exit(EXIT_FAILURE);
    }
    Potato potato;
    potato.currNum = hopNum;
    potato.hopNum = hopNum;
    //potato.index = 0;
    potato.playerNum = playerNum;
    //potato.sequence.resize(hopNum,0);
    std::cout << "Potato Ringmaster" << endl;
    std::cout << "Players = " << to_string(playerNum) <<endl;
    std::cout << "Hops = " << to_string(hopNum) <<endl;

    const char *port = argv[1];
    Ringmaster rm;
    rm.playerNum = playerNum;
    rm.hopNum = hopNum;
    rm.connectPlayer(port,potato);
    return 0;
}