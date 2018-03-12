
#include "jscmp_dump.h"
#include <string.h>
#include <stdio.h>

static void jscmp_dump_spaces(int cnt)
{
	int i;
	for (i = 0; i < cnt; i++) printf("  ");
}

static void jscmp_dump_node(jscmp_doc_t *doc, jscmp_node_t node, int first_spaces, int nest)
{
	int i;
	jscmp_node_t cn;
	jscmp_node_t end;

	if (first_spaces) jscmp_dump_spaces(nest);

	switch (jscmp_type(node)) {
	case JSCMP_TYPE_STR:
		printf("\"%s\"", jscmp_str_val(doc, node));
		break;
	case JSCMP_TYPE_INT:
		printf("%d", jscmp_int_val(node));
		break;
	case JSCMP_TYPE_NULL:
		printf("null");
		break;
	case JSCMP_TYPE_BOOL:
		printf("%s", jscmp_bool_val(node) ? "true" : "false");
		break;
	case JSCMP_TYPE_ARRAY:
		printf("[\n");
		end = jscmp_array_end(node);
		for (i = 0, cn = jscmp_array_begin(node); cn < end; cn = jscmp_array_next(cn))
		{
			if (i > 0) printf(",\n");
			jscmp_dump_node(doc, cn, 1, nest + 1);
			i++;
		}
		printf("\n");
		jscmp_dump_spaces(nest);
		printf("]");
		break;
	case JSCMP_TYPE_OBJECT:
		printf("{\n");
		end = jscmp_object_end(node);
		for (i = 0, cn = jscmp_object_begin(node); cn < end; cn = jscmp_object_next(cn))
		{
			if (i > 0) printf(",\n");

			jscmp_dump_spaces(nest + 1);
			printf("\"%s\":", jscmp_str_val(doc, cn));

			jscmp_dump_node(doc, jscmp_object_val(cn), 0, nest + 1);
			i++;
		}
		printf("\n");
		jscmp_dump_spaces(nest);
		printf("}");
		break;
	}
}

void jscmp_dump(jscmp_doc_t *doc)
{
	jscmp_dump_node(doc, jscmp_root(doc), 1, 0);
	printf("\n");
}
