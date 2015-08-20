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
#endif

#include "timing.h"

#include "generic_fb.h"


/******************
 * args from file functions
 ******************/

int file_exist (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

void read_args_handle_line(char *line, struct swfb_thread_args_struct **swfb_args_ptr, struct hwfb_args_struct **hwfb_args_ptr, struct stats_thread_args_struct **stats_args_ptr) {
  int size = 1024;
  char *cmd = (char *)malloc(size);
  char *option = (char *)malloc(size);
  char *value0 = (char *)malloc(size);
  char *value1 = (char *)malloc(size);
  char *value2 = (char *)malloc(size);
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



  sscanf(line,"%s %s %s %s %s", cmd, option, value0, value1, value2);

  /*
  if (verbose_bool) {
    printf("cmd: %s\n", cmd);
    printf("option: %s\n", option);
    printf("value0: %s\n", value0);
    printf("value1: %s\n", value1);
    printf("value2: %s\n", value2);
  }
  */

  if (!strcmp(cmd, "config")) {
    // start new struct

    if (!strcmp(option, "SWFB_THREAD")) {
      // start new swfb_thread

      num_swfb_threads++;
      current_thread_type = SWFB_THREAD;
      
      swfb_args_tmp = realloc(swfb_args, num_swfb_threads*sizeof(*swfb_args));
      if (swfb_args_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      swfb_args = swfb_args_tmp;
      

      swfb_args[num_swfb_threads-1].id = strtoul(value0, NULL, 10);
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

      hwfb_args[num_hwfbs-1].id = atoi(value0);
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
      stats_args[num_stats_threads-1].id = atoi(value0);
    }
    else if (!strcmp(option, "RECEIVE_THREAD")) {
      // initialize the receive_thread (there is only one receive_thread)
      current_thread_type = RECEIVE_THREAD;
    }
    else if (!strcmp(option, "ETH_BLOCK")) {
      current_thread_type = ETH_BLOCK;
    }
  } 
  else if (!strcmp(cmd, "option")) {
    // set new argument

    if (current_thread_type == SWFB_THREAD) {
      // set new SW FB argument
      if (!strcmp(option, "delay_per_packet_cycles")) {
        swfb_args[num_swfb_threads-1].params.cycles_packet_bool = 1;
        swfb_args[num_swfb_threads-1].params.delay_per_packet_cycles = strtoull(value0, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_byte_cycles")) {
        swfb_args[num_swfb_threads-1].params.cycles_byte_bool = 1;
        swfb_args[num_swfb_threads-1].params.delay_per_byte_cycles =strtoull(value0, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_packet_secs")) {
        swfb_args[num_swfb_threads-1].params.cycles_packet_bool = 0;
        swfb_args[num_swfb_threads-1].params.delay_per_packet_secs = strtod(value0, NULL);
      }
      else if (!strcmp(option, "delay_per_byte_secs")) {
        swfb_args[num_swfb_threads-1].params.cycles_byte_bool = 0;
        swfb_args[num_swfb_threads-1].params.delay_per_byte_secs = strtod(value0, NULL);
      }
      else if (!strcmp(option, "drop_type")) {
        swfb_args[num_swfb_threads-1].params.drop_type = strtoul(value0, NULL, 10);
      }
      else if (!strcmp(option, "drop_value")) {
        swfb_args[num_swfb_threads-1].params.drop_value = strtoul(value0, NULL, 10);
      }
      else if (!strcmp(option, "dst_type")) {
        if (!strcmp(value0, "SWFB_THREAD")) {
          swfb_args[num_swfb_threads-1].dst_thread_type[0] = SWFB_THREAD;
        }
        else if (!strcmp(value0, "STATS_THREAD")) {
          swfb_args[num_swfb_threads-1].dst_thread_type[0] = STATS_THREAD;
        }
        else {
          printf("[main] ERROR: dst_thread_type \"%s\" not valid!\n", value0);
        }
      }
      else if (!strcmp(option, "dst_id")) {
        swfb_args[num_swfb_threads-1].dst_thread_id[0] = strtoul(value0, NULL, 10);
      }
    }
    else if (current_thread_type == HWFB) {
      // set new HW FB argument

      if (!strcmp(option, "delay_per_packet_cycles")) {
        hwfb_args[num_hwfbs-1].params.cycles_packet_bool = 1;
        hwfb_args[num_hwfbs-1].params.delay_per_packet_cycles = strtoull(value0, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_byte_cycles")) {
        hwfb_args[num_hwfbs-1].params.cycles_byte_bool = 1;
        hwfb_args[num_hwfbs-1].params.delay_per_byte_cycles =strtoull(value0, NULL, 10);
      }
      else if (!strcmp(option, "delay_per_packet_secs")) {
        hwfb_args[num_hwfbs-1].params.cycles_packet_bool = 0;
        hwfb_args[num_hwfbs-1].params.delay_per_packet_secs = strtod(value0, NULL);
      }
      else if (!strcmp(option, "delay_per_byte_secs")) {
        hwfb_args[num_hwfbs-1].params.cycles_byte_bool = 0;
        hwfb_args[num_hwfbs-1].params.delay_per_byte_secs = strtod(value0, NULL);
      }
      else if (!strcmp(option, "drop_type")) {
        hwfb_args[num_hwfbs-1].params.drop_type = strtoul(value0, NULL, 10);
      }
      else if (!strcmp(option, "drop_value")) {
        hwfb_args[num_hwfbs-1].params.drop_value = strtoul(value0, NULL, 10);
      }
      else if (!strcmp(option, "randgen_seed")) {
        hwfb_args[num_hwfbs-1].randgen_seed = strtoll(value0, NULL, 16);
      }
      else if (!strcmp(option, "dst_global_addr")) {
        hwfb_args[num_hwfbs-1].dst_global_addr = strtoul(value0, NULL, 10);
      }
      else if (!strcmp(option, "dst_local_addr")) {
        hwfb_args[num_hwfbs-1].dst_local_addr = strtoul(value0, NULL, 10);
      }
    }
    else if (current_thread_type == STATS_THREAD) {
      // no options available yet
    }
    else if (current_thread_type == RECEIVE_THREAD) {
      // add new dst_addr for receive_thread
      receive_args->num_dst_addrs++;

      // allocate space
      unsigned int *dst_idp_tmp;
      dst_idp_tmp = realloc(receive_args->dst_idp, receive_args->num_dst_addrs*sizeof(*(receive_args->dst_idp)));
      if (dst_idp_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      receive_args->dst_idp = dst_idp_tmp;

      thread_type_enum *dst_thread_type_tmp;
      dst_thread_type_tmp = realloc(receive_args->dst_thread_type, receive_args->num_dst_addrs*sizeof(*(receive_args->dst_thread_type)));
      if (dst_thread_type_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      receive_args->dst_thread_type = dst_thread_type_tmp;

      unsigned int *dst_thread_id_tmp;
      dst_thread_id_tmp = realloc(receive_args->dst_thread_id, receive_args->num_dst_addrs*sizeof(*(receive_args->dst_thread_id)));
      if (dst_thread_id_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      receive_args->dst_thread_id = dst_thread_id_tmp;

      // insert values
      receive_args->dst_idp[receive_args->num_dst_addrs - 1] = strtod(value0, NULL);
      if (!strcmp(value1, "SWFB_THREAD")) {
        receive_args->dst_thread_type[receive_args->num_dst_addrs - 1] = SWFB_THREAD;
      }
      else if (!strcmp(value1, "STATS_THREAD")) {
        receive_args->dst_thread_type[receive_args->num_dst_addrs - 1] = STATS_THREAD;
      }
      else {
        printf("[main] ERROR: dst_thread_type \"%s\" not valid!\n", value1);
      }
      receive_args->dst_thread_id[receive_args->num_dst_addrs - 1] = strtod(value2, NULL);

    }
    else if (current_thread_type == ETH_BLOCK) {
      // add new mapping for ETH block
      eth_args->num_mappings++;

      // allocate space
      unsigned long long *hash_tmp;
      hash_tmp = realloc(eth_args->hash, eth_args->num_mappings*sizeof(*(eth_args->hash)));
      if (hash_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      eth_args->hash = hash_tmp;

      unsigned int *dst_idp_tmp;
      dst_idp_tmp = realloc(eth_args->dst_idp, eth_args->num_mappings*sizeof(*(eth_args->dst_idp)));
      if (dst_idp_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      eth_args->dst_idp = dst_idp_tmp;

      unsigned int *noc_addr_tmp;
      noc_addr_tmp = realloc(eth_args->noc_addr, eth_args->num_mappings*sizeof(*(eth_args->noc_addr)));
      if (noc_addr_tmp == NULL) {
        printf("[main] ERROR: realloc failed\n");
        exit(1);
      }
      eth_args->noc_addr = noc_addr_tmp;


      // insert values
      eth_args->hash[eth_args->num_mappings - 1] = strtoull(value0, NULL, 16);

      eth_args->dst_idp[eth_args->num_mappings - 1] = strtod(value1, NULL);

      if (!strcmp(value2, "TO_H2S")) {
        eth_args->noc_addr[eth_args->num_mappings - 1] = TO_H2S;
      }
      else if (!strcmp(value2, "TO_FB0")) {
        eth_args->noc_addr[eth_args->num_mappings - 1] = TO_FB0;
      }
      else if (!strcmp(value2, "TO_FB1")) {
        eth_args->noc_addr[eth_args->num_mappings - 1] = TO_FB1;
      }
      else if (!strcmp(value2, "TO_FB2")) {
        eth_args->noc_addr[eth_args->num_mappings - 1] = TO_FB2;
      }
      else {
        printf("[main] ERROR: noc_addr \"%s\" not valid!\n", value2);
      }
    }
  }
  
  free(cmd);
  free(option);
  free(value0);
  free(value1);
  free(value2);
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

void print_args_from_file() {
  int i;

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

  // print receive_args
  printf("[main] RECEIVE_THREAD routing:\n");
  for (i=0; i<receive_args->num_dst_addrs; i++) {
    if (receive_args->dst_thread_type[i] == SWFB_THREAD) {
      printf("        idp %u -> SWFB_THREAD %u\n", receive_args->dst_idp[i], receive_args->dst_thread_id[i]);
    }
    else if (receive_args->dst_thread_type[i] == STATS_THREAD) {
      printf("        idp %u -> STATS_THREAD %u\n", receive_args->dst_idp[i], receive_args->dst_thread_id[i]);
    }
    else {
      printf("ERROR: invalid dst_thread_type \"%d\" -> cannot be printed\n", receive_args->dst_thread_type[i]);
    }
  }

  // print eth_args
  printf("[main] ETH_BLOCK routing:\n");
  for (i=0; i<eth_args->num_mappings; i++) {
    if (eth_args->noc_addr[i] == TO_H2S) {
      printf("        hash 0x%08llx -> idp %u, TO_H2S\n", eth_args->hash[i], eth_args->dst_idp[i]);
    }
    else if (eth_args->noc_addr[i] == TO_FB0) {
      printf("        hash 0x%08llx -> idp %u, TO_FB0\n", eth_args->hash[i], eth_args->dst_idp[i]);
    }
    else if (eth_args->noc_addr[i] == TO_FB1) {
      printf("        hash 0x%08llx -> idp %u, TO_FB1\n", eth_args->hash[i], eth_args->dst_idp[i]);
    }
    else if (eth_args->noc_addr[i] == TO_FB2) {
      printf("        hash 0x%08llx -> idp %u, TO_FB2\n", eth_args->hash[i], eth_args->dst_idp[i]);
    }
    else {
      printf("ERROR: invalid noc_addr \"%d\" -> cannot be printed\n", eth_args->noc_addr[i]);
    }
  }
}
