#include "filesystem.h"

//El header se encuentra en el primer bloque de mi disco
static void set_sac_header()
{
	sac_header = (GHeader*) disk_blk_pointer;
	log_msje_info("Name : [ %s ]", sac_header->sac);
	log_msje_info("Version : [ %u ]", sac_header->version);
	log_msje_info("Bloque de bitmap: [ %u ]", sac_header->blk_bitmap);
	log_msje_info("Bitmap bloques : [ %u ]", sac_header->size_bitmap);
}

static void set_sac_bitmap()
{
	int bitmap_blk = sac_header->blk_bitmap;
	int bitmap_size_bytes = disk_size / BLOCKSIZE / 8;
	log_msje_info("Bitmap size in bytes is [ %d ]", bitmap_size_bytes);
	sac_bitarray = bitarray_create_with_mode((char *)disk_blk_pointer + bitmap_blk, bitmap_size_bytes, LSB_FIRST);
}


static void set_sac_nodetable()
{
	void* nodetable_addr = disk_blk_pointer + 1 + sac_header->size_bitmap;

	sac_nodetable = (GFile*)nodetable_addr;
	sac_nodetable[0].state = 2;//el primer nodo es la raiz
	sac_nodetable[0].parent_dir_block = 3;//la raiz no tiene padre
	sac_nodetable[0].file_size = BLOCKSIZE;
	sac_nodetable[0].m_date = get_current_time();
	log_msje_info("Nodetable seteado");
}

void fs_set_config()
{
	log_msje_info("Seteo estructuras del filesystem");
	set_sac_header();
	set_sac_bitmap();
	set_sac_nodetable();
}

bool fs_map_disk_in_memory(char *disk_name)
{
	disk_size = get_filesize(disk_name);

	disk_fd = open(disk_name, O_RDWR, 0);
	disk_addr = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_SHARED, disk_fd, 0);
	disk_blk_pointer = (GBlock *)disk_addr;

	return disk_blk_pointer != MAP_FAILED;
}

void fs_munmap_disk()
{
	int res1 = munmap(disk_blk_pointer, disk_size);
	close(disk_fd);
	log_msje_info("munmap disk %d: %s", res1, strerror(errno));
}

//desc: busca dentro de los candidatos aquel que sea la raiz
static void fs_find_root_father(t_list *blks_candidatos)
{
	for(int i=0; i < blks_candidatos->elements_count; i++)
	{
		GBlk_nominee *candidato =  list_get(blks_candidatos, i);
		if(candidato->blk_father != 0){//El padre no es la raiz
			list_remove(blks_candidatos, i);
		}
	}
}

//desc: actualiza la lista de bloques candidatos para un filename
static int fs_find_blk_nominees_by_name(char *filename, t_list *blks_candidatos)
{
	if(list_is_empty(blks_candidatos))//Estamos buscando blks del ultimo argumento del path
	{
		for(int i=0; i < MAX_FILE_NUMBER; i++)//Recorro la tabla de nodos
		{
			if(strcmp( sac_nodetable[i].fname , filename) == 0)//Encontre el filename
			{
				GBlk_nominee *blk_candidato = malloc(sizeof(GBlk_nominee));
				blk_candidato->the_blk = i; //nro de bloque dentro tabla de nodos
				blk_candidato->blk_father = sac_nodetable[i].parent_dir_block;

				list_add(blks_candidatos, blk_candidato);

			}
		}
		if(list_is_empty(blks_candidatos))
			return -1;
	}
	else //Estamos buscando dentro de blks padres candidatos == filename
	{
		for(int i=0; i < blks_candidatos->elements_count; i++)//Recorro candidatos
		{
			GBlk_nominee *candidato = list_get(blks_candidatos, i);
			char *father_name =  sac_nodetable[candidato->blk_father].fname;

			if(strcmp(filename, father_name) == 0)//Califica
			{
				//Me guardo el padre del padre, mantengo candidato blk
				candidato->blk_father = sac_nodetable[candidato->blk_father].parent_dir_block;
				list_replace(blks_candidatos, i, candidato);
			}
			else
			{
				//Saco del concurso al candidato
				list_remove(blks_candidatos, i);
			}
		}
	}
	return 0;
}

