#ifndef RWCLOCK_BUFREF_JSON_HPP_
#define RWCLOCK_BUFREF_JSON_HPP_

#include <ArduinoJson.h>

class BufRefJsonDocument : public ArduinoJson::JsonDocument {
  public:
    BufRefJsonDocument(char* buf, size_t capacity) : JsonDocument(buf, capacity) {}

    BufRefJsonDocument() = delete;
    BufRefJsonDocument(const BufRefJsonDocument&) = delete;
    BufRefJsonDocument(BufRefJsonDocument&&) = delete;
    BufRefJsonDocument& operator=(const BufRefJsonDocument&) = delete;
    BufRefJsonDocument& operator=(BufRefJsonDocument&&) = delete;
};

#endif  // RWCLOCK_BUFREF_JSON_HPP_
