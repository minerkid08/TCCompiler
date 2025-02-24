#pragma once

typedef struct
{
  char* data;
  int len;
  int maxLen;
} Buffer;

void bufferInit(Buffer* buf);
void bufferWrite(Buffer* buf, const char* fmt, ...);
