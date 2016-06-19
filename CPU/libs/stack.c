
#include "stack.h"

/*
 * Creates a new stack entry container.
 */
t_stack_entry * stack_entry_create(void) {
    t_stack_entry * new_stack_entry = calloc(1, sizeof(t_stack_entry));
}

/*
 * Adds a new ret_var to the stack entry.
 *
 * stack_entry : pointer to the stack entry position.
 * ret_var : return variable to be added.
 */
int add_ret_var(t_stack_entry ** stack_entry, t_ret_var *ret_var) {
    void * aux_buffer = NULL;
    aux_buffer = realloc(*stack_entry, ((*stack_entry)->cant_ret_vars + 1)* sizeof(t_ret_var));
    if(aux_buffer == NULL) {
        return ERROR;
    }
    (*stack_entry)->ret_vars = aux_buffer;
    if( memcpy(((*stack_entry)->ret_vars + (*stack_entry)->cant_ret_vars), ret_var, sizeof(t_ret_var)) == NULL) {
        return ERROR;
    }
    (*stack_entry)->cant_ret_vars++;
    return SUCCESS;
}


/*
 * Adds a new arg to the stack entry.
 *
 * stack_entry : pointer to the stack entry position.
 * arg : argument variable to be added.
 */
int add_arg(t_stack_entry ** stack_entry, t_arg *arg) {
    void * aux_buffer = NULL;
    aux_buffer = realloc(*stack_entry, ((*stack_entry)->cant_args + 1)* sizeof(t_arg));
    if(aux_buffer == NULL) {
        return ERROR;
    }
    (*stack_entry)->args = aux_buffer;
    if( memcpy(((*stack_entry)->args + (*stack_entry)->cant_args), arg, sizeof(t_arg)) == NULL) {
        return ERROR;
    }
    (*stack_entry)->cant_args++;
    return SUCCESS;
}

/*
 * Adds a new local variable to the stack entry.
 *
 * stack_entry : pointer to the stack entry position.
 * var : variable to be added.
 */
int add_var(t_stack_entry ** stack_entry, t_var *var) {
    void * aux_buffer = NULL;
    aux_buffer = realloc((*stack_entry)->vars, ((*stack_entry)->cant_vars + 1)* sizeof(t_var));
    if(aux_buffer == NULL) {
        return ERROR;
    }
    (*stack_entry)->vars = aux_buffer;
    if( memcpy(((*stack_entry)->vars + (*stack_entry)->cant_vars), var, sizeof(t_var)) == NULL) {
        return ERROR;
    }
    (*stack_entry)->cant_vars++;
    return SUCCESS;
}

/*
 * Serializa las entradas del stack en una char*.
 *
 * stack  : El stack que se quiere serializar.
 * buffer : El buffer donde se quiere almacenar el stack serializado.
 * buffer_size : Variable donde se almacenara el tamanio final que terminara teniendo el buffer.
 */
void serialize_stack (t_stack *stack, void **buffer, int *buffer_size) {

    //1) Agarrro el t_list elements (t_link_element {t_link_element *head, int elements_count})
    //2) voy recorriendo agarrando head y recorriendo el next
    //(t_link_element {void *data, t_link_element *next}) , next sera NULL para el ultimo elemento
    //3) a cada link element tengo que serializarle el data, que va a ser un t_stack_entry
    //(t_stack_entry {int pos, int cant_args, char *args, int cant_vars, char *vars,
    //                int ret_pos, int cant_ret_vars, char *ret_vars})

    t_link_element *link_actual = NULL;
    t_stack_entry *entrada_actual = NULL;
    void *stack_entry_item_buffer = NULL, *stack_list_buffer = *buffer, *next = NULL;
    u_int32_t cantidad_elementos_stack = 0;
    t_stack_entry *data = NULL;
    u_int32_t indice = 0;
    size_t stack_entry_item_size = 0, stack_entry_list_buffer_size = *buffer_size;

    //Stack / Queue
    t_list *elementos = stack-> elements; //Tomo la lista de elementos de la queue
    //t_link_element *element = elementos->head; //Tomo el primer elemento de la lista

    //Lista de la Queue
    link_actual = elementos->head; //Tomo la data del primer link de la lista
    cantidad_elementos_stack = (u_int32_t) elementos->elements_count; //Cantidad de links totales en el stack

    //El primer elemento que se agrega es la cantidad de elementos totales que tendra el stack
    serialize_data(&cantidad_elementos_stack,sizeof(int),
                   buffer, buffer_size);

    if(link_actual == NULL || cantidad_elementos_stack == 0) {
        return;
    }

    for(indice = 0; indice < cantidad_elementos_stack; indice++) {
        entrada_actual = (t_stack_entry*) link_actual->data; //Tomo la data del primer link de la lista
        serialize_stack_entry(entrada_actual, buffer, buffer_size);
        link_actual = link_actual->next; //En la ultima iteracion, entrada_actual terminara siendo igualada a NULL
    }

    //Ya se leyeron todos los datos (t_stack_entry) del stack y se los serializaron el buffer
    //Tendria que haber cantidad_elementos_stack elementos en stack_entry_list_buffer
    //y stack_entry_list_buffer_size tendria que tener el size total del stack_entry_list_buffer
}


