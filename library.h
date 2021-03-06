#include "album.h"

struct library
{
  struct album *album;
  struct library *next;
};

struct library *make_library();
struct library *add_to_library(struct library *data, char *filename);
struct library *remove_from_library(struct library *data, char *name);
struct library *save_library(struct library *data);
struct library *read_library();
void print_library(struct library *data);
int get_library_len(struct library *data);
struct library *get_nth_album(struct library *data, int n);
struct library *sort(struct library *data, int direction);
struct library *sortedInsertAsc(struct library *head, struct library *node);
struct library *sortedInsertDes(struct library *head, struct library *node);
struct library *sort_album(struct library *data, int direction, int type);
