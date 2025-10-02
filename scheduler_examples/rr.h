//
// Created by telma on 01/10/2025.
//

#ifndef RR_H
#define RR_H
#include "queue.h"
#include <stdint.h>

/**
 * @brief Round Robin (RR) scheduling algorithm.
 *
 * O RR limita o tempo que cada processo pode ocupar o CPU (time quantum).
 * Quando o processo chega ao fim do quantum mas ainda não terminou,
 * ele é colocado de volta na fila de prontos (ready queue).
 *
 * @param current_time_ms Tempo atual da simulação em ms.
 * @param rq Ponteiro para a fila de processos prontos.
 * @param cpu_task Ponteiro para o processo atualmente em execução no CPU.
 * @param time_quantum_ms O quantum de tempo máximo atribuído a cada processo.
 */
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task, uint32_t time_quantum_ms);
#endif //RR_H
