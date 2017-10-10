////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Lector simple del index de un repositorio GIT
//
// Autor: Germán Martínez
//
// Referencias: https://github.com/git/git/blob/master/Documentation/technical/index-format.txt
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>

#include "git.h"

#define SHA_BYTE_SIZE 20

#if 1
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#else
uint16_t ntohs(uint16_t n)
{
	int i = 1;
	char* p = (char*)&i;

	if(p[0] == 1)
	{
		char c;
		
		c = ((char*)&n)[0];
		((char*)&n)[0] = ((char*)&n)[1];
		((char*)&n)[1] = c;
	}
	return n;
}
uint32_t ntohl(uint32_t n)
{
	int i = 1;
	char* p = (char*)&i;

	if(p[0] == 1)
	{
		char c;
		
		c = ((char*)&n)[0];
		((char*)&n)[0] = ((char*)&n)[3];
		((char*)&n)[3] = c;

		c = ((char*)&n)[1];
		((char*)&n)[1] = ((char*)&n)[2];
		((char*)&n)[2] = c;
	}
	return n;
}
#endif


//
// Index tree
//
struct tree
{
	struct tree**	children;
	int				children_count;
	uint8_t			sha1[20];
	char			path[1];
};

//
// Leer texto ASCII hasta encontrar el caracter 'stop'
//
void read_ascii_text(FILE* fp, char* s, int stop)
{
	int c;
	do
	{
		*s++ = c = fgetc(fp);
	}
	while(c != stop);
	*s = 0;
}

//
// Leer árbol
//
struct tree* read_tree(FILE* fp)
{
	char tmp[256];
	int  i;
	int  len;
	struct tree* tree;
	int  entry_count;
	int  subtrees_count;

	// Leer el path
	read_ascii_text(fp, tmp, 0);
	len = strlen(tmp);
	
	// Crear el nodo del árbol
	tree = (struct tree*)malloc(sizeof(struct tree) + len);
	memset(tree, 0, sizeof(struct tree) + len);
	
	// Asignar el path
	strcpy(tree->path, tmp);
	
	// Leer la cantidad de entradas
	read_ascii_text(fp, tmp, ' ');
	entry_count = atoi(tmp);
	
	// Leer la cantidad de subtrees
	read_ascii_text(fp, tmp, '\n');
	subtrees_count = atoi(tmp);
	
	// Leer SHA
	if(entry_count != -1)
	{
		fread(tree->sha1, SHA_BYTE_SIZE, 1, fp);
	}
//	printf("tree->path: %s\n", tree->path);
//	printf("entry_count: %d\n", entry_count);
//	printf("  subtrees_count: %d\n", subtrees_count);

	//
	// Leer los subtrees
	//
	if(subtrees_count > 0)
	{
		tree->children = (struct tree**)malloc(sizeof(struct tree*) * subtrees_count);
		tree->children_count = subtrees_count;
		
		for(i = 0; i < subtrees_count; i++)
		{
			tree->children[i] = read_tree(fp);
		}
	}
	return tree;
}
//
// Imprimir árbol
//
void print_tree(struct tree* t, int level)
{
	int i;

	printf("  ");
	for(i = 0; i < level; i++)
	{
		printf("  ");
	}
	printf(t->path);
	printf("\n");
	for(i = 0; i < t->children_count; i++)
	{
		print_tree(t->children[i], level+1);
	}
}

//
// Eliminar árbol recursivamente
//
void delete_tree(struct tree* t)
{
	// Eliminar hijos
	if(t->children_count > 0)
	{
		int i;
		for(i = 0; i < t->children_count; i++)
			delete_tree(t->children[i]);
		free(t->children);
	}
	// Eliminar nodo
	free(t);
}


