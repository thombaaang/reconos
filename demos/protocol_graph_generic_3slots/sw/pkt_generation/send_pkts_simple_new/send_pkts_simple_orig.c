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
#include<curses.h>

#include <termios.h>
#include <fcntl.h>

#define ETH_INTERFACE "eth1"
#define BUF_SIZE 1600
#define UP 0
#define DOWN 1

long unsigned int hash = 0xabababababababab;
unsigned int packet_len = 250;
unsigned int upper_packet_rate = 1000;  // pps
unsigned int lower_packet_rate = 1;    // pps
unsigned int interval = 20;  // in s

char sendbuf[BUF_SIZE];
int sock_send;
struct ifreq if_idx;
struct ifreq if_mac;
struct sockaddr_ll socket_address;
struct ether_header *eh = (struct ether_header *) sendbuf;
int tx_len = sizeof(struct ether_header);

//! check if specific key has been hit
int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	//if(ch != EOF)
	if(ch == '\n' || ch == 27 || ch=='q'|| ch=='Q')
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}

//! parse parameters
int parse_parameter(int argc, char **argv)
{
	int c;
	
	// parse parameter
	while ((c = getopt (argc, argv, "s:p:i:u:l:h")) != -1)
	{
		switch (c)
		{
			case 's':
				sscanf(optarg, "%lx", &hash);
				break;
			case 'p':
				sscanf(optarg, "%u", &packet_len);
				if (packet_len < 64)
				{
					printf("Error: packet length < 64 bytes (%d)\n", packet_len);
					return -2;
				}
				if (packet_len > 1500)
				{
					printf("Error: packet length > 1500 bytes (%d)\n", packet_len);
					return -2;
				}
				break;
			case 'i':
				sscanf(optarg, "%u", &interval);
				break;
			case 'u':
				sscanf(optarg, "%u", &upper_packet_rate);
				if (upper_packet_rate == 0)
				{
					printf("Error: upper packet rate == 0 (%d)\n", upper_packet_rate);
					return -2;
				}
				break;
			case 'l':
				sscanf(optarg, "%u", &lower_packet_rate);
				if (lower_packet_rate == 0)
				{
					printf("Error: lower packet rate == 0 (%d)\n", lower_packet_rate);
					return -2;
				}
				break;
			case 'h':
				printf("program usage\n");
				printf("  -s \thash value (64 bit)\n");
				printf("  -p \tpacket length (in bytes)\n");
				printf("  -l \tlower packet rate (pps)\n");	
				printf("  -u \tupper packet rate (pps)\n");
				printf("  -i \tinterval between lower and upper packet rate (in sec.)\n");
				return -1;
			default:
				break;
		}
	}

	if (upper_packet_rate<lower_packet_rate)
	{
		printf("Error: lower packet rate (%d) > upper packet rate (%d)\n", lower_packet_rate, upper_packet_rate);
		return -2;
	}

	printf("parameter:\n");
	printf("----------------------------------------------\n");
	printf("hash: %lx\n", hash);
	printf("packet length: %d\n", packet_len);
	printf("interval: %d\n", interval);
	printf("upper packet rate (pps): %d\n", upper_packet_rate);
	printf("lower packet rate (pps): %d\n", lower_packet_rate);
	printf("----------------------------------------------\n\n");
	return 0;
}


