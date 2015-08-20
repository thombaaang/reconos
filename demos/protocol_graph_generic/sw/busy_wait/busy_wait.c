/*

arguments:

  -o:   operations per sample (default: 1000000):   how many times the dummy operation is executed for every sample loop
  -s:   samples (default: 1):                       how many samples are collected
  -p:   output_for_plot (default: false):           option to enable or disable special printing (for piping into file to automatically generate histogram)
  -a    output_avg_only (default: false):           only print average of execution time per operation averaged over all samples AND input parameters


*/


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

// forward declaration of functions
unsigned long timevaldiff(struct timeval *starttime, struct timeval *finishtime);
void busy_wait(struct timeval delay);
void busy_wait_stats(long long operations, long long samples, char output_for_print, char output_avg_only);
void read_cpu_stat(char *line);

unsigned long timevaldiff(struct timeval *starttime, struct timeval *finishtime)
{
  unsigned long usec;
  usec = abs(finishtime->tv_sec-starttime->tv_sec)*1000000;
  usec += abs(finishtime->tv_usec-starttime->tv_usec);
  return usec;
}

void busy_wait(struct timeval delay) {
    struct timeval current, end;
    long num = 0;
    
    gettimeofday(&current, NULL);
    
    if (current.tv_usec + delay.tv_usec > 1000000) {
        end.tv_sec = current.tv_sec + delay.tv_sec + 1;
        end.tv_usec = current.tv_usec + delay.tv_usec - 1000000;
    }
    else {
        end.tv_sec = current.tv_sec + delay.tv_sec;
        end.tv_usec = current.tv_usec + delay.tv_usec;
    }
    
    while(current.tv_sec < end.tv_sec || ((current.tv_sec = end.tv_usec) && (current.tv_usec < end.tv_usec))) {
        // dummy calculation
        num += 1;
        
        // refresh time value
        gettimeofday(&current, NULL);
    }
    
}

void read_cpu_stat(char *line) {
  FILE *fp;

  /* Open the command for reading. */
  fp = popen("cat /proc/stat | grep cpu ", "r"); // last blank space is important to select "cpu" and not all "cpu" and "cpu0" and so on!
  if (fp == NULL) {
    printf("[read_cpu_stat] Failed to run command\n" );
    exit(1);
  }

  // read "file" into variable 'line'
  fgets(line, 1000-1, fp);
  

  /* Read the output a line at a time - output it. */
  /*
  while (fgets(line, 1000-1, fp) != NULL) {
    printf("[read_cpu_stat] %s\n", line);
  }
  */

  pclose(fp);
}

void busy_wait_stats(long long operations, long long samples, char output_for_print, char output_avg_only) {
  struct timeval start, end, current;
  unsigned long diff;
  double sum = 0;
  double execution_time = 0;
  long long i, k;
  unsigned long long int jiffies1[10], jiffies2[10];
  long working_jiffies1, working_jiffies2, total_jiffies1, total_jiffies2;
  char line[1000];
  double cpu_load = 0;
  
  for (i=0; i < samples; i++) {
    // store jiffies1
    if (!output_for_print && !output_avg_only) {
      read_cpu_stat(&line);
      //printf("line: %s", line);
      sscanf(line,"%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", 
        &jiffies1[0], &jiffies1[1], &jiffies1[2], &jiffies1[3], &jiffies1[4], &jiffies1[5], &jiffies1[6], &jiffies1[7], &jiffies1[8], &jiffies1[9]);
    }

    // store start timestamp
    gettimeofday(&start, NULL);
    
    for(k=0; k < operations; k++) {
      // dummy operation (to keep the cpu busy)
      
      //gettimeofday(&current, NULL);
      asm("nop");
    }
    //sleep(1);
    
    // store end timestamp
    gettimeofday(&end, NULL);
    
    // store jiffies2
    if (!output_for_print && !output_avg_only) {
      read_cpu_stat(&line);
      sscanf(line,"%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", 
        &jiffies2[0], &jiffies2[1], &jiffies2[2], &jiffies2[3], &jiffies2[4], &jiffies2[5], &jiffies2[6], &jiffies2[7], &jiffies2[8], &jiffies2[9]);
    }
    
    // calculate time diff  
    diff = timevaldiff(&end, &start);
    
    // calculate cpu load
    working_jiffies1 = jiffies1[0] + jiffies1[1] + jiffies1[2];
    working_jiffies2 = jiffies2[0] + jiffies2[1] + jiffies2[2];
    total_jiffies1 = working_jiffies1 + jiffies1[3] + jiffies1[4] + jiffies1[5] + jiffies1[6] + jiffies1[7] + jiffies1[8] + jiffies1[9];
    total_jiffies2 = working_jiffies2 + jiffies2[3] + jiffies2[4] + jiffies2[5] + jiffies2[6] + jiffies2[7] + jiffies2[8] + jiffies2[9];
    cpu_load = 100 * ((double) (working_jiffies2 - working_jiffies1)) / ((double) (total_jiffies2 - total_jiffies1));
    
    execution_time = ((double) diff)/((double) operations);
    sum += execution_time;
    
    // print stuff
    if (output_for_print && !output_avg_only) {
      // print for histogram plot (for piping into file to automatically generate histogram)
      if (i == samples-1) {
        printf("%f", execution_time); 
      }
      else {
        printf("%f;", execution_time); 
      }
    }
    else if (!output_avg_only) {
      // print for inspection on command line
      printf("-------------------\nsample no. %lld\n", i+1);
      printf("execution time for executing the dummy op %lld times: %lu us\n", operations, diff);
      printf("execution time per dummy operation: %f us\n", execution_time); 
      printf("cpu_load: %1.2f%%\n", cpu_load);
    }
  }
  if (output_avg_only) {
    // indicate new run
    printf("-----------------\n");
    // print program parameters
    printf("#operations per sample: %lld\n#samples: %lld\n", operations, samples);
    // average execution time per dummy op averaged over all samples collected
    printf("average execution time per dummy op: %f us\n", sum/samples);
  }
}


// MAIN ////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  long long operations = 1000000;  // how many times the dummy operation is executed for every sample loop
  long long samples = 1;  // how many samples are collected
  char output_for_plot = 0; // option to enable or disable special printing (for piping into file to automatically generate histogram)
  char output_avg_only = 0; // only print average of execution time per operation averaged over all samples AND input parameters
  int c;
  
  // parse arguments
  while ((c = getopt(argc, argv, "o:s:pa")) != -1) {
    switch (c) {
      case 'o':
        operations = atoll(optarg);
        break;
      case 's':
        samples = atoll(optarg);
        break;
      case 'p':
        output_for_plot = 1;
        break;
      case 'a':
        output_avg_only = 1;
        break;
      case '?':
        if (optopt == 'o')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (optopt == 's')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
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
  
  // busy wait
  busy_wait_stats(operations, samples, output_for_plot, output_avg_only);
	
	return 0; 
}
