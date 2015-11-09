
#include "jscmp.h"
#include <stdint.h>
#include <string.h>

#define JSCMP_POS_FIXINT_E		0x7F
//#define JSCMP_FIXOBJ_S		0x80
//#define JSCMP_FIXARRAY_S	0x90
#define JSCMP_FIXSTR_S		0xA0
#define JSCMP_FIXSTR_E		0xBF
#define JSCMP_NULL			0xC0
#define JSCMP_PRE_STR8		0xC1
#define JSCMP_FALSE			0xC2
#define JSCMP_TRUE			0xC3
#define JSCMP_UINT8			0xCC
#define JSCMP_UINT16		0xCD
#define JSCMP_UINT32		0xCE
#define JSCMP_INT8			0xD0
#define JSCMP_INT16			0xD1
#define JSCMP_INT32			0xD2
#define JSCMP_STR8			0xD9
#define JSCMP_STR16			0xDA
//#define JSCMP_STR32			0xDB
#define JSCMP_ARRAY16		0xDC
//#define JSCMP_ARRAY32		0xDD
#define JSCMP_MAP16			0xDE
//#define JSCMP_MAP32			0xDF
#define JSCMP_PRE_STR16		0xDF
#define JSCMP_NEG_FIXINT_S	0xE0

#define BUF_SIZE	64

int findstr(const char **strtbl, int strtbl_len, const char *s)
{
	int i;
	for (i = 0; i < strtbl_len; i++) {
		if (strcmp(strtbl[i], s) == 0)
			return i;
	}
	return -1;
}

typedef struct {
	istream_t	*src;
	int			unch;
} jscmp_ch_reader_t;

void jscmp_init_reader(jscmp_ch_reader_t *reader, istream_t *s)
{
	reader->src = s;
	reader->unch = -1;
}

int jscmp_getch(jscmp_ch_reader_t *reader)
{
	int result;
	if (reader->unch >= 0) {
		result = reader->unch;
	}
	else {
		result = istream_read_byte(reader->src);
	}
	reader->unch = -1;

	return result;
}

void jscmp_ungetch(jscmp_ch_reader_t *reader, int ch)
{
	reader->unch = ch;
}

