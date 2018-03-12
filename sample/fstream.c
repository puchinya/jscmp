
#define _CRT_SECURE_NO_WARNINGS
#include "fstream.h"

const istream_vtbl_t g_ifstream_vtbl =
{
	ifstream_read,
	ifstream_read_byte
};

int ifstream_init(ifstream_t *self)
{
	self->vtbl = &g_ifstream_vtbl;
	self->fp = NULL;
	return 0;
}

int ifstream_open(ifstream_t *self, const char *filename)
{
	self->fp = fopen(filename, "rb");
	if (self->fp != NULL) {
		return 0;
	}
	else {
		return -1;
	}
}

int ifstream_close(ifstream_t *self)
{
	fclose(self->fp);
	self->fp = NULL;
	return 0;
}

int ifstream_read(ifstream_t *self, void *data, int size)
{
	return fread(data, 1, size, self->fp);
}

int ifstream_read_byte(ifstream_t *self)
{
	int r;
	char ch[1];
	r = fread(ch, 1, 1, self->fp);
	if (r <= 0) return -1;
	return ch[0];
}

int ifstream_length(ifstream_t *self)
{
	int result, bk;

	bk = ftell(self->fp);
	fseek(self->fp, 0, SEEK_END);
	result = ftell(self->fp);
	fseek(self->fp, bk, SEEK_SET);

	return result;
}
