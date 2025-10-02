#include "rr.h"
#include "msg.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @brief Round Robin (RR) scheduling algorithm.
 */
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task, uint32_t time_quantum_ms) {
    static uint32_t quantum_counter = 0; // Conta quanto tempo o processo atual já usou

    if (*cpu_task) {
        // Incrementar o tempo de execução do processo
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        quantum_counter += TICKS_MS;

        // Caso o processo tenha terminado
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
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
            quantum_counter = 0; // Reinicia o contador do quantum
        }
        // Caso o processo não tenha terminado mas esgotou o quantum
        else if (quantum_counter >= time_quantum_ms) {
            enqueue_pcb(rq, *cpu_task); // volta para o fim da fila
            *cpu_task = NULL;
            quantum_counter = 0; // Reinicia o quantum
        }
    }

    // Se o CPU está livre, escolhe o próximo processo da fila
    if (*cpu_task == NULL) {
        *cpu_task = dequeue_pcb(rq);
    }
}