int jscmp_parse(jscmp_doc_t *doc, istream_t *src, char *dst_buf, int dst_buf_size, const char **strtbl, int strtbl_len)
{
	int ch;
	int line = 0;
	int cr = 0;
	int esc;
	char buf[BUF_SIZE + 1];
	int buf_idx;
	int buf_ovr;
	int dst_buf_pos = 0;
	int str_len_write_pos;
	uint16_t length_write_val[JSCMP_NEST_LEVEL];
	int container_node_stack[JSCMP_NEST_LEVEL];
	int container_node_stack_cnt = 0;
	jscmp_ch_reader_t reader;

	doc->dst_buf = dst_buf;
	doc->dst_buf_size = dst_buf_size;
	doc->strtbl = strtbl;
	doc->strtbl_len = strtbl_len;

	jscmp_init_reader(&reader, src);

	for (;;) {
		ch = jscmp_getch(&reader);
		if (ch < 0) {
			break;
		}

		if (ch == ' ' || ch == '\t') {
			continue;
		}

		if (ch == '\r') {
		}
		else if (ch == '\n') {
			line++;
		} else if (ch == ',') {

		} else if (ch == ':') {

		}
		else if (ch == 't') { /* true */
			int ch1, ch2, ch3;
			ch1 = jscmp_getch(&reader);
			ch2 = jscmp_getch(&reader);
			ch3 = jscmp_getch(&reader);

			if (ch1 < 0 || ch2 < 0 || ch3 < 0) {
				return JSCMP_E_READ;
			}

			if (ch1 == 'r' && ch2 == 'u' && ch3 == 'e'){
				dst_buf[dst_buf_pos++] = JSCMP_TRUE;

				if (container_node_stack_cnt > 0) {
					length_write_val[container_node_stack_cnt - 1]++;
				}
			}
			else {
				return JSCMP_E_SYNTAX;
			}

		}
		else if (ch == 'f') { /* false */
			int ch1, ch2, ch3, ch4;
			ch1 = jscmp_getch(&reader);
			ch2 = jscmp_getch(&reader);
			ch3 = jscmp_getch(&reader);
			ch4 = jscmp_getch(&reader);

			if (ch1 < 0 || ch2 < 0 || ch3 < 0 || ch4 < 0) {
				return JSCMP_E_READ;
			}

			if (ch1 == 'a' && ch2 == 'l' && ch3 == 's' && ch4 == 'e'){
				dst_buf[dst_buf_pos++] = JSCMP_FALSE;

				if (container_node_stack_cnt > 0) {
					length_write_val[container_node_stack_cnt - 1]++;
				}
			}
			else {
				return JSCMP_E_SYNTAX;
			}

		}
		else if (ch == 'n') {	/* null */
			int ch1, ch2, ch3;
			ch1 = jscmp_getch(&reader);
			ch2 = jscmp_getch(&reader);
			ch3 = jscmp_getch(&reader);

			if (ch1 < 0 || ch2 < 0 || ch3 < 0) {
				return JSCMP_E_READ;
			}

			if (ch1 == 'u' && ch2 == 'l' && ch3 == 'l'){
				dst_buf[dst_buf_pos++] = JSCMP_NULL;

				if (container_node_stack_cnt > 0) {
					length_write_val[container_node_stack_cnt - 1]++;
				}
			}
			else {
				return JSCMP_E_SYNTAX;
			}

		} else if (ch == '-' || (ch >= '0' && ch <= '9')) {
			int sign = 0;
			int32_t ival = 0;

			if (ch == '-') {
				sign = 1;
			}
			else {
				ival = ch - '0';
			}

			for (;;) {
				ch = jscmp_getch(&reader);
				if (ch < 0) {
					return JSCMP_E_READ;
				}
				if (ch >= '0' && ch <= '9') {
					ival = ival * 10 + (ch - '0');
				}
				else {
					jscmp_ungetch(&reader, ch);
					break;
				}
			}

			if (sign) ival = -ival;

			if (ival >= 0 && ival <= 0x7F) {
				dst_buf[dst_buf_pos++] = ival;
			}
			else if (ival >= -32 && ival <= -1) {
				dst_buf[dst_buf_pos++] = ival;
			}
			else {
				if (-128 <= ival && ival <= 127) {
					dst_buf[dst_buf_pos++] = JSCMP_INT8;
					dst_buf[dst_buf_pos++] = ival;
				} else if (-32768 <= ival && ival <= 32767) {
					dst_buf[dst_buf_pos++] = JSCMP_INT16;
					dst_buf[dst_buf_pos++] = ival;
					dst_buf[dst_buf_pos++] = ival >> 8;
				} else {
					dst_buf[dst_buf_pos++] = JSCMP_INT32;
					dst_buf[dst_buf_pos++] = ival;
					dst_buf[dst_buf_pos++] = ival >> 8;
					dst_buf[dst_buf_pos++] = ival >> 16;
					dst_buf[dst_buf_pos++] = ival >> 24;
				}
			}

			if (container_node_stack_cnt > 0) {
				length_write_val[container_node_stack_cnt - 1]++;
			}
		}
		else if (ch == '"') {
			/* str */
			esc = 0;
			buf_idx = 0;
			buf_ovr = 0;

			for (;;) {

				ch = jscmp_getch(&reader);
				if (ch < 0) {
					return JSCMP_E_READ;
				}

				if (ch == '"') {
					/* end of str */
					if (!buf_ovr) {
						int idx;
						buf[buf_idx] = '\0';
						idx = findstr(strtbl, strtbl_len, buf);
						if (idx >= 0) {
							if (idx >= 256) {
								dst_buf[dst_buf_pos++] = JSCMP_PRE_STR16;
								dst_buf[dst_buf_pos++] = (uint8_t)idx;
								dst_buf[dst_buf_pos++] = (uint8_t)(idx >> 8);
							}
							else {
								dst_buf[dst_buf_pos++] = JSCMP_PRE_STR8;
								dst_buf[dst_buf_pos++] = (uint8_t)idx;
							}
						}
						else{
							if (buf_idx <= 15) {
								dst_buf[dst_buf_pos++] = JSCMP_FIXSTR_S + buf_idx;
							}
							else {
								dst_buf[dst_buf_pos++] = JSCMP_STR8;
								dst_buf[dst_buf_pos++] = buf_idx;
							}
							memcpy(&dst_buf[dst_buf_pos], buf, buf_idx);
							dst_buf_pos += buf_idx;
							dst_buf[dst_buf_pos++] = '\0';
						}
					}
					else {
						dst_buf[str_len_write_pos] = buf_idx;
						dst_buf[str_len_write_pos + 1] = buf_idx >> 8;
						dst_buf[dst_buf_pos++] = '\0';
					}

					if (container_node_stack_cnt > 0) {
						length_write_val[container_node_stack_cnt - 1]++;
					}

					break;
				}
				else{
					if (buf_ovr) {
						buf_idx++;
						dst_buf[dst_buf_pos++] = ch;
					}
					else if (buf_idx >= BUF_SIZE) {
						buf_ovr = 1;

						dst_buf[dst_buf_pos++] = JSCMP_STR16;
						str_len_write_pos = dst_buf_pos;
						dst_buf_pos += 2;
						memcpy(&dst_buf[dst_buf_pos], buf, buf_idx);
						dst_buf_pos += buf_idx;
						dst_buf[dst_buf_pos++] = ch;
					}
					else {
						buf[buf_idx++] = ch;
					}
				}
			}

		} else if (ch == '[') {
			/* array */

			if (container_node_stack_cnt > 0) {
				length_write_val[container_node_stack_cnt - 1]++;
			}

			container_node_stack[container_node_stack_cnt] = dst_buf_pos;
			dst_buf[dst_buf_pos] = JSCMP_ARRAY16;
			dst_buf_pos += 5;
			length_write_val[container_node_stack_cnt] = 0;

			container_node_stack_cnt++;
			if (container_node_stack_cnt > JSCMP_NEST_LEVEL) {
				return JSCMP_E_NEST_LEVEL_OVER;
			}
		}
		else if (ch == '{') {
			/* object */

			if (container_node_stack_cnt > 0) {
				length_write_val[container_node_stack_cnt - 1]++;
			}

			container_node_stack[container_node_stack_cnt] = dst_buf_pos;
			dst_buf[dst_buf_pos] = JSCMP_MAP16;
			dst_buf_pos += 5;
			length_write_val[container_node_stack_cnt] = 0;

			container_node_stack_cnt++;
			if (container_node_stack_cnt > JSCMP_NEST_LEVEL) {
				return JSCMP_E_NEST_LEVEL_OVER;
			}
		}
		else if (ch == ']' || ch == '}') {
			int container_node_pos;
			int container_len;
			int container_size;

			container_node_stack_cnt--;

			container_node_pos = container_node_stack[container_node_stack_cnt];
			container_len = length_write_val[container_node_stack_cnt];
			container_size = dst_buf_pos - container_node_pos;
				
			dst_buf[container_node_pos + 1] = container_len;
			dst_buf[container_node_pos + 2] = container_len >> 8;
			dst_buf[container_node_pos + 3] = container_size;
			dst_buf[container_node_pos + 4] = container_size >> 8;
		}
	}

	if (container_node_stack_cnt > 0) {
		uint8_t type = dst_buf[container_node_stack[container_node_stack_cnt - 1]];
		if (type == JSCMP_ARRAY16) {
			return JSCMP_E_SYNTAX_UNMATCH_ARRAY_BRACKET;
		}
		else {
			return JSCMP_E_SYNTAX_UNMATCH_OBJECT_BRACKET;
		}
	}

	doc->dst_buf_pos = dst_buf_pos;

	return 0;
}

