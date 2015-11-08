
#ifndef IOSTREAM_H
#define IOSTREAM_H

#define E_IO_EOS	-2
#define E_IO_READ	-3
#define E_IO_WRITE	-4

typedef struct {
	int(*read)(void *self, void *data, int size);
	int(*read_byte)(void *self);
} istream_vtbl_t;

typedef struct {
	const istream_vtbl_t	*vtbl;
} istream_t;

typedef struct {
	int(*write)(void *self, const void *data, int size);
	int(*write_byte)(void *self, int data);
	int(*flush)(void *self);
} ostream_vtbl_t;

typedef struct {
	const ostream_vtbl_t	*vtbl;
} ostream_t;

int istream_read(void *self, void *data, int size);
int istream_read_byte(void *self);

int ostream_write(void *self, const void *data, int size);
int ostream_write_byte(void *self, int data);
int ostream_flush(void *self);

#endif
