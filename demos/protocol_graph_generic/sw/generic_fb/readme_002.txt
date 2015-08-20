COMPILATION
===========

Compile for Desktop Ubuntu:
> make dt

Compile for Microblaze (FPGA Softcore)
> make mb


OPTIONS
=========

-h <secs>   : Measure packet rate in the HW generic FB (WITHOUT any SW threads running (except main)) [double]
              NOTE: idp cannot be selected, all packets that pass through the HW FB are counted
              [not working]

-w <secs>   : Measure packet rate in the HW generic FB (WITH SW threads running) [double]
              NOTE: idp cannot be selected, all packets that pass through the HW FB are counted
              [not yet implemented]

-r <secs>   : Measure packet rate in the stats_thread, measure for <secs> seconds [double]
              NOTE: idp must be selected (-s argument)

-t          : Measure the time between 2 packets, the time is measured <num> times -> output = <num> values [integer]
              NOTE: idp must be selected (-s argument)

-i <num>    : select idp for measurement [integer]

-n <num>    : number of repetition of a measurement (works with -h, -r or -t option) [integer]

-c          : continously (every second) print the counter values for each idp (values counted in the stats_thread) [bool]

-v          : verbose (print debug infos) [bool]

-l          : logging mode (don't print anything but the measurements) [bool]









NOTES
=====
 - evt. replace -pthread with -lpthread for microblaze compilation in the Makefile (http://stackoverflow.com/questions/1662909/undefined-reference-to-pthread-create-in-linux)

 - packets are not counted in HW FB if the receive thread is not created !!!
