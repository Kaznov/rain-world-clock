#ifndef RWCLOCK_CONNECTION_HPP_
#define RWCLOCK_CONNECTION_HPP_

#include <optional>

void connectToWiFi();
void configNTP();
void updateLocalDataFromServer();
bool waitForNTPUpdate(unsigned long timeout_ms = 5000);

#endif  // RWCLOCK_CONNECTION_HPP_
