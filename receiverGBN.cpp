#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <functional>
#include <limits.h>
using namespace std;
#define MAXLINE 100000
int counti =0;
int flag =0;
//int flago=0;
int rportnp,maxp;
double per;
bool nextBool(double probability)
{
    return (rand() / (double)RAND_MAX) < probability;
}
class Timer
{
private:
	// Type aliases to make accessing nested type easier
	using clock_t = std::chrono::high_resolution_clock;
	using second_t = std::chrono::duration<double, std::ratio<1> >;
	
	std::chrono::time_point<clock_t> m_beg;
 
public:
	Timer() : m_beg(clock_t::now())
	{
	}
	
	void reset()
	{
		m_beg = clock_t::now();
	}
	
	double elapsed() const
	{
		return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
	}
};
Timer t[MAXLINE];
double ttaken[MAXLINE] = {0};
int retrans[MAXLINE] = {0};
int main(int argc, char **argv)
{
int i=1;
while(i<argc){
      if(strcmp(argv[i],"-d")==0) {
             flag=1;
             i++;
             }else{
		      if(strcmp(argv[i],"-p")==0) rportnp = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-n")==0) maxp = atoi(argv[i+1]); 
		       if(strcmp(argv[i],"-e")==0) per = atof(argv[i+1]);
		      i = i+2;
             }
     }
     int sockfd;
    char buffer[256];
    int PORT= rportnp ;
    struct sockaddr_in servaddr, cliaddr;
      
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
      
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
      
    socklen_t len;
  
    len = sizeof(cliaddr);  //len is value/resuslt
    while(1){
      bzero(buffer,256);
      memset(&cliaddr, 0, sizeof(cliaddr));
     int x = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        buffer[x] = '\0';
        //cout<<buffer<<endl;
    char *s = strtok(buffer," ");
    //cout<<"YES"<<endl;
    char nacseq[100];
    bzero(nacseq,100);
    strcpy(nacseq,s);
   int seqn = atoi(s);
    retrans[seqn]++;
   if(!nextBool(per) && seqn == counti){
           ttaken[seqn] = t[seqn].elapsed();
           counti++;
       //cout<<retrans[seqn]<<" "<<counti<<" "<<seqn<<endl;
    string s1 = "ACK ";
    s1 += to_string(counti);
	    sendto(sockfd, s1.c_str(), s1.size(), 
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		    len);
          cout<<"Seq"<<seqn<<": Time Received:"<<ttaken[seqn]<<" Packet dropped: false"<<"\n";
         }
      }
       close(sockfd);
      
}
 
