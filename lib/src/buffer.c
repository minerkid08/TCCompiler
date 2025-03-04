#include "buffer.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void bufferInit(Buffer* buf)
{
	buf->data = malloc(256);
	assert(buf->data);
	buf->len = 0;
	buf->maxLen = 256;
}

void bufferFree(Buffer* buf)
{
	free(buf->data);
	buf->data = 0;
}

void bufferWrite(Buffer* buf, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int newLen = buf->len + vsnprintf(0, 0, fmt, va);
	if (newLen >= buf->maxLen)
	{
		buf->maxLen += 256;
		buf->data = realloc(buf->data, buf->maxLen);
		assert(buf->data);
	}
	va_start(va, fmt);
	buf->len += vsnprintf(buf->data + buf->len, buf->maxLen - buf->len, fmt, va);
}
