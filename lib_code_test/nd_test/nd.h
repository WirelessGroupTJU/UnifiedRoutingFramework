#ifndef ND_H
#define ND_H

#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "nd_cfg.h"
#include "nd_interface.h"
#include "nd_timer_queue.h"

int init_socket(char* IF);
int bind_socket (int fd, int listen_port, struct sockaddr_in *addr);
int init_interface_entry_list();
void clean_interface_entry_list();

void *main_loop(void* arg);

//send a hello message and prepare a new hello message for a timer
void send_hello_message(void *data);
void start_discovery();

//delete a old one-hop entry when a timer is timeout
void delete_one_hop_entry(void *data);

//deal timer queue
void timeout_handle(int type);
void register_signal();

//ND API
int nd_turn_up();
void nd_turn_down();

int nd_set_hello_interval(char* name, uint64_t new_hello_interval);
int nd_send_hello_now(char* name);


void debug();

#endif
