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
	else //Se desconecto el cliente
		header.cod_operacion = COD_DESC;

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


//errno especifico
package_t slz_res_error(int errnum)
{
	package_t paquete;

	int tam_payload = sizeof(int);
	paquete.header = header_get('S', COD_ERROR, tam_payload);
	paquete.payload = malloc(tam_payload);
	memcpy(paquete.payload, &errnum, sizeof(int));

	return paquete;
}

void dslz_res_error(void *buffer, int *errnum)
{
	memcpy(errnum, buffer, sizeof(int));
}


package_t slz_simple_res(cod_operation cod)
{
	package_t paquete;
	paquete.header = header_get('S', cod, 0);
	return paquete;
}

package_t slz_path_with_cod(const char *path, cod_operation cod)
{
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path;

	paquete.header = header_get('C', cod, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload, &tam_path, sizeof(int));
	memcpy(paquete.payload+sizeof(int), path, tam_path);

	return paquete;
}

void dslz_payload_with_path(void *buffer, char**path)
{
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path + 1);
	memcpy(ruta, buffer+sizeof(int), tam_path);

	ruta[tam_path]='\0';
	*path = ruta;
}

/*
 * ------------ Protocolo Operaciones SAC CLI ------------ *
 */


package_t slz_cod_readdir(const char *path, uint32_t dir)
{
	log_msje_info("ENVIO READDIR BLK NUMBER: [ %d ]", dir);
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path + sizeof(int);

	paquete.header = header_get('C', COD_READDIR, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload              		,&tam_path  ,sizeof(int));
	memcpy(paquete.payload+sizeof(int)			,path		,tam_path);
	memcpy(paquete.payload+sizeof(int)+tam_path	,&dir		,sizeof(uint32_t));

	return paquete;
}

