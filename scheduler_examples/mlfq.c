#include "mlfq.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "msg.h"

#define LEVELS 3
#define QUANTUM_LEVEL_0 100
#define QUANTUM_LEVEL_1 200
#define QUANTUM_LEVEL_2 400

typedef struct {
    queue_t queues[LEVELS];
} mlfq_t;

static mlfq_t scheduler = {0};

void mlfq_scheduler(uint32_t current_time_ms, queue_t *ready_queue, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->cpu_time_ms += TICKS_MS;
        (*cpu_task)->quantum_used_ms += TICKS_MS;

        int quantum;
        if ((*cpu_task)->mlfq_level == 0) quantum = QUANTUM_LEVEL_0;
        else if ((*cpu_task)->mlfq_level == 1) quantum = QUANTUM_LEVEL_1;
        else quantum = QUANTUM_LEVEL_2;

        if ((*cpu_task)->cpu_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free(*cpu_task);
            *cpu_task = NULL;
        } else if ((*cpu_task)->quantum_used_ms >= quantum && (*cpu_task)->mlfq_level < LEVELS - 1) {
            // desce um nível
            (*cpu_task)->mlfq_level++;
            (*cpu_task)->quantum_used_ms = 0;
            enqueue_pcb(&scheduler.queues[(*cpu_task)->mlfq_level], *cpu_task);
            *cpu_task = NULL;
        }
    }

    if (*cpu_task == NULL) {
        for (int i = 0; i < LEVELS; i++) {
            pcb_t *task = dequeue_pcb(&scheduler.queues[i]);
            if (task) {
                *cpu_task = task;
                break;
            }
        }
    }

    // Adiciona novos processos recebidos para o nível 0
    queue_elem_t *elem = ready_queue->head;
    while (elem) {
        pcb_t *pcb = elem->pcb;
        pcb->mlfq_level = 0;   // todos começam no nível 0
        enqueue_pcb(&scheduler.queues[0], pcb);
        elem = elem->next;
    }
    ready_queue->head = ready_queue->tail = NULL;
}
