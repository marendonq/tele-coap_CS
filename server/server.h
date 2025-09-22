#ifndef SERVER_H
#define SERVER_H

#include "logger.h"
#include "coap_parser.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

void start_server(int port, Logger &logger);

#endif
