// rr.c (substitui a função inteira)
#include "rr.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TICKS_MS 100

void rr_scheduler(uint32_t current_time_ms, queue_t *ready_queue, pcb_t **cpu_task, uint32_t time_quantum_ms) {

    // Se houver processo na CPU, atualiza tempos
    if (*cpu_task) {
        (*cpu_task)->cpu_time_ms += TICKS_MS;
        (*cpu_task)->quantum_used_ms += TICKS_MS;
        (*cpu_task)->last_update_time_ms = current_time_ms;

        // Debug: mostra estado atual
        printf("[RR DEBUG] Tick %u ms | PID %d | CPU: %u ms | Quantum: %u ms | Total: %u ms | Status: %d\n",
               current_time_ms, (*cpu_task)->pid, (*cpu_task)->cpu_time_ms,
               (*cpu_task)->quantum_used_ms, (*cpu_task)->time_ms, (*cpu_task)->status);

        // 1️⃣ Verifica se terminou
        if ((*cpu_task)->cpu_time_ms >= (*cpu_task)->time_ms) {
            int64_t elapsed_time_ms = (int64_t)current_time_ms - (int64_t)(*cpu_task)->arrival_time_ms;
            if (elapsed_time_ms < 0) elapsed_time_ms = 0; // proteção

            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = (uint32_t)elapsed_time_ms
            };


            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t))
                perror("write");

            printf("[RR] Process %d FINISHED at %u ms | CPU: %u ms | Elapsed: %u ms\n",
                   (*cpu_task)->pid, current_time_ms, (*cpu_task)->cpu_time_ms, elapsed_time_ms);

            free(*cpu_task);
            *cpu_task = NULL;
        }
        // 2️⃣ Verifica quantum (preempção)
        else if ((*cpu_task)->quantum_used_ms >= time_quantum_ms) {
            printf("[RR] Process %d PREEMPTED at %u ms (used %u ms / quantum %u ms)\n",
                   (*cpu_task)->pid, current_time_ms, (*cpu_task)->quantum_used_ms, time_quantum_ms);

            // Atualiza estado antes de voltar para a fila
            (*cpu_task)->quantum_used_ms = 0;  // reinicia quantum
            (*cpu_task)->status = TASK_RUNNING;
            (*cpu_task)->last_update_time_ms = current_time_ms;

            // Enqueue para o final da fila (round-robin)
            enqueue_pcb(ready_queue, *cpu_task);

            // CPU fica livre para escolher outro processo neste mesmo tick
            *cpu_task = NULL;
        }
    }

    // 3️⃣ Pega próximo processo da fila se CPU estiver vazia
    if (*cpu_task == NULL) {
        pcb_t *next = dequeue_pcb(ready_queue);
        if (next) {
            next->quantum_used_ms = 0;
            next->status = TASK_RUNNING;
            if (next->arrival_time_ms == 0)  // <--- Proteção final
                next->arrival_time_ms = current_time_ms;
            next->last_update_time_ms = current_time_ms;
            *cpu_task = next;
            printf("[RR] Process %d SCHEDULED at %u ms (arrival=%u)\n",
                   next->pid, current_time_ms, next->arrival_time_ms);
        }
    }

    }

