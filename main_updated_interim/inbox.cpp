#include "inbox.h"

InboxMessage inbox[MAX_INBOX_MESSAGES];
int  inboxCount    = 0;
int  nextMessageId = 1;
int  unviewedCount = 0;

void drawInboxPixels() {
  for (int i = 0; i < MAX_INBOX_MESSAGES; i++) {
    bool lit = (i < unviewedCount);
    display->drawPixel(i, 31, lit ? display->color565(255, 255, 255) : 0);
  }
}

void clearInboxPixels() {
  for (int i = 0; i < MAX_INBOX_MESSAGES; i++) {
    display->drawPixel(i, 31, 0);
  }
}

void addInboxMessage(const String& name, const String& text, const String& timeStr) {
  if (inboxCount >= MAX_INBOX_MESSAGES) {
    for (int i = 0; i < MAX_INBOX_MESSAGES - 1; i++) inbox[i] = inbox[i + 1];
    inboxCount = MAX_INBOX_MESSAGES - 1;
  }
  InboxMessage m;
  m.id      = nextMessageId++;
  m.name    = name;
  m.text    = text;
  m.timeStr = timeStr;
  m.viewed  = false;

  inbox[inboxCount++] = m;
  unviewedCount++;
  Serial.printf("Inbox ← [%d] %s: %s\n", m.id, name.c_str(), text.c_str());
  drawInboxPixels();
}

bool viewMessage(int id) {
  for (int i = 0; i < inboxCount; i++) {
    if (inbox[i].id == id && !inbox[i].viewed) {
      inbox[i].viewed = true;
      if (unviewedCount > 0) unviewedCount--;
      drawInboxPixels();
      return true;
    }
  }
  return false;
}