jscmp_node_t jscmp_root(jscmp_doc_t *doc)
{
	return doc->dst_buf;
}

int jscmp_node_size(jscmp_node_t n)
{
	int type = n[0];
	
	if (JSCMP_FIXSTR_S <= type && type <= JSCMP_FIXSTR_E) {
		return 1 + (type - JSCMP_FIXSTR_S) + 1;
	}
	else if (type == JSCMP_PRE_STR8) {
		return 2;
	}
	else if (type == JSCMP_UINT8 || type == JSCMP_INT8) {
		return 2;
	}
	else if (type == JSCMP_PRE_STR16 || type == JSCMP_UINT16 || type == JSCMP_INT16) {
		return 3;
	}
	else if (type == JSCMP_UINT32 || type == JSCMP_INT32) {
		return 5;
	}
	else if (type == JSCMP_STR8) {
		int len = n[1];
		return 2 + len + 1;
	}
	else if (type == JSCMP_STR16) {
		int len_l = n[1];
		int len_h = n[2];
		int len = len_l | (len_h << 8);
		return 2 + len + 1;
	}
	else if (type == JSCMP_ARRAY16 || type == JSCMP_MAP16) {
		int size_l = n[3];
		int size_h = n[4];
		int size = size_l | (size_h << 8);
		return size;
	}

	return 1;
}

