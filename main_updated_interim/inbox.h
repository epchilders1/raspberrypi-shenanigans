#ifndef INBOX_H
#define INBOX_H

#include <Arduino.h>
#include "config.h"

#define MAX_INBOX_MESSAGES 20

struct InboxMessage {
  int    id;
  String name;
  String text;
  String timeStr;
  bool   viewed;
};

extern InboxMessage inbox[MAX_INBOX_MESSAGES];
extern int  inboxCount;
extern int  nextMessageId;
extern int  unviewedCount;

void drawInboxPixels();
void clearInboxPixels();
void addInboxMessage(const String& name, const String& text, const String& timeStr);
bool viewMessage(int id);

#endif