
#include "iostream.h"

int istream_read(void *self, void *data, int size)
{
	istream_t *s = (istream_t *)self;
	return s->vtbl->read(self, data, size);
}

int istream_read_byte(void *self)
{
	int result;
	istream_t *s = (istream_t *)self;
	if (s->vtbl->read_byte) {
		result = s->vtbl->read_byte(self);
	}
	else {
		char d[1];
		int r;

		r = istream_read(self, d, 1);
		if (r != 1) {
			result = r;
		}
		else {
			result = d[0];
		}
	}

	return result;
}
