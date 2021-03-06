/* 
*
* receiver.c -- a datagram sockets 'client'
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "udp_stream.h"


// Get sockaddr, IPv4 or IPv6
void *getinaddr(struct sockaddr *sa)
{
    // See if this is IPv4
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    // Else get the IPv6 address
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Start listening to messages on PORT
int receiver_start() {
    
    int sockfd; // Socket file descriptor
    struct addrinfo hints, *servinfo;
    int rv;
    char s[INET6_ADDRSTRLEN];               // Use INET6 length since this will work for IPv4 too.
    unsigned long framecount = 0; 
    unsigned long bytecount = 0;

    // Clear hints and fill out relevant fields
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;            // Unspecified
    hints.ai_socktype = SOCK_DGRAM;         // UDP
    hints.ai_flags = AI_PASSIVE;            // Use the server's IP 

    // Get address info
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    } 

    // Bind to to socket
    if ((rv = receiver_bindsocket(&sockfd, servinfo)) != 0)
    {
        fprintf(stderr, "bindsocket: failed to bind socket\n");
        return rv;
    } 

    printf("client: sockfd = %d\n", sockfd);

    // Free up the servinfo struct now that we are bound
    freeaddrinfo(servinfo);
    printf("client: waiting for recvfrom...\n");

    // Receive packets
    while((rv = receiver_receiveframe(&sockfd, &bytecount)) != -1)
    {
        // Success
        //return rv;
        framecount++;
        if ((framecount % 100000) == 0)
        {
            printf("receiver: received %lu frames\n", bytecount);
        }
    }
    
    // Else exit with failure
    close(sockfd);
    exit(1);

}

// Bind with error checking
int receiver_bindsocket(int *sfd, struct addrinfo *si)
{
    // Bind call return status
    int stat;
    struct addrinfo *p;

    // Loop through all possible sockets given our addinfo and bind 
    // the first one we can
    for (p = si; p != NULL; p->ai_next)
    {
        if ((*sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }
        
        // Bind the socket to PORT
        if (stat = bind(*sfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(*sfd);
            perror("client: bind");
            return stat;
        }
        
        break;
        
    }
    
    if (p == NULL) 
    {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    // Success
    printf("client: bound to port %s\n",PORT);
    return stat;
}

// Receive message
int receiver_receive(int *sfd, unsigned long *bytecount)
{

    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len = sizeof(their_addr);
    int numbytes;
    char s[INET6_ADDRSTRLEN];           // Use INET6 length since this will work for IPv4 too.

    if ((numbytes = recvfrom(*sfd, buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        return numbytes;
    }
    
    *bytecount += numbytes;

    printf("client: got packet from %s\n", 
            inet_ntop(their_addr.ss_family,
            getinaddr((struct sockaddr *)&their_addr),
            s,
            sizeof(s)));
    printf("client: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("client: packet contains \"%s\"\n",buf);

    return 0;
}

int receiver_receiveframe(int *sfd, unsigned long *bytecount) 
{
    struct sockaddr_storage their_addr;
    float buf[MAXBUFLEN];
    socklen_t addr_len = sizeof(their_addr);
    int numbytes;
    char s[INET6_ADDRSTRLEN];           // Use INET6 length since this will work for IPv4 too.

    if ((numbytes = recvfrom(*sfd, &buf, MAXBUFLEN-1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        return numbytes;
    }
    
    *bytecount += (unsigned long)numbytes;

//    printf("client: got packet from %s\n", 
//            inet_ntop(their_addr.ss_family,
//            getinaddr((struct sockaddr *)&their_addr),
//            s,
//            sizeof(s)));
//    printf("client: packet is %d bytes long\n", numbytes);
//    buf[numbytes] = '\0';
//    printf("client: packet contains\"\n");
//    
//    int i;
//    for(i = 0; i < NCHAN*BUFSAMP; i++)
//    {
//        printf("%f\n",buf[i]);
//    }

    return 0;
    
}
