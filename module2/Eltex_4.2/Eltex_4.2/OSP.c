#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define MAX_CHAR 20

typedef struct queue
{
	struct queue* next;
	char text[MAX_CHAR];
	uint8_t priority; // „тобы ровно 0-255 как в задании
} queue;

bool InitQueue(queue* q, uint8_t priority, char* text)
{
	q->next = NULL;
	q->priority = priority;
	strcpy_s(q->text, MAX_CHAR, text);
	return true;
}

bool Add(queue** head, queue* q)
{
	if (*head == NULL)
	{
		*head = q;
		return true;
	}
	else
	{
		queue* tmp = *head;
		queue* prev = NULL;
		while (tmp->next && tmp->priority >= q->priority)
		{
			prev = tmp;
			tmp = tmp->next;
		}

		if (prev == NULL)
		{
			if (tmp->priority < q->priority)
			{
				q->next = tmp;
				*head = q;
			}
			else
				tmp->next = q;

		}
		else if (tmp->priority < q->priority)
		{
			q->next = tmp;
			prev->next = q;
		}
		else
		{
			q->next = tmp->next;
			tmp->next = q;
		}
	}


	return true;
}

void PrintQueue(queue* head)
{
	if (head == NULL) {
		printf("Queue is empty\n");
		return;
	}

	while (head != NULL)
	{
		printf("P = %u; text = %s\n", head->priority, head->text);
		head = head->next;
	}
}

char* Pop1st(queue** head)
{
	if (*head == NULL)
		return NULL;

	char* text_to_return = malloc(MAX_CHAR);
	if (text_to_return == NULL)
		return NULL;

	strcpy_s(text_to_return, MAX_CHAR, (*head)->text);

	queue* old_head = *head;
	*head = (*head)->next;
	old_head->next = NULL;

	return text_to_return; // ¬ызывающа€ сторона должна вызвать free()
}

char* PopEqualPriority(queue** head, uint8_t priority)
{
	if (*head == NULL)
		return NULL;

	queue* current = *head;
	queue* prev = NULL;

	while (current != NULL)
	{
		if (current->priority == priority)
		{
			queue* to_delete = current;
			char* found_text = (char*)malloc(MAX_CHAR * sizeof(char)); // выдел€ем пам€ть дл€ текста

			if (found_text != NULL)
			{
				strcpy_s(found_text, MAX_CHAR, current->text); // копируем текст
			}

			if (prev == NULL)
			{
				*head = current->next;
			}
			else
			{
				prev->next = current->next;
			}

			free(to_delete); // освобождаем узел очереди
			return found_text; // возвращаем найденный текст
		}
		else
		{
			prev = current;
			current = current->next;
		}
	}

	return NULL; // не найден
}

bool PopNotLessPriority(queue** head, uint8_t priority)
{
	if (*head == NULL)
		return false;

	queue* current = *head;
	queue* prev = NULL;
	bool found = false;

	while (*head != NULL && (*head)->priority >= priority)
	{
		found = true;
		*head = (*head)->next;
	}

	if (*head == NULL)
		return found;

	current = *head;
	prev = *head;

	while (current != NULL)
	{
		if (current->priority >= priority)
		{
			found = true;
			prev->next = current->next;
			current = current->next;
		}
		else
		{
			prev = current;
			current = current->next;
		}
	}

	return found;
}

char* PopEqualOrHigherPriority(queue** head, uint8_t priority)
{
	if (*head == NULL)
		return NULL;

	queue* current = *head;
	queue* prev = NULL;
	queue* prevprev = NULL;

	while (current != NULL)
	{
		if (current->priority == priority)
		{
			// Ќашли точное совпадение - удал€ем и возвращаем
			char* found_text = (char*)malloc(MAX_CHAR * sizeof(char));
			if (found_text != NULL)
				strcpy_s(found_text, MAX_CHAR, current->text);

			if (prev == NULL)
				*head = current->next;
			else
				prev->next = current->next;

			return found_text;
		}
		prevprev = prev;
		prev = current;
		current = current->next;
	}

	if (prev != NULL)
	{
		if (prev->priority > priority)
		{
			char* found_text = (char*)malloc(MAX_CHAR * sizeof(char));
			if (found_text != NULL)
				strcpy_s(found_text, MAX_CHAR, prev->text);

			if (prevprev == NULL)
				*head = prev->next;
			else
				prevprev->next = prev->next;

			return found_text;
		}
	}

	return "No such prioritised aim";
}

int main()
{
	queue* head = NULL;
	queue q1, q2, q3, q4;
	InitQueue(&q1, (uint8_t)1, "q1 text");
	InitQueue(&q2, (uint8_t)2, "q2 text");
	InitQueue(&q3, (uint8_t)3, "q3 text");
	InitQueue(&q4, (uint8_t)2, "q4 text");

	Add(&head, &q4);
	PrintQueue(head);
	printf("\n");

	Add(&head, &q2);
	PrintQueue(head);
	printf("\n");

	Add(&head, &q3);
	PrintQueue(head);
	printf("\n");

	Add(&head, &q1);
	PrintQueue(head);
	printf("\n");

	//char* text = Pop1st(&head);
	//printf("Pop 1st in queue, its text = %s\n", text);

	//PrintQueue(head);
	//printf("\n");

	//PopEqualPriority(head, 2);
	//PopNotLessPriority(&head, 2);

	PrintQueue(head);
	printf("\n");

	int choice = -1;
	int result;
	while (choice < 0 || choice > 4)
	{
		printf_s("Type 0, 1, 2, 3 or 4 to use as argument of PopEqualOrHigherPriority function: ");
		result = scanf_s("%d", &choice);

		if (result != 1)
		{
			while (getchar() != '\n');
			choice = -1;
		}
	}

	printf(PopEqualOrHigherPriority(&head, choice));
	printf("\n");
	PrintQueue(head);
	printf("\n");

	/*
	Add(&head, &q2);
	Add(&head, &q3);
	Add(&head, &q1);
	Add(&head, &q4);
	PrintQueue(head);
	*/
	//free(text);

	printf_s("Type anything to quit: ");
	result = scanf_s("%d", &choice);

	return 0;
}