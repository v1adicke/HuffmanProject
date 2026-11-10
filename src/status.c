#include "status.h"


const char* status_message(HufStatus status)
{
    switch (status)
    {
        case HUF_OK:
            return "success";
        case HUF_ERR_ARGS:
            return "invalid arguments";
        case HUF_ERR_NO_MEMORY:
            return "out of memory";
        case HUF_ERR_OPEN_INPUT:
            return "cannot open input file";
        case HUF_ERR_OPEN_OUTPUT:
            return "cannot open output file";
        case HUF_ERR_READ:
            return "cannot read input file";
        case HUF_ERR_WRITE:
            return "cannot write output file";
        case HUF_ERR_BAD_ARCHIVE:
            return "input is not a valid archive";
        default:
            return "unknown error";
    }
}
