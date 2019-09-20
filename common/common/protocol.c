#include "protocol.h"


static header_t header_get(char cod_proc, int cod_op, int tam_pay)
{
	header_t header;
	header.cod_proceso = cod_proc;
	header.cod_operacion = cod_op;
	header.tam_payload = tam_pay;
	return header;
}

static void header_dslz(header_t *header, void *buffer)
{
	int tam_buf, cod_op;
	char proceso;
	// Deserializo el header
	memcpy(&proceso,	buffer, 							sizeof(char));
	memcpy(&cod_op, 	buffer+sizeof(char), 				sizeof(int));
	memcpy(&tam_buf,	buffer+sizeof(char)+sizeof(int),	sizeof(int));

	// Armo el heaader
	(*header).cod_proceso = proceso;
	(*header).cod_operacion = cod_op;
	(*header).tam_payload = tam_buf;
}

static header_t header_recibir(int socket)
{
	header_t header;
	void* buffer = malloc(TAM_HEADER);
	bzero(buffer, TAM_HEADER);

	size_t size = recv_bytes(socket, buffer, TAM_HEADER);
	if(size > 0)
		header_dslz(&header, buffer);
	else
		header.cod_operacion = COD_ERROR;

	return header;
}

static size_t header_enviar(int socket, header_t header)
{
	void* buffer = malloc(TAM_HEADER);
	bzero(buffer, TAM_HEADER);

	memcpy(buffer, &header, TAM_HEADER);

	size_t size = send_bytes(socket, buffer, TAM_HEADER);

	free(buffer);
	buffer = NULL;

	return size;

}

void handshake_enviar(int socket, char cod_proc)
{
	header_t header = header_get(cod_proc, COD_HANDSHAKE, 0);

	size_t size = header_enviar(socket, header);
	if (size < 0)
		log_msje_error("Error al enviar el handshake");
}

void handshake_recibir(int socket){
	header_t header;
	header = header_recibir(socket);

	if(header.cod_operacion == COD_HANDSHAKE)
	{
		switch(header.cod_proceso)
		{
			case 'S':
				log_msje_info("Conectado con SAC SERVER");
				break;
			case 'C':
				log_msje_info("Conectado con SAC CLIENTE");
				break;
			default:
				log_msje_error("Error, codigo de proceso no valido");
		}
	}
	else
		log_msje_error("Cod operacion invalido. Se esperaba un handshake");
}

package_t paquete_recibir(int socket)
{
	package_t paquete;

	paquete.header = header_recibir(socket);

	if(paquete.header.tam_payload > 0){
		paquete.payload = malloc(paquete.header.tam_payload * sizeof(char));
		recv_bytes(socket, paquete.payload, paquete.header.tam_payload);
	}

	return paquete;
}

bool paquete_enviar(int socket, package_t paquete)
{
	log_msje_info("Paquete enviar");
	size_t tam_payload = paquete.header.tam_payload * sizeof(char);
	size_t tam_buffer = TAM_HEADER + tam_payload;

	log_msje_info("tam buffer [ %d ]", tam_buffer);

	void* buffer = malloc(tam_buffer);
	bzero(buffer, tam_buffer);

	memcpy(buffer, 				&paquete.header, TAM_HEADER);
	memcpy(buffer+TAM_HEADER, 	paquete.payload, tam_payload);

	size_t size = send_bytes(socket, buffer, tam_buffer);

	log_msje_info("bytes enviados [ %d ]", size);

	free(buffer);
	buffer = NULL;

	return size == tam_buffer;
}


/*
 * Protocolo Operaciones
 */

//Desc: serializa operacion readdir. sac cli
package_t slz_cod_readdir(const char *path)
{
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path;

	paquete.header = header_get('C', COD_READDIR, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload                     	,&tam_path     	        ,sizeof(int));
	memcpy(paquete.payload+sizeof(int)			,path					,tam_path);

	return paquete;
}

//Desc: deserializa el payload, guarda el contenido en path
void dslz_cod_readdir(void *buffer, char**path)
{
	int tam_path;
	memcpy(&tam_path,	buffer, 				sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta,		buffer+sizeof(int), 	tam_path);
	ruta[tam_path]='\0';
	*path = ruta;
}