/*
 * Serializa una entrada del stack a un buffer, almacenando su tamanio.
 *
 * serialized_data format >> pos|cant_args|args|cant_vars|vars|cant_ret_vars|ret_vars.
 *
 * entry  : La entrada que se quiere serializar.
 * buffer : El buffer donde se almacenara la entrada serializada.
 * buffer_size : Variable que terminara con el valor del tamanio final del buffer.
 */
void serialize_stack_entry(t_stack_entry *entry, void **buffer, int *buffer_size) {
    int i = 0;
    serialize_data(&entry->pos, sizeof(int), buffer, buffer_size);
    serialize_data(&entry->cant_args, sizeof(int), buffer, buffer_size);
    //serialize_data(&entry->args, (size_t) sizeof(t_arg)*entry->cant_args, buffer, buffer_size);
    for(i = 0; i < entry->cant_args; i++) {
        serialize_data(&entry->args->page_number, (size_t) sizeof(int), buffer, buffer_size);
        serialize_data(&entry->args->offset, (size_t) sizeof(int), buffer, buffer_size);
        serialize_data(&entry->args->tamanio, (size_t) sizeof(int), buffer, buffer_size);
    }
    serialize_data(&entry->cant_vars, sizeof(int), buffer, buffer_size);
    //serialize_data(&entry->vars, (size_t) sizeof(t_var)*entry->cant_vars, buffer, buffer_size);
    for(i = 0; i < entry->cant_vars; i++) {
        serialize_data(&entry->vars->var_id, (size_t) sizeof(char), buffer, buffer_size);
        serialize_data(&entry->vars->page_number, (size_t) sizeof(int), buffer, buffer_size);
        serialize_data(&entry->vars->offset, (size_t) sizeof(int), buffer, buffer_size);
        serialize_data(&entry->vars->tamanio, (size_t) sizeof(int), buffer, buffer_size);
    }
    serialize_data(&entry->cant_ret_vars, sizeof(int), buffer, buffer_size);
    //serialize_data(&entry->ret_vars, (size_t) sizeof(t_ret_var)*entry->cant_vars, buffer, buffer_size);
    for(i = 0; i < entry->cant_ret_vars; i++) {
        serialize_data(&entry->ret_vars->page_number, (size_t) sizeof(int), buffer, buffer_size);
        serialize_data(&entry->ret_vars->offset, (size_t) sizeof(int), buffer, buffer_size);
        serialize_data(&entry->ret_vars->tamanio, (size_t) sizeof(int), buffer, buffer_size);
    }
    serialize_data(&entry->ret_pos, sizeof(int), buffer, buffer_size);
}

/*
 * Deserializa un t_stack de un conjunto de bytes.
 *
 * serialized_data format >> cantidadLinks|[entrySize(0)|entry(0)...entrySize(cantidadLinks-1)|entry(cantidadLinks-1)]
 *
 * **stack    : Donde el stack se almacenara.
 * serialized_data : Conjunto de bytes serializados.
 * serialized_data_size : Tamanio total del conjunto de bytes.
 */
void deserialize_stack(t_stack **stack, void **serialized_data, int *serialized_data_size) {
    u_int32_t cantidad_links = 0, indice = 0;
    t_stack_entry *stack_entry = NULL;

    *stack = queue_create(); //Creo el stack

    deserialize_data(&cantidad_links, sizeof(int), serialized_data, serialized_data_size);
    for(indice = 0; indice < cantidad_links; indice++) {
        stack_entry = create_new_stack_entry();
        deserialize_stack_entry(&stack_entry, serialized_data, serialized_data_size);
        queue_push(*stack, (void*) stack_entry); //Agrego elementos al stack
    }
}

t_stack_entry *create_new_stack_entry() {
    t_stack_entry *stack_entry;
    stack_entry = calloc(1, sizeof(t_stack_entry));
    stack_entry->args = calloc(1, sizeof(t_arg));
    stack_entry->vars = calloc(1, sizeof(t_var));
    stack_entry->ret_vars = calloc(1, sizeof(t_ret_var));
    return stack_entry;
}

/*
 * Deserializa un t_stack_entry de un conjunto de bytes.
 *
 * serialized_data format >> pos|cant_args|args|cant_vars|vars|cant_ret_vars|ret_vars
 *
 * **entry    : Donde el stack_entry se almacenara.
 * serialized_data : Conjunto de bytes serializados.
 * serialized_data_size : Tamanio total del conjunto de bytes.
 */
void deserialize_stack_entry(t_stack_entry **entry, void **serialized_data, int *serialized_data_size) {

    deserialize_data(&(*entry)->pos, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->cant_args, sizeof(int), serialized_data, serialized_data_size);
    //deserialize_data((*entry)->args, (size_t) (*entry)->cant_args, serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->args->page_number, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->args->offset, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->args->tamanio, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->cant_vars, sizeof(int), serialized_data, serialized_data_size);
    //deserialize_data((*entry)->vars, (size_t) (*entry)->cant_vars, serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->vars->var_id, sizeof(char), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->vars->page_number, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->vars->offset, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->vars->tamanio, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->cant_ret_vars, sizeof(int), serialized_data, serialized_data_size);
    //deserialize_data((*entry)->ret_vars, (size_t) (*entry)->cant_vars, serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->ret_vars->page_number, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->ret_vars->offset, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->ret_vars->tamanio, sizeof(int), serialized_data, serialized_data_size);
    deserialize_data(&(*entry)->ret_pos, sizeof(int), serialized_data, serialized_data_size);
}