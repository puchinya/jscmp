
#include "../include/jscmp.h"
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

int jscmp_findstr(const char **strtbl, int strtbl_len, const char *s)
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
} jscmp_parser_ctx_t;

void jscmp_init_ctx(jscmp_parser_ctx_t *reader, istream_t *s)
{
	reader->src = s;
	reader->unch = -1;
}

int jscmp_getch(jscmp_parser_ctx_t *ctx)
{
	int result;
	if (ctx->unch >= 0) {
		result = ctx->unch;
	}
	else {
		result = istream_read_byte(ctx->src);
	}
	ctx->unch = -1;

	return result;
}

void jscmp_ungetch(jscmp_parser_ctx_t *ctx, int ch)
{
	ctx->unch = ch;
}

int jscmp_hex(int ch)
{
	if ('0' <= ch && ch <= '9') {
		return ch - '0';
	}

	if ('a' <= ch && ch <= 'f') {
		return ch - 'a' + 10;
	}

	if ('A' <= ch && ch <= 'F') {
		return ch - 'A' + 10;
	}

	return -1;
}

int jscmp_parse(jscmp_doc_t *doc, istream_t *src, char *dst_buf, int dst_buf_size, const char **strtbl, int strtbl_len)
{
	int ch;
	char buf[BUF_SIZE + 1];
	int buf_idx;
	int buf_ovr;
	int dst_buf_pos = 0;
	int str_len_write_pos;
	uint16_t length_write_val[JSCMP_NEST_LEVEL];
	uint16_t container_node_stack[JSCMP_NEST_LEVEL];
	int container_node_stack_cnt = 0;
	jscmp_parser_ctx_t ctx;

	if (!doc || !src || !dst_buf) {
		return JSCMP_E_ARG;
	}

	doc->dst_buf = dst_buf;
	doc->dst_buf_size = dst_buf_size;
	doc->strtbl = strtbl;
	doc->strtbl_len = strtbl_len;

	jscmp_init_ctx(&ctx, src);

	for (;;) {
		ch = jscmp_getch(&ctx);
		if (ch < 0) {
			break;
		}

		if (ch == ' ' || ch == '\t') {
			continue;
		}

		if (ch == '\r') {
		} else if (ch == '\n') {

		} else if (ch == ',') {

		} else if (ch == ':') {

		} else if (ch == 't') { /* true */
			
			int ch1, ch2, ch3;

			ch1 = jscmp_getch(&ctx);
			ch2 = jscmp_getch(&ctx);
			ch3 = jscmp_getch(&ctx);

			if (ch1 < 0 || ch2 < 0 || ch3 < 0) {
				return JSCMP_E_READ;
			}

			if (ch1 == 'r' && ch2 == 'u' && ch3 == 'e'){

				if (dst_buf_pos + 1 > dst_buf_size) {
					return JSCMP_E_NOMEM;
				}

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
			
			ch1 = jscmp_getch(&ctx);
			ch2 = jscmp_getch(&ctx);
			ch3 = jscmp_getch(&ctx);
			ch4 = jscmp_getch(&ctx);

			if (ch1 < 0 || ch2 < 0 || ch3 < 0 || ch4 < 0) {
				return JSCMP_E_READ;
			}

			if (ch1 == 'a' && ch2 == 'l' && ch3 == 's' && ch4 == 'e'){

				if (dst_buf_pos + 1 > dst_buf_size) {
					return JSCMP_E_NOMEM;
				}

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
			
			ch1 = jscmp_getch(&ctx);
			ch2 = jscmp_getch(&ctx);
			ch3 = jscmp_getch(&ctx);

			if (ch1 < 0 || ch2 < 0 || ch3 < 0) {
				return JSCMP_E_READ;
			}

			if (ch1 == 'u' && ch2 == 'l' && ch3 == 'l'){

				if (dst_buf_pos + 1 > dst_buf_size) {
					return JSCMP_E_NOMEM;
				}

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
				ch = jscmp_getch(&ctx);
				if (ch < 0) {
					return JSCMP_E_READ;
				}
				if (ch >= '0' && ch <= '9') {
					ival = ival * 10 + (ch - '0');
				}
				else {
					jscmp_ungetch(&ctx, ch);
					break;
				}
			}

			if (sign) ival = -ival;

			if (ival >= 0 && ival <= 0x7F) {

				if (dst_buf_pos + 1 > dst_buf_size) {
					return JSCMP_E_NOMEM;
				}

				dst_buf[dst_buf_pos++] = ival;
			}
			else if (ival >= -32 && ival <= -1) {

				if (dst_buf_pos + 1 > dst_buf_size) {
					return JSCMP_E_NOMEM;
				}

				dst_buf[dst_buf_pos++] = ival;
			}
			else {
				if (-128 <= ival && ival <= 127) {

					if (dst_buf_pos + 2 > dst_buf_size) {
						return JSCMP_E_NOMEM;
					}

					dst_buf[dst_buf_pos++] = JSCMP_INT8;
					dst_buf[dst_buf_pos++] = ival;
				} else if (-32768 <= ival && ival <= 32767) {

					if (dst_buf_pos + 3 > dst_buf_size) {
						return JSCMP_E_NOMEM;
					}

					dst_buf[dst_buf_pos++] = JSCMP_INT16;
					dst_buf[dst_buf_pos++] = ival;
					dst_buf[dst_buf_pos++] = ival >> 8;
				} else {

					if (dst_buf_pos + 5 > dst_buf_size) {
						return JSCMP_E_NOMEM;
					}

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
			buf_idx = 0;
			buf_ovr = 0;

			for (;;) {

				ch = jscmp_getch(&ctx);
				if (ch < 0) {
					return JSCMP_E_READ;
				}
				
				if (ch == '"') {
					/* end of str */
					if (!buf_ovr) {
						int idx;
						buf[buf_idx] = '\0';
						idx = jscmp_findstr(strtbl, strtbl_len, buf);
						if (idx >= 0) {
							if (idx >= 256) {

								if (dst_buf_pos + 3 > dst_buf_size) {
									return JSCMP_E_NOMEM;
								}

								dst_buf[dst_buf_pos++] = JSCMP_PRE_STR16;
								dst_buf[dst_buf_pos++] = (uint8_t)idx;
								dst_buf[dst_buf_pos++] = (uint8_t)(idx >> 8);
							}
							else {

								if (dst_buf_pos + 3 > dst_buf_size) {
									return JSCMP_E_NOMEM;
								}

								dst_buf[dst_buf_pos++] = JSCMP_PRE_STR8;
								dst_buf[dst_buf_pos++] = (uint8_t)idx;
							}
						}
						else{
							if (buf_idx <= 15) {

								if (dst_buf_pos + 1 + buf_idx + 1 > dst_buf_size) {
									return JSCMP_E_NOMEM;
								}

								dst_buf[dst_buf_pos++] = JSCMP_FIXSTR_S + buf_idx;
							}
							else {

								if (dst_buf_pos + 2 + buf_idx + 1 > dst_buf_size) {
									return JSCMP_E_NOMEM;
								}

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

						if (dst_buf_pos + 1 > dst_buf_size) {
							return JSCMP_E_NOMEM;
						}

						dst_buf[dst_buf_pos++] = '\0';
					}

					if (container_node_stack_cnt > 0) {
						length_write_val[container_node_stack_cnt - 1]++;
					}

					break;
				} else {
					int cnt;
					int i;
					char s[3];

					if (ch == '\\') {
						ch = jscmp_getch(&ctx);
						if (ch < 0) {
							return JSCMP_E_READ;
						}

						if (ch == 'u') {
							int ch1, ch2, ch3, ch4;
							int d1, d2, d3, d4;
							int ucs;

							ch1 = jscmp_getch(&ctx);
							ch2 = jscmp_getch(&ctx);
							ch3 = jscmp_getch(&ctx);
							ch4 = jscmp_getch(&ctx);

							if (ch1 < 0 || ch2 < 0 || ch3 < 0 || ch4 < 0) {
								return JSCMP_E_READ;
							}

							d1 = jscmp_hex(ch1);
							d2 = jscmp_hex(ch2);
							d3 = jscmp_hex(ch3);
							d4 = jscmp_hex(ch4);

							if (d1 < 0 || d2 < 0 || d3 < 0 || d4 < 0) {
								return JSCMP_E_SYNTAX;
							}

							ucs = (d3 << 24) | (d4 << 16) | (d1 << 8) | d2;
							
							if (ucs <= 0x7F) {
								s[0] = (char)ucs;
								cnt = 1;
							}
							else if (ucs <= 0x07FF) {
								s[0] = (char)((ucs & 0x3F) | 0x80);
								s[1] = (char)((ucs >> 6) | 0xC0);
								cnt = 2;
							}
							else {
								s[0] = (char)((ucs & 0x3F) | 0x80);
								s[1] = (char)(((ucs >> 6) & 0x3F) | 0x80);
								s[2] = (char)(((ucs >> 12) & 0x0F) | 0xE0);
								cnt = 3;
							}
						}
						else {
							if (ch == 'b') {
								ch = '\b';
							}
							else if (ch == 'f') {
								ch = '\f';
							}
							else if (ch == 'n') {
								ch = '\n';
							}
							else if (ch == 'r') {
								ch = '\r';
							}
							else if (ch == 't') {
								ch = '\t';
							}

							s[0] = ch;
							cnt = 1;
						}
					}
					else {
						s[0] = ch;
						cnt = 1;
					}
					
					for (i = 0; i < cnt; i++) {
						ch = s[i];
						if (buf_ovr) {
							if (buf_idx > 0xFFFF) {
								return JSCMP_E_TOO_LONG_TEXT;
							}

							if (dst_buf_pos + 1 > dst_buf_size) {
								return JSCMP_E_NOMEM;
							}

							buf_idx++;
							dst_buf[dst_buf_pos++] = ch;
						}
						else if (buf_idx >= BUF_SIZE) {
							buf_ovr = 1;

							if (dst_buf_pos + 4 + buf_idx > dst_buf_size) {
								return JSCMP_E_NOMEM;
							}

							dst_buf[dst_buf_pos++] = JSCMP_STR16;
							str_len_write_pos = dst_buf_pos;
							dst_buf_pos += 2;
							memcpy(&dst_buf[dst_buf_pos], buf, buf_idx);
							dst_buf_pos += buf_idx;
							buf_idx++;
							dst_buf[dst_buf_pos++] = ch;
						}
						else {
							buf[buf_idx++] = ch;
						}
					}
				}
			}

		} else if (ch == '[') {
			/* array */
			if (dst_buf_pos + 5 > dst_buf_size) {
				return JSCMP_E_NOMEM;
			}

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
			if (dst_buf_pos + 5 > dst_buf_size) {
				return JSCMP_E_NOMEM;
			}

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

			if (container_size > 0xFFFF) {
				return JSCMP_E_TOO_LONG_TEXT;
			}
			
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
	return (jscmp_node_t)doc->dst_buf;
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
		return 3 + len + 1;
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
		return (const char *)&n[1];
	}
	if (type == JSCMP_PRE_STR8) {
		return doc->strtbl[n[1]];
	}
	else if (type == JSCMP_PRE_STR16) {
		int idx = (int)n[1] | ((int)n[2] << 8);
		return doc->strtbl[idx];
	}
	else if (type == JSCMP_STR8) {
		return (const char *)&n[2];
	}
	else if (type == JSCMP_STR16) {
		return (const char *)&n[3];
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

