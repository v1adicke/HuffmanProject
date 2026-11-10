#ifndef STATUS_H
#define STATUS_H


/**
 * Result codes returned by functions that can fail.
 */
typedef enum
{
    HUF_OK = 0,
    HUF_ERR_ARGS,
    HUF_ERR_NO_MEMORY,
    HUF_ERR_OPEN_INPUT,
    HUF_ERR_OPEN_OUTPUT,
    HUF_ERR_READ,
    HUF_ERR_WRITE,
    HUF_ERR_BAD_ARCHIVE
} HufStatus;

/**
 * Returns a human-readable description of a status code.
 * @param status Status code.
 * @return Static string, must not be freed.
 */
const char* status_message(HufStatus status);

#endif // STATUS_H
