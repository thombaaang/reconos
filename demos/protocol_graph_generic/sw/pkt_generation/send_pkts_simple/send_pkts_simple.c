#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h>                     //For standard things
#include<stdlib.h>                    //malloc
#include<string.h>                    //strlen 
#include<netinet/if_ether.h>          //For ETH_P_ALL
#include<net/ethernet.h>              //For ether_header
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>

#define ETH_INTERFACE "eth1"
#define BUF_SIZE 1600
#define PAYLOAD_LEN 250

char sendbuf[BUF_SIZE];
int sock_send;
struct ifreq if_idx;
struct ifreq if_mac;
struct sockaddr_ll socket_address;
struct ether_header *eh = (struct ether_header *) sendbuf;


//! send a packet 
void send_packet( int size){
	/*int i,tx_len = sizeof(struct ether_header);
	
	// do not change the Ethernet header
	memset(&sendbuf[tx_len], 0, BUF_SIZE-tx_len);
	// set parameter
	for (i=0;i<PAYLOAD_LEN;i++){
		sendbuf[tx_len++] = (char) 1; //(rand() % 256 + 1);
	}*/
	// send packet
	if (sendto(sock_send, sendbuf, size, 0,(struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
		printf("error: send packet failed\n");

}

//! main function
int main()
{
	int data_size, ret,i;
        int counter = 0;
	long int counter2 = 0;
        unsigned char *buffer = (unsigned char *) malloc(1514); 
	int tx_len = sizeof(struct ether_header);
	int sock_recv;
        //Receive Packets Part 
        sock_recv = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
        if(sock_recv < 0)
        {
  	      //Print the error with proper message
        	perror("Socket Error");
       		return 1;
        }
        ret = setsockopt(sock_recv , SOL_SOCKET , SO_BINDTODEVICE , ETH_INTERFACE , strlen("eth1")+ 1 );
        if (ret < 0)
        {
        	perror("setsockopt");
        	return 1;
        }

	// socket to send packets /////////////////////////////////////////////////////////////
	// Open RAW socket to send on
	if ((sock_send = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1) {
		perror("socket");
	}
	// Get the index of the interface to send on
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, ETH_INTERFACE, IFNAMSIZ-1);
	if (ioctl(sock_send, SIOCGIFINDEX, &if_idx) < 0)
		perror("SIOCGIFINDEX");
	// Get the MAC address of the interface to send on
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, ETH_INTERFACE, IFNAMSIZ-1);
	if (ioctl(sock_send, SIOCGIFHWADDR, &if_mac) < 0)
		perror("SIOCGIFHWADDR");

	memset(sendbuf, 0, BUF_SIZE);

	// Index of the network device
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	// Address length
	socket_address.sll_halen = ETH_ALEN;
	// Destination MAC (here: broadcast address = ff:ff:ff:ff:ff:ff)
	memset(socket_address.sll_addr,0xff,6);
	// Ethernet header: SRC MAC
	memcpy(eh->ether_shost,&if_mac.ifr_hwaddr.sa_data,6);
	memset(eh->ether_dhost,0x01,6);
	// Ethertype field
	eh->ether_type = htons(0xACDC); //htons(ETH_P_IP);
	
	// do not change the Ethernet header
	memset(&sendbuf[tx_len], 0, BUF_SIZE-tx_len);
	// set packet payload
	for (i=0;i<PAYLOAD_LEN;i++){
		sendbuf[tx_len++] = (char) 1; //(rand() % 256 + 1);
	}

	// send packets
	for (i=0;i<1000;i++){
		send_packet();
		//nanosleep(...);
	}

        close(sock_recv);
	close(sock_send);


	printf("Finished (%ld)\n",counter2);
	return 0;
}


