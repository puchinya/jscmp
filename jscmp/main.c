
#include "fstream.h"
#include "jscmp_dump.h"

char g_out_buf[16 * 1024];

const char *g_strtbl[] = {
	"servlet-name",
	"servlet-class",
	"init-param"
};

int main(int argc, char **argv)
{
	ifstream_t ifs;
	jscmp_doc_t doc;
	const char *fn = "test.json";

	if (argc >= 2) {
		fn = argv[1];
	}

	ifstream_init(&ifs);
	ifstream_open(&ifs, fn);

	printf("%s:%dbyte\n", fn, ifstream_length(&ifs));

	jscmp_parse(&doc, (istream_t *)&ifs, g_out_buf, 16 * 1024, g_strtbl, sizeof(g_strtbl)/sizeof(g_strtbl[0]));

	ifstream_close(&ifs);

	printf("used:%dbyte\n", doc.dst_buf_pos);

	jscmp_dump(&doc);

	return 0;
}
