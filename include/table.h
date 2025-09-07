#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stddef.h>

#include "parser.tab.h"
#include "ast.h"

#ifndef TABLE_SIZE
#define TABLE_SIZE 32
#endif

typedef struct TableEntry TableEntry;
typedef struct HashTable HashTable;

enum PayloadKind {PAYLOAD_METHOD, PAYLOAD_SYMBOL};

enum SymKind {SYMBOL_PARAM, SYMBOL_LOCAL};
static const char *sym_kind_str[] = {
	[SYMBOL_PARAM] = "parameter",
	[SYMBOL_LOCAL] = "variable"
};

typedef struct {
	enum PayloadKind kind;
	YYLTYPE loc;
	union {
		struct {
			enum DataType return_type;
			unsigned int param_count;
			unsigned int local_count;
			HashTable *symbols;
			char *label;
		} method;

		struct {
			enum DataType symbol_type;
			enum SymKind kind;
			int offset; 
		} symbol;
	};
} Payload;

struct TableEntry {
	char *key;
	uint64_t hash;

	Payload payload;

	struct TableEntry *next;
};

struct HashTable {
	size_t table_size;
	uint64_t mask;

	TableEntry **buckets;
	size_t n_entries;
};

HashTable *ht_new(size_t table_size);
TableEntry *ht_prepend_entry(const char *key, uint64_t hash, Payload payload, TableEntry *next);

void ht_add_entry(HashTable *ht, const char *key, Payload payload);
TableEntry *ht_find_entry(const HashTable *ht, const char *key);

unsigned int ht_from_ast(const ASTNode *node, HashTable **gt);

void ht_print(const HashTable *ht);

void ht_free(HashTable *ht);
void ht_free_entry(TableEntry *e);
void ht_free_payload(Payload p);

#endif