//Esta funcion de paso valida que la ruta existe
int fs_get_blk_by_fullpath(char *fullpath)
{
	log_msje_info("Buscando el bloque para path [%s]", fullpath);

	char **filenames = string_split(fullpath, "/"); //separo en ["home", "lala.txt", NULL]

	int size_filenames = get_size_filenames(filenames);

	if(size_filenames == 0) { return 0; } //Para la ruta -> /

	if( size_filenames > 1){//Si tiene mas de 1 un argumento
		reverse_string_vector(filenames);
	}

	t_list *blks_candidatos = list_create();
	while(*filenames != NULL)
	{
		int res = fs_find_blk_nominees_by_name(*filenames, blks_candidatos);
		if(res == -1)
			break;
		filenames++;
	}

	if(size_filenames == 1)//Para rutas estilo /lala.txt de un solo nombre
		fs_find_root_father(blks_candidatos);

	GBlk_nominee *blk_nominado;

	if(blks_candidatos->elements_count == 1)
	{
		blk_nominado = list_get(blks_candidatos, 0);
		if(blk_nominado->blk_father == 0)//el padre es la raiz
		{
			log_msje_info("El bloque es [ %d ]", blk_nominado->the_blk);
		}
	}
	else//blks_candidatos es cero
	{
		log_msje_error("No existe la ruta [ %s ]", fullpath);
		return -1;
	}

	list_destroy(blks_candidatos);
	return blk_nominado->the_blk;
}

//desc: Devuelve un blk nodo libre o EDQUOT si no hay mas nodos libres
int fs_get_free_blk_node()
{
	int node = 0;
	while(sac_nodetable[node].state != 0 && node < MAX_FILE_NUMBER){
		node++;
	}

	if(node >= MAX_FILE_NUMBER)
		return EDQUOT; //Disc quota exceeded

	return node;
}

//desc: chequea que la ruta exista
bool fs_path_exist(char *path)
{
	int res = fs_get_blk_by_fullpath(path);
	return res != -1;
}

void fs_get_child_filenames_of(uint32_t blk_father, t_list *filenames)
{

	for(int i=0; i < MAX_FILE_NUMBER; i++)
	{
		if(sac_nodetable[i].parent_dir_block == blk_father && sac_nodetable[i].state != 0)
		{
			list_add(filenames, sac_nodetable[i].fname);
		}
	}

}

//desc: devuelve el indice donde se encuentra el primer bloque de datos en el disco
static int get_first_blk_data_index()
{
	// 1 BLK HEADER + 1024 BLKS NODOS + BITMAP BLKS
	int blks_metadata = 1 + 1024 + sac_header->size_bitmap;

	//Blk de datos comienza uno siguiente dsps de blk metadata
	return blks_metadata + 1;
}

//desc: devuelve el indice donde se encuentra el ultimo bloque de datos en el disco
static int get_last_blk_data_index()
{
	//Cant de bloques total, el ultimo
	return disk_size / BLOCKSIZE;
}

//desc: devuelve direccion de un bloque de datos libre
int get_free_blk_data_dir()
{
	int first_datablk = get_first_blk_data_index();
	log_msje_info("Primer bloque de datos es [ %d ]", first_datablk);
	int last_datablk = get_last_blk_data_index();
	log_msje_info("Ultimo bloque de datos es [ %d ]", last_datablk);

	int datablk_index;
	bool taken = true;//ocupado

	log_msje_info("Buscando un blk de datos libre, recorro bitmap...");

	//mutex
	for(datablk_index = first_datablk; datablk_index < last_datablk; datablk_index++)
	{
		taken = bitarray_test_bit(sac_bitarray, datablk_index);
		if(!taken){
			bitarray_set_bit(sac_bitarray, datablk_index);
			//sync a disco?
			break;
		}
	}
	//mutex

	if(taken)//todos ocupados
		return -1;

	log_msje_info("NRO DE BLOQUE DE DATOS LIBRE, es el [ %d ]", datablk_index);

	return datablk_index;
}


int fs_get_blk_ind_with_data_blk()
{
	bool done = false;

	int blk_is = get_free_blk_data_dir();
	GBlock_IndSimple *blk_ind = (GBlock_IndSimple*) disk_blk_pointer + blk_is;

	log_msje_info("Blk ind simple is [ %d ]", blk_is);

	if(blk_is != -1){

		int data_blk =  get_free_blk_data_dir();

		log_msje_info("Data block is [ %d ]", data_blk);

		if(data_blk != -1){
			blk_ind->blk_datos[0] = data_blk;
			done = true;
		}
	}

	if(!done)
		return EDQUOT; // Quota of disks blocks has bean exhausted
	else
		return blk_is;

}


