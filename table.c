#include "table.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

HashTable *new_ht(size_t table_size) {
	if ((table_size & (table_size - 1)) != 0) {
		fprintf(stderr, "implementation error: table size must be a power of 2\n");
		exit(1);
	}

	HashTable *ht = malloc(sizeof(HashTable));

	ht->table_size = table_size;
	ht->mask = table_size - 1;

	ht->n_entries = 0;

	ht->buckets = calloc(table_size, sizeof(TableEntry *));

	return ht;
}

TableEntry *prepend_entry(const char *key, uint64_t hash, Payload payload, TableEntry *prev) {
	TableEntry *e = malloc(sizeof(TableEntry));

	e->key = strdup(key);
	e->hash = hash;

	e->next = prev;

	e->payload = payload;

	return e;
}

uint64_t fnv1a64_hash(const char *str) {
	// Hash algorithm and parametes: https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
	uint64_t h = 0xcbf29ce484222325UL; // 64-bit offset basis
	while (*str) {
		h ^= (unsigned char)(*str++);
		h *= 0x00000100000001b3UL; // FNV prime
	}
	return h;
}

void add_ht_entry(HashTable *ht, const char *key, Payload payload) {
	uint64_t hash = fnv1a64_hash(key);
	size_t index = hash & (ht->mask);

	ht->buckets[index] = prepend_entry(key, hash, payload, ht->buckets[index]);
	ht->n_entries += 1;
}

TableEntry *find_ht_entry(HashTable *ht, const char *key) {
	uint64_t hash = fnv1a64_hash(key);
	size_t index = hash & (ht->mask);

	TableEntry *e = ht->buckets[index];

	while(e != NULL) {
		if (e->hash == hash && strcmp(e->key, key) == 0) return e;
		e = e->next;
	}

	return NULL;
}

void free_ht(HashTable *ht) {
	for (int i = 0 ; i < ht->table_size ; i++) {
		free_entry(ht->buckets[i]);
		ht->buckets[i] = NULL;
	}
	free(ht->buckets);
	ht->buckets = NULL;

	free(ht);
}

void free_entry(TableEntry *e) {
	while (e != NULL) {
		TableEntry *next = e->next;

		free(e->key);
		free_payload(e->payload);
		
		free(e);

		e = next;
	}
}

void free_payload(Payload p) {
	switch(p.kind) {
		case PAYLOAD_METHOD:
			free_ht(p.method.symbols);
			free(p.method.label);
			break;

		case PAYLOAD_SYMBOL:
			/* nothing to free */
			break;
	}
}
