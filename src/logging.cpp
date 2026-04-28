#include "logging.h"
#include "utils.h"
#include <cstdio>

namespace replikon {

namespace {
FILE *log_out = stderr;
}
void setLogFile(FILE *file) { log_out = file; }
FILE *getLogFile() { return log_out; }

} // namespace replikon