size_t fs_read_file(char *buf, size_t size, off_t offset, uint32_t node_blk)
{
	log_msje_info("Preparando para leer blk de datos");
	log_msje_info("Cantidad de bytes por leer [ %d ]", size);

	GFile *file_node = (GFile *)sac_nodetable + node_blk;

	//check
	if(offset == file_node->file_size)
		return 0;

	//Me paro sobre el bloque corresp, y offset
	int bk = (int) (offset / BLOCKSIZE);
	off_t offs = offset - (bk * BLOCKSIZE);

	//Como bk puede ser mayor a 1024... calculo nro de blk ind simple
	int bk_is = (int) bk /GFILEBYTABLE;
	int bk_data = bk - (bk_is * GFILEBYTABLE); //bk data dentro bk ind simple

	int nro_blk_ind = file_node->blk_indirect[bk_is];
	GBlock_IndSimple * blk_indsimple = (GBlock_IndSimple *) disk_blk_pointer + nro_blk_ind;

	/*
	 * Lectura de varios bloques
	 *
	 */
	int leido = 0;
	int size_to_read = size;
	int size_missing = 0;
	int buf_offs = 0;
	int left_space_in_blk = BLOCKSIZE - offs;

	while(leido != size)
	{
		if (size_missing > 0)//necesito seguir leyendo
		{
			size_to_read = size_missing;
			bk_data = bk_data + 1; //el sig blk data

			if(bk_data >= 1024)//tengo que avanzar bk ind simple
			{
				bk_is = bk_is + 1;
				nro_blk_ind = file_node->blk_indirect[bk_is];
				blk_indsimple = (GBlock_IndSimple *) disk_blk_pointer + nro_blk_ind;
				bk_data = 0;
			}

			left_space_in_blk = BLOCKSIZE;
			offs = 0;
		}

		int nro_blk_data = blk_indsimple->blk_datos[bk_data];
		void* blk_addr = (void *)(disk_blk_pointer + nro_blk_data);

		if(size_to_read > left_space_in_blk)
		{
			size_missing = size_to_read - left_space_in_blk;
			size_to_read = left_space_in_blk;
		}

		memcpy(buf + buf_offs, (char *)blk_addr + offs, size_to_read);

		leido += size;
		buf_offs += leido;
	}

	log_msje_info("Lei [ %d ] bytes, falto leer [ %d ] bytes", leido, size-leido);
	return leido;
}


size_t fs_write_file(uint32_t node_blk, char *buffer, size_t size, off_t offset)
{
	log_msje_info("Preparando para escribir blk de datos");
	log_msje_info("Cantidad de bytes por escribir [ %d ]", size);

	GFile *file_node = (GFile *)sac_nodetable + node_blk;

	//Comienzo calculando el nro de bloque de datos a escribir
	int bk = (int) (offset / BLOCKSIZE);//bk --> nro de bloque de datos
	off_t offs = offset - (bk * BLOCKSIZE); //offs -->offset dentro del bloque

	//Como bk puede ser mayor a 1024... calculo nro de blk ind simple
	int bk_is = (int) bk /GFILEBYTABLE;
	int bk_data = bk - (bk_is * GFILEBYTABLE); //bk data dentro bk ind simple

	if(!node_has_blk_ind_assigned(node_blk, bk_is))
	{
		int new_bk_data = get_free_blk_data_dir(); //new blk data
		int blk_index = fs_get_next_index_blk_indsimple_to_assign(node_blk);
		file_node->blk_indirect[blk_index] = new_bk_data;
	}

	//Cargo el blk ind simple correspondiente
	int nro_blk_ind = file_node->blk_indirect[bk_is];
	GBlock_IndSimple * blk_indsimple = (GBlock_IndSimple *) disk_blk_pointer + nro_blk_ind;

	if (!node_has_blk_assigned(node_blk, bk))
	{
		int new_bk_data = get_free_blk_data_dir(); //new blk data
		int blk_index = fs_get_next_index_blk_data_to_assign(nro_blk_ind);
		blk_indsimple->blk_datos[blk_index] = new_bk_data;
	}

	/*
	 * Escritura en varios bloques
	 */

	int escrito = 0;
	int size_to_write = size;
	int size_missing = 0;
	int free_space_in_blk = BLOCKSIZE - offs;
	int buf_offs = 0;

	while (escrito != size)
	{
		if (size_missing > 0)//necesito otro blk de datos
		{
			bk_data = get_free_blk_data_dir(); //new blk data
			int blk_index = fs_get_next_index_blk_data_to_assign(nro_blk_ind);
			blk_indsimple->blk_datos[blk_index] = bk_data;
			free_space_in_blk = BLOCKSIZE;
			size_to_write = size_missing;
			offs = 0;
		}

		//Cargo el blk dato correspondiente
		int nro_blk_data = blk_indsimple->blk_datos[bk_data];

		void* blk_addr = (void *)(disk_blk_pointer +nro_blk_data);

		if(size_to_write > free_space_in_blk)
		{
			size_missing = size_to_write - free_space_in_blk;
			size_to_write = free_space_in_blk;
		}

		//Escribo
		memcpy((char *)blk_addr + offs, buffer + buf_offs, size_to_write);

		escrito += size_to_write;
		buf_offs += escrito;
	}

	file_node->file_size += escrito;

	log_msje_info("Se escribio [ %d ] bytes", escrito);
	return escrito;
}

