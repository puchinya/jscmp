
#include "fstream.h"
#include "jscmp.h"

char g_out_buf[16 * 1024];

const char *g_strtbl[] = {
	"Pow"
};

int main(void)
{
	ifstream_t ifs;

	ifstream_init(&ifs);
	ifstream_open(&ifs, "test.json");

	jscmp_parse((istream_t *)&ifs, g_out_buf, 16 * 1024, g_strtbl, 1);

	ifstream_close(&ifs);

	return 0;
}
