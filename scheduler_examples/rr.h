//
// Created by telma on 01/10/2025.
//

#ifndef RR_H
#define RR_H
#include "queue.h"
#include <stdint.h>

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task, uint32_t time_quantum_ms);
#endif //RR_H
