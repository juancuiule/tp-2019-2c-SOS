#include "hilolay_internal.h"

#include <stdio.h>
#include "hilolay_alumnos.h"
#include "hilolay.h"
#include "cliente.h"


// TODO: Check what happens when a thread is closed

/* Writes administrative info of a TCB */
void set_tcb(struct TCB* tcb, int id, enum State state) {
    tcb->state = state;
    tcb->id = id;
    tcb->next = NULL;
}

void init_internal(struct hilolay_operations* ops){
	/* Saves operations for later use */
	main_ops = ops;

	/* Initializes main as a thread */
	/* TODO: Abstract*/
    current_ult = &ults[0];
    current_ult->context = malloc(sizeof(ucontext_t));
    set_tcb(current_ult, MAIN_THREAD_ID, RUNNING);

    /* Call function on clients code */
    main_ops->suse_create(current_ult->id);
}

void* wrap(void *(*start_routine)(void *), void *arg, int tid){
	void* res = start_routine(arg);
	main_ops->suse_close(hilolay_get_tid());
	hilolay_yield();
	return res;
}

/* Creates the context of a new thread */
void create_context(struct TCB* new_ult, void *(*start_routine)(void *), void *arg) {
    new_ult->context = malloc(sizeof(ucontext_t));

    // Gets the current context as a reference, to be overwritten
    // TODO: Why current context? I assume this may affect context for global variables, but it's worth checking
    getcontext(new_ult->context);

    new_ult->context->uc_link = 0;
    new_ult->context->uc_stack.ss_sp = malloc(STACK_SIZE);
    new_ult->context->uc_stack.ss_size = STACK_SIZE;
    makecontext(new_ult->context, (void*) wrap, 3, start_routine, arg);
}

// TODO: Refactor
TCB* find_free_tcb(){
    struct TCB *new_ult = NULL;
    for (int i = 0; i < MAX_ULTS; i++) {
        if (ults[i].state == FREE) {
        	new_ult = &ults[i];
            break;
        }
    }

	return new_ult;
}

// TODO: Make referentially transparent
int get_next_id(){
	return NEXT_ID++;
}

/* Starts an ult */ //TODO: Set ERRNO
int hilolay_create(hilolay_t *thread, const hilolay_attr_t *attr, void *(*start_routine)(void *), void *arg){
    /* Finds a free TCB */
	struct TCB *new_ult = find_free_tcb();
    if (new_ult == NULL) { return ERROR_TOO_MANY_ULTS; }
    /* Creates the TCB */
    int tid = get_next_id();
    create_context(new_ult, start_routine, arg);
    set_tcb(new_ult, tid, READY);
    thread->tid = tid;

    /* Calls selected operation */
    main_ops->suse_create(tid);

    return 0;
}

TCB* get_next_tid(){ // TODO: Error cases
	int tid = main_ops->suse_schedule_next();
	for (int i = 0; i < MAX_ULTS; i++){
		if (ults[i].id == tid){
			return &(ults[i]);
		}
	}

	return NULL;
}

int swap_context(TCB *selected_ult){
	ucontext_t 	*old = current_ult->context,
				*new = selected_ult->context;

	current_ult->state = READY;
	selected_ult->state = RUNNING;

	current_ult = selected_ult;

	return swapcontext(old, new);
}

int hilolay_yield(void){
	return swap_context(get_next_tid());
}

/* Returns the running thread id */
int hilolay_get_tid(void) {
    return current_ult->id;
}

int hilolay_join(hilolay_t *thread){
	main_ops->suse_join(thread->tid);
	return hilolay_yield();
}

int enviar_mensaje(char* mensaje) {
	struct sockaddr_in cliente;
	  struct hostent *servidor;
	  servidor = gethostbyname("localhost");

	  if(servidor == NULL)
	  {
	    printf("Host erróneo\n");
	    return 1;
	  }

	  int conexion;
	  char buffer[100];
	  conexion = socket(AF_INET, SOCK_STREAM, 0); //Asignación del socket
	  bzero((char *)&cliente, sizeof((char *)&cliente)); //Rellena toda la estructura de 0's
	  cliente.sin_family = AF_INET; //asignacion del protocolo
	  cliente.sin_port = htons(5003); //asignacion del puerto
	  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));

	  if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
	  { //conectando con el host
	    printf("Error conectando con el host\n");
	    close(conexion);
	    return 1;
	  }
	  printf("Conectado con %s:%d\n",inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	  printf("Escribe un mensaje: ");
	  fgets(mensaje, 100, stdin);
	  send(conexion, mensaje, 100, 0); //envio
	  bzero(mensaje, 100);
	  recv(conexion, mensaje, 100, 0); //recepción
	  printf("%s", mensaje);
	  return 0;
}

int* suse_create(int tid) {
	enviar_mensaje("crear_ult");
	return NULL;
}
















