#include "sjf.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->cpu_time_ms += TICKS_MS;  // incrementa o tempo de CPU
        if ((*cpu_task)->cpu_time_ms >= (*cpu_task)->time_ms) {
            // processo terminou
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }

    if (*cpu_task == NULL) {
        // Seleciona o processo mais curto da fila
        queue_elem_t *prev = NULL;
        queue_elem_t *shortest_prev = NULL;
        queue_elem_t *elem = rq->head;
        queue_elem_t *shortest_elem = NULL;

        while (elem != NULL) {
            if (shortest_elem == NULL || elem->pcb->time_ms < shortest_elem->pcb->time_ms) {
                shortest_elem = elem;
                shortest_prev = prev;
            }
            prev = elem;
            elem = elem->next;
        }

        if (shortest_elem != NULL) {
            // remove o escolhido da fila
            if (shortest_prev == NULL) {
                rq->head = shortest_elem->next;
            } else {
                shortest_prev->next = shortest_elem->next;
            }
            if (shortest_elem == rq->tail) {
                rq->tail = shortest_prev;
            }

            *cpu_task = shortest_elem->pcb;
            free(shortest_elem);
        }
    }
}