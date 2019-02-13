#include <stdio.h>
#include "../utils/crc32.h"
#include <string.h>

int main()
{
	auto h1 = CCRC32("Lorem ipsum");
	auto h2 = "Lorem ipsum"_crc32;

	if (h1 ^ h2)
		return 1;

	if (h1 ^ 0xF44BFB59)
		return 2;

	auto h3 = "Lorem ipsum dolor sit amet, modo probo patrioque eos ne, no porro admodum aliquando pro. Posse pertinax erroribus sed at, sed apeirian ocurreret intellegebat ne, te qui facete quaeque dolorum. Quem dolor sed at, usu nonumes facilisi ne. Postea vocibus luptatum id sed."_crc32;

	if (0xFF9776B0 ^ h3)
		return 3;

	const char* lipsum_text = "Lorem ipsum dolor sit amet, modo probo patrioque eos ne, no porro admodum aliquando pro. Posse pertinax erroribus sed at, sed apeirian ocurreret intellegebat ne, te qui facete quaeque dolorum. Quem dolor sed at, usu nonumes facilisi ne. Postea vocibus luptatum id sed.";

	auto h4 = Crc32(lipsum_text, strlen(lipsum_text));
	auto h5 = Crc32(lipsum_text);

	if (0xFF9776B0 ^ h4)
		return 4;

	if (h4 ^ h5)
		return 5;

	return 0;
}
