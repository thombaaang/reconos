#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>
#include <ctype.h>

// ReconOS
#ifdef _MICROBLAZE_
  #include "reconos.h"
  #include "mbox.h"
  #include "timing.h"
#endif

#include "generic_fb.h"

/******************
 * General util
 ******************/

int file_exist (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

void read_args_handle_line(char *line, struct swfb_thread_args_struct **swfb_args_ptr, struct hwfb_args_struct **hwfb_args_ptr, struct stats_thread_args_struct **stats_args_ptr) {
  int size = 1024;
  char *cmd = (char *)malloc(size);
  char *option = (char *)malloc(size);
  char *value = (char *)malloc(size);

  // this tries to solve the problem with the realloc and "invalid old size", but it still doesn't work
  //  http://stackoverflow.com/questions/11406311/realloc-invalid-old-size
  //  stackoverflow.com/questions/766893/how-do-i-modify-a-pointer-that-has-been-passed-into-a-function-in-c
  //  NOTE: In C, EVERYTHING is passed by value! -> therefore pass pointer of pointer (this passes var "by reference")
  //  but it still didn't work
  //struct swfb_thread_args_struct *swfb_args = *swfb_args_ptr;
  //struct hwfb_args_struct *hwfb_args = *hwfb_args_ptr;
  //struct stats_thread_args_struct *stats_args = *stats_args_ptr;
  //
  // now workaround -> use global variables

  struct swfb_thread_args_struct *swfb_args_tmp;
  struct hwfb_args_struct *hwfb_args_tmp;
  struct stats_thread_args_struct *stats_args_tmp;



  sscanf(line,"%s %s %s", cmd, option, value);

  //printf("cmd: %s\n", cmd);
  //printf("option: %s\n", option);
  //printf("value: %s\n", value);

  if (!strcmp(cmd, "config")) {
    // start new struct

    if (!strcmp(option, "SWFB_THREAD")) {
      // start new swfb_thread

      num_swfb_threads++;
      current_thread_type = SWFB_THREAD;

      /*
      swfb_args_tmp = malloc(num_swfb_threads*sizeof(*swfb_args));
      memcpy(swfb_args_tmp, swfb_args, (num_swfb_threads-1)*sizeof(*swfb_args));
      free(swfb_args);
      swfb_args = swfb_args_tmp;  
      */
      
      swfb_args_tmp = realloc(swfb_args, num_swfb_threads*sizeof(*swfb_args));
      if (swfb_args_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      swfb_args = swfb_args_tmp;
      

      swfb_args[num_swfb_threads-1].id = strtoul(value, NULL, 10);
      // allocate memory for dst_addr
      swfb_args[num_swfb_threads-1].num_dst_addrs = 1;
      swfb_args[num_swfb_threads-1].dst_thread_type = malloc(sizeof(*(swfb_args[num_swfb_threads-1].dst_thread_type)));
      swfb_args[num_swfb_threads-1].dst_thread_id = malloc(sizeof(*(swfb_args[num_swfb_threads-1].dst_thread_id)));
      if (swfb_args[num_swfb_threads-1].dst_thread_type == NULL || swfb_args[num_swfb_threads-1].dst_thread_id == NULL) {
        printf("[main] ERROR: malloc of dst_thread_type OR dst_thread_id failed\n");
        exit(1);
      }
    }
    else if (!strcmp(option, "HWFB")) {
      // start new hwfb

      num_hwfbs++;
      current_thread_type = HWFB;
      hwfb_args_tmp = realloc(hwfb_args, num_hwfbs*sizeof(*hwfb_args));
      if (hwfb_args_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      hwfb_args = hwfb_args_tmp;

      hwfb_args[num_hwfbs-1].id = atoi(value);
    }
    else if (!strcmp(option, "STATS_THREAD")) {
      // start new stats_thread

      num_stats_threads++;
      current_thread_type = STATS_THREAD;
      stats_args_tmp = realloc(stats_args, num_stats_threads*sizeof(*stats_args));
      if (stats_args_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      stats_args = stats_args_tmp;
      stats_args[num_stats_threads-1].id = atoi(value);
    }
  } 
  else if (!strcmp(cmd, "option")) {
    // set new argument

    if (current_thread_type == SWFB_THREAD) {
      // set new SW FB argument
      if (!strcmp(option, "delay_per_packet_cycles")) {
        swfb_args[num_swfb_threads-1].params.cycles_packet_bool = 1;
        swfb_args[num_swfb_threads-1].params.delay_per_packet_cycles = strtoull(value, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_byte_cycles")) {
        swfb_args[num_swfb_threads-1].params.cycles_byte_bool = 1;
        swfb_args[num_swfb_threads-1].params.delay_per_byte_cycles =strtoull(value, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_packet_secs")) {
        swfb_args[num_swfb_threads-1].params.cycles_packet_bool = 0;
        swfb_args[num_swfb_threads-1].params.delay_per_packet_secs = strtod(value, NULL);
      }
      else if (!strcmp(option, "delay_per_byte_secs")) {
        swfb_args[num_swfb_threads-1].params.cycles_byte_bool = 0;
        swfb_args[num_swfb_threads-1].params.delay_per_byte_secs = strtod(value, NULL);
      }
      else if (!strcmp(option, "drop_type")) {
        swfb_args[num_swfb_threads-1].params.drop_type = strtoul(value, NULL, 10);
      }
      else if (!strcmp(option, "drop_value")) {
        swfb_args[num_swfb_threads-1].params.drop_value = strtoul(value, NULL, 10);
      }
      else if (!strcmp(option, "dst_type")) {
        if (!strcmp(value, "SWFB_THREAD")) {
          swfb_args[num_swfb_threads-1].dst_thread_type[0] = SWFB_THREAD;
        }
        else if (!strcmp(value, "STATS_THREAD")) {
          swfb_args[num_swfb_threads-1].dst_thread_type[0] = STATS_THREAD;
        }
        else {
          printf("[main] ERROR: dst_thread_type \"%s\" not valid!\n", value);
        }
      }
      else if (!strcmp(option, "dst_id")) {
        swfb_args[num_swfb_threads-1].dst_thread_id[0] = strtoul(value, NULL, 10);
      }
    }
    else if (current_thread_type == HWFB) {
      // set new HW FB argument

      if (!strcmp(option, "delay_per_packet_cycles")) {
        hwfb_args[num_hwfbs-1].params.cycles_packet_bool = 1;
        hwfb_args[num_hwfbs-1].params.delay_per_packet_cycles = strtoull(value, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_byte_cycles")) {
        hwfb_args[num_hwfbs-1].params.cycles_byte_bool = 1;
        hwfb_args[num_hwfbs-1].params.delay_per_byte_cycles =strtoull(value, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_packet_secs")) {
        hwfb_args[num_hwfbs-1].params.cycles_packet_bool = 0;
        hwfb_args[num_hwfbs-1].params.delay_per_packet_secs = strtod(value, NULL);
      }
      else if (!strcmp(option, "delay_per_byte_secs")) {
        hwfb_args[num_hwfbs-1].params.cycles_byte_bool = 0;
        hwfb_args[num_hwfbs-1].params.delay_per_byte_secs = strtod(value, NULL);
      }
      else if (!strcmp(option, "drop_type")) {
        hwfb_args[num_hwfbs-1].params.drop_type = strtoul(value, NULL, 10);
      }
      else if (!strcmp(option, "drop_value")) {
        hwfb_args[num_hwfbs-1].params.drop_value = strtoul(value, NULL, 10);
      }
      else if (!strcmp(option, "randgen_seed")) {
        hwfb_args[num_hwfbs-1].randgen_seed = strtoll(value, NULL, 16);
      }
      else if (!strcmp(option, "dst_global_addr")) {
        hwfb_args[num_hwfbs-1].dst_global_addr = strtoul(value, NULL, 10);
      }
      else if (!strcmp(option, "dst_local_addr")) {
        hwfb_args[num_hwfbs-1].dst_local_addr = strtoul(value, NULL, 10);
      }
    }
    else if (current_thread_type == STATS_THREAD) {
      // no options available yet
    }
  }
}

// based on http://stackoverflow.com/questions/3081289/how-to-read-a-line-from-a-text-file-in-c-c

void read_args_from_file(char *config_file, struct swfb_thread_args_struct **swfb_args, struct hwfb_args_struct **hwfb_args, struct stats_thread_args_struct **stats_args) {
  int size = 1024, pos;
  int c;
  char *buffer = (char *)malloc(size);

  num_swfb_threads = 0;
  num_hwfbs = 0;
  num_stats_threads = 0;

  current_thread_type = NONE;

  FILE *f = fopen(config_file, "r");
  if(f) {
    do { // read all lines in file
      pos = 0;
      do{ // read one line
        c = fgetc(f);
        if(c != EOF) buffer[pos++] = (char)c;
        if(pos >= size - 1) { // increase buffer length - leave room for 0
          size *=2;
          buffer = (char*)realloc(buffer, size);
        }
      }while(c != EOF && c != '\n');
      buffer[pos] = 0;
      // line is now in buffer
      read_args_handle_line(buffer, swfb_args, hwfb_args, stats_args);
      // now handle line:
    } while(c != EOF); 
    fclose(f);           
  }
  free(buffer);
}

void print_pkt(unsigned int *ptr, int pkt_len) {
  int i;

  for (i=0;i<(pkt_len/4);i++){
    printf("0x%08x  ",ptr[i]);
    if ((i+1)%4==0){ 
      printf("\n");
    }
  }
  printf("\n");
}

unsigned long timevaldiff(struct timeval *starttime, struct timeval *finishtime) {
  unsigned long usec;
  usec = abs(finishtime->tv_sec-starttime->tv_sec)*1000000;
  usec += abs(finishtime->tv_usec-starttime->tv_usec);
  return usec;
}



/******************
 * HARDWARE util
 ******************/


void hw_config_eth(unsigned int hash_1, unsigned int hash_2, unsigned int idp, int addr) {
  int ret = 0;
  unsigned int config_eth_hash_1 = hash_1;
  unsigned int config_eth_hash_2 = hash_2;
  unsigned int config_eth_idp = idp;
  unsigned int config_eth_address = addr; //global 4 bits, local 2 bits 
#ifdef _MICROBLAZE_
  mbox_put(&mb_in[HWT_ETH], config_eth_hash_1 );
  mbox_put(&mb_in[HWT_ETH], config_eth_hash_2);
  mbox_put(&mb_in[HWT_ETH], config_eth_idp);
  mbox_put(&mb_in[HWT_ETH], config_eth_address);
  ret = mbox_get(&mb_out[HWT_ETH]);
#endif
}



void hw_creating_delegate_threads() {
  int i;

#ifdef _MICROBLAZE_
  for (i=0; i<NUM_SLOTS; i++){
    // mbox init
    if (verbose_bool) {printf("mbox_init\n");}
    mbox_init(&mb_in[i],  10);
    mbox_init(&mb_out[i], 10);
    // define resources
    if (verbose_bool) {printf("define resources\n");}
    res[i][0].type = RECONOS_TYPE_MBOX;
    res[i][0].ptr  = &mb_in[i];     
    res[i][1].type = RECONOS_TYPE_MBOX;
    res[i][1].ptr  = &mb_out[i];
    // start delegate threads
    if (verbose_bool) {printf("start delegate threads\n");}
    if (verbose_bool) {printf("reconos_hwt_setresources: \n");}
    sleep(0.2); // tries to prevent the "Illegal Instruction" Exception, it didn't help
    reconos_hwt_setresources(&hwt[i],res[i],2);  // this instruction sometimes leads to the error "Illegal Instruction" on FPGA
    if (verbose_bool) {printf("reconos_hwt_create:\n");}
    reconos_hwt_create(&hwt[i],i,NULL);
  }
#endif
}

void hw_setup_noc(void) {
  shared_mem_h2s = malloc(4096); 
  memset(shared_mem_h2s, 0, 4096);
  shared_mem_s2h = malloc(4096); 
  memset(shared_mem_s2h, 0, 4096);
#ifdef _MICROBLAZE_
  mbox_put(&mb_in[HWT_H2S], (unsigned int) shared_mem_h2s);
  mbox_put(&mb_in[HWT_S2H], (unsigned int) shared_mem_s2h);
#endif
  hw_config_eth(0xabababab, 0xabababab, 2, TO_FB1); // HW only
  hw_config_eth(0xcdcdcdcd, 0xcdcdcdcd, 3, TO_H2S); // SW only
  hw_config_eth(0xefefefef, 0xefefefef, 4, TO_FB1); // HW & SW
  hw_config_eth(0xafafafaf, 0xafafafaf, 5, TO_H2S); // no FB
}

// FB1 functions /////////////////////////////////////

void hw_set_addr(unsigned int global_addr, unsigned int local_addr) {

  // set global and local addr
  unsigned int addr = 0;
  addr |= global_addr << 2;
  addr |= local_addr << 0;
#ifdef _MICROBLAZE_
  mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_GET_ADDR);
  mbox_put(&mb_in[HWT_FB1], (unsigned int) addr);
#endif
}

void hw_init(struct hwfb_args_struct *hwfb_args) {
  unsigned long delay_per_packet_h;
  unsigned long delay_per_packet_l;
  unsigned long delay_per_byte_h;
  unsigned long delay_per_byte_l;

  if (hwfb_args->params.cycles_packet_bool) {
    // set delay directly from cycles value
    delay_per_packet_h = (unsigned long) (((hwfb_args->params.delay_per_packet_cycles) >> 32) & 0xffffffff);
    delay_per_packet_l = (unsigned long) ((hwfb_args->params.delay_per_packet_cycles) & 0xffffffff);
  }
  else {
    // set delay by calculating number of cycles from number of secs
    unsigned long long delay_per_packet_cycles;

    // note: normal round function works for double value (in secs) that corresponds to 2^64-1 cycles (at least on desktop ubuntu)  
    delay_per_packet_cycles = round(hwfb_args->params.delay_per_packet_secs * HW_FREQ);

    delay_per_packet_h = (unsigned long) (((delay_per_packet_cycles) >> 32) & 0xffffffff);
    delay_per_packet_l = (unsigned long) ((delay_per_packet_cycles) & 0xffffffff);
  }

  if (hwfb_args->params.cycles_byte_bool) {
    // set delay directly from cycles value
    delay_per_byte_h = (unsigned long) (((hwfb_args->params.delay_per_byte_cycles) >> 32) & 0xffffffff);
    delay_per_byte_l = (unsigned long) ((hwfb_args->params.delay_per_byte_cycles) & 0xffffffff);
  }
  else {
    // set delay by calculating number of cycles from number of secs
    unsigned long long delay_per_byte_cycles;

    // note: normal round function works for double value (in secs) that corresponds to 2^64-1 cycles (at least on desktop ubuntu)  
    delay_per_byte_cycles = round(hwfb_args->params.delay_per_byte_secs * HW_FREQ);

    delay_per_byte_h = (unsigned long) (((delay_per_byte_cycles) >> 32) & 0xffffffff);
    delay_per_byte_l = (unsigned long) ((delay_per_byte_cycles) & 0xffffffff);
  }

  // DEBUG
  /*
  if (verbose_bool) {
    printf("delay_per_packet_h: 0x%lx\n", delay_per_packet_h);
    printf("delay_per_packet_l: 0x%lx\n", delay_per_packet_l);
    printf("delay_per_byte_h: 0x%lx\n", delay_per_byte_h);
    printf("delay_per_byte_l: 0x%lx\n", delay_per_byte_l);
  }
  */

#ifdef _MICROBLAZE_
  // put command to set all initial values for FB
  mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_GET_ALL);
  // set delay_per_packet high byte
  mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_packet_h);
  // set delay_per_packet low byte
  mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_packet_l);
  // set delay_per_byte high byte
  mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_byte_h);
  // set delay_per_byte low byte
  mbox_put(&mb_in[HWT_FB1], (unsigned int) delay_per_byte_l);
  // set drop_type
  mbox_put(&mb_in[HWT_FB1], (unsigned int) hwfb_args->params.drop_type);
  if (hwfb_args->params.drop_type == 2) {
      // set randgen_seed (not necessary if drop_type != 2)
      mbox_put(&mb_in[HWT_FB1], (unsigned int) hwfb_args->randgen_seed);
  }
  if (hwfb_args->params.drop_type != 0) {
      // set delay_value (not necessary if drop_type = 0)
      mbox_put(&mb_in[HWT_FB1], (unsigned int) hwfb_args->params.drop_value);
  }
#endif
}

void hw_get_stats_fb(unsigned long *hwfb_packets_arrived, unsigned long *hwfb_packets_forwarded) {
#ifdef _MICROBLAZE_
  // put command to set all initial values for FB1
  mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_PUT_STATS);
  // get stats
  *hwfb_packets_arrived = mbox_get(&mb_out[HWT_FB1]);
  *hwfb_packets_forwarded = mbox_get(&mb_out[HWT_FB1]);
#endif
}

void hw_get_addr(unsigned int *global_addr, unsigned int *local_addr) {
#ifdef _MICROBLAZE_
  unsigned int addr = 0;
  // put command to get addr
  mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_PUT_ADDR);
  // get addr
  addr = mbox_get(&mb_out[HWT_FB1]);
  // clear unused parts
  *global_addr = (addr >> 2);
  *local_addr = (addr & ~(0xbb << 2));
#endif
}

int hw_get_dt_fb1(void) {
#ifdef _MICROBLAZE_
  // put command to get addr
  mbox_put(&mb_in[HWT_FB1], (unsigned int) CMD_PUT_DT);
  // get addr
  return mbox_get(&mb_out[HWT_FB1]);
#endif
#ifndef _MICROBLAZE_
  return -1;
#endif
}


/******************
 * SOFTWARE util
 ******************/

void sw_setup(void) {
  int i;

  // allocate memory

  // shared memory between threads
  shared_mem_swfb_threads = malloc(num_swfb_threads*sizeof(*shared_mem_swfb_threads));
  shared_mem_stats_threads = malloc(num_stats_threads*sizeof(*shared_mem_stats_threads));

  // semaphores
  swfb_pkt_signal = malloc(num_swfb_threads*sizeof(*swfb_pkt_signal)); 
  swfb_done_signal = malloc(num_swfb_threads*sizeof(*swfb_done_signal));
  stats_pkt_signal = malloc(num_stats_threads*sizeof(*stats_pkt_signal)); 
  stats_done_signal = malloc(num_stats_threads*sizeof(*stats_done_signal));

  // packet counters
  swfb_packets_arrived = malloc(num_swfb_threads*sizeof(*swfb_packets_arrived));
  swfb_packets_forwarded = malloc(num_swfb_threads*sizeof(*swfb_packets_forwarded));
  

  // initialization

  // semaphores and variables of swfb_threads
  for (i=0; i<num_swfb_threads; i++) {
    // semaphores
    sem_init(&swfb_pkt_signal[i], 0, 0);
    sem_init(&swfb_done_signal[i], 0, 1);

    // packet count variables
    swfb_packets_arrived[i] = 0;
    swfb_packets_forwarded[i] = 0;
  }

  // semaphores and variables of stats_threads
  for (i=0; i<num_stats_threads; i++) {
    // semaphores
    sem_init(&stats_pkt_signal[i], 0, 0);
    sem_init(&stats_done_signal[i], 0, 1);
  }
  
  // initialize random generator
  srand(time(NULL));

  // init stats from stats_thread (one array element for each idp (0 and 1 are not used at the moment))
  for (i=0; i<6; i++) {
    stats_pkt_count[i] = 0;
  }
}

void sw_busy_wait(double delay) {
  unsigned long long num_nops;
  unsigned long long k;

  num_nops = round(delay/(USECS_PER_NOP/1000000));
  if (verbose_bool) {printf("[busy_wait] num_nops: %llu\n", num_nops);}

  for(k=0; k < num_nops; k++) {
    // dummy operation (to keep the cpu busy)
    asm("nop");
  }
}

/******************
 * SOFTWARE Threads
 ******************/

////////////////////////////////////////////////
// receive_thread
////////////////////////////////////////////////

void *receive(void *receive_thread_args) {
  struct receive_thread_args_struct *args;
  unsigned int pkt_len;
  int dst_idp, generate_count;
  unsigned int distribute_count;

  args = (struct receive_thread_args_struct *) receive_thread_args;
  generate_count = 0;
  distribute_count = 0;
  
  if (verbose_bool) {printf("[receive] thread created!\n");}

  // start handling pkts
  while (1) {
    // receive pkt from h2s / generate pkt
    ////////////////////////////////////////

#ifndef _MICROBLAZE_
    // DEBUG: generate packets
    if (verbose_bool) {printf("[receive] generating pkt\n");}
    sleep(1);
    if (generate_count % 5 == 0) {
      shared_mem_h2s[0] = 0x00000301; 
      shared_mem_h2s[1] = 0x00000000;
      shared_mem_h2s[2] = 0x00000003; 
      pkt_len = 12; //in bytes (one array entry = 4 bytes)
    }
    else if (generate_count % 5 == 1) {
      shared_mem_h2s[0] = 0x00000302; 
      shared_mem_h2s[1] = 0x00000000;
      shared_mem_h2s[2] = 0x00000004;
      pkt_len = 12; //in bytes (one array entry = 4 bytes)
    }
    else if (generate_count % 5 == 2) {
      shared_mem_h2s[0] = 0x00000303;
      shared_mem_h2s[1] = 0x00000000;
      shared_mem_h2s[2] = 0x00000003;
      pkt_len = 12; //in bytes (one array entry = 4 bytes)
    }
    else if (generate_count % 5 == 3) {
      shared_mem_h2s[0] = 0x00000304;
      shared_mem_h2s[1] = 0x00000000;
      shared_mem_h2s[2] = 0x00000003;
      pkt_len = 12; //in bytes (one array entry = 4 bytes)
    }
    else {
      shared_mem_h2s[0] = 0x00000305;
      shared_mem_h2s[1] = 0x00000000;
      shared_mem_h2s[2] = 0x00000003;
      pkt_len = 12; //in bytes (one array entry = 4 bytes)
    }
#endif
#ifdef _MICROBLAZE_
    // receive message from h2s
    if (verbose_bool) {printf("[receive] waiting for packet from h2s...\n");}
    pkt_len = mbox_get(&mb_out[HWT_H2S]);
    // flush cache
    reconos_cache_flush();
#endif

    // DEBUG: print received packet
    if (verbose_bool) {
      printf("[receive] packet: \n");
      print_pkt(shared_mem_h2s, pkt_len);
    }

    // forward packet
    //////////////////////////////////////////
    // TODO: use args->dst_addr

    dst_idp = shared_mem_h2s[2];
    if (verbose_bool) {printf("[receive] dst_idp: %d\n", dst_idp);}

    if ( ((dst_idp == 3) || (dst_idp == 4)) && (num_swfb_threads >= 1) ) {
      // forward packet to swfb_thread
      ///////////////////////////////////

      int dst_swfb = distribute_count % num_swfb_threads;

      // wait for done_signal (done means shared variables can now be overwritten)
      if (verbose_bool) {printf("[receive] waiting for done_signal (from swfb_thread %u) ...\n", dst_swfb);}
      sem_wait(&swfb_done_signal[dst_swfb]); // blocks if signal has not been sent yet, immediately returns if signal has been sent before
      if (verbose_bool) {printf("[receive] done_signal signal received. \n");}

      if (verbose_bool) {printf("[receive] handing over pkt to swfb_thread %u\n", dst_swfb);}
      shared_mem_swfb_threads[dst_swfb].pkt_data = shared_mem_h2s;
      shared_mem_swfb_threads[dst_swfb].pkt_len = pkt_len;
      shared_mem_swfb_threads[dst_swfb].src_thread_type = RECEIVE_THREAD;
      shared_mem_swfb_threads[dst_swfb].src_thread_id = 0;

      // send pkt_signal
      if (verbose_bool) {printf("[receive] send pkt_signal to swfb_thread %u\n", dst_swfb);}
      sem_post(&swfb_pkt_signal[dst_swfb]);

      distribute_count++;
    }
    else if (dst_idp == 2 || dst_idp == 5) {
      // pass packet to stats_thread
      ////////////////////////////////

      // wait for done_signal (done means shared variables can now be overwritten)
      if (verbose_bool) {printf("[receive] waiting for done_signal (from stats_thread 0) ...\n");}
      sem_wait(&stats_done_signal[0]); // blocks if signal has not been sent yet, immediately returns if signal has been sent before
      if (verbose_bool) {printf("[receive] done_signal signal received. \n");}
      
      if (verbose_bool) {printf("[receive] handing over pkt to stats_thread 0\n");}
      shared_mem_stats_threads[0].pkt_data = shared_mem_h2s;
      shared_mem_stats_threads[0].pkt_len = pkt_len;
      shared_mem_stats_threads[0].src_thread_type = RECEIVE_THREAD;
      shared_mem_stats_threads[0].src_thread_id = 0;

      // send pkt_signal
      if (verbose_bool) {printf("[receive] send pkt_signal\n");}
      sem_post(&stats_pkt_signal[0]);
    }
    else {
      // unexpected dst_idp -> drop packet
      if (verbose_bool) {printf("[receive] unexpected dst_idp %d -> dropped packet\n", dst_idp);}
    }

    // allocate new memory for shared_mem_h2s
    /////////////////////////////////////////////
    shared_mem_h2s = malloc(4096);
    memset(shared_mem_h2s, 0, 4096);

#ifdef _MICROBLAZE_
    // send ack to h2s (when finished with processing packet in receive_thread)
    mbox_put(&mb_in[HWT_H2S],(unsigned int) shared_mem_h2s);
#endif

    generate_count++;

    // DEBUG
    //if (count >= 1) break; // only do loop once
  }

  pthread_exit(NULL);
}


////////////////////////////////////////////////
// swfb_thread
////////////////////////////////////////////////

void *swfb(void *swfb_thread_args) {
  struct swfb_thread_args_struct *args;
  double total_delay;
  unsigned int *pkt_data;
  unsigned long drop_count;
  double rand_num;
  int pkt_len;
  thread_type_enum src_thread_type;
  unsigned int src_thread_id;

  args = (struct swfb_thread_args_struct *) swfb_thread_args;
  if (verbose_bool) {printf("[swfb%d] thread created!\n", args->id);}

  // initialize variables
  if (args->params.drop_value > 0) {
    drop_count = args-> params.drop_value - 1;
  }
  else {
    drop_count = 0;
  }

  // start handling pkts
  while (1) {
    // fetch next packet from receive_thread
    ////////////////////////////////////////

    // wait for pkt_signal (from reiceive_thread)
    if (verbose_bool) {printf("[swfb%d] waiting for pkt_signal (from receive_thread OR another swfb_thread) ...\n", args->id);}
    sem_wait(&swfb_pkt_signal[args->id]);

    // copy content of shared variables to local variables
    pkt_data = shared_mem_swfb_threads[args->id].pkt_data;
    pkt_len = shared_mem_swfb_threads[args->id].pkt_len;
    src_thread_type = shared_mem_swfb_threads[args->id].src_thread_type;
    src_thread_id = shared_mem_swfb_threads[args->id].src_thread_id ;
    swfb_packets_arrived[args->id]++;

    // print where packet came from (receive_thread or swfb_thread)
    if (verbose_bool) {
      if (src_thread_type == RECEIVE_THREAD) {
        printf("[swfb%d] pkt_signal received from receive_thread\n", args->id);
      }
      else if (src_thread_type == SWFB_THREAD) {
        printf("[swfb%d] pkt_signal received from swfb_thread %u\n", args->id, src_thread_id);
      }
      else {
        printf("[swfb%d] ERROR: pkt_signal received from an unknown thread_type\n", args->id);
      }
    }

    // send done_signal (i.e. other threads can now safely overwrite the shared variables)
    if (verbose_bool) {printf("[swfb%d] send done_signal\n", args->id);}
    sem_post(&swfb_done_signal[args->id]);

    // DEBUG: print received packet
    if (verbose_bool) {
      printf("[swfb%d] packet:\n", args->id);
      print_pkt(pkt_data, pkt_len);
    }

    ////////////////////////////////////////
    // process packet
    ////////////////////////////////////////

    // eventually drop the packet
    if (args->params.drop_type == 0) {
      // drop_disabled (never drop a packet)
      // -> don't drop (i.e. do nothing here)
      if (verbose_bool) {printf("[swfb%d] [DROP] didn't drop the packet\n", args->id);}
    }
    else if (args->params.drop_type == 1) {
      // fix_drop (drop every drop_value-th packet)
      ///////////////////////////////////////////////
      if (args->params.drop_value == 0) {
        // invalid drop_value (i.e. drop every zeroest-packet)
        // -> don't drop (i.e. do nothing here)
      }
      else {
        if (drop_count == 0) {
          // drop the packet (continue with next loop iteration)
          drop_count = args->params.drop_value -1;
          if (verbose_bool) {printf("[swfb%d] [DROP] DROPPED the packet\n", args->id);}
          continue;
        }
        else {
          // don't drop the paket
          drop_count--;
          if (verbose_bool) {printf("[swfb%d] [DROP] didn't drop the packet\n", args->id);}
        }
      }
    }
    else if (args->params.drop_type == 2) {
      // random_drop (drop every ((2^32)/drop-value)-th packet on average)
      ///////////////////////////////////////////////
      if (args->params.drop_value == 0) {
        // invalid drop_value (i.e. drop every infinitieth-packet)
        // -> don't drop (i.e. do nothing here)
      }
      else {
        rand_num = (rand()/( (double) RAND_MAX ));
        if (verbose_bool) {printf("[swfb%d] [DROP] rand_drop compare: don't drop if %f > %f\n", args->id, rand_num, (args->params.drop_value/( (double) 0xffffffff )));}
        if ( rand_num > (args->params.drop_value/( (double) 0xffffffff )) ) {
          // don't drop the paket (i.e. do nothing)
          if (verbose_bool) {printf("[swfb%d] [DROP] didn't drop the packet\n", args->id);}
        }
        else {
          // drop the packet (continue with next loop iteration)
          if (verbose_bool) {printf("[swfb%d] [DROP] DROPPED the packet\n", args->id);}
          continue;
        }
      }
    }

    // delay the packet
    /////////////////////////////////////////////////
    double packet_secs = 0;
    double byte_secs = 0;

    if (args->params.cycles_packet_bool) {
      packet_secs = (1/((double) HW_FREQ)) * args->params.delay_per_packet_cycles;
    }
    else {
      packet_secs = args->params.delay_per_packet_secs;
    }

    if (args->params.cycles_byte_bool) {
      byte_secs = (1/((double) HW_FREQ)) * args->params.delay_per_byte_cycles;
    }
    else {
      byte_secs = args->params.delay_per_byte_secs;
    }

    total_delay = packet_secs + pkt_len*byte_secs;

    if (verbose_bool) {printf("[swfb%d] busy_wait started\n", args->id);}
    sw_busy_wait(total_delay);
    if (verbose_bool) {printf("[swfb%d] busy_wait finished\n", args->id);}

    // forward packet to the stats_thread
    /////////////////////////////////////////

    // TODO: adapt sending that it allows alternating sending to all dst_addrs in the array (right now it always sends the pkt to the first entry of the array)

    thread_type_enum *dst_thread_type = &(args->dst_thread_type[0]);
    unsigned int *dst_thread_id = &(args->dst_thread_id[0]);

    if (*dst_thread_type == SWFB_THREAD) {
      // wait for done_signal (from stats_thread)
      if (verbose_bool) {printf("[swfb%d] waiting for done_signal (from swfb_thread %u) ...\n", args->id, *dst_thread_id);}
      sem_wait(&swfb_done_signal[*dst_thread_id]);
      if (verbose_bool) {printf("[swfb%d] done_signal received\n", args->id);}

      // copy content of local variables to shared variables
      if (verbose_bool) {printf("[swfb%d] handing over pkt to swfb_thread\n", args->id);}
      shared_mem_swfb_threads[*dst_thread_id].pkt_data = pkt_data;
      shared_mem_swfb_threads[*dst_thread_id].pkt_len = pkt_len;
      shared_mem_swfb_threads[*dst_thread_id].src_thread_type = SWFB_THREAD;
      shared_mem_swfb_threads[*dst_thread_id].src_thread_id = args->id;
      swfb_packets_forwarded[args->id]++;

      // send pkt_signal to stats_thread
      if (verbose_bool) {printf("[swfb%d] send pkt_signal (to swfb_thread %u)\n", args->id, *dst_thread_id);}
      sem_post(&swfb_pkt_signal[*dst_thread_id]);
    }
    else if (*dst_thread_type == STATS_THREAD) {
      // wait for done_signal (from stats_thread)
      if (verbose_bool) {printf("[swfb%d] waiting for done_signal (from stats_thread %u) ...\n", args->id, *dst_thread_id);}
      sem_wait(&stats_done_signal[*dst_thread_id]);
      if (verbose_bool) {printf("[swfb%d] done_signal received\n", args->id);}

      // copy content of local variables to shared variables
      if (verbose_bool) {printf("[swfb%d] handing over pkt to stats_thread %u\n", args->id, *dst_thread_id);}
      shared_mem_stats_threads[*dst_thread_id].pkt_data = pkt_data;
      shared_mem_stats_threads[*dst_thread_id].pkt_len = pkt_len;
      shared_mem_stats_threads[*dst_thread_id].src_thread_type = SWFB_THREAD;
      shared_mem_stats_threads[*dst_thread_id].src_thread_id = args->id;
      swfb_packets_forwarded[args->id]++;

      // send pkt_signal to stats_thread
      if (verbose_bool) {printf("[swfb%d] send pkt_signal (to stats_thread %u)\n", args->id, *dst_thread_id);}
      sem_post(&stats_pkt_signal[*dst_thread_id]);
    }
    else {
      if (verbose_bool) {printf("[swfb%d] ERROR: dst_addr is unknown. \n", args->id);}
    }
    

    if (verbose_bool) {printf("[swfb%d] [STATS] pkts_arrived: %llu, pkts_forwarded: %llu, pkts_dropped: %llu\n", args->id, swfb_packets_arrived[args->id], swfb_packets_forwarded[args->id], swfb_packets_arrived[args->id] - swfb_packets_forwarded[args->id]);}
  }

  pthread_exit(NULL);
}

////////////////////////////////////////////////
// stats_thread
////////////////////////////////////////////////

void *stats(void *stats_thread_args) {
  struct stats_thread_args_struct *args;
  thread_type_enum src_thread_type;
  unsigned int src_thread_id;
  int i, dst_idp;
  long time_between_pkts_count = -1;
  struct timeval start, end;
  unsigned long *time_between_pkts;

  time_between_pkts = malloc(num_reps*sizeof(time_between_pkts_count));

  args = (struct stats_thread_args_struct *) stats_thread_args;
  if (verbose_bool) {printf("[stats%d] thread created!\n", args->id);}

  // start handling pkts
  while (1) {

    // wait for pkt_signal 

    if (verbose_bool) {printf("[stats%d] waiting for packet_signal (from swfb_thread OR receive_thread) ...\n", args->id);}
    sem_wait(&stats_pkt_signal[args->id]);

    src_thread_type = shared_mem_stats_threads[args->id].src_thread_type;
    src_thread_id = shared_mem_stats_threads[args->id].src_thread_id;

    // print where packet came from (receive_thread or swfb_thread)
    if (verbose_bool) {
      if (src_thread_type == RECEIVE_THREAD) {
        if (verbose_bool) {printf("[stats%d] pkt_signal received from receive_thread\n", args->id);}
      }
      else if (src_thread_type == SWFB_THREAD) {
        if (verbose_bool) {printf("[stats%d] pkt_signal received from swfb_thread %u\n", args->id, src_thread_id);}
      }
      else {
        if (verbose_bool) {printf("[stats%d] ERROR: pkt_signal received from an unknown thread_type\n", args->id);}
      }
    }

    // process pkt
    if (verbose_bool) {
      printf("[stats%d] packet: \n", args->id);
      print_pkt( (unsigned int *) shared_mem_stats_threads[args->id].pkt_data, shared_mem_stats_threads[args->id].pkt_len);
    } 
    dst_idp = shared_mem_stats_threads[args->id].pkt_data[2];
    stats_pkt_count[dst_idp]++;

    // free memory allocated for this packet
    free(shared_mem_stats_threads[args->id].pkt_data);

    // measure time between packets
    if (time_between_pkts_bool) {
      if (dst_idp == idp_select) {
        if (time_between_pkts_count < num_reps) {
          // collect data
          if (time_between_pkts_count == -1) {
            gettimeofday(&start, NULL);
            time_between_pkts_count++;
          }
          else {
            gettimeofday(&end, NULL);

            time_between_pkts[time_between_pkts_count] = timevaldiff(&start, &end);
            start = end;
            time_between_pkts_count++;
          }
        }
        else {
          // print collected data & exit
          if (!logging_bool) {printf("[stats%d] delay between pkts with idp=%d:\n", args->id, idp_select);}

          for (i=0; i<num_reps; i++) {
            printf("%lu\n", time_between_pkts[i]);
          }

          free(time_between_pkts);

          exit(0);
        }
      }
    }

    // send done_signal
    if (verbose_bool) {printf("[stats%d] send done_signal\n", args->id);}
    sem_post(&stats_done_signal[args->id]);
  }

  pthread_exit(NULL);
}


/******************
 * MAIN
 ******************/
// MAIN ////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{ 
  int c;
  int rc;
  int i;
  double pkt_rate_hwfb_sleep; // HW FB throughput test
  double pkt_rate_stats_thread_sleep; // output measured packet rate in stats_thread (needs the -i argument (to specify idp))
  int without_threads_bool;
  // int idp_select; // specify idp // now global var
  int pkt_count_stats_thread_bool; // output pkt_count values of stats_thread
  // int time_between_pkts_bool; // output delay between packets measured in stats_thread (globally defined variable)
  char *config_file; // path to config file to read the parameters from

  unsigned int global_addr, local_addr;
  unsigned long hwfb_packets_arrived, hwfb_packets_forwarded;

  //struct swfb_thread_args_struct *swfb_args; // now global var
  //struct hwfb_args_struct *hwfb_args; // now global var
  //struct stats_thread_args_struct *stats_args; // now global var


  // initialization (defaults)
  pkt_rate_hwfb_sleep = 0;
  pkt_rate_stats_thread_sleep = 0;
  without_threads_bool = 0;
  time_between_pkts_bool = 0;
  idp_select = 0;
  num_reps = 1;
  pkt_count_stats_thread_bool = 0;

  verbose_bool = 0;
  logging_bool = 0;

  config_file = (char *)malloc(1024);
  strcpy(config_file, "./generic_fb.config");

  num_swfb_threads = 0;
  num_stats_threads = 0;
  num_hwfbs = 0;

  // TODO: dynamic allocation of memory doesn't work yet with swfb_threads and hwfbs -> problem with realloc
  swfb_args = malloc(1*sizeof(*swfb_args)); 
  hwfb_args = malloc(1*sizeof(*hwfb_args));
  stats_args = malloc(sizeof(*stats_args));
  

  /******************
  * Parse CLI Arguments
  ******************/

  while ((c = getopt(argc, argv, "h:w:r:tci:n:vlf:")) != -1) {
    switch (c) {
      case 'h':
        pkt_rate_hwfb_sleep = strtod(optarg, NULL);
        without_threads_bool = 1;
        break;
      case 'w':
        pkt_rate_hwfb_sleep = strtod(optarg, NULL);
        without_threads_bool = 0;
        break;
      case 'r':
        pkt_rate_stats_thread_sleep = strtod(optarg, NULL);
        break;
      case 't':
        time_between_pkts_bool = 1;
        break;
      case 'c':
        pkt_count_stats_thread_bool = 1;
        break;
      case 'i':
        idp_select = atoi(optarg);
        break;
      case 'n':
        num_reps = atoi(optarg);
        break;
      case 'v':
        verbose_bool = 1;
        break;
      case 'l':
        logging_bool = 1;
        break;
      case 'f':
        config_file = optarg;
        break;
      case '?':
        if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort();
    }
  }

  if (!logging_bool) {
    printf( "-------------------------------------------------------\n"
          "generic_fb\n"
          "(" __FILE__ ")\n"
          "Compiled on " __DATE__ ", " __TIME__ ".\n"
          "-------------------------------------------------------\n\n" );
  }
    
  /******************
  * Read arguments from config file
  ******************/

  // check if file exists
  if (!file_exist(config_file)) {
    printf("Config file \"%s\" doesn't exist. \n", config_file);
    exit(-1);
  }

  if (!logging_bool) {printf("[main] Read config parameters from config file\n");}
  read_args_from_file(config_file, &swfb_args, &hwfb_args, &stats_args);

  if (!logging_bool) {
    printf("[main] Route from ETH to stats contains: \n");
    printf("        %d hardware FB(s) \n", num_hwfbs);
    printf("        %d software FB(s)\n", num_swfb_threads);
    printf("        %d stats thread(s)\n", num_stats_threads);
  
  
    // print hwfb_args
    for (i=0; i<num_hwfbs; i++) {
      printf("[main] HWFB %d:\n", hwfb_args[i].id);
      if (hwfb_args[i].params.cycles_packet_bool) {
        printf("        delay_per_packet_cycles: %llu\n", hwfb_args[i].params.delay_per_packet_cycles);
      } 
      else {
        printf("        delay_per_packet_secs: %f\n", hwfb_args[i].params.delay_per_packet_secs);
      }
      if (hwfb_args[i].params.cycles_byte_bool) {
        printf("        delay_per_byte_cycles: %llu\n", hwfb_args[i].params.delay_per_byte_cycles);
      } 
      else {
        printf("        delay_per_byte_secs: %f\n", hwfb_args[i].params.delay_per_byte_secs);
      }
      printf("        drop_type: %lu\n", hwfb_args[i].params.drop_type);
      printf("        drop_value: %lu\n", hwfb_args[i].params.drop_value);
      printf("        randgen_seed: 0x%lx\n", hwfb_args[i].randgen_seed);
      printf("        dst_global_addr: %u\n", hwfb_args[i].dst_global_addr);
      printf("        dst_local_addr: %u\n", hwfb_args[i].dst_local_addr);
    }

    // print swfb_args
    for (i=0; i<num_swfb_threads; i++) {
      printf("[main] SWFB_THREAD %d:\n", swfb_args[i].id);
      if (swfb_args[i].params.cycles_packet_bool) {
        printf("        delay_per_packet_cycles: %llu\n", swfb_args[i].params.delay_per_packet_cycles);
      } 
      else {
        printf("        delay_per_packet_secs: %f\n", swfb_args[i].params.delay_per_packet_secs);
      }
      if (swfb_args[i].params.cycles_byte_bool) {
        printf("        delay_per_byte_cycles: %llu\n", swfb_args[i].params.delay_per_byte_cycles);
      } 
      else {
        printf("        delay_per_byte_secs: %f\n", swfb_args[i].params.delay_per_byte_secs);
      }
      printf("        drop_type: %lu\n", swfb_args[i].params.drop_type);
      printf("        drop_value: %lu\n", swfb_args[i].params.drop_value);
      printf("        num_dst_addrs: %u\n", swfb_args[i].num_dst_addrs);

      if (swfb_args[i].dst_thread_type[0] == SWFB_THREAD) {
        printf("        dst_thread_type: SWFB_THREAD\n");
      }
      else if (swfb_args[i].dst_thread_type[0] == STATS_THREAD) {
        printf("        dst_thread_type: STATS_THREAD\n");
      }
      else {
        printf("ERROR: invalid dst_thread_type \"%d\" -> cannot be printed\n", swfb_args[i].dst_thread_type[0]);
      }
      
      printf("        dst_thread_id: %u\n", swfb_args[i].dst_thread_id[0]);
    }

    // print  stats_args
    for (i=0; i<num_stats_threads; i++) {
      printf("[main] STATS_THREAD %d:\n", stats_args[i].id);
    }
  }



  /******************
   * Initialize HW
   ******************/

  if (!logging_bool) {printf("[main] Initialize ReconOS.\n");}
#ifdef _MICROBLAZE_
  reconos_init_autodetect();
#endif

  if (!logging_bool) {printf("[main] Creating delegate threads.\n");}
  hw_creating_delegate_threads();

  if (!logging_bool) {printf("[main] Setup NoC\n");}
  hw_setup_noc();

  /******************
   * Initialize HW FB
   ******************/
   
  if (num_hwfbs >= 1) {
    // set global and local addr of FB1
    if (!logging_bool) {printf("[main] Setting global and local address.\n");}
    hw_set_addr(hwfb_args->dst_global_addr, hwfb_args->dst_local_addr);

    // print current global and local address
    hw_get_addr(&global_addr, &local_addr);
    if (!logging_bool) {
      printf("[main] Global Address HW FB: 0x%01x\n", global_addr);
      printf("       Local Address HW FB: 0x%01x\n", local_addr);
    }

    // set all parameters of HW FB1
    hw_init(hwfb_args);
  }

  /******************
   * Initialize SW
   ******************/

  sw_setup();


  /******************
   * Create threads
   ******************/

  pthread_t receive_thread;
  pthread_t *swfb_threads;
  pthread_t *stats_threads;


  // only create threads if hwfb pkt rate measurement option is not set

  if (!logging_bool) {printf("[main] creating threads\n");}

  // receive thread preparation
  ///////////////////////////////////////
  struct receive_thread_args_struct receive_args;

  // swfb thread preparation
  //////////////////////////////////////
  swfb_threads = malloc(num_swfb_threads*sizeof(*swfb_threads));

  //stats thread prepaprations
  //////////////////////////////////////
  stats_threads = malloc(num_stats_threads*sizeof(*stats_threads));


  // create threads

  // receive thread
  rc = pthread_create(&receive_thread, NULL, receive, (void *)&receive_args);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

  // swfb thread
  for (i=0; i<num_swfb_threads; i++) {
    rc = pthread_create(&swfb_threads[i], NULL, swfb, (void *)&swfb_args[i]);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  // stats thread
  for (i=0; i<num_stats_threads; i++) {
    rc = pthread_create(&stats_threads[i], NULL, stats, (void *)&stats_args[i]);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }


  /******************
   * Output Statistics
   ******************/
  
  // -h OR -w option
  if (pkt_rate_hwfb_sleep != 0) {
    struct timeval time_start, time_end;
    unsigned long timediff; // time difference in usecs
    unsigned long long pktdiff; // number of received pkts
    double pkt_rate;
    unsigned long long pkt_count_start, pkt_count_end;

#ifdef _MICROBLAZE_
    if (without_threads_bool) {
      if (verbose_bool) {printf("[main] setting global_addr=0x0, local_addr=0x0\n");}
      hw_set_addr(0x0, 0x0); // prevent blocking from software
    }
#endif
    for (i=0; i<num_reps; i++) {
      if (verbose_bool) {printf("[main] gettimeofday time_start\n");}
      gettimeofday(&time_start, NULL);
#ifdef _MICROBLAZE_
      if (verbose_bool) {printf("[main] get stats\n");}
      hw_get_stats_fb(&hwfb_packets_arrived, &hwfb_packets_forwarded); 
      pkt_count_start = hwfb_packets_forwarded;
#endif

      if (verbose_bool) {printf("[main] sleep\n");}
      sleep(pkt_rate_hwfb_sleep);

      if (verbose_bool) {printf("[main] gettimeofday time_end\n");}
      gettimeofday(&time_end, NULL);
#ifdef _MICROBLAZE_
      if (verbose_bool) {printf("[main] get stats\n");}
      hw_get_stats_fb(&hwfb_packets_arrived, &hwfb_packets_forwarded); 
      pkt_count_end = hwfb_packets_forwarded;
#endif

      if (verbose_bool) {printf("[main] calculate packet rate\n");}
      timediff = timevaldiff(&time_start, &time_end);
      pktdiff = pkt_count_end - pkt_count_start;

      pkt_rate = ((double) pktdiff) / (((double) timediff) / 1000000.0);

      if (!logging_bool) {printf("[main] packet rate in hwfb:\n");}
      printf("%f\n", pkt_rate);
    }
    exit(0);
  }

  // -r option
  if (pkt_rate_stats_thread_sleep != 0) {
    struct timeval time_start, time_end;
    unsigned long timediff; // time difference in usecs
    unsigned long long pktdiff; // number of received pkts
    double pkt_rate;
    unsigned long long pkt_count_start, pkt_count_end;

    for (i=0; i<num_reps; i++) {
      gettimeofday(&time_start, NULL);
      pkt_count_start = stats_pkt_count[idp_select];

      sleep(pkt_rate_stats_thread_sleep);

      gettimeofday(&time_end, NULL);
      pkt_count_end = stats_pkt_count[idp_select];

      timediff = timevaldiff(&time_start, &time_end);
      pktdiff = pkt_count_end - pkt_count_start;

      pkt_rate = ((double) pktdiff) / (((double) timediff) / 1000000.0);

      if (!logging_bool) {printf("[main] packet rate in stats_thread of idp=%d:\n", idp_select);}
      printf("%f\n", pkt_rate);
    }
    exit(0);
  }

  // -c option
  if (pkt_count_stats_thread_bool) {
    while (1) {
      // print stats from hwfb
#ifdef _MICROBLAZE_
      hw_get_stats_fb(&hwfb_packets_arrived, &hwfb_packets_forwarded); 
      printf("[main] stats from HW FB:\n");
      printf("arrived: %lu\n", hwfb_packets_arrived);
      printf("forwarded: %lu\n", hwfb_packets_forwarded);
      printf("dropped %lu\n", hwfb_packets_arrived - hwfb_packets_forwarded);
#endif

      // print stats from stats_thread
      printf("[main] stats from stats_thread:\n");
      for (i=2; i<6; i++) {
        printf("idp=%d: %llu\n", i, stats_pkt_count[i]);
      }

      sleep(1);
    }
  }


  /******************
   * Join threads
   ******************/

  // wait for threads to finish (actually never happens -> but this keeps the main thread running which is necessary
  // for some tools to measure the memory usage of the program)
  pthread_join(receive_thread, NULL);
  for (i=0; i<num_swfb_threads; i++) {
    pthread_join(swfb_threads[i], NULL);
  }
  for (i=0; i<num_stats_threads; i++) {
    pthread_join(stats_threads[i], NULL);
  }

  // TODO: clean up all dynamically allocated variables
  // (useless since this part of the program is never reached)
  	
  pthread_exit(NULL);
}