int jscmp_type(jscmp_node_t n) {
	
	int v = n[0];
	
	if (v <= JSCMP_POS_FIXINT_E || v >= JSCMP_NEG_FIXINT_S || v == JSCMP_INT8 || v == JSCMP_INT16 || v == JSCMP_INT32) {
		return JSCMP_TYPE_INT;
	}
	if ((JSCMP_FIXSTR_S <= v && v <= JSCMP_FIXSTR_E) || v == JSCMP_PRE_STR8 || v == JSCMP_PRE_STR16 || v == JSCMP_STR8 || v == JSCMP_STR16) {
		return JSCMP_TYPE_STR;
	}
	if (v == JSCMP_ARRAY16) {
		return JSCMP_TYPE_ARRAY;
	}
	if (v == JSCMP_MAP16) {
		return JSCMP_TYPE_OBJECT;
	}
	if (v == JSCMP_TRUE || v == JSCMP_FALSE) {
		return JSCMP_TYPE_BOOL;
	}

	return JSCMP_TYPE_NULL;
}

int32_t jscmp_int_val(jscmp_node_t n)
{
	int type = n[0];
	
	if (type <= JSCMP_POS_FIXINT_E) {
		return type;
	}
	if (type >= JSCMP_NEG_FIXINT_S) {
		return (int8_t)type;
	}
	if (type == JSCMP_INT8) {
		return (int8_t)n[1];
	}
	else if (type == JSCMP_INT16) {
		return (int16_t)n[1] | ((int16_t)n[2] << 8);
	}
	else if (type == JSCMP_INT32) {
		return (int32_t)n[1] | ((int32_t)n[2] << 8) | ((int32_t)n[3] << 16) | ((int32_t)n[4] << 24);
	}
	else{
		return 0;
	}
}

const char *jscmp_str_val(jscmp_doc_t *doc, jscmp_node_t n)
{
	int type = n[0];

	if (JSCMP_FIXSTR_S <= type && type <= JSCMP_FIXSTR_E) {
		return &n[1];
	}
	if (type == JSCMP_PRE_STR8) {
		return doc->strtbl[n[1]];
	}
	else if (type == JSCMP_PRE_STR16) {
		int idx = (int)n[1] | ((int)n[2] << 8);
		return doc->strtbl[idx];
	}
	else if (type == JSCMP_STR8) {
		return &n[2];
	}
	else if (type == JSCMP_STR16) {
		return &n[3];
	}
	else{
		return 0;
	}
}

int jscmp_str_len(jscmp_doc_t *doc, jscmp_node_t n)
{
	int type = n[0];

	if (JSCMP_FIXSTR_S <= type && type <= JSCMP_FIXSTR_E) {
		return type - JSCMP_FIXSTR_S;
	}
	else if (type == JSCMP_PRE_STR8) {
		return strlen(doc->strtbl[n[1]]);
	}
	else if (type == JSCMP_PRE_STR16) {
		int idx = (int)n[1] | ((int)n[2] << 8);
		return strlen(doc->strtbl[idx]);
	}
	else if (type == JSCMP_STR8) {
		return n[1];
	}
	else if (type == JSCMP_STR16) {
		return (int)n[0] | ((int)n[1] << 8);
	}
	else{
		return -1;
	}
}

int jscmp_bool_val(jscmp_node_t n)
{
	int type = n[0];
	if (type == JSCMP_TRUE) {
		return 1;
	} else {
		return 0;
	}
}

jscmp_node_t jscmp_next(jscmp_node_t n)
{
	return &n[jscmp_node_size(n)];
}

int jscmp_array_len(jscmp_node_t array_node)
{
	int len_l = array_node[1];
	int len_h = array_node[2];
	int len = len_l | (len_h << 8);

	return len;
}

jscmp_node_t jscmp_array_begin(jscmp_node_t array_node)
{
	return &array_node[5];
}

jscmp_node_t jscmp_array_end(jscmp_node_t array_node)
{
	int size_l = array_node[3];
	int size_h = array_node[4];
	int size = size_l | (size_h << 8);

	return &array_node[size];
}

jscmp_node_t jscmp_array_next(jscmp_node_t n)
{
	return jscmp_next(n);
}

int jscmp_object_len(jscmp_node_t object_node)
{
	int len_l = object_node[1];
	int len_h = object_node[2];
	int len = len_l | (len_h << 8);

	return len >> 1;
}

jscmp_node_t jscmp_object_begin(jscmp_node_t object_node)
{
	return &object_node[5];
}

jscmp_node_t jscmp_object_end(jscmp_node_t object_node)
{
	int size_l = object_node[3];
	int size_h = object_node[4];
	int size = size_l | (size_h << 8);

	return &object_node[size];
}

jscmp_node_t jscmp_object_val(jscmp_node_t n)
{
	return jscmp_next(n);
}

jscmp_node_t jscmp_object_next(jscmp_node_t n)
{
	return jscmp_next(jscmp_next(n));
}

