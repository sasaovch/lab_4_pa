#include "child_work.h"
#include "ipc.h"
#include "pipes_const.h"
#include "time_work.h"
#include "common.h"
#include "pa2345.h"
#include "time_work.h"
#include "banking.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


int init_child_work(void* __child_state) {

    ChildState* child_state = (ChildState *) __child_state;
    local_id child_id = child_state->fork_id;
    int N = child_state->N;        

    pipe_info.fork_id = child_id;
    pipe_info.N = N;
    pipe_info.local_time = child_state->child_time;
    pipe_info.is_mutexl = child_state->is_mutexl;
    pipe_info.received_done_msg = 0;

    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            for(int k = 0; k < 2; k++) {
                pipe_info.pm[i][j][k] = pm[i][j][k];
            }
        }
    }

    timestamp_t time_started = get_lamport_time();

    local_id line = 0;
    local_id column = 0;

    while (line < N) {
        column = 0;
        while (column < N) {
            if (line == column) {
                column++;
            } else {
                int to_close;
                
                if (column != child_id && pm[line][column][0] != -1) {
                    to_close = pm[line][column][0];
                    pm[line][column][0] = -1;
                    close(to_close);                    
                }
                
                if (line != child_id && pm[line][column][1] != -1) {
                    to_close = pm[line][column][1];
                    pm[line][column][1] = -1;
                    close(to_close);
                }
                
                column++;
            }
        }
        line++;
    }

    Message start_msg;
    int payload_len = sprintf(start_msg.s_payload, log_started_fmt, time_started, child_id, getpid(), getppid(), 0);
    pipe_info.local_time++;

    start_msg.s_header.s_magic = MESSAGE_MAGIC;
    start_msg.s_header.s_payload_len = payload_len;
    start_msg.s_header.s_type = STARTED;
    start_msg.s_header.s_local_time = get_lamport_time();

    send_multicast(&pipe_info, &start_msg);
    
    local_id childs = 1;
    while (childs < N) {
        if (childs == child_id) {
            childs++;
            continue;
        }
        Message msg;

        int status = receive(&pipe_info, childs, &msg);
        if (status != 0) continue;
        if (msg.s_header.s_type == STARTED) {
            sync_lamport_time(&pipe_info, msg.s_header.s_local_time);
            childs++;
        }

        msg.s_header.s_payload_len = 0;
        memset(msg.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
    }

    child_state->child_time = pipe_info.local_time;

    return 0;
}

int handle_transfers(void* __child_state) {
    ChildState* child_state = (ChildState *) __child_state;
    local_id child_id = child_state->fork_id;
    int N = child_state->N;
    int is_mutexl = pipe_info.is_mutexl;
 
    Message msg_r;
    msg_r.s_header.s_type = 0;

    int messages_number = child_id * 5;
  
    if (is_mutexl) {
        request_cs(&pipe_info);        
    }
  

    for (int i = 1; i <= messages_number; i++) {
        char str[128];
        memset(str, 0, sizeof(str));
        sprintf(str, log_loop_operation_fmt, child_id, i, messages_number);
        print(str);
        
        fprintf(elf, log_loop_operation_fmt, child_id, i, messages_number);
        fflush(elf);

        fprintf(stdout, log_loop_operation_fmt, child_id, i, messages_number);
        fflush(stdout);

    }
  
    if (is_mutexl) {
        release_cs(&pipe_info);
    }

    int wait_for_others_to_stop = N - 2 - pipe_info.received_done_msg;

    Message done_msg;
    timestamp_t time = get_lamport_time();

    int payload_len = sprintf(done_msg.s_payload, log_done_fmt, time, child_id, 0);

    pipe_info.local_time++;

    done_msg.s_header.s_magic = MESSAGE_MAGIC;
    done_msg.s_header.s_payload_len = payload_len;
    done_msg.s_header.s_type = DONE;
    done_msg.s_header.s_local_time = get_lamport_time();

    send_multicast(&pipe_info, &done_msg);
    
    Message msg_d;
    while (wait_for_others_to_stop > 0) {
        msg_d.s_header.s_type = 0;
        msg_d.s_header.s_payload_len = 0;
        memset(msg_d.s_payload, '\0', sizeof(char) * MAX_PAYLOAD_LEN);
        
        pipe_info.local_time++;
        int from_id = receive_any(&pipe_info, &msg_d);

        sync_lamport_time(&pipe_info, msg_d.s_header.s_local_time);

        if (msg_d.s_header.s_type <= DONE) {
            wait_for_others_to_stop--;
        }
        if (msg_d.s_header.s_type == CS_REQUEST) {

            Message reply_msg;
            pipe_info.local_time++;

            reply_msg.s_header.s_magic = MESSAGE_MAGIC;
            reply_msg.s_header.s_payload_len = 0;
            reply_msg.s_header.s_type = CS_REPLY;
            reply_msg.s_header.s_local_time = get_lamport_time();            
        
            send(&pipe_info, from_id, &reply_msg);
        }
    }

    for (local_id i = 0; i < N; i++) {
        for (local_id j = 0; j < N; j++) {
        if (i != j) {
            if (pm[i][j][0] != -1) close(pm[i][j][0]);
            if (pm[i][j][1] != -1) close(pm[i][j][1]);
        }
        }
    } 

    return 0;
}
