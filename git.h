////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Index de un repositorio GIT
//
// Autor: Germán Martínez
//
// Referencias: https://github.com/git/git/blob/master/Documentation/technical/index-format.txt
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ___GIT_H___
#define ___GIT_H___
#include <stdint.h>


#include <pshpack1.h>

//
// Header de archivo index de git
//
struct git_index_header
{
	char		signature[4];	// "DIRC" (significa "dircache")
	uint32_t	version;		// Número de la versión (Network Byte Order)
	uint32_t	entries;		// Número de entradas
};

//
// Entradas en el archivo index de git
//
struct git_index_entry
{
	uint32_t	ctime_seconds;
	uint32_t	ctime_nanosecond;
	uint32_t	mtime_seconds;
	uint32_t	mtime_nanosecond;
	uint32_t	dev;
	uint32_t	ino;
	uint32_t	mode;
	uint32_t	uid;
	uint32_t	gid;
	uint32_t	file_size;
	uint8_t		sha1[20];		// 160-bit SHA-1
	uint16_t	flags;
	uint16_t	version;		// Sólo para versiones del index mayor ó igual a 3
};

//
// Header de la extensión dentro del archivo index de git
//
struct git_index_extension_header
{
	char		signature[4];
	uint32_t	size;
};

#include <poppack.h>

#endif
