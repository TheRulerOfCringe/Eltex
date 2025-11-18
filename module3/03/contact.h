#ifndef CONTACT_H
#define CONTACT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define NAME_LENGTH 50
#define PHONE_LENGTH 20
#define EMAIL_LENGTH 50
#define FILENAME "contacts.dat"
#define MAX_CONTACTS 100

struct contact
{
    char name[NAME_LENGTH];
    char fullname[NAME_LENGTH];
    char phone[PHONE_LENGTH];
    char email[EMAIL_LENGTH];
    int id;
};

extern struct contact contacts[MAX_CONTACTS];
extern int contact_count;
extern int next_id;

int load_contacts(void);
int save_contacts(void);
void add_contact(void);
void list_contacts(void);
void search_contact(void);
void delete_contact(void);
void print_menu(void);
void init_contact(struct contact *contact);
void safe_string_input(char *buffer, int size, const char *prompt);
void clear_input_buffer(void);

#endif
