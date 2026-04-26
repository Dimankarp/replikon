#include "logging.h"
#include "utils.h"
#include <cstdio>

namespace replikon {

namespace {
FILE *LOG_OUT = stderr;
}
void SetLogFile(FILE *file) { LOG_OUT = file; }
FILE *GetLogFile() { return LOG_OUT; }

} // namespace replikon