#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

	/* eb == even byte
	 * ob == odd byte
	 */
	FILE *eb_in, *ob_in, *comb_out, *comb_patched_out, *eb_out, *ob_out, *update_in;
	char buf[256];
	char *cmd, *mem;
	uint32_t start, len;

	/* XXX: Terrible syntax checking */
	if (argc != 5) {
		printf("Usage: %s even_byte.bin odd_byte.bin update_in.prg output\n", argv[0]);
		return 1;
	}

	/* Attempt to open files */
	/* XXX: Absolutely no error checking */
	eb_in = fopen(argv[1], "r");
	ob_in = fopen(argv[2], "r");
	update_in = fopen(argv[3], "r");
	snprintf(buf, sizeof(buf), "%s-combined.bin", argv[4]);
	comb_out = fopen(buf, "w+");
	snprintf(buf, sizeof(buf), "%s-patched.bin", argv[4]);
	comb_patched_out = fopen(buf, "w+");
	snprintf(buf, sizeof(buf), "%s-patched.bin", argv[1]);
	eb_out = fopen(buf, "w+");
	snprintf(buf, sizeof(buf), "%s-patched.bin", argv[2]);
	ob_out = fopen(buf, "w+");

	/* Test to ensure that both input dump files are the same length */
	fseek(eb_in, 0, SEEK_END);
	fseek(ob_in, 0, SEEK_END);
	if (ftell(eb_in) != ftell(ob_in)) {
		printf("Err: Input files must be the same size\n");
		return 1;
	}
	rewind(eb_in);
	rewind(ob_in);

	/* Combine even and odd byte files in to a single proper file,
	 * comb_out. A copy is also written to comb_patched_out since the
	 * next step is to apply the update_in patch
	 */
	fread(&buf[0], 1, 1, eb_in);
	fread(&buf[1], 1, 1, ob_in);
	do {
		fwrite(&buf, 1, 2, comb_out);
		fwrite(&buf, 1, 2, comb_patched_out);
		fread(&buf[0], 1, 1, eb_in);
		fread(&buf[1], 1, 1, ob_in);
	} while (!feof(eb_in));

	/* Read instructions from manufacture firmware update file.
	 * Specifically looking for DL commands which have the format:
	 * DL<start>,<length>\r\n
	 * Where start and length are hex values with no preceeding 0x
	 * There can be multiple DL commands in a file.
	 *
	 * A naive parser is used here
	 *
	 * Error checking here only looks to make sure that DL exists,
	 * followed by some attempt of converting a string to an int, a comma,
	 * and then another attmpt to convert a string to an int.
	 *
	 * fgets() is used to get whole lines. This has the side effect of
	 * positioning the stream at the first byte after \r\n of the DL
	 * command. Meaning once the DL command has been parsed, the next
	 * read is firmware data from the update file.
	 *
	 * The inner while() is used as an if() with the ability to bail out of
	 * it with a break if the parser doesn't see the comma where it expects
	 * one.
	 */
	rewind(update_in);
	while (!feof(update_in)) {
		fgets(buf, sizeof(buf), update_in);
		if (feof(update_in)) break;
		cmd = strstr(buf, "DL");
		while (cmd != NULL) {
			/* Found something matching, still need to parse */
			/* Jump to where we expect the number to start */
			cmd += 2;
			start = strtol(cmd, NULL, 16);
			while (*cmd >= '0' && *cmd <= '9') cmd++;
			if (*cmd != ',') break;
			cmd++;
			len = strtol(cmd, NULL, 16);
			printf("Patching combined file starting at 0x%X, len 0x%X\n", start, len);
			fseek(comb_patched_out, start, SEEK_SET);
			mem = malloc(len);
			fread(mem, 1, len, update_in);
			fwrite(mem, 1, len, comb_patched_out);
			break;
		}
	}

	/* Write out split patched files */
	rewind(comb_patched_out);
	fread(&buf, 1, 2, comb_patched_out);
	do {
		fwrite(&buf[0], 1, 1, eb_out);
		fwrite(&buf[1], 1, 1, ob_out);
		fread(&buf, 1, 2, comb_patched_out);
	} while (!feof(comb_patched_out));

}
