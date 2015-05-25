extern "C"
{
	#include <string.h>
	#include <stdarg.h>
	#include <stdlib.h>

	#include "common.h"
}

#include "wifi.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>

extern "C"
{
	void
	removeNewline(char *buf)
	{
		int len;
		len = strlen(buf) - 1;
		if (buf[len] == '\n')
			buf[len] = 0;
	}

	void
	dotToUnderscore(char *buf)
	{
		int i = 0;

		while (buf[i]) {
			if (buf[i] == '.')
				buf[i] = '_';
			i++;
		}
		buf[i] = '\0';
	}
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

string
strCmd(const char *pFmt, ...)
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

	FILE* pipe = popen(cmd, "r");
	string result = "";
	char buffer[128];
	if (pipe) {
		while(!feof(pipe)) {
			if(fgets(buffer, 128, pipe) != NULL)
				result += buffer;
		}
	}
	pclose(pipe);
	result.erase(result.find_last_not_of("\n")+1);
	return result;
}

string
readFile(char *path)
{
	ifstream file(path);
	string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return str;
}

string
toStr(int num)
{
	ostringstream result;
	result << num;
	return result.str();
}

static struct uci_context *uci_ctx;
static struct uci_ptr ptr;

struct uci_package *
init_package(const char *config)
{
	struct uci_context *ctx = uci_ctx;
	struct uci_package *p = NULL;

	if (!ctx) {
		ctx = uci_alloc_context();
		uci_ctx = ctx;
	} else {
		p = uci_lookup_package(ctx, config);
		if (p)
			uci_unload(ctx, p);
	}

	if (uci_load(ctx, config, &p))
		return NULL;

	return p;
}

static inline int
uciGetPtr(const char *p, const char *s, const char *o, const char *t)
{
	memset(&ptr, 0, sizeof(ptr));
	ptr.package = p;
	ptr.section = s;
	ptr.option = o;
	ptr.value = t;
	return uci_lookup_ptr(uci_ctx, &ptr, NULL, true);
}

const char*
uciGet(const char *p, const char *s, const char *o)
{
	const char *value = NULL;

	if(uciGetPtr(p, s, o, NULL))
		return NULL;

	if(ptr.o->type == UCI_TYPE_STRING)
		value = ptr.o->v.string;

	return value;
}

void
uciSet(const char *p, const char *s, const char *o, const char *t)
{
	uciGetPtr(p, s, o, (t)?(t):(""));
	uci_set(uci_ctx, &ptr);
}

void
uciCommit(const char *p)
{
	if(uciGetPtr(p, NULL, NULL, NULL))
		return;
	uci_commit(uci_ctx, &ptr.p, false);
}

const char *
ugets(struct uci_section *s, const char *opt)
{
	const char *value = NULL;
	value = uci_lookup_option_string(uci_ctx, s, opt);
	return value;
}

int
ugeti(struct uci_section *s, const char *opt)
{
	const char *value = NULL;
	int ret = 0;
	if ((value = uci_lookup_option_string(uci_ctx, s, opt))) ret = atoi(value);
	return ret;
}

void
uset(struct uci_section *s, const char *opt, const char *value)
{
	int found = string(s->package->path).find_last_of('/');
	const char *path = string(s->package->path).substr(found+1).c_str();
	uciGetPtr(path, s->e.name, opt, (value)?(value):(""));
	uci_set(uci_ctx, &ptr);
}

void
ucommit(struct uci_section *s)
{
	int found = string(s->package->path).find_last_of('/');
	const char *path = string(s->package->path).substr(found+1).c_str();
	if(uciGetPtr(path, NULL, NULL, NULL))
		return;
	uci_commit(uci_ctx, &ptr.p, false);
}