//! send a packet 
void send_packet( int size)
{
	// send packet
	if (sendto(sock_send, sendbuf, size, 0,(struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
		printf("error: send packet failed\n");

}


//! send packet stream
/*void send_packet_stream_old()
{
	int i;
	double current_rate =  lower_packet_rate;
	struct timeval t_start, t_stop, t_meas;
	int current_direction = UP;

	printf("start packet rate =  %d pps\n\n", (int)current_rate);
	printf("press [ENTER], [ESC] or [q] to quit\n\n");

	while(!kbhit())
	{

		printf("%d pps\n", (int)current_rate);
		gettimeofday(&t_start, NULL);

		// send current_rate packets...
		for (i=0;i<current_rate;i++)
		{
			send_packet(tx_len);
		}

		if (current_direction == UP)
		{
			current_rate += ((double)(upper_packet_rate-lower_packet_rate))/interval;
		} 
		else 
		{
			current_rate -= ((double)(upper_packet_rate-lower_packet_rate))/interval;
		}		

		if (current_rate > upper_packet_rate)
		{
			current_direction = DOWN;
			current_rate = upper_packet_rate;
		} 
		else  if (current_rate < lower_packet_rate) 
		{
			current_direction = UP;
			current_rate = lower_packet_rate;
		}

		gettimeofday(&t_stop, NULL);
		timersub(&t_stop, &t_start, &t_meas);

		if (t_meas.tv_sec < 1)
		{

			usleep((1000000-t_meas.tv_usec));
		}		
	}
}*/



//! send packet stream
void send_packet_stream()
{
	int i;
	double current_rate =  lower_packet_rate;
	struct timeval t_start, t_stop, t_meas;
	int current_direction = UP;
	double time_between_packets = 1000.0; // in microseconds
	long unsigned int measured_time = 0;

	printf("start packet rate =  %d pps\n\n", (int)current_rate);
	printf("press [ENTER], [ESC] or [q] to quit\n\n");
	gettimeofday(&t_start, NULL);

	while(!kbhit())
	{
		printf("%d pps\n", (int)current_rate);
		//gettimeofday(&t_start, NULL);

		send_packet(tx_len);

		gettimeofday(&t_stop, NULL);
		timersub(&t_stop, &t_start, &t_meas);

		// compute interpacket time needed for current packet rate
		time_between_packets = ((double) 1000000.0 )/current_rate;
		// wait until this time is up
		
		measured_time = t_meas.tv_usec + (1000000 * t_meas.tv_sec); 
		if (measured_time < time_between_packets)
		{
			usleep((time_between_packets-measured_time));
		}
		//else
		//{
		//	printf("too slow\n");
		//}

		gettimeofday(&t_start, NULL);

		// update current packet rate
		if (current_direction == UP)
		{
			current_rate += ((double) time_between_packets/1000000)*((double)(upper_packet_rate-lower_packet_rate))/interval;
		} 
		else 
		{
			current_rate -= ((double) time_between_packets/1000000)*((double)(upper_packet_rate-lower_packet_rate))/interval;
		}		

		if (current_rate > upper_packet_rate)
		{
			current_direction = DOWN;
			current_rate = upper_packet_rate;
		} 
		else  if (current_rate < lower_packet_rate) 
		{
			current_direction = UP;
			current_rate = lower_packet_rate;
		}
	}
}


//! main function
int main(int argc, char **argv)
{
	
	int ret = parse_parameter(argc,argv);
	if (ret<0) return ret;

	int data_size,i;
        int counter = 0;
        unsigned char *buffer = (unsigned char *) malloc(BUF_SIZE); 
	//int tx_len = sizeof(struct ether_header);
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
	if ((sock_send = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1)
	{
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
	//memset(&sendbuf[tx_len], 0, BUF_SIZE-tx_len);
	memset(&sendbuf[tx_len-1], 0, BUF_SIZE-tx_len);
	
	// set hash value
	//memcpy(&sendbuf[tx_len], &hash, 8);
	memcpy(&sendbuf[tx_len-1], &hash, 8);
	tx_len+=8;

	// set packet payload (14 byte MAC header, 8 byte hash)
	for (i=0;i<(packet_len-8-14);i++)
	{
		sendbuf[tx_len++] = (char) 0xa1; //(rand() % 256 + 1);
	}

	// send packet stream
	send_packet_stream();
	
	// close sockets
        close(sock_recv);
	close(sock_send);

	printf("Finished\n");
	return 0;
}


