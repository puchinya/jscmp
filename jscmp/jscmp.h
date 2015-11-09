/* JSON Compressing Parser */

#ifndef JSCMP_H
#define JSCMP_H

#include <stdint.h>
#include "iostream.h"

#define JSCMP_TYPE_NULL		0x00
#define JSCMP_TYPE_INT		0x01
#define JSCMP_TYPE_FLOAT	0x02
#define JSCMP_TYPE_STR		0x03
#define JSCMP_TYPE_BOOL		0x04
#define JSCMP_TYPE_ARRAY	0x05
#define JSCMP_TYPE_OBJECT	0x06

typedef uint8_t *jscmp_node_t;
typedef struct {
	const char	**strtbl;
	int			strtbl_len;
	char		*dst_buf;
	int			dst_buf_size;
	int			dst_buf_pos;
} jscmp_doc_t;

int jscmp_parse(jscmp_doc_t *doc, istream_t *src, char *dst_buf, int dst_buf_size, const char **strtbl, int strtbl_len);

jscmp_node_t jscmp_root(jscmp_doc_t *doc);

int jscmp_type(jscmp_node_t n);
int32_t jscmp_int_val(jscmp_node_t n);
float jscmp_float_val(jscmp_node_t n);
const char *jscmp_str_val(jscmp_doc_t *doc, jscmp_node_t n);
int jscmp_str_len(jscmp_doc_t *doc, jscmp_node_t n);
int jscmp_bool_val(jscmp_node_t n);

jscmp_node_t jscmp_next(jscmp_node_t n);

int jscmp_array_len(jscmp_node_t array_node);
jscmp_node_t jscmp_array_begin(jscmp_node_t array_node);
jscmp_node_t jscmp_array_end(jscmp_node_t array_node);
jscmp_node_t jscmp_array_next(jscmp_node_t n);

int jscmp_object_len(jscmp_node_t object_node);
jscmp_node_t jscmp_object_begin(jscmp_node_t object_node);
jscmp_node_t jscmp_object_end(jscmp_node_t object_node);
jscmp_node_t jscmp_object_val(jscmp_node_t n);
jscmp_node_t jscmp_object_next(jscmp_node_t n);

#endif
