COMPILATION
===========

Compile for Desktop Ubuntu (FPGA / ReconOS specific parts will be omitted):
> make dt

Compile for Microblaze (FPGA soft core)
> make mb

STRUCTURE of the SOFTWARE
=========================
generic_fb      : contains the main part of the implementation
                  contains all the threads (main, receive, swfb, stats), the hardware & 
                  software initialization, the part to perform measurements

args_from_file  : contains all parts necessary to read in the configuration from the 
                  configuration file
                  
timing          : taken over without modification from the 'app' example software



OPTIONS
=========

-h <secs>   : Measures the packet rate in the HW generic functional block (WITHOUT any 
              software threads running (except the main thread)). The packet rate is 
              measured over an interval of <secs> seconds. 
              NOTE: The IDP cannot be selected, all packets that pass through the HW FB 
              are counted. 
              [double]

-w <secs>   : Measures the packet rate in the HW generic FB (WITH the software threads 
              running). The packet rate is measured over an interval of <secs> seconds. 
              NOTE: The IDP cannot be selected, all packets that pass through the HW FB 
              are counted. 
              [double]

-r <secs>   : Measures the packet rate in the stats_thread. The packet rate is measured 
              over an interval of <secs> seconds. 
              NOTE: The IDP must be selected with the -i argument. 
              [double]
              
-p <secs>   : Measures the packet rate in stats_threads over time. Logs packet count and 
              timestamp for 2 predefined IDPs (no. 8 & 9) into an array and prints it 
              after the measurement of all samples is done (in order not to influence the 
              measurement with printf())
              <secs> is the interval between two samples. With -n the number of samples 
              is selected. 
              Output format: Every line consists of: 
              <timestamp in usecs>,<packet count idp=8>,<packet count idp=9>
              [double]

-t          : Measures the time between 2 consecutive packets.  
              NOTE: The IDP must be selected with the -i argument. 
              [bool]

-i <num>    : Selects the IDP for the measurement (works with -r, -t)
              [integer]

-n <num>    : Number of repetitions of a measurement (works with -h, -w, -r, -p or -t)
              [integer]

-c          : Continously (every second) prints the counter values for all IDPs. The 
              values are counted in the stats_thread.  
              [bool]

-d          : Continously (every second) prints the counter values of all HW generic FBs.  
                [bool]

-v          : Verbose (print debug infos) 
              [bool]

