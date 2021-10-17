#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <functional>
#include <limits.h>
#include <vector>
#include <string>
#include <bits/stdc++.h>
using namespace std;
int npackets=0;
//#define MAXLINE 100000
char RIpadd[100];
int rportnp,mpl,pgr,maxp,wsize,bsize;
int sf=0;
int sn=0;
int flag =0;
int flago=0;
//int counti[MAXLINE] = {0};
int sockfd;
//std::vector<std::string> data;
//int counti[MAXLINE] = {0};
//int ackpac[MAXLINE] = {0};
//int rtt[MAXLINE] = {0};
double avgrtt = 0;
int noack=0;
int notrans=0;
pthread_mutex_t loc;
pthread_mutex_t loc1;
void timer_start(std::function<void(void)> func)
{
    std::thread([func]() {
            func();
    }).detach();
}
void Timer_start(std::function<void(void)> func, unsigned int interval)
{
    std::thread([func, interval]() {
        while (true)
        {
            auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
            func();
            std::this_thread::sleep_until(x);
        }
    }).detach();
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
//Timer t[MAXLINE];
//Timer T[MAXLINE];
//double avgrtt=0;
struct data1{
   string msgi;
   int counti;
   //int ackpac;
   Timer T;
   Timer t;
};
vector<data1> store;
void packetgen(){
if(npackets < bsize)
{
        string msg = "";
         msg += to_string(npackets);
	  int leno = msg.size();
           msg+= " ";
	 for(int i=0;i<(mpl-leno);i++)
   {
	      msg+= "a";
	  }
        data1 x;
        x.msgi= msg;
        x.counti =0;
        x.t.reset();
        x.T.reset();
       store.push_back(x);
        npackets ++;
}

}
void sendi(){
while(1){

  pthread_mutex_lock(&loc);

 if((sn-sf)< wsize && (sn-sf)<store.size()){ 
      //cout<<"IN SEND "<<sn<<" "<<store.size()<<"\n";
    notrans++;
    char hello[10000] ;
    bzero(hello,10000);
    strcpy(hello,store[sn].msgi.c_str());
    store[sn].t.reset();
    struct sockaddr_in     servaddr;
    int PORT = rportnp;
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(RIpadd);
    sendto(sockfd, (const char *)hello, strlen(hello),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
   // cout<<"SENT "<<hello<<endl;
    store[sn].t.reset();
    store[sn].T.reset();
    store[sn].counti++;
     sn++;
  }
  pthread_mutex_unlock(&loc);
  if(flago==1) break;
 }
}
void receive(){
   while(1){
     if(store.size() != 0)
   {     // cout<<"IN RECIVE \n";
   int r;
   char buffer[256];
   socklen_t len;
  // int sockfd;
    struct sockaddr_in     cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
   int x = recvfrom(sockfd, (char *)buffer, 1024, 
                MSG_WAITALL, (struct sockaddr *) &cliaddr,
                &len);
    buffer[x] = '\0';

   // cout<<buffer<<endl;
    pthread_mutex_lock(&loc);
    pthread_mutex_lock(&loc1);
   if(strncmp(buffer,"ACK",3)==0){
          noack++;
          char a[100];
          bzero(a,100);
          strcpy(a,buffer+4);
          r = atoi(a);
         // cout<<r<<endl;
          sn--;
avgrtt = (avgrtt*(noack-1) + store[sf].T.elapsed())/noack ;
          store[sf].t.reset();
  if(flag==1) cout<<"Seq "<<r-1<<": Time Generated:"<<store[sf].T.elapsed()<<" RTT:"<<avgrtt<<" Number of attempts:"<<store[sf].counti<<"\n";
          store.erase(store.begin());
          
         if(noack>=maxp) flago=1; 
         }
        pthread_mutex_unlock(&loc);
        pthread_mutex_unlock(&loc1);
  if(flago==1) break;
  }
   }
}
void timeexpire(){
while(1){
  pthread_mutex_lock(&loc1);
   if(store.size() != 0)
     {       if(store[sf].counti>=5) flago=1;
            if(noack<10){
           	 if(store[sf].t.elapsed() > 100 && store[sf].counti<5){
                     store[sf].t.reset();
                    //store[sf].counti++;
                      sn=0;
                     
                    }
              }else{
                   if(store[sf].t.elapsed() > 2*avgrtt && store[sf].counti<5 ){
                        store[sf].t.reset();
                    //store[sf].counti++;
                      sn=0;
                         
                         }
                   }
       if(flago==1) break;
  }
  pthread_mutex_unlock(&loc1);
}
}
int main(int argc, char **argv)
{
int i=1;
while(i<argc)
{
      if(strcmp(argv[i],"-d")==0) 
      {
          flag=1;
          i = i+1;
      }
      else
      {
		      if(strcmp(argv[i],"-s")==0) strcpy(RIpadd,argv[i+1]);
		      if(strcmp(argv[i],"-p")==0) rportnp = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-l")==0) mpl = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-r")==0) pgr = atof(argv[i+1]);
		      if(strcmp(argv[i],"-n")==0) maxp = atoi(argv[i+1]); 
		      if(strcmp(argv[i],"-w")==0) wsize = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-b")==0) bsize = atoi(argv[i+1]);
		      i = i+2;
      }
}
//cout<<RIpadd<<rportnp<<mpl<<pgr<<maxp<<wsize<<bsize<<endl;
 // int sockfd;
    struct sockaddr_in     servaddr;
    int PORT = rportnp;
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
     servaddr.sin_addr.s_addr = INADDR_ANY;
    //  int x = 100;
    //   while(x--)
    //     packetgen();
    int n, len;
     Timer_start(std::bind(packetgen), (1000/pgr));
     timer_start(sendi);
    timer_start(receive);
    timeexpire();
     close(sockfd);
   double rtrf = (notrans*1.0/noack);
   cout<<"RTR:"<<rtrf<<" AVG RTT:"<<avgrtt<<"\n";

}
