#include <stdio.h>
#include <stdint.h>
#include <string.h> // проблемы с strdup без этой бибилотеки
#include <stdbool.h>
#include <stdlib.h>

uint32_t ip_to_uint(char* ip)
{
	char* ip_copy = strdup(ip);
	uint32_t final_ip = 0;
	uint8_t octet;
	char* context = NULL; // указатель контекста
	char* token = strtok_s(ip_copy, ".", &context);
	for (int i = 0; i < 4; i++)
	{
		if (token != NULL)
		{
			octet = (uint8_t)atoi(token);
			final_ip = (final_ip << 8) | octet;
			token = strtok_s(NULL, ".", &context);
		}
	}

	free(ip_copy);
	return final_ip;
}

bool ip_in_web(char* gw, char* mask, char* ip)
{
	uint32_t umask = ip_to_uint(mask);
	//printf("\n%u\n", ip_to_uint(gw) & umask);
	//printf("%u", ip_to_uint(ip) & umask);
	return ((ip_to_uint(gw) & umask) == (ip_to_uint(ip) & umask));
}

char* generate_random_ip()
{
	char* ip[16];
	snprintf(ip, 16, "%d.%d.%d.%d",
		rand() % 256, rand() % 256, rand() % 256, rand() % 256);

	return ip;
}

double Ntest(int N, char* gw, char* mask)
{
	int correct_results = 0;
	for (int i = 0; i < N; i++)
		if (ip_in_web(gw, mask, generate_random_ip()))
			correct_results++;
	return 100.0 * correct_results / N;
}

char* create_mask(int ones_count)
{
	char* mask_str = malloc(16);
	if (mask_str == NULL) {
		return NULL;
	}

	uint32_t mask;
	if (ones_count == 0)
		mask = 0;
	else
		mask = (0xFFFFFFFFU << (32 - ones_count));

	snprintf(mask_str, 16, "%d.%d.%d.%d",
		(mask >> 24) & 0xFF,
		(mask >> 16) & 0xFF,
		(mask >> 8) & 0xFF,
		mask & 0xFF);

	return mask_str;
}

int main()
{
	char str_ip[16];
	uint32_t ip;
	printf("Enter gate (SHLYUZ) ip: ");


	//str_ip = "192.168.1.15";
	strncpy_s(str_ip, sizeof(str_ip), "192.168.1.15", 16);
	ip = ip_to_uint(str_ip);
	printf("%u", ip);

	char mask[16] = "255.0.0.0", gw[16] = "192.168.1.0", test_ip[16] = "192.168.1.5";
	ip_in_web(gw, mask, test_ip);
	//printf("\n%s", generate_random_ip());
	for (int i = 0; i < 33; i++)
		printf("\nAmount of correct answers of mask with %d ones: %f%", i, Ntest(1000000, gw, create_mask(i)));
}