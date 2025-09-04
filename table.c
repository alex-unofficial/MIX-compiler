#include "table.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

HashTable *ht_new(size_t table_size) {
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

TableEntry *ht_prepend_entry(const char *key, uint64_t hash, Payload payload, TableEntry *prev) {
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

void ht_add_entry(HashTable *ht, const char *key, Payload payload) {
	uint64_t hash = fnv1a64_hash(key);
	size_t index = hash & (ht->mask);

	ht->buckets[index] = ht_prepend_entry(key, hash, payload, ht->buckets[index]);
	ht->n_entries += 1;
}

TableEntry *ht_find_entry(HashTable *ht, const char *key) {
	uint64_t hash = fnv1a64_hash(key);
	size_t index = hash & (ht->mask);

	TableEntry *e = ht->buckets[index];

	while(e != NULL) {
		if (e->hash == hash && strcmp(e->key, key) == 0) return e;
		e = e->next;
	}

	return NULL;
}

void ht_print(const HashTable *ht) {
	for (size_t i = 0 ; i < ht->table_size ; i++) {
		TableEntry *e = ht->buckets[i];

		while (e != NULL) {
			switch (e->payload.kind) {
				case PAYLOAD_METHOD:
					printf("METHOD (%s): return_type=%s, label='%s', n_params=%d, n_locals=%d\n", 
							e->key, data_type_str[e->payload.method.return_type], e->payload.method.label,
							e->payload.method.param_count, e->payload.method.local_count);
					ht_print(e->payload.method.symbols);
					break;

				case PAYLOAD_SYMBOL:
					const char *symbol_kind_str[] = {
						[SYMBOL_LOCAL] = "LOCAL",
						[SYMBOL_PARAM] = "PARAM",
					};

					printf("  %s (%s): data_type=%s, offset=%+d\n", 
							symbol_kind_str[e->payload.symbol.kind], e->key,
							data_type_str[e->payload.symbol.symbol_type], e->payload.symbol.offset);
					break;
			}

			e = e->next;
		}
	}
}

void ht_free(HashTable *ht) {
	for (int i = 0 ; i < ht->table_size ; i++) {
		ht_free_entry(ht->buckets[i]);
		ht->buckets[i] = NULL;
	}
	free(ht->buckets);
	ht->buckets = NULL;

	free(ht);
}

void ht_free_entry(TableEntry *e) {
	while (e != NULL) {
		TableEntry *next = e->next;

		free(e->key);
		ht_free_payload(e->payload);
		
		free(e);

		e = next;
	}
}

void ht_free_payload(Payload p) {
	switch(p.kind) {
		case PAYLOAD_METHOD:
			ht_free(p.method.symbols);
			free(p.method.label);
			break;

		case PAYLOAD_SYMBOL:
			/* nothing to free */
			break;
	}
}
