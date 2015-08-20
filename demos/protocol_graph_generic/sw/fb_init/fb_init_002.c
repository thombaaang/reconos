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


// number of hardware threads
#define NUM_SLOTS 4

// delegate threads (harddware threads)
#define HWT_S2H 0
#define HWT_H2S 1
#define HWT_ETH 2
#define HWT_FB1 3

#define TO_H2S 5  // 0001 01
#define TO_ETH 4  // 0001 00
#define TO_FB1 1  // 0000 01

// command constants
// control
#define CMD_EXIT_THREAD     0xffffffff
#define CMD_GET_ADDR        0x00000001
#define CMD_PUT_ADDR 		0x0000000a
#define CMD_PUT_FB_ID       0x00000002
// GET parameters user_logic
#define CMD_GET_ALL         0x00000003
// PUT stats user_logic
#define CMD_PUT_STATS       0x00000004
// PUT parameters user_logic
#define CMD_PUT_DPP         0x00000005
#define CMD_PUT_DPB         0x00000006
#define CMD_PUT_DT          0x00000007
#define CMD_PUT_DV          0x00000008
#define CMD_PUT_SEED        0x00000009

struct reconos_resource res[NUM_SLOTS][2];
struct reconos_hwt hwt[NUM_SLOTS];
struct mbox mb_in[NUM_SLOTS];
struct mbox mb_out[NUM_SLOTS];

unsigned char *shared_mem_h2s;
unsigned char *shared_mem_s2h;


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
}

// FB1 functions /////////////////////////////////////

void set_addr_fb1(unsigned int global_addr, unsigned int local_addr) {
    // set global and local addr
    unsigned int addr = 0;
    addr |= global_addr << 2;
    addr |= local_addr << 0;
	mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_GET_ADDR);
	mbox_put(&mb_in[HWT_FB1], (unsigned int) addr);
}

void init_fb1(unsigned int delay_per_packet_h, unsigned int delay_per_packet_l, unsigned int delay_per_byte_h, 
              unsigned int delay_per_l, unsigned int drop_type, unsigned int drop_value, unsigned int randgen_seed) {
    // put command to set all initial values for FB1
	mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_GET_ALL);
	// set delay_per_packet high byte
	mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_packet_h);
	// set delay_per_packet low byte
	mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_packet_l);
	// set delay_per_byte high byte
	mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_byte_h);
	// set delay_per_byte low byte
	mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_l);
	// set drop_type
	mbox_put(&mb_in[HWT_FB1], (unsigned int) drop_type);
	if (drop_type == 2) {
	    // set randgen_seed (not necessary if drop_type != 2)
	    mbox_put(&mb_in[HWT_FB1], (unsigned int) randgen_seed);
	}
	if (drop_type != 0) {
	    // set delay_value (not necessary if drop_type = 0)
	    mbox_put(&mb_in[HWT_FB1], (unsigned int) drop_value);
	}
}

void get_stats_fb1(unsigned int *packets_arrived, unsigned int *packets_forwarded) {
    // put command to set all initial values for FB1
	mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_PUT_STATS);
	// get stats
    *packets_arrived = mbox_get(&mb_out[HWT_FB1]);
    *packets_forwarded = mbox_get(&mb_out[HWT_FB1]);
}

void get_addr_fb1(unsigned int *global_addr, unsigned int *local_addr) {
    unsigned int addr = 0;
    // put command to get addr
	mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_PUT_ADDR);
	// get addr
    addr = mbox_get(&mb_out[HWT_FB1]);
    // clear unused parts
    *global_addr = (addr >> 2);
    *local_addr = (addr & ~(0xbb << 2));
}

int get_dt_fb1(void) {
    // put command to get addr
	mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_PUT_DT);
	// get addr
    return mbox_get(&mb_out[HWT_FB1]);
}

// MAIN ////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	int i, cnt=1, msg_len;
	int max_cnt = 2;
	char print_flag = 0;
	unsigned int * ptr = (unsigned int *) shared_mem_h2s;
	unsigned int packets_arrived = 999;
	unsigned int packets_forwarded = 999;

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
	
	
	// FB1 ///////////////
	
	// set global and local addr of FB1
    printf("[app] Setting global and local address.\n");
    set_addr_fb1(0x1, 0x1);
    
    // print current global and local address
    unsigned int global_addr, local_addr;
    get_addr_fb1(&global_addr, &local_addr);
    printf("global address: 0x%01x, local address: 0x%01x\n", global_addr, local_addr);
    
    if (argc >= 5) {
        // initialize FB1 (set all parameters)
        // delay_per_packet_h, delay_per_packet_l, delay_per_byte_h, delay_per_byte_l, drop_type, drop_value, randgen_seed
        init_fb1(0x00000000, (unsigned int) strtoul(argv[1], NULL, 10), 0x00000000, (unsigned int) strtoul(argv[2], NULL, 10), 
                 (unsigned int) strtoul(argv[3], NULL, 10), (unsigned int) strtoul(argv[4], NULL, 10), 0x74d9a2fb);
    }
    else {
        // initialize FB1 (set all parameters)
        // delay_per_packet_h, delay_per_packet_l, delay_per_byte_h, delay_per_byte_l, drop_type, drop_value, randgen_seed
        init_fb1(0x00000000, 0x00000002, 0x00000000, 0x00000001, 0x00000001, 0x00000001, 0x74d9a2fb);
    }
    
    if (argc == 6) {
       // print option is present 
       print_flag = atoi(argv[5]);
    }
    
    // print drop type read from fb
    printf("drop type: 0x%08x\n", get_dt_fb1());    
	
	/*
	// continously print stats
	while(1) {
	    get_stats_fb1(&packets_arrived, &packets_forwarded);  
	    printf("[stats] arrived: %d, forwarded: %d, dropped %d\n", packets_arrived, packets_forwarded, packets_arrived - packets_forwarded);
	    sleep(2);
	}
	*/
	
	
	
	ptr = (unsigned int *) shared_mem_h2s;
	printf("[app] Waiting for packets...\n");
	
	// receive packets
	while(1) {
		// receive message from h2s
		msg_len = mbox_get(&mb_out[HWT_H2S]);

		// flush cache
		reconos_cache_flush();

		

		
		
		if(print_flag) {
		    printf("\n\n[app] Packet no. %04d\n",cnt);
		    // print packet
		    for (i=0;i<(msg_len/4);i++){
			    printf("0x%08x  ",ptr[i]);
			    if ((i+1)%4==0){ 
				    printf("\n");
			    }
		    }
		
		    // print stats from FB1
	        get_stats_fb1(&packets_arrived, &packets_forwarded);  
	        printf("\n[stats] arrived: %d, forwarded: %d, dropped %d\n", packets_arrived, packets_forwarded, packets_arrived - packets_forwarded);
	    }

		// send ack to h2s
		mbox_put(&mb_in[HWT_H2S],(unsigned int) shared_mem_h2s);

		// stop after max_cnt packets
		//if(cnt == max_cnt)
			//break;
		cnt++;
	}
	
	printf("\n");
	return 0;
}

