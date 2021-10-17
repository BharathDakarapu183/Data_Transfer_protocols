#include <stdio.h>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
#include <math.h>
using namespace std;
#define MAXLINE 100000
char RIpadd[100];
int rportno,snfl,mpl,pgr,maxp,wsize,bsize;
int npackets=0;
int seqn;
int sf=0;
int sn=0;
//int slider=0;
int flag =0;
int flago=0;
int sockfd;
pthread_mutex_t loc;
pthread_mutex_t loc1;
//std::vector<std::string> data;
//int counti[MAXLINE] = {0};
//int ackpac[MAXLINE] = {0};
double avgrtt = 0;
int noack=0;
int notrans=0;
int times =10;
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
Timer tim;
//Timer T[MAXLINE];
struct data1{
   string msgi;
   int counti;
   int ackpac;
   Timer T;
   Timer t;
   int seq;
};
vector<data1> store;
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
int rand(int lower, int upper, int count)
{
    int num;
    for (int j = 0; j < count; j++)
    {
        num = (rand() %
               (upper - lower + 1)) +
              lower;
        // printf("%d ", num);
    }
   // cout<<lower<<upper<<num<<"\n";
    return num;
}
void packetgen(){
    // cout<<"IN PACKET GEN\n";
if(npackets < bsize) {
     int randi = rand(40,mpl,1);
        string msg = "";
	seqn = npackets%(1<<snfl) ;
         msg = to_string(seqn);
	  int leno = msg.size();
           msg+= " ";
	 for(int i=0;i<(randi-leno);i++){
	      msg+= "a";
	     }
       data1 x;
        x.msgi= msg;
        x.counti =0;
        x.ackpac=0;
        x.seq = seqn;
        x.t.reset();
        x.T.reset();
       store.push_back(x);
        npackets ++;
     }

}
void sendi(){
 while(1){
   pthread_mutex_lock(&loc);
   //cout<<notrans<<noack<<endl;
 if((sn-sf)< wsize && (sn-sf)<store.size()){ 
     // cout<<"IN SEND \n";
    notrans++;
    char hello[10000];
    bzero(hello,10000);
   strcpy(hello,store[sn].msgi.c_str());
     //cout<<hello<<endl;
    store[sn].t.reset();
    //int sockfd;
    struct sockaddr_in     servaddr;
    int PORT = rportno;
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(RIpadd);
    sendto(sockfd, (const char *)hello, strlen(hello),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
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
     // cout<<notrans<<noack<<endl;
       int in;
      // cout<<"IN RECIVE \n";
   if(store.size() != 0 ){
   int r,slk;
   char buffer[256];
   socklen_t len;
  // int sockfd;
    struct sockaddr_in     cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
   int x = recvfrom(sockfd, (char *)buffer, 1024, 
                MSG_WAITALL, (struct sockaddr *) &cliaddr,
                &len);
    buffer[x] = '\0';

    //cout<<buffer<<endl;
    pthread_mutex_lock(&loc);
    pthread_mutex_lock(&loc1);
   if(strncmp(buffer,"ACK",3)==0){
          noack++;
          char a[100];
          bzero(a,100);
          strcpy(a,buffer+4);
          r = atoi(a);
          for(in=0;in<wsize;in++){
                 if(store[in].seq ==r) break;
                 }
         // cout<<r<<endl;
          store[in].ackpac=1;
           avgrtt = (avgrtt*(noack-1) + store[in].T.elapsed())/noack ;
           //cout<<avgrtt<<"AVGRTT"<<endl;
          store[in].t.reset();
           if(flag==1 && noack<=maxp) cout<<"Seq "<<r<<": Time Generated:"<<tim.elapsed()<<" RTT:"<<avgrtt<<" Number of attempts:"<<store[in].counti<<"\n";
          int p=0;
          while(store[p].ackpac==1){
                sn--;
                p++;
                store.erase(store.begin());
               }
         if(noack>=maxp) {//cout<<"noack"<<noack<<endl;
                         flago=1;} 
         }
         pthread_mutex_unlock(&loc);
        pthread_mutex_unlock(&loc1);
  if(flago==1) break;
   }
  }
}
void timerexpire(){
  //cout<<"timer"<<endl;
  while(1){
      pthread_mutex_lock(&loc1);
      if(store.size()!=0)
    { 
        for(int i=0;i<(sn);i++) { 
            if(store[i].counti>=10) {//cout<<"noack"<<store[i].counti<<endl;
                                     flago=1;}
            if(store[i].seq<10 && times ==10){
           	 if(store[i].t.elapsed() > 300 && store[i].counti<10 && store[i].ackpac==0){
                    // cout<<"In ReSend"<<store[i].ackpac<<endl;
                     notrans++;
                     store[i].t.reset();
                     store[i].counti++;
                      char hello[10000];
                       bzero(hello,10000);
                      strcpy(hello,store[i].msgi.c_str());
                            // int sockfd;
			     struct sockaddr_in     servaddr;
			    int PORT = rportno;
			    memset(&servaddr, 0, sizeof(servaddr));
			      
			    // Filling server information
			    servaddr.sin_family = AF_INET;
			    servaddr.sin_port = htons(PORT);
			    servaddr.sin_addr.s_addr = inet_addr(RIpadd);
			    sendto(sockfd, (const char *)hello, strlen(hello),
				MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
				    sizeof(servaddr));
                     
                    }
              }else{
                    times++;
                   if(store[i].t.elapsed() > 10*avgrtt && store[i].counti<10 && store[i].ackpac==0){
                       // cout<<"In ReSend1"<<store[i].ackpac<<endl;
                     notrans++;
                     store[i].t.reset();
                     store[i].counti++;
                      char hello[10000];
                       bzero(hello,10000);
                      strcpy(hello,store[i].msgi.c_str());
                            // int sockfd;
			     struct sockaddr_in     servaddr;
			    int PORT = rportno;
			    memset(&servaddr, 0, sizeof(servaddr));
			      
			    // Filling server information
			    servaddr.sin_family = AF_INET;
			    servaddr.sin_port = htons(PORT);
			    servaddr.sin_addr.s_addr = inet_addr(RIpadd);
			    sendto(sockfd, (const char *)hello, strlen(hello),
				MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
				    sizeof(servaddr));
                         
                         }
                   }
            }
       if(flago==1) {break;}
     }
    pthread_mutex_unlock(&loc1);
  }
}
int main(int argc, char **argv)
{
int i=1;
while(i<argc){
      if(strcmp(argv[i],"-d")==0) {
                   flag=1;
                   i = i+1;
                 }else{
		      if(strcmp(argv[i],"-s")==0) {
                                               strcpy(RIpadd,argv[i+1]);
                                               
                                              }
               
		      if(strcmp(argv[i],"-p")==0) rportno = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-n")==0) snfl = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-L")==0) mpl = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-R")==0) pgr = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-N")==0) maxp = atoi(argv[i+1]); 
		      if(strcmp(argv[i],"-W")==0) wsize = atoi(argv[i+1]);
		      if(strcmp(argv[i],"-B")==0) bsize = atoi(argv[i+1]);
		      i = i+2;
          }
     }
   // int sockfd;

   //cout<<rportno<<" "<<RIpadd<<" "<<snfl<<" "<<mpl<<" "<<bsize<<endl;
    struct sockaddr_in     servaddr;
    int PORT = rportno;
  
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
      
    int n, len;
   // int x = 100;
   // while(x--)
     //   packetgen();
 Timer_start(std::bind(packetgen), (1000/pgr));
 timer_start(sendi);
  timer_start(receive);
 timerexpire();
     close(sockfd);
   //cout<<notrans<<noack<<flago<<endl;
   double rtrf = (notrans*1.0/noack);
   cout<<"RTR:"<<rtrf<<" AVG RTT:"<<avgrtt<<"\n";
}
