#include <vector>

using namespace std;

class Potato{
public:
    int playerNum;
    int hopNum;
    int currNum;
    int index;
    int sequence[512];
    int start;
    int next; //will be set to -1 to indicate currNum is 0 by last player

    
    Potato(){
        playerNum = 0;
        hopNum = 0;
        int currNum = 0;
        int index = 0;
        memset(sequence,0,sizeof(sequence));
        int start = 0;
        int next = 0;        
    }
    /*
    Potato(int pnum, int hnum){
        playerNum = pnum;
        hopNum = hnum;
        int currNum = hnum;
        int index = 0;
        sequence.assign(512,0);
        int start = 0;
        int next = 0;
    }
    */
};

class FirstInfo{
public:
    int playerNum;
    int myID;
    int hopNum;

    FirstInfo():playerNum(0),myID(-1),hopNum(0){}
};

class NextInfo{
public:
    int socket_fd;
    struct addrinfo *hostlist;
};

