#include "contact.h"

//hexdump -C contacts.dat

int main(void)
{
    printf("=== Contact Management System ===\n");
    
    if (load_contacts())
        printf("Loaded %d contacts from file.\n", contact_count);
    else
        printf("No existing contacts found. Starting with empty list.\n");
    
    int running = 1;
    while (running)
    {
        print_menu();
        
        char choice[10];
        if (fgets(choice, sizeof(choice), stdin) == NULL)
            break;
        
        switch (choice[0])
        {
            case '1':
                add_contact();
                break;
                
            case '2':
                list_contacts();
                break;
                
            case '3':
                search_contact();
                break;
                
            case '4':
                delete_contact();
                break;
                
            case '5':
                running = 0;
                break;
                
            default:
                printf("Invalid option! Please choose 1-5.\n");
        }
    }
    
    if (save_contacts())
        printf("Contacts saved successfully. Goodbye!\n");
    else
        printf("Error saving contacts! Data may be lost.\n");
    
    return 0;
}
