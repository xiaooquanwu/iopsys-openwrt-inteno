#ifndef __mtd_h
#define __mtd_h

#include <stdbool.h>

#define JFFS2_EOF "\xde\xad\xc0\xde"

extern int quiet;
extern int mtd_replace_jffs2(const char *mtd, int fd, int ofs, const char *filename, int nand_erasesize);

#endif /* __mtd_h */
