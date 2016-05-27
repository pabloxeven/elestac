#ifndef SERIALIZATION_PCB_H
#define SERIALIZATION_PCB_H

#include <elf.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include "serialize.h"
#include "stack.h"

typedef enum {NEW, READY, EXECUTING, BLOCKED, EXIT} enum_queue;
typedef struct {
    uint32_t pid;
    uint32_t program_counter;
    uint32_t stack_pointer;
    t_stack *stack_index;
    enum_queue status;
    t_size instrucciones_size;
    char *instrucciones_serializado;
} t_pcb;

void serialize_pcb(t_pcb *pcb, char **buffer, t_size *buffer_size);
void deserialize_pcb(t_pcb **pcb, char **serialized_data, t_size *serialized_data_size);

void serialize_instrucciones(t_intructions *intructions, char **buffer, t_size *buffer_size);
void deserialize_instrucciones(t_intructions **intructions, char **serialized_data, t_size *serialized_data_size);

#endif //SERIALIZATION_PCB_H
