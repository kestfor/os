#ifndef PROXY_CHANNEL_H
#define PROXY_CHANNEL_H

#include <unistd.h>
#include <stdbool.h>


typedef struct channel channel;
channel *channel_create();
size_t channel_get_size(channel *ch);
void channel_set_empty(channel *ch);
void channel_clear(channel *ch);
void channel_set_whole(channel *ch);
int channel_add(channel *ch, const char *data, size_t size);
int channel_write(channel *ch, const char *data, size_t size);
bool channel_is_whole(channel *ch);
bool channel_read_available(channel *ch, char *dest, int offset, int size, int *actual_read_num);
void channel_wait_for_data(channel *ch);

#endif //PROXY_CHANNEL_H
