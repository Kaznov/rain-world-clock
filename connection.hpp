#ifndef RWCLOCK_CONNECTION_HPP_
#define RWCLOCK_CONNECTION_HPP_

#include <optional>

void connectToWiFi();
void configNTP();
void updateLocalDataFromServer();

#endif  // RWCLOCK_CONNECTION_HPP_
