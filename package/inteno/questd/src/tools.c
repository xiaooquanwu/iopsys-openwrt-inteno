#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "questd.h"

void 
remove_newline(char *buf)
{
	int len;
	len = strlen(buf) - 1;
	if (buf[len] == '\n') 
		buf[len] = 0;
}

void
replace_char(char *buf, char a, char b)
{
	int i = 0;

	while (buf[i]) {
		if (buf[i] == a)
			buf[i] = b;
		i++;
	}
	buf[i] = '\0';
}

void
runCmd(const char *pFmt, ...)
{
	va_list ap;
	char cmd[256] = {0};
	int len=0, maxLen;

	maxLen = sizeof(cmd);

	va_start(ap, pFmt);

	if (len < maxLen)
	{
		maxLen -= len;
		vsnprintf(&cmd[len], maxLen, pFmt, ap);
	}

	system(cmd);

	va_end(ap);
}

const char*
chrCmd(const char *pFmt, ...)
{
	va_list ap;
	char cmd[256] = {0};
	int len=0, maxLen;

	maxLen = sizeof(cmd);

	va_start(ap, pFmt);

	if (len < maxLen)
	{
		maxLen -= len;
		vsnprintf(&cmd[len], maxLen, pFmt, ap);
	}

	va_end(ap);

	FILE *pipe;
	char buffer[128];
	if ((pipe = popen(cmd, "r")))
		fgets(buffer, sizeof(buffer), pipe);
	pclose(pipe);

	if (strlen(buffer))
		return strdup(buffer);
	else
		return NULL;
}
