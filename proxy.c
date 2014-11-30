/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Andrew Carnegie, ac00@cs.cmu.edu 
 *     Harry Q. Bovik, bovik@cs.cmu.edu
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"
#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);


/*
 * Number of pthreads
*/

pthread_t tid[100];
char buffer[MAXBUF];


struct params{
	int newsockfd;
};

/*
 Request Handler function will be use to process all the request
*/

void * requestHandler(void * data){

	char requestBuffer[MAXBUF];
	char responseBuffer[MAXBUF];
	char hostname[MAXLINE];
	char pathname[MAXLINE];
	char URI[MAXLINE];
	char method[5];

	int clientid;


	memset(requestBuffer,0,sizeof(requestBuffer));
	memset(responseBuffer,0,sizeof(responseBuffer));
	rio_t rio_client, rio_server;
	 printf("Thread created ....\n");

	 rio_readinitb(&rio_client, data);
	 rio_readlineb(&rio_client, requestBuffer, MAXLINE);


	 sscanf(requestBuffer, "%s %s", method, URI);

	 int *port=80;


	 parse_uri(URI,hostname,pathname,port);

	 printf("HTTP request from browser: %s\n", requestBuffer);
	 printf("Hostname: %s\n", hostname);
	 printf("PathName: %s\n", pathname);

	 clientid=open_clientfd(hostname,80);

	 rio_writen(&clientid,hostname,MAXLINE);
	 rio_readlineb(&clientid, responseBuffer, MAXLINE);
	 Rio_writen(&rio_client,responseBuffer,MAXBUF);


	 /*	 while(1){

	 		read((int *)data, buffer, MAXBUF);




	 		if(buffer[0]=='Q')
	 			{
	 				buffer[0]='\0';
	 				break;
	 			}
	 		else
	 			{

	 				printf("we heard something %s\n",&buffer);
	 				buffer[0]='\0';
	 				fflush(stdout);
	 			}

	 	 }

*/



	 printf("Since we process all the request, It's time to shut down the thread\n");
	 close(data);
	 pthread_exit(0);

}



/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{

    /* Check arguments */
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }

    pthread_t thread;

    printf("Listnening to port %s\n",argv[1]);

    	int sockfd, newsockfd, portno, clilen;


    	int *newsocketid;

       struct sockaddr_in serv_addr, cli_addr;

       if (argc < 2) {
           fprintf(stderr,"ERROR, no port provided\n");
           exit(1);
       }
       sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
       if (sockfd < 0)
          error("ERROR opening socket");


       bzero((char *) &serv_addr, sizeof(serv_addr));
       portno = atoi(argv[1]);
       serv_addr.sin_family = AF_INET;
       serv_addr.sin_addr.s_addr = INADDR_ANY;
       serv_addr.sin_port = htons(portno);
       if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
                error("ERROR on binding");
       listen(sockfd,5);







       int threadcnt=0;
       struct params s;

       while(1)
       {
    	   clilen = sizeof(cli_addr);
    	   newsocketid = accept(sockfd, (struct processsockaddr *) &cli_addr, &clilen);
    	   if(newsocketid <=0){
    		   // do nothing ..  this is error
    	   }else{

    		   //we will simply start new thread without waiting for it.
    		   pthread_create(&thread, 0, requestHandler, (void *)newsocketid);
    		   pthread_detach(thread);
    		   thread=NULL;

    	   }


       }


       return 0;

}


/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
	hostname[0] = '\0';
	return -1;
    }
       
    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
    /* Extract the port number
    *port = 80;  default
    if (*hostend == ':')   
	*port = atoi(hostend + 1);
    */
    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
	pathname[0] = '\0';
    }
    else {
	pathbegin++;	
	strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s", time_str, a, b, c, d, uri);
}


