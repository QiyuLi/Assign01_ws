#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(__APPLE__)
  #include <malloc.h>
#endif
#include "linked_list.h"
#include "cas.h"

request_node *requests;

// pops the top node off the stack and returns it
request_node* get_request()
{
  request_node *old;
  request_node *new;


  do {
    while (requests->file_descriptor == NULL) {}
    old = requests;
    if (requests->next == NULL) {
      new = init_null_node();
    } else {
      new = requests->next;
    }
  } while (__cas((void *)&requests, old, new));
  return old;
}

request_node* init_node(int fd)
{
  request_node *new_node;
  new_node = init_null_node();
  new_node->file_descriptor = fd;
  return new_node;
}

request_node* init_null_node()
{
  request_node *new_node = (request_node *)malloc(sizeof(request_node));
  new_node->file_descriptor = NULL;
  new_node->next = NULL;
  return new_node;
}

void request_list_init()
{
  requests = (request_node *)malloc(sizeof(request_node));
  requests->next = NULL;
  requests->file_descriptor = NULL;
}

void put_request(request_node *r)
{
   request_node *old;
   do {
      old = requests;
      r->next = requests;
  } while (__cas((void *)&requests, old, r));
  return;
}
