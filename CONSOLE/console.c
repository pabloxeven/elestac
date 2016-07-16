#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include "socketCommons/socketCommons.h"
#include <signal.h>
#include <commons/config.h>
#include <commons/log.h>

struct {
		int 	PUERTO_KERNEL;
		char*	IP_KERNEL;
	} setup;


void tratarSeniales(int);
int loadConfig(char* configFile);
t_log *console_log;

int main(int argc, char *argv[]) {
	signal(SIGINT, tratarSeniales);

	console_log = log_create("console.log", "Elestac-CONSOLE", true, LOG_LEVEL_TRACE);


	int kernelSocketClient;
	char *kernel_reply = calloc(1, 4);
	//create kernel client
	if (argc != 3) {
		puts("usage: console console.config ansisopFile");
		return -1;
	}

    if (loadConfig(argv[1]) < 0){
        log_error(console_log, "No se encontró el archivo de configuración");
        return -1;
    }

	getClientSocket(&kernelSocketClient, setup.IP_KERNEL, setup.PUERTO_KERNEL);

	log_info(console_log, "Se cargó el setup con IP %s y puerto %d de KERNEL", setup.IP_KERNEL, setup.PUERTO_KERNEL);
	/*le voy a mandar al kernel un 0 para iniciar el handshake + sizeMsj en 4B + (el código como viene),
	y me va a devolver el client_id (un número) que me represente con el cual él me conoce*/

	FILE *fp;
	fp = fopen(argv[2], "r");
	if (!fp) {
		log_error(console_log, "No se puedo abrir el ansisop.");
		exit(1);
	}

	fseek(fp, SEEK_SET, 0);
	fseek(fp, 0L, SEEK_END);
	long int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	char hash_bang[100];
	fgets(hash_bang, 100, fp);
	sz = sz - strlen(hash_bang) - 1;

	char *prog = (char *) calloc(1, sz);

	fread(prog, sz + 1, 1, fp);
	fclose(fp);

	int sizeMsj = strlen("0") + 5 + (int) sz;
	char *mensaje = (char *) calloc(1, sizeMsj);

	char buffer[20];
	sprintf(buffer, "%04d", (int) sz);

	strcpy(mensaje, "0");
	strcat(mensaje, buffer);
	strcat(mensaje, prog);
	send(kernelSocketClient, mensaje, sizeMsj, 0);
	printf("El tamanio del programa es %s, y se envio al kernel: %s", buffer, mensaje);

	free(prog);
	free(mensaje);

	if (recv(kernelSocketClient, kernel_reply, 4, 0) < 0) {
		log_error(console_log, "Kernel no responde");
		return EXIT_FAILURE;
	} else if (!strcmp(kernel_reply, "0000")) {
		log_error(console_log, "Kernel contesto: %s.No hay espacio en memoria para ejecutar el programa", kernel_reply);
	} else {
		log_info(console_log, "El pid de la consola es %d. Se esta ejecutando el programa.", atoi(kernel_reply));
	}
    free(kernel_reply);

	void *kernelBuffer = NULL;
    int valor = 0;
    int textLen = 0;
	bool continua = true;
    void *kernel_operation = NULL;
	while (continua) {
        kernel_operation = calloc(1, 1);
		if (recv(kernelSocketClient, kernel_operation, 1, 0) > 0) {
			log_info(console_log, "Kernel dijo: %d", atoi(kernel_operation));
			switch (atoi(kernel_operation)) {
				case 0:// Program END
					continua = false;
					log_info(console_log, "Vamo a recontra calmarno. El programa finalizó correctamente");
					break;
				case 1:// Print value
					//recibo 4 bytes -> valor_variable
                    kernelBuffer = calloc(1,4);
					recv(kernelSocketClient, kernelBuffer, 4, 0);
                    valor = atoi(kernelBuffer);
					log_info(console_log, "El valor de la variable es: %d", valor);
					free(kernelBuffer);
					break;

				case 2:// Print text
					//recibo 4 del tamaño + texto
                    kernelBuffer = calloc(1,4);
					recv(kernelSocketClient, kernelBuffer, 4, 0);
					textLen = atoi(kernelBuffer);
                    free(kernelBuffer);
                    if(textLen > 0) {
                        kernelBuffer = calloc(1, textLen);
                        recv(kernelSocketClient, kernelBuffer, textLen, 0);
                        log_info(console_log, "%s.", kernelBuffer);
                    } else {
                        log_error(console_log, "Se recibio un tamanio de texto invalido");
                    }
                    free(kernelBuffer);
					break;
			}
		} else {
            continua = false;
            log_error(console_log, "El programa finalizo repentinamente");
        }
	}

	free(kernel_operation);
	close(kernelSocketClient);
	log_info(console_log, "Se cerro la conexion con el kernel");
	puts("Terminated console.");
	log_destroy(console_log);
	return 0;
}

int loadConfig(char* configFile){
	if(configFile == NULL){
		return -1;
	}
	t_config *config = config_create(configFile);
	log_info(console_log, " .:: Loading settings ::.");

	if(config != NULL){
		setup.PUERTO_KERNEL=config_get_int_value(config,"PUERTO_KERNEL");
		setup.IP_KERNEL=config_get_string_value(config,"IP_KERNEL");
	}
	return 0;
}

void tratarSeniales(int senial) {
	printf("Tratando seniales\n");
	printf("\nSenial: %d\n", senial);
	switch (senial) {
	case SIGINT:
		// Detecta Ctrl+C y evita el cierre.
		printf("Esto acabará con el sistema. Presione Ctrl+C una vez más para confirmar.\n\n");
		signal(SIGINT, SIG_DFL); // solo controlo una vez.
		break;
	}
}

