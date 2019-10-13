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
 * ------------ Protocolo Operaciones SAC CLI ------------ *
 */

//OK
package_t slz_cod_readdir(const char *path, intptr_t dir)
{
	log_msje_info("ENVIO READDIR POINTER ADRESS DIR: [ %p ]", dir);
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path + sizeof(intptr_t);

	paquete.header = header_get('C', COD_READDIR, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload              		,&tam_path  ,sizeof(int));
	memcpy(paquete.payload+sizeof(int)			,path		,tam_path);
	memcpy(paquete.payload+sizeof(int)+tam_path	,&dir		,sizeof(intptr_t));

	return paquete;
}

//Desc: arma un paquete para la operacion opendir con su corresp path
package_t slz_cod_opendir(const char *path)
{
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path;

	paquete.header = header_get('C', COD_OPENDIR, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload              ,&tam_path  ,sizeof(int));
	memcpy(paquete.payload+sizeof(int)	,path		,tam_path);

	return paquete;
}

//desc: arma un paquete para la operacion releasedir ok?
package_t slz_cod_releasedir(const char *path, intptr_t dir)
{
	log_msje_info("ENVIO REALESEDIR POINTER ADRESS DIR: [ %p ]", dir);

	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path + sizeof(intptr_t);

	paquete.header = header_get('C', COD_RELEASEDIR, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload              		,&tam_path  ,sizeof(int));
	memcpy(paquete.payload+sizeof(int)			,path		,tam_path);
	memcpy(paquete.payload+sizeof(int)+tam_path	,&dir		,sizeof(intptr_t));

	return paquete;
}

//desc: dslz el payload respuesta de server, guarda la direccion del DIR
void dslz_res_opendir(void *buffer, intptr_t* dir)
{
	memcpy(dir, buffer, sizeof(intptr_t));
}

//dlsz_res_readdir
void dslz_res_readdir(void *buffer, t_list** filenames)
{
	int numfiles;
	memcpy(&numfiles, buffer, sizeof(int));
	int offset = sizeof(int);

	int c=0;
	do{
		int size;
		memcpy(&size, buffer+offset, sizeof(int));
		offset+=sizeof(int);

		char *name = malloc(size+1);
		memcpy(name, buffer+offset, size);
		name[size]='\0';
		offset+=size;

		list_add(*filenames, name);
		c++;
	}while(c != numfiles);

}

/*
 * ------------- Protocolo Operaciones SAC Server ----------- *
 */

//desc: dslz op readdir, recibe path y dir *
void dslz_cod_readdir(void *buffer, char**path, intptr_t *dir)
{
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+sizeof(int), tam_path);
	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(dir,	buffer+sizeof(int)+tam_path, sizeof(intptr_t));
	log_msje_info("RECIBO READDIR POINTER ADRESS DIR: [ %p ]", *dir);

}

//desc: deserializa el payload, guarda el contenido en path
void dslz_cod_opendir(void *buffer, char**path)
{
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+sizeof(int), tam_path);
	ruta[tam_path]='\0';
	*path = ruta;
}

void dslz_cod_releasedir(void* buffer, char** path, intptr_t* dir){
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+sizeof(int), tam_path);
	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(dir,	buffer+sizeof(int)+tam_path, sizeof(intptr_t));
	log_msje_info("RECIBO RELEASEDIR POINTER ADRESS DIR: [ %p ]", *dir);
}

//desc: arma paquete con el pointer adress de DIR
package_t slz_res_opendir(DIR *dp, bool error)
{
	package_t paquete;

	log_msje_info("POINTER ADRESS : [ %p ]", dp);
	log_msje_info("POINTER size : [ %d ]", sizeof(dp));
	log_msje_info("INTPTR size : [ %d ]", sizeof(intptr_t));

	if (error){
		paquete.header = header_get('S', COD_ERROR, 0);
	}else{
		int tam_payload = sizeof(intptr_t);
		paquete.header = header_get('S', COD_OPENDIR, tam_payload);
		paquete.payload = malloc(tam_payload);
		memcpy(paquete.payload, &dp, sizeof(intptr_t));//ACÁ LE ESTOY PASANDO POINTER ADRESS DE DIR
	}

	return paquete;
}

static int get_fullsize(t_list *filenames)
{
	int filesizes = 0;
	t_list * files = list_duplicate(filenames);

	t_link_element *element = files->head;
	t_link_element *aux = NULL;
	do{
		aux = element->next;
		filesizes += strlen(element->data);
		log_msje_info("element data list: [ %s ]", element->data);
		element = aux;

	}while(element != NULL);

	return filesizes;
}

package_t slz_res_readdir(t_list * filenames, bool error)
{
	package_t paquete;
	int numfiles = list_size(filenames);

	if (error){
		paquete.header = header_get('S', COD_ERROR, 0);

	} else {
		int tam_payload = sizeof(int) + sizeof(int)*numfiles + get_fullsize(filenames);
		paquete.header = header_get('S', COD_READDIR, tam_payload);
		paquete.payload = malloc(tam_payload);

		memcpy(paquete.payload, &numfiles, sizeof(int));

		t_link_element *element = filenames->head;
		t_link_element *aux = NULL;
		int bytes = sizeof(int);
		do{
			aux = element->next;
			int filesize = strlen(element->data);
			memcpy(paquete.payload + bytes, 			&filesize,				sizeof(int));
			memcpy(paquete.payload +sizeof(int)+ bytes, &(element->data[0]), 	filesize);
			bytes += filesize+sizeof(int);
			element=aux;

		}while (element != NULL);

	}
	return paquete;
}

package_t slz_res_releasedir(bool error)
{
	package_t paquete;

	if (error){
		paquete.header = header_get('S', COD_ERROR, 0);
	} else {
		paquete.header = header_get('S', COD_RELEASEDIR, 0);
	}

	return paquete;
}