//desc: arma un paquete para la operacion releasedir
package_t slz_cod_releasedir(const char *path, intptr_t dir)
{
	log_msje_info("ENVIO REALESEDIR POINTER ADRESS DIR: [ %d ]", dir);

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

package_t slz_cod_open(const char *path, int flags)
{
	package_t paquete;

	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path + sizeof(int);

	paquete.header = header_get('C', COD_OPEN, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload              		,&tam_path  ,sizeof(int));
	memcpy(paquete.payload+sizeof(int)			,path		,tam_path);
	memcpy(paquete.payload+sizeof(int)+tam_path	,&flags		,sizeof(int));

	return paquete;
}

package_t slz_cod_read(const char *path, int fd, size_t size, off_t offset)
{
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path + sizeof(int) + sizeof(size_t) + sizeof(off_t);

	paquete.header = header_get('C', COD_READ, tam_payload);
	paquete.payload = malloc(tam_payload);

	int offs=0;
	memcpy(paquete.payload, &tam_path  ,sizeof(int));
	offs+=sizeof(int);
	memcpy(paquete.payload+offs, path, tam_path);
	offs+=tam_path;
	memcpy(paquete.payload+offs, &fd, sizeof(int));
	offs+=sizeof(int);
	memcpy(paquete.payload+offs, &size, sizeof(size_t));
	offs+=sizeof(size_t);
	memcpy(paquete.payload+offs, &offset, sizeof(off_t));

	return paquete;
}

package_t slz_cod_release(const char *path, int fd)
{
	package_t paquete;
	int tam_path = strlen(path);
	int tam_payload = sizeof(int) + tam_path + sizeof(int);

	paquete.header = header_get('C', COD_RELEASE, tam_payload);
	paquete.payload = malloc(tam_payload);

	int offs=0;
	memcpy(paquete.payload, &tam_path  ,sizeof(int));
	offs+=sizeof(int);
	memcpy(paquete.payload+offs, path, tam_path);
	offs+=tam_path;
	memcpy(paquete.payload+offs, &fd, sizeof(int));

	return paquete;
}


package_t slz_cod_write(const char *path, const char *buffer, int fd, size_t size, off_t offset)
{
	package_t paquete;
	int tam_path = strlen(path);
	int tam_buff = strlen(buffer);
	int tam_payload = sizeof(int) + tam_path +sizeof(int) + tam_buff + sizeof(int) + sizeof(size_t) + sizeof(off_t);

	paquete.header = header_get('C', COD_WRITE, tam_payload);
	paquete.payload = malloc(tam_payload);

	int offs=0;
	memcpy(paquete.payload, &tam_path  ,sizeof(int));
	offs+=sizeof(int);
	memcpy(paquete.payload+offs, path, tam_path);
	offs+=tam_path;
	memcpy(paquete.payload+offs, &tam_buff, sizeof(int));
	offs+=sizeof(int);
	memcpy(paquete.payload+offs, buffer, tam_buff);
	offs+=tam_buff;
	memcpy(paquete.payload+offs, &fd, sizeof(int));
	offs+=sizeof(int);
	memcpy(paquete.payload+offs, &size, sizeof(size_t));
	offs+=sizeof(size_t);
	memcpy(paquete.payload+offs, &offset, sizeof(off_t));

	return paquete;
}

//desc: dslz el payload respuesta de server, guarda la direccion del DIR
void dslz_res_opendir(void *buffer, uint32_t* dir)
{
	memcpy(dir, buffer, sizeof(uint32_t));
}

//dlsz_res_readdir
void dslz_res_readdir(void *buffer, t_list** filenames)
{
	int numfiles;
	memcpy(&numfiles, buffer, sizeof(int));

	int offset = sizeof(int);

	int c=0;
	while(c != numfiles)
	{
		int size;
		memcpy(&size, buffer+offset, sizeof(int));
		offset+=sizeof(int);

		char *name = malloc(size+1);
		memcpy(name, buffer+offset, size);
		name[size]='\0';
		offset+=size;

		list_add(*filenames, name);
		c++;

	}
}
void dslz_res_open(void *buffer, int *fd)
{
	memcpy(fd, buffer, sizeof(int));
}

void dslz_res_getattr(void *buffer, uint32_t *size, uint64_t *m_date, int *state)
{
	int offs = 0;
	memcpy(size, buffer, sizeof(uint32_t));
	offs += sizeof(uint32_t);

	memcpy(m_date, buffer + offs, sizeof(uint64_t));
	offs += sizeof(uint64_t);

	memcpy(state, buffer + offs, sizeof(int));
}

void dslz_res_read(void *buffer, char *buf, int *size)
{
	int buff_size;
	memcpy(&buff_size, buffer, sizeof(int));
	memcpy(buf, buffer+sizeof(int), buff_size);
	memcpy(size, buffer+sizeof(int)+buff_size, sizeof(int));
}

void dslz_res_write(void *buffer, int *size)
{
	memcpy(size, buffer, sizeof(int));
}

/*
 * ------------- Protocolo Operaciones SAC Server ----------- *
 */

//desc: dslz op readdir, recibe path y dir *
void dslz_cod_readdir(void *buffer, char**path, uint32_t *dir)
{
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+sizeof(int), tam_path);
	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(dir,	buffer+sizeof(int)+tam_path, sizeof(uint32_t));
	log_msje_info("RECIBO READDIR BLK NUMBER: [ %d ]", *dir);

}

void dslz_cod_releasedir(void* buffer, char** path, uint32_t* dir)
{
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+sizeof(int), tam_path);
	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(dir,	buffer+sizeof(int)+tam_path, sizeof(uint32_t));
	log_msje_info("RECIBO RELEASEDIR POINTER ADRESS DIR: [ %d ]", *dir);
}

void dslz_cod_open(void *buffer, char **path, int *flags)
{
	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+sizeof(int), tam_path);
	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(flags,	buffer+sizeof(int)+tam_path, sizeof(int));
}

void dslz_cod_read(void *buffer, char **path, int *fd, size_t *size, off_t *offset)
{
	int offs = 0;

	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));
	offs += sizeof(int);

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+offs, tam_path);
	offs += tam_path;

	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(fd, buffer+offs, sizeof(int));
	offs += sizeof(int);

	memcpy(size, buffer+offs, sizeof(size_t));
	offs += sizeof(int);

	memcpy(offset, buffer+offs, sizeof(off_t));

}

void dslz_cod_release(void *buffer, char **path, int *fd)
{
	int offs = 0;

	int tam_path;
	memcpy(&tam_path, buffer, sizeof(int));
	offs += sizeof(int);

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, buffer+offs, tam_path);
	offs += tam_path;

	ruta[tam_path]='\0';
	*path = ruta;

	memcpy(fd, buffer+offs, sizeof(int));
}

