

#ifndef __ARGS_FROM_FILE_H__
#define __ARGS_FROM_FILE_H__

/******************
 * Functions Prototypes
 ******************/
int file_exist (char *filename);
void read_args_handle_line(char *line, struct swfb_thread_args_struct **swfb_args_ptr, struct hwfb_args_struct **hwfb_args_ptr, struct stats_thread_args_struct **stats_args_ptr);
void read_args_from_file(char *config_file, struct swfb_thread_args_struct **swfb_args, struct hwfb_args_struct **hwfb_args, struct stats_thread_args_struct **stats_args);
void print_args_from_file();


#endif // __ARGS_FROM_FILE_H__

