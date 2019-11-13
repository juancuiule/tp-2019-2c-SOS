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
	GBlock* node_blk_start = disk_blk_pointer + 1 + sac_header->size_bitmap;
	sac_nodetable = (GFile*)node_blk_start;
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
	int disk_fd = open(disk_name, O_RDWR, 0);
	disk = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, disk_fd, 0);
	disk_blk_pointer = disk;
	return disk != MAP_FAILED;
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