-l          : Logging mode (don't print anything except the measurements) 
              [bool]
              
-f <file>   : Filename of the file that should be used as config file. Without this option
              the default filename "generic_fb.config" is used. 
              [string]


CONFIGURATION FILE
==================

Every valid line has the following structure:
<command> <option> <value0> [<value1>] [<value2>]
All other lines are ignored (including the lines starting with a #-character). 

The lines are grouped into blocks. Each blocks starts with the 'config'
command which is followed by zero or more 'option' command lines which configure the 
entity described in the previous 'config' line. 
The structure of the line that starts a new entity is as follows:
config <entity name> <entity ID>

NOTE: All IDs of the entities (e.g. config HWFB 0) must start from 0 and must not skip 
      one number! The order of the blocks with respect to their IDs doesn't matter. Also 
      the order of the options within a block doesn't matter. 

NOTE: The behavior if one of the options below is not explicitly specified is not tested 
      and therefore the software might not work as expected in such cases.

Hardware gneric FB (Entity name: HWFB)
------------------
EXAMPLE:

config HWFB 0
	option delay_per_packet_secs 	5
	option delay_per_byte_cycles 	0
	option drop_type 				      0
	option drop_value 				    0
	option randgen_seed 			    0x74d9a2fb
	option dst_global_addr			  1
	option dst_local_addr			    1

OPTIONS:
delay_per_packet_<?>  : In the functional block every packet is delayed by this value. 
                        The delay per packet can be specified either by a number of 
                        cycles (delay_per_packet_cycles) OR a number of seconds 
                        (delay_per_packet_secs). Internally the value is set in number of 
                        cycles as a 64 bit value.
                        cycles: [decimal integer] (valid range: 0 to 2^64 - 1)
                        secs: [double]
                        
delay_per_byte_<?>    : For every byte the packet in the functional block is delayed by 
                        this value. The delay per byte can be specified either by a 
                        number of cycles (delay_per_byte_cycles) OR a number of seconds 
                        (delay_per_byte_secs). Internally the value is set in number of 
                        cycles as a 64 bit value. 
                        cycles: [decimal integer] (valid range: 0 to 2^64 - 1)
                        secs: [double]

drop_type             : There are three drop types which must be set by specifying an
                        integer value. The three types are
                          0: drop disabled: no packets are dropped
                          1: drop fix: every drop_value-th packet will be dropped
                          2: drop random: every ((2^32)/drop_value)-th packet will be 
                             dropped on average
                        For both cases drop_type=1 and drop_type=2 the setting 
                        drop_value=0 is not valid and will disable the drop functionality,
                        i.e. no packets are dropped. To drop every packet the parameters 
                        can be set to drop_type=1 and drop_value=1. With this 
                        configuration the generic functional block acts as a black hole.
                        [decimal integer]

drop_value            : The value which is used to for the drop functionality. The exact
                        meaning depends on the drop_type. 
                        [decimal integer] (valid range: 0 to 2^32 - 1)

randgen_seed          : This option is used to specify the seed for the random generator
                        which is used for the 'drop random' mode of the drop 
                        functionality.
                        [32 bit hexadecimal integer]

dst_global_addr       : Specifies the destination switch in the NoC for the packets that 
                        are sent out of the hardware generic functional block. The setup 
                        of the NoC is described in the readme.txt in the root directory 
                        of this demo.
                        [decimal integer]

dst_local_addr        : Specifies the destination block in the NoC for the packets that 
                        are sent out of the hardware generic functional block. The setup 
                        of the NoC is described in the readme.txt in the root directory 
                        of this demo.
                        [decimal integer]

	
Software generic FB (Entity name: SWFB_THREAD)
-------------------
EXAMPLE:

config SWFB_THREAD                0
	option delay_per_packet_cycles 	500
	option delay_per_byte_secs 	    0
	option drop_type 		            1
	option drop_value 		          2
	option dst_type			            SWFB_THREAD
	option dst_id			              1
	
OPTIONS:
delay_per_packet_<?>  : In the functional block every packet is delayed by this value. 
                        The delay per packet can be specified either by a number of 
                        cycles (delay_per_packet_cycles) OR a number of seconds 
                        (delay_per_packet_secs). 
                        cycles: [decimal integer]
                        secs: [double]
                        
delay_per_byte_<?>    : For every byte the packet in the functional block is delayed by 
                        this value. The delay per byte can be specified either by a 
                        number of cycles (delay_per_byte_cycles) OR a number of seconds 
                        (delay_per_byte_secs). 
                        cycles: [decimal integer]
                        secs: [double]
                        
drop_type             : There are three drop types which must be set by specifying an
                        integer value. The three types are
                          0: drop disabled: no packets are dropped
                          1: drop fix: every drop_value-th packet will be dropped
                          2: drop random: every ((2^32)/drop_value)-th packet will be 
                             dropped on average
                        For both cases drop_type=1 and drop_type=2 the setting 
                        drop_value=0 is not valid and will disable the drop functionality,
                        i.e. no packets are dropped. To drop every packet the parameters 
                        can be set to drop_type=1 and drop_value=1. With this 
                        configuration the generic functional block acts as a black hole.
                        [decimal integer]

drop_value            : The value which is used to for the drop functionality. The exact
                        meaning depends on the drop_type. 
                        [decimal integer] (valid range: 0 to 2^32 - 1)

dst_type              : The thread type of the thread to which the packets shall be 
                        forwarded.  
                        [valid values: SWFB_THREAD, STATS_THREAD]

dst_id                : The thread ID of the thread to which the packets shall be 
                        forwarded.
                        [decimal integer]


Stats Thread (Entity name: STATS_THREAD)
------------
EXAMPLE:

config STATS_THREAD 0

OPTIONS:
There are no options available for this entity. 


Receive Thread (Entity name: RECEIVE_THREAD)
--------------
EXAMPLE: 

config RECEIVE_THREAD
	option idp 3 SWFB_THREAD  0
	option idp 4 STATS_THREAD 0
	
NOTE: For this entity there is no ID required since there is only one instance of this 
entity.  

OPTIONS:
idp                   : Specifies the routing of packets with the corresponding IDP. 
                        This option can be specified multiple times.  
                         value0: IDP [decimal integer]
                         value1: The thread type of the thread to which the packets shall 
                                 be forwarded. [valid values: SWFB_THREAD, STATS_THREAD]
                         value2: The thread ID of the thread to which the packets shall  
                                 be forwarded. [decimal integer]


ETH Block (Entity name: ETH_BLOCK)
---------
EXAMPLE:
 
config ETH_BLOCK
	option hash 0xabababa0abababa0 2 TO_FB0
	option hash 0xcdcdcdcdcdcdcdcd 3 TO_H2S
	
NOTE: For this entity there is no ID required since there is only one instance of this 
entity.  

OPTIONS:
hash                  : Specifies the mapping from the 8 byte hash (external header) to 
                        the IDP and the NoC address (internal header) which is used for 
                        the header translation in the ETH block. This implicitly defines
                        the routing of the packets at the ETH block. 
                        This option can be specified multiple times.
                         value0: hash [hexadecimal integer]
                         value1: IDP [decimal integer]
                         value2: NoC address [valid values: TO_H2S, TO_FB0, TO_FB1, 
                                 TO_FB2]



NOTES
=====
 - The implementation is prepared to create multiple stats_threads but this has not been
   tested extensively and therefore might not work as expected 
 - It might be necessary to replace -pthread with -lpthread for compilation on different 
   platforms in the Makefile (http://stackoverflow.com/questions/1662909/undefined-reference-to-pthread-create-in-linux)
 - Packets are not counted in HW FB if the receive thread is not created (maybe they 
   don't even pass through the HW FB if the receive thread is not created)
