#include "contact.h"

struct contact contacts[MAX_CONTACTS];
int contact_count = 0;
int next_id = 1;

void init_contact(struct contact *contact)
{
    memset(contact, 0, sizeof(struct contact));
}

void safe_string_input(char *buffer, int size, const char *prompt)
{
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

void clear_input_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int load_contacts(void)
{
    int fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    // O_WRONLY - только запись
    // O_CREAT - создать если нет
    // O_TRUNC - очистить файл если существует
    
    if (fd == -1)
    {
        return 0;
    }
    
    struct contact c;
    ssize_t bytes_read;
    
    while ((bytes_read = read(fd, &c, sizeof(struct contact))) > 0)
    {
        if (bytes_read == sizeof(struct contact) && contact_count < MAX_CONTACTS)
        {
            contacts[contact_count] = c;
            contact_count++;
            
            if (c.id >= next_id)
            {
                next_id = c.id + 1;
            }
        }
    }
    
    close(fd);
    return 1;
}

int save_contacts(void)
{
    int fd = open(FILENAME, O_CREAT | O_RDWR, 0666);
    // O_CREAT - cоздать, если не существует
    // O_RDWR - запись и чтение
    if (fd == -1)
    {
        perror("Error opening file for writing");
        return 0;
    }
    
    for (int i = 0; i < contact_count; i++)
    {
        ssize_t bytes_written = write(fd, &contacts[i], sizeof(struct contact));
        if (bytes_written != sizeof(struct contact))
        {
            perror("Error writing contact");
            close(fd);
            return 0;
        }
    }
    
    close(fd);
    return 1;
}

void add_contact(void)
{
    if (contact_count >= MAX_CONTACTS)
    {
        printf("Contact list is full!\n");
        return;
    }
    
    struct contact new_contact;
    init_contact(&new_contact);
    new_contact.id = next_id;
    next_id++;
    
    safe_string_input(new_contact.name, NAME_LENGTH, "Enter name: ");
    safe_string_input(new_contact.fullname, NAME_LENGTH, "Enter full name: ");
    safe_string_input(new_contact.phone, PHONE_LENGTH, "Enter phone: ");
    safe_string_input(new_contact.email, EMAIL_LENGTH, "Enter email: ");
    
    contacts[contact_count] = new_contact;
    contact_count++;
    
    printf("Contact added successfully! ID: %d\n", new_contact.id);
}

void list_contacts(void)
{
    if (contact_count == 0)
    {
        printf("No contacts found.\n");
        return;
    }
    
    printf("\n=== Contact List (%d contacts) ===\n", contact_count);
    for (int i = 0; i < contact_count; i++)
    {
        printf("ID: %d\n", contacts[i].id);
        printf("Name: %s\n", contacts[i].name);
        printf("Full Name: %s\n", contacts[i].fullname);
        printf("Phone: %s\n", contacts[i].phone);
        printf("Email: %s\n", contacts[i].email);
        printf("-------------------\n");
    }
}

void search_contact(void)
{
    char search_term[NAME_LENGTH];
    safe_string_input(search_term, NAME_LENGTH, "Enter name to search: ");
    
    int found = 0;
    printf("\n=== Search Results ===\n");
    
    for (int i = 0; i < contact_count; i++)
    {
        if (strstr(contacts[i].name, search_term) != NULL || 
            strstr(contacts[i].fullname, search_term) != NULL)
        {
            printf("ID: %d\n", contacts[i].id);
            printf("Name: %s\n", contacts[i].name);
            printf("Full Name: %s\n", contacts[i].fullname);
            printf("Phone: %s\n", contacts[i].phone);
            printf("Email: %s\n", contacts[i].email);
            printf("-------------------\n");
            found = 1;
        }
    }
    
    if (!found)
    {
        printf("No contacts found with: %s\n", search_term);
    }
}

void delete_contact(void)
{
    if (contact_count == 0)
    {
        printf("No contacts to delete.\n");
        return;
    }
    
    int id;
    printf("Enter contact ID to delete: ");
    
    if (scanf("%d", &id) != 1)
    {
        printf("Invalid input!\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();
    
    int found_index = -1;
    for (int i = 0; i < contact_count; i++)
    {
        if (contacts[i].id == id)
        {
            found_index = i;
            break;
        }
    }
    
    if (found_index == -1)
    {
        printf("Contact with ID %d not found.\n", id);
        return;
    }
    
    printf("Delete contact '%s' (ID: %d)? (y/n): ", 
           contacts[found_index].name, contacts[found_index].id);
    
    char confirm;
    scanf("%c", &confirm);
    clear_input_buffer();
    
    if (confirm != 'y' && confirm != 'Y')
    {
        printf("Deletion cancelled.\n");
        return;
    }
    
    for (int i = found_index; i < contact_count - 1; i++)
    {
        contacts[i] = contacts[i + 1];
    }
    
    contact_count--;
    printf("Contact deleted successfully.\n");
}

void print_menu(void)
{
    printf("\n=== Contact Book ===\n");
    printf("1. Add contact\n");
    printf("2. List contacts\n");
    printf("3. Search contact\n");
    printf("4. Delete contact\n");
    printf("5. Exit\n");
    printf("Choose option: ");
}