//truncate in process
void fs_truncate_file(int node, off_t newsize)
{
	GFile *file_node = (GFile *)sac_nodetable + node;

	log_msje_info("File to truncate  [ %s ]", file_node->fname);
	log_msje_info("Truncate to size [ %d ]", newsize);
	log_msje_info("Original size [ %d ]", file_node->file_size);

	//si newsize > file
	if(newsize > file_node->file_size)
	{
		int blks_already_assigned = file_node->file_size / BLOCKSIZE;
		int blks_needed = newsize / BLOCKSIZE;

		if(blks_needed > blks_already_assigned)
		{
			int blks_to_assign = blks_needed - blks_already_assigned;

			int i=1;
			do{
				int bk_data = get_free_blk_data_dir();

				//todo buscar el ultimo blk ind simple que tenga el nodo
				int bk_ind = file_node->blk_indirect[0];

				//cargo el blk ind simple
				GBlock_IndSimple * blk_addr = (GBlock_IndSimple *)(disk_blk_pointer +bk_ind);
				//mmap(blk_addr, BLOCKSIZE, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED | MAP_FIXED, disk_fd, bk_ind*BLOCKSIZE);

				//tengo que asignar desde el ultimo blk el siguiente
				blk_addr->blk_datos[i] = bk_data;

				blks_to_assign--;
				i++;
			}while(blks_to_assign == 0);

		}
	}

	//else
		//todo remove
}


//devuelve para un node file, la cantidad de blks asignados
static int fs_get_cant_blks_datos_asignados(int node)
{
	GFile *file_node = (GFile *)sac_nodetable + node;

	int i=0;
	int cont = 0;
	int blk_ind;
	while((blk_ind = file_node->blk_indirect[i]) != 0)
	{
		GBlock_IndSimple * blk_is = (GBlock_IndSimple *)(disk_blk_pointer + blk_ind);

		int j=0;
		int blk = blk_is->blk_datos[j];
		while( blk >= get_first_blk_data_index() && blk <= get_last_blk_data_index())
		{
			cont++;
			j++;
			blk = blk_is->blk_datos[j];
		}

		i++;
	}
	return cont;
}

//Dado un node file, dice si el nro blk dado esta dentro de sus bloques asignados
bool node_has_blk_assigned(int node, int blk_data)
{
	int cantidad = fs_get_cant_blks_datos_asignados(node);
	return (blk_data >= 0 && blk_data < cantidad);
}

static int fs_get_cant_blks_indsimples_asignados(int node)
{
	GFile *file_node = (GFile *)sac_nodetable + node;

	int cont = 0;
	while(file_node->blk_indirect[cont] != 0)
	{
		cont++;
	}
	return cont;
}

bool node_has_blk_ind_assigned(int node, int blk_ind)
{
	int cantidad = fs_get_cant_blks_indsimples_asignados(node);
	return (blk_ind >= 0 && blk_ind < cantidad);
}


//dado un nodo, me dice el sig bloque libre a asignar
int fs_get_next_index_blk_indsimple_to_assign(int node)
{
	return fs_get_cant_blks_indsimples_asignados(node);
}

//dado un blk ind simple, devuelve el indice del sig bloque de datos a asignar
int fs_get_next_index_blk_data_to_assign(int blk_ind)
{
	GBlock_IndSimple * blk_is = (GBlock_IndSimple *)(disk_blk_pointer + blk_ind);

	int pos = 0;
	int blk = blk_is->blk_datos[pos];
	while( blk >= get_first_blk_data_index() && blk <= get_last_blk_data_index())
	{
		pos++;
		blk = blk_is->blk_datos[pos];
	}

	return pos;
}
