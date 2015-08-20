

#ifndef __GENERIC_FB_H__
#define __GENERIC_FB_H__

/******************
 * Types
 ******************/

typedef enum {NONE, RECEIVE_THREAD, SWFB_THREAD, STATS_THREAD, HWFB} thread_type_enum;

/******************
 * Structs
 ******************/

// functional block parameters

struct fb_params_struct {
  int cycles_packet_bool; // indicates whether delay values are set in cycles or in secs
  int cycles_byte_bool; // indicates whether delay values are set in cycles or in secs
  unsigned long long delay_per_packet_cycles;
  unsigned long long delay_per_byte_cycles;
  double delay_per_packet_secs;
  double delay_per_byte_secs;
  unsigned long drop_type;
  unsigned long drop_value;
};

// shared memory between threads

struct shared_mem_threads_struct {
  unsigned int *pkt_data;
  unsigned int pkt_len;
  thread_type_enum src_thread_type;
  unsigned int src_thread_id;
};

// threads arguments

struct receive_thread_args_struct {
  unsigned int num_dst_addrs; // addr = (thread_type, thread_id)
  thread_type_enum *dst_thread_type; // array
  unsigned int *dst_thread_id; // array
};

struct swfb_thread_args_struct {
  int id;
  struct fb_params_struct params;
  unsigned int num_dst_addrs; // addr = (thread_type, thread_id)
  thread_type_enum *dst_thread_type; // array
  unsigned int *dst_thread_id; // array
};

struct stats_thread_args_struct {
	int id;
};

// hwfb arguments

struct hwfb_args_struct {
  int id;
  struct fb_params_struct params;
  unsigned long randgen_seed; // only used for HW
  unsigned int dst_global_addr;
  unsigned int dst_local_addr;
};



/******************
 * Constants
 ******************/

#define HW_FREQ 100000000
#define USECS_PER_NOP 0.1105  // processing time (in usecs) per nop loop

// HARDWARE SETUP
// --------------
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
#define CMD_PUT_ADDR        0x0000000a
#define CMD_PUT_swfb_id       0x00000002
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


/******************
 * Global Variables
 ******************/

#ifdef _MICROBLAZE_
struct reconos_resource res[NUM_SLOTS][2];
struct reconos_hwt hwt[NUM_SLOTS];
struct mbox mb_in[NUM_SLOTS];
struct mbox mb_out[NUM_SLOTS];
#endif

// random generator parameter
long long sw_randgen_seed;

// swfb stats
unsigned long long *swfb_packets_arrived;
unsigned long long *swfb_packets_forwarded;

// shared variables (shared between threads)
unsigned int *shared_mem_h2s;
unsigned int *shared_mem_s2h;
struct shared_mem_threads_struct *shared_mem_swfb_threads;
struct shared_mem_threads_struct *shared_mem_stats_threads;
// potential problem: originally the shared_mem_h2s and shared_mem_s2h was a ptr of type (unsigned char *)

// semaphores / conditional variables / mutex

sem_t *swfb_pkt_signal; 
sem_t *swfb_done_signal;
sem_t *stats_pkt_signal;
sem_t *stats_done_signal;
//TODO rename signal into signals for all 4 semaphores-array

// pkt counts
unsigned long long stats_pkt_count[6];

// settings
int verbose_bool; // print debug info
int logging_bool; // don't print anything except data when in loggin mode
int idp_select;
int num_reps; // number of repetition of a measurement
int time_between_pkts_bool;

// number of threads read from config file
int num_swfb_threads; // number of SW FBs
int num_hwfbs;
int num_stats_threads; // number of stats_threads


// read config from file
thread_type_enum current_thread_type;

// args
struct swfb_thread_args_struct *swfb_args;
struct hwfb_args_struct *hwfb_args;
struct stats_thread_args_struct *stats_args;

/******************
 * Functions Prototypes
 ******************/
// general util
int file_exist (char *filename);
void read_args_handle_line(char *line, struct swfb_thread_args_struct **swfb_args_ptr, struct hwfb_args_struct **hwfb_args_ptr, struct stats_thread_args_struct **stats_args_ptr);
void read_args_from_file(char *config_file, struct swfb_thread_args_struct **swfb_args, struct hwfb_args_struct **hwfb_args, struct stats_thread_args_struct **stats_args);
void print_pkt(unsigned int *ptr, int pkt_len);
unsigned long timevaldiff(struct timeval *starttime, struct timeval *finishtime);


// hw FB util
void hw_config_eth(unsigned int hash_1, unsigned int hash_2, unsigned int idp, int addr);
void hw_creating_delegate_threads();
void hw_setup_noc(void);
void hw_set_addr(unsigned int global_addr, unsigned int local_addr);
void hw_init(struct hwfb_args_struct *hwfb_args);
void hw_get_stats_fb1(unsigned int *packets_arrived, unsigned int *packets_forwarded);
void hw_get_addr(unsigned int *global_addr, unsigned int *local_addr);
int hw_get_dt_fb1(void);

// sw FB util
void sw_setup(void);
void sw_busy_wait(double delay);

// threads
void *receive(void * receive_thread_id);
void *swfb(void *swfb_thread_args);
void *stats(void *stats_thread_id);


#endif // __GENERIC_FB_H__
