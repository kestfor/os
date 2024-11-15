#ifndef PROXY_CHANNEL_H
#define PROXY_CHANNEL_H

#include <unistd.h>
#include <stdbool.h>


typedef struct channel channel;
channel *new_channel();
size_t get_size(channel *ch);
void empty_channel(channel *ch);
void clear_channel(channel *ch);
void set_whole(channel *ch);
int add_to_channel(channel *ch, const char *data, size_t size);
int write_to_channel(channel *ch, const char *data, size_t size);
bool is_whole(channel *ch);
bool read_available(channel *ch, char *dest, int offset, int size, int *actual_read_num);

#endif //PROXY_CHANNEL_H
