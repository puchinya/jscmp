
#ifndef __FSTREAM_H
#define __FSTREAM_H

#include "iostream.h"
#include <stdio.h>

typedef struct {
	const istream_vtbl_t	*vtbl;
	FILE					*fp;
} ifstream_t;

const istream_vtbl_t g_ifstream_vtbl;

int ifstream_init(ifstream_t *self);
int ifstream_open(ifstream_t *self, const char *filename);
int ifstream_close(ifstream_t *self);
int ifstream_read(ifstream_t *self, void *data, int size);
int ifstream_read_byte(ifstream_t *self);
int ifstream_length(ifstream_t *self);

#endif