//
// Main
//
int main(int argc, char* argv[])
{
	FILE* fpIndex;
	char  szIndexPath[256];
	if(argc > 1)
	{
		int len;

		strcpy(szIndexPath, argv[1]);
		len = strlen(szIndexPath);
		if(szIndexPath[len-1] == '/' || szIndexPath[len-1] == '\\')
		{
			len--;
		}
		strcpy(&szIndexPath[len], "/.git/index");
	}
	else
	{
		return -1;
	}
	printf("Reading index: %s\n\n", szIndexPath);
	if(NULL != (fpIndex = fopen(szIndexPath, "rb")))
	{
		uint32_t i;
		struct git_index_header hdr;
		struct git_index_entry  entry;
		char name[256];
		long fsize;
		
		//
		// Calcular el tamaño del archivo
		//
		fseek(fpIndex, 0, SEEK_END);
		fsize = ftell(fpIndex);
		fseek(fpIndex, 0, SEEK_SET);
		
		//
		// Leer el header
		//
		fread(&hdr, sizeof(struct git_index_header), 1, fpIndex);
		//
		// Traducir los enteros de "Network Byte Order" a "Host Byte Order"
		//
		hdr.version = ntohl(hdr.version);
		hdr.entries = ntohl(hdr.entries);
		//
		// Imprimit header
		//
		printf("Header:\n");
		printf("  signature = %c%c%c%c\n", hdr.signature[0], hdr.signature[1], hdr.signature[2], hdr.signature[3]);
		printf("  version   = %d\n", hdr.version);
		printf("  entries   = %d\n", hdr.entries);
		
		//
		// Leer entradas
		//
		printf("\nEntries:\n");
		for(i = 0; i < hdr.entries; i++)
		{
			int pos = ftell(fpIndex);
			
			if(hdr.version >= 3)
				fread(&entry, sizeof(struct git_index_entry), 1, fpIndex);
			else
				fread(&entry, sizeof(struct git_index_entry)-2, 1, fpIndex);
			
			//
			// Traducir los enteros de "Network Byte Order" a "Host Byte Order"
			//
			entry.ctime_seconds    = ntohl(entry.ctime_seconds);
			entry.ctime_nanosecond = ntohl(entry.ctime_nanosecond);
			entry.mtime_seconds    = ntohl(entry.mtime_seconds);
			entry.mtime_nanosecond = ntohl(entry.mtime_nanosecond);
			entry.dev              = ntohl(entry.dev);
			entry.ino              = ntohl(entry.ino);
			entry.mode             = ntohl(entry.mode);
			entry.uid              = ntohl(entry.uid);
			entry.gid              = ntohl(entry.gid);
			entry.file_size        = ntohl(entry.file_size);
			entry.flags            = ntohs(entry.flags);
			entry.version          = ntohs(entry.version);
			
			//
			// Leer nombre de archivo
			//
			{
				uint16_t j;
				uint16_t len = (entry.flags&0xFFF);
				for(j = 0; j < len; j++)
				{
					name[j] = fgetc(fpIndex);
				}
				name[j] = 0;
				fseek(fpIndex, 8-(ftell(fpIndex)-pos)%8, SEEK_CUR);
			}
			//
			// Imprimir nombre de archivo
			//
			printf("  file: %s\n", name);
		}
		//
		// Leer extensiones
		//
		printf("\nExtensions:\n");

		while(ftell(fpIndex) < (fsize-20)) // 20 representa los 160-bit SHA-1 para el checksum
		{
			struct git_index_extension_header ext;
			
			// Leer el header de la extensión
			fread(&ext, sizeof(struct git_index_extension_header), 1, fpIndex);
			// Traducir los enteros de "Network Byte Order" a "Host Byte Order"
			ext.size = ntohl(ext.size);
			
			printf("  name: %c%c%c%c\n", ext.signature[0], ext.signature[1], ext.signature[2], ext.signature[3]);
			printf("  size: %d bytes\n", ext.size);
			
			// Extensión: "Cached tree"
			if(0 == memcmp(ext.signature, "TREE", 4))
			{
				struct tree* tree;
				if(NULL != (tree = read_tree(fpIndex)))
				{
					print_tree(tree, 0);
					delete_tree(tree);
				}
			}
			// Extensión: "Resolve undo"
			else if(0 == memcmp(ext.signature, "REUC", 4))
			{
				fseek(fpIndex, ext.size, SEEK_CUR);
			}
			// Extensión: "Split index"
			else if(0 == memcmp(ext.signature, "link", 4))
			{
				fseek(fpIndex, ext.size, SEEK_CUR);
			}
			// Extensión: "Untracked cache"
			else if(0 == memcmp(ext.signature, "UNTR", 4))
			{
				fseek(fpIndex, ext.size, SEEK_CUR);
			}
			// Saltar extensión desconocida
			else
			{
				fseek(fpIndex, ext.size, SEEK_CUR);
			}
		}
		fclose(fpIndex);
	}
	return 0;
}



