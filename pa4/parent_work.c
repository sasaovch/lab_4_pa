#include "ipc.h"
#include "pipes_const.h"
#include "pa2345.h"
#include "banking.h"
#include "time_work.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

int init_parent_work(void *__info, int N) {
    local_id line = 0;
    local_id column = 0;

    while (line < N) {
        column = 0;
        while (column < N) {
            if (line == column) {
                column++;
            } else {
                int to_close;
                
                if (column != 0 && pm[line][column][0] != -1) {
                    to_close = pm[line][column][0];
                    pm[line][column][0] = -1;
                    close(to_close);                    
                }
                
                if (line != 0 && pm[line][column][1] != -1) {
                    to_close = pm[line][column][1];
                    pm[line][column][1] = -1;
                    close(to_close);
                }
                
                column++;
            }
        }
        line++;
    }

    local_id child_i = 1;
    while (child_i < N) {
        Info info = {.fork_id = 0, .N = N};

        for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 10; j++) {
                for(int k = 0; k < 2; k++) {
                    info.pm[i][j][k] = pm[i][j][k];
                }
            }
        }
        Message started_msg;

        int status = receive(&info, child_i, &started_msg);
        if (status != 0) continue;

        if (started_msg.s_header.s_type == STARTED) {
            sync_lamport_time(&pipe_info, started_msg.s_header.s_local_time);
            child_i++;
        }

        started_msg.s_header.s_payload_len = 0;
        memset(started_msg.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
    }
    return 0;
}

void do_parent_work(void *__info, int N) {
    Info *parent_info = (Info *) __info;

    local_id child_i = 1;
    while (child_i < N) {
        Message receive_msg;
        receive(parent_info, child_i, &receive_msg);
        sync_lamport_time(parent_info, receive_msg.s_header.s_local_time);

        if (receive_msg.s_header.s_type == DONE) {
            child_i++;
        }

        receive_msg.s_header.s_payload_len = 0;
        receive_msg.s_header.s_type = 0;
    }
}

void parent_are_waiting(void *__info, int N) {
    local_id child = 1;
    while (child < N) {
        wait(NULL);
        child++;
    }
}
