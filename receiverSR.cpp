#include <stdio.h>
#include <bits/stdc++.h>
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
#include <math.h>
using namespace std;
#define MAXLINE 10000000
int counti =0;
int nofack[10000] = {0};
int retrans[10000] = {0};
int flag =0;
int rportno,snfl,maxp,wsize,bsize;
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
		return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count()*1000;
	}
};
Timer t[MAXLINE];
double ttaken[MAXLINE] = {0};
int main(int argc, char **argv)
{
int i=1;
while(i<argc){
      if(strcmp(argv[i],"-d")==0) {
             flag=1;
             i++;
             }else{
		      if(strcmp(argv[i],"-p")==0) rportno = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-n")==0) snfl = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-N")==0) maxp = atoi(argv[i+1]); 
		      if(strcmp(argv[i],"-W")==0) wsize = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-B")==0) bsize = atoi(argv[i+1]);
		       if(strcmp(argv[i],"-e")==0) per = atof(argv[i+1]);
		      i = i+2;
             }
     }
int sockfd;
    char buffer[256];
    int PORT= rportno ;
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
     recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        //cout<<buffer<<endl;
    char *s = strtok(buffer," ");
   int seqn = atoi(s);
   retrans[seqn]++;
   if(!nextBool(per)){
           retrans[seqn]=0;
           int pn = (nofack[seqn])*(1<<snfl) + seqn;
           ttaken[pn] = t[seqn].elapsed();
            nofack[seqn]++;
           counti++;
            string s1 = "ACK ";
           s1 += to_string(seqn);
            //cout<<s1<<endl;
	    sendto(sockfd, s1.c_str(), s1.size(), 
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		    len);
           if(retrans[seqn]>=5) break;
            if(counti>=maxp) break;
         }
     
      }
      if(flag==1 ){
                 for(int j=0;j<counti;j++) {cout<<"Seq"<<j%(1<<snfl)<<": Time Received:"<<ttaken[j]<<"\n";}
                }
}