void dslz_cod_write(void *payload, char **path, char **buffer, int *fd, size_t *size, off_t *offset)
{
	int offs = 0;

	int tam_path;
	memcpy(&tam_path, payload, sizeof(int));
	offs += sizeof(int);

	char *ruta = malloc(tam_path+1);
	memcpy(ruta, payload+offs, tam_path);
	offs += tam_path;

	ruta[tam_path]='\0';
	*path = ruta;

	int tam_buffer;
	memcpy(&tam_buffer, payload+offs, sizeof(int));
	offs += sizeof(int);

	char *ruta2 = malloc(tam_buffer+1);
	memcpy(ruta2, payload+offs, tam_buffer);
	offs += tam_buffer;

	ruta2[tam_buffer]='\0';
	*buffer = ruta2;

	memcpy(fd, payload+offs, sizeof(int));
	offs += sizeof(int);

	memcpy(size, payload+offs, sizeof(size_t));
	offs += sizeof(int);

	memcpy(offset, payload+offs, sizeof(off_t));
}

//desc: arma paquete con el pointer adress de DIR
package_t slz_res_opendir(uint32_t blk_number)
{
	package_t paquete;

	int tam_payload = sizeof(uint32_t);
	paquete.header = header_get('S', COD_OPENDIR, tam_payload);
	paquete.payload = malloc(tam_payload);
	memcpy(paquete.payload, &blk_number, sizeof(uint32_t));

	return paquete;
}

static int get_fullsize(t_list *filenames)
{
	int filesizes = 0;

	if(list_is_empty(filenames)) return 0;

	t_list *files = list_duplicate(filenames);
	t_link_element *element = files->head;
	t_link_element *aux = NULL;

	while(element != NULL)
	{
		aux = element->next;

		filesizes += strlen(element->data);
		log_msje_info("element data list: [ %s ]", element->data);

		element = aux;
	}

	return filesizes;
}

package_t slz_res_readdir(t_list * filenames)
{
	package_t paquete;
	int numfiles = list_size(filenames);

	int tam_payload = sizeof(int) + sizeof(int)*numfiles + get_fullsize(filenames);
	paquete.header = header_get('S', COD_READDIR, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload, &numfiles, sizeof(int));

	t_link_element *element = filenames->head;
	t_link_element *aux = NULL;

	int offs = sizeof(int);
	while(element != NULL)
	{
		aux = element->next;
		int filesize = strlen(element->data);

		memcpy(paquete.payload + offs, 	&filesize,	sizeof(int));
		offs += sizeof(int);

		memcpy(paquete.payload + offs,  element->data, filesize);//&(element->data[0])
		offs += filesize;

		element = aux;
	}

	return paquete;
}

package_t slz_res_open(int fd)
{
	package_t paquete;

	int tam_payload = sizeof(int);
	paquete.header = header_get('S', COD_OPEN, tam_payload);
	paquete.payload = malloc(tam_payload);
	memcpy(paquete.payload, &fd, sizeof(int));

	return paquete;
}

package_t slz_res_getattr(uint32_t size, uint64_t m_date, int state)
{
	package_t paquete;

	int tam_payload = sizeof(uint32_t) + sizeof(uint64_t);
	paquete.header = header_get('S', COD_GETATTR, tam_payload);
	paquete.payload = malloc(tam_payload);

	int offs = 0;
	memcpy(paquete.payload, &size, sizeof(uint32_t));
	offs += sizeof(uint32_t);

	memcpy(paquete.payload + offs, &m_date, sizeof(uint64_t));
	offs += sizeof(uint64_t);

	memcpy(paquete.payload + offs, &state, sizeof(int));

	return paquete;
}

package_t slz_res_read(char *buf, ssize_t ssize)
{
	package_t paquete;

	int tam_buff = strlen(buf);
	int tam_payload = sizeof(int) + tam_buff +sizeof(ssize_t);

	paquete.header = header_get('S', COD_READ, tam_payload);
	paquete.payload = malloc(tam_payload);

	memcpy(paquete.payload, &tam_buff, sizeof(int));
	memcpy(paquete.payload+sizeof(int), buf, tam_buff);
	memcpy(paquete.payload+sizeof(int)+tam_buff, &ssize, sizeof(ssize_t));

	return paquete;
}

package_t slz_res_write(int size)
{
	package_t paquete;

	int tam_payload = sizeof(int);
	paquete.header = header_get('S', COD_WRITE, tam_payload);
	paquete.payload = malloc(tam_payload);
	memcpy(paquete.payload, &size, sizeof(int));

	return paquete;
}
