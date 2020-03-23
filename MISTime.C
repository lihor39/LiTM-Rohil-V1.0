// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <string.h>
#include "gettime.h"
#include "utils.h"
#include "graph.h"
#include "parallel.h"
#include <typeinfo>
#include "IO.h"
#include "graphIO.h"
#include "parseCommandLine.h"
#include "MIS.h"
using namespace std;
using namespace benchIO;

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8080 
int primOrRep;
//#include <stdio.h> 
//#include <sys/socket.h> 
#include <arpa/inet.h> 
//#include <unistd.h> 
//#include <string.h> 
//#define PORT 8080 

int batchSize;

void timeMIS(graph<intT> G, int rounds, char* outFile) {
  graph<intT> H = G.copy(); //because MIS might modify graph
  char* flags = maximalIndependentSet(H);
  printf("\n HERE \n");
  printf("round is: ");
  printf("%d", rounds);
  for (int i=0; i < rounds; i++) {
	cout << flags;
	printf("\n LOOP \n");
	//for (int z =0; z < len(flags._array[]); z++){
	uint64_t check;
	std::istringstream iss(flags);
	iss >> check;
	//int r = atoi(flags);
	printf("\n This is flags:");
	// cout << flags;
	printf(flags);
	cout << flags[0];
	printf("\n this was flags \n");
	//}
	
	printf(typeid(flags).name());
    free(flags);
    H.del();
    H = G.copy();
    startTime();	
    flags = maximalIndependentSet(H);
    nextTimeN();
	printf("\n Flags: ");
	cout << flags;
	cout << "\n that was flags";
  }
  printf("\n DONE");
  cout << endl;

  if (outFile != NULL) {
    int* F = newA(int, G.n);
    for (int i=0; i < G.n; i++) 
    {
      F[i] = flags[i];
    }
    writeIntArrayToFile(F, G.n, outFile);
	printf("reached11");
    free(F);
  }

  free(flags);
  G.del();
  H.del();
}

int runAsPrimary(char* name){
  printf("running as primary");
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *fileName = name; 
	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
	send(sock , fileName , strlen(fileName) , 0 ); 
	printf("Filename message sent\n"); 
	valread = read( sock , buffer, 1024); 
	printf("%s\n",buffer ); 
	return 0;
}

int runAsReplica(){
  printf("running as replica");
  int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *ack = (char*) string("acknowledge").c_str(); 
	
	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, 
								sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		perror("accept"); 
		exit(EXIT_FAILURE); 
	} 
	valread = read( new_socket , buffer, 1024); 
	printf("%s\n",buffer ); 
	send(new_socket , ack , strlen(ack) , 0 ); 
	printf("Acknowledge message sent\n"); 
	return 0;
}

int parallel_main(int argc, char* argv[]) {
  commandLine P(argc, argv, "[-o <outFile>] [-p <primary>] [-r <rounds>] [-b <batchSize>] [-c <threadCount>] <inFile>");
  printf("reached22");
  char* iFile = P.getArgument(0);
  char* oFile = P.getOptionValue("-o");
  int rounds = P.getOptionIntValue("-r",1);
  int primary = P.getOptionIntValue("-p", 1);
  batchSize = P.getOptionIntValue("-b", 10000);
  int cilkThreadCount = P.getOptionIntValue("-c", -1);
  primOrRep = primary;
  if(primary == 1){
    runAsPrimary(iFile);
  } else {
    runAsReplica();
  }
  if(cilkThreadCount > 0)
	{
		//std::string s = std::to_string(cilkThreadCount);
		char num[3];
		sprintf(num,"%d",cilkThreadCount);
		__cilkrts_end_cilk();
		__cilkrts_set_param("nworkers", num);
		__cilkrts_init();
		std::cout << "The number of threads " << cilkThreadCount << std::endl;
	}

  graph<intT> G = readGraphFromFile<intT>(iFile);
  timeMIS(G, rounds, oFile);
}