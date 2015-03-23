#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>

// ReconOS
#include "reconos.h"
#include "mbox.h"

#include "timing.h"


#define NUM_SLOTS 4

#define HWT_S2H 0
#define HWT_H2S 1
#define HWT_ETH 2
#define HWT_FB1 3

#define TO_H2S 5
#define TO_ETH 4
#define TO_FB1 1

#define PACKET_LENGTH_1 64
#define PACKET_LENGTH_2 84

struct reconos_resource res[NUM_SLOTS][2];
struct reconos_hwt hwt[NUM_SLOTS];
struct mbox mb_in[NUM_SLOTS];
struct mbox mb_out[NUM_SLOTS];

unsigned char *shared_mem_h2s;
unsigned char *shared_mem_s2h;

struct noc_header {
	unsigned char hw_addr_switch:4,
	   hw_addr_block:2,
	   priority:2;
	unsigned char direction:1,
	   latency_critical:1,
	   reserved:6;
	unsigned short payload_len;
	unsigned int src_idp;
	unsigned int dst_idp;
};


void config_eth(unsigned int hash_1, unsigned int hash_2, unsigned int idp, int addr)
{
	int ret = 0;
	unsigned int config_eth_hash_1 = hash_1;
	unsigned int config_eth_hash_2 = hash_2;
	unsigned int config_eth_idp = idp;
	unsigned int config_eth_address = addr; //global 4 bits, local 2 bits 
	mbox_put(&mb_in[HWT_ETH], config_eth_hash_1 );
	mbox_put(&mb_in[HWT_ETH], config_eth_hash_2);
	mbox_put(&mb_in[HWT_ETH], config_eth_idp);
	mbox_put(&mb_in[HWT_ETH], config_eth_address);
	ret = mbox_get(&mb_out[HWT_ETH]);
}

void setup_noc(void)
{
	shared_mem_h2s = malloc(4096); 
	memset(shared_mem_h2s, 0, 4096);
	shared_mem_s2h = malloc(4096); 
	memset(shared_mem_s2h, 0, 4096);
	mbox_put(&mb_in[HWT_H2S], (unsigned int) shared_mem_h2s);
	mbox_put(&mb_in[HWT_S2H], (unsigned int) shared_mem_s2h);
	config_eth(0xabababab, 0xabababab, 3, TO_H2S); // SW
	config_eth(0xcdcdcdcd, 0xcdcdcdcd, 2, TO_FB1); // HW
	//mbox_put(&mb_in[HWT_ETH], TO_H2S );
}



// MAIN ////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	int i, msg_len,cnt=0;
	unsigned char * ptr;
	unsigned char * ptr2;
	unsigned int * ptr_int;
	unsigned int * ptr2_int;
	unsigned int packet_select = 0;

	printf( "-------------------------------------------------------\n"
		    "APP DEMO\n"
		    "(" __FILE__ ")\n"
		    "Compiled on " __DATE__ ", " __TIME__ ".\n"
		    "-------------------------------------------------------\n\n" );

	printf("[app] Initialize ReconOS.\n");
	reconos_init_autodetect();

	printf("[app] Creating delegate threads.\n\n");
	for (i=0; i<NUM_SLOTS; i++){
		// mbox init
		mbox_init(&mb_in[i],  10);
		mbox_init(&mb_out[i], 10);
		// define resources
		res[i][0].type = RECONOS_TYPE_MBOX;
		res[i][0].ptr  = &mb_in[i];	  	
		res[i][1].type = RECONOS_TYPE_MBOX;
		res[i][1].ptr  = &mb_out[i];
		// start delegate threads
		reconos_hwt_setresources(&hwt[i],res[i],2);
		reconos_hwt_create(&hwt[i],i,NULL);
	}

	printf("[app] Setup NoC\n");
	setup_noc();

	ptr = (unsigned char *) shared_mem_s2h;
	ptr2 = (unsigned char *) (shared_mem_s2h+2048);
	ptr_int = (unsigned int *) ptr;
	ptr2_int = (unsigned int *) ptr2;
	printf("[app] Sending packets...\n");

	// generate packet packet 1 - NoC header
	struct noc_header * noc_header_1 = malloc(sizeof(struct noc_header));
	noc_header_1->hw_addr_switch = 1;
	noc_header_1->hw_addr_block = 0;
	noc_header_1->priority = 0;
	noc_header_1->direction = 0;
	noc_header_1->latency_critical = 0;
	noc_header_1->reserved = 0;
	noc_header_1->payload_len = PACKET_LENGTH_1;
	noc_header_1->src_idp = 3;
	noc_header_1->dst_idp = 2;
	memcpy(ptr,noc_header_1,sizeof(struct noc_header));

	// set packet data 1
	for(i=0;i<PACKET_LENGTH_1;i++)
	{
		ptr[i+sizeof(struct noc_header)] = (unsigned char) (i+1);
	}

	printf("\n\n[app] print packet 1\n\t");
	for (i=0;i<((PACKET_LENGTH_1+sizeof(struct noc_header))/4);i++)
	{
		printf("0x%08x  ",ptr_int[i]);
		if ((i+1)%4==0)
		{ 
			printf("\n\t");
		}
	}

	// generate packet packet 2 - NoC header
	struct noc_header * noc_header_2 = malloc(sizeof(struct noc_header));
	noc_header_2->hw_addr_switch = 1;
	noc_header_2->hw_addr_block = 0;
	noc_header_2->priority = 0;
	noc_header_2->direction = 0;
	noc_header_2->latency_critical = 0;
	noc_header_2->reserved = 0;
	noc_header_2->payload_len = PACKET_LENGTH_2;
	noc_header_2->src_idp = 2;
	noc_header_2->dst_idp = 3;
	memcpy(ptr2,noc_header_2,sizeof(struct noc_header));

	// set packet data 2
	for(i=0;i<PACKET_LENGTH_2;i++)
	{
		ptr2[i+sizeof(struct noc_header)] = (unsigned char) (i+20);
	}

	printf("\n\n[app] print packet 2\n\t");
	for (i=0;i<((PACKET_LENGTH_2+sizeof(struct noc_header))/4);i++)
	{
		printf("0x%08x  ",ptr2_int[i]);
		if ((i+1)%4==0)
		{ 
			printf("\n\t");
		}
	}

	printf("\n\n");

	// flush cache
	reconos_cache_flush();

	// send packets
	while(1) 
	{
		// flush cache
		//reconos_cache_flush();

		// 1. send length
		if (packet_select==0)
			mbox_put(&mb_in[HWT_S2H], (unsigned int) (PACKET_LENGTH_1+12)); // NoC header (12 bytes) + packet length 1
		else
			mbox_put(&mb_in[HWT_S2H], (unsigned int) (PACKET_LENGTH_2+12)); // NoC header (12 bytes) + packet length 2

		// 2. wait for ack
		msg_len = mbox_get(&mb_out[HWT_S2H]);

		// 3. set next s2h shared mem addr
		if (packet_select==0)
		{
			packet_select=1;
			mbox_put(&mb_in[HWT_S2H], (unsigned int) ptr2);
		}
		else
		{
			packet_select=0;
			mbox_put(&mb_in[HWT_S2H], (unsigned int) ptr);
		}

		cnt++;
		printf("sent packet no. %05d\n", cnt);

	}
	printf("\n");
	return 0;
}

