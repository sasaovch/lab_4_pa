#include "ipc.h"
#include "pa2345.h"
#include "child_work.h"
#include "priority_queue.h"
#include "pipes_const.h"
#include "time_work.h"
#include "banking.h"
#include <stdio.h>

int request_cs(const void *self) {
    Info* pipe_info = (Info *) self;
    local_id pipe_id = pipe_info->fork_id;
    int N = pipe_info->N;

    pipe_info->local_time++;
    
    PriorityQueueElement element;
    element.pipe_id = pipe_id;
    element.timestamp = get_lamport_time();
               
    push(element);

    Message request_msg;
    request_msg.s_header.s_magic = MESSAGE_MAGIC;
    request_msg.s_header.s_type = CS_REQUEST;
    request_msg.s_header.s_local_time = get_lamport_time();
    request_msg.s_header.s_payload_len = 0;

    timestamp_t timestamp = get_lamport_time();


    send_multicast(pipe_info, &request_msg);

    int replies_left = N - 2;
    while (1) {
        if (replies_left <= 0 && peek().pipe_id == pipe_id) {
            break;
        }

        Message received_msg;
        PriorityQueueElement received_element;
        received_element.pipe_id = 0;
        received_element.timestamp = 0;
        local_id from_id = receive_any(pipe_info, &received_msg);
        sync_lamport_time(pipe_info, received_msg.s_header.s_local_time);

        switch (received_msg.s_header.s_type) {
            case CS_REPLY:
                replies_left--;
                break;

            case CS_REQUEST:
                replies_left--;
                received_element.pipe_id = from_id;
                received_element.timestamp = received_msg.s_header.s_local_time;
         
                push(received_element);

                // fprintf(stdout, "%d pipe %d\n", pipe_id, timestamp);
                // fflush(stdout);

                // fprintf(stdout, "%d recieve %d\n", received_element.pipe_id, received_element.timestamp);
                // fflush(stdout);

                // fprintf(stdout, "%d pipe %d - %d request %d - %d recieve %d\n", pipe_id, timestamp, peek().pipe_id, peek().timestamp, received_element.pipe_id, received_element.timestamp);
                // fflush(stdout);

                if (peek().pipe_id != pipe_id) {
                    Message reply_msg;
                    pipe_info->local_time++;

                    reply_msg.s_header.s_magic = MESSAGE_MAGIC;
                    reply_msg.s_header.s_payload_len = 0;
                    reply_msg.s_header.s_type = CS_REPLY;
                    reply_msg.s_header.s_local_time = get_lamport_time();                      
                    send(pipe_info, from_id, &reply_msg);
                }

                break;

            case CS_RELEASE:  
                replies_left--;             
                pop();
                break;

            case DONE:
                pipe_info->received_done_msg++;
                break;
        }
    }
    return 0;
}

int release_cs(const void *self) {
    Info* pipe_info = (Info *) self;
    pop();

    Message release_msg;
    pipe_info->local_time++;

    release_msg.s_header.s_magic = MESSAGE_MAGIC;
    release_msg.s_header.s_payload_len = 0;
    release_msg.s_header.s_type = CS_RELEASE;
    release_msg.s_header.s_local_time = get_lamport_time(); 

    send_multicast(pipe_info, &release_msg);
    return 0;
}
