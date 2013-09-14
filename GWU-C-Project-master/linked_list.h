#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct request_node {
  int file_descriptor;
  struct request_node *next;
} request_node;

// initializers
void request_list_init();
request_node* init_node(int fd);
request_node* init_null_node();

// poppers and pushers
request_node* get_request();
void put_request(request_node *r);

// global (ick) request variable
extern request_node *requests;
#endif
