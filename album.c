#include "album.h"

struct album *make_album()
{
    struct album *album = calloc(1, sizeof(struct album));
    album->data = NULL;
    album->next = NULL;
    return album;
}

struct album *add_to_album(struct album *data, struct tags *new_mp3)
{
    struct album *head = data;
    struct album *new_data = make_album();
    strcpy(new_data->name, data->name);
    new_data->data = new_mp3;
    if (data->data == NULL)
    {
        data = new_data;
        return data;
    }
    else
    {
        while (data->next != NULL)
        {
            data = data->next;
        }
        data->next = new_data;
        return head;
    }
}

/*void save_album(struct album *data, char *filename){
  int i = open(filename, O_TRUNC | O_WRONLY | O_CREAT | O_APPEND, 0644);
  while(data -> next != NULL){
    write(i, data -> data, sizeof(struct tags));
    data = data -> next;
  }
  write(i, data -> data, sizeof(struct tags));
  close(i);
}*/

struct album *get_nth_track(struct album *data, int n)
{
    int i = 0;
    while (data)
    {
        if (i == n)
            return data;

        ++i;
        data = data->next;
    }
}

int get_album_len(struct album *data)
{
    int i = 0;

    while (data)
    {
        ++i;
        data = data->next;
    }

    return i;
}

struct album *sorted_insert_name_asc(struct album *head, struct album *node)
{
    if (head == NULL || strcasecmp(head->data->title, node->data->title) > 0)
    {
        node->next = head;
        return node;
    }
    else
    {
        struct album *current = head;
        while (current->next != NULL && strcasecmp(current->next->data->title, node->data->title) < 0)
            current = current->next;
        node->next = current->next;
        current->next = node;
    }

    return head;
}

struct album *sorted_insert_name_des(struct album *head, struct album *node)
{
    if (head == NULL || strcasecmp(head->data->title, node->data->title) < 0)
    {
        node->next = head;
        return node;
    }
    else
    {
        struct album *current = head;
        while (current->next != NULL && strcasecmp(current->next->data->title, node->data->title) > 0)
            current = current->next;
        node->next = current->next;
        current->next = node;
    }

    return head;
}

struct album *sorted_insert_artist_asc(struct album *head, struct album *node)
{
    if (head == NULL || strcasecmp(head->data->artist, node->data->artist) > 0)
    {
        node->next = head;
        return node;
    }
    else
    {
        struct album *current = head;
        while (current->next != NULL && strcasecmp(current->next->data->artist, node->data->artist) < 0)
            current = current->next;
        node->next = current->next;
        current->next = node;
    }

    return head;
}

struct album *sorted_insert_artist_des(struct album *head, struct album *node)
{
    if (head == NULL || strcasecmp(head->data->artist, node->data->artist) < 0)
    {
        node->next = head;
        return node;
    }
    else
    {
        struct album *current = head;
        while (current->next != NULL && strcasecmp(current->next->data->artist, node->data->artist) > 0)
            current = current->next;
        node->next = current->next;
        current->next = node;
    }

    return head;
}

struct album *sorted_insert_duration_asc(struct album *head, struct album *node)
{
    if (head == NULL || head->data->duration > node->data->duration)
    {
        node->next = head;
        return node;
    }
    else
    {
        struct album *current = head;
        while (current->next != NULL && current->next->data->artist < node->data->artist)
            current = current->next;
        node->next = current->next;
        current->next = node;
    }

    return head;
}

struct album *sorted_insert_duration_des(struct album *head, struct album *node)
{
    if (head == NULL || head->data->artist < node->data->artist)
    {
        node->next = head;
        return node;
    }
    else
    {
        struct album *current = head;
        while (current->next != NULL && current->next->data->artist > node->data->artist)
            current = current->next;
        node->next = current->next;
        current->next = node;
    }

    return head;
}