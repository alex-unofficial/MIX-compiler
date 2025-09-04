#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stddef.h>

#include "parser.tab.h"
#include "ast.h"

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

HashTable *symbols_from_ast(const ASTNode *root);

HashTable *new_ht(size_t table_size);
TableEntry *prepend_entry(const char *key, uint64_t hash, Payload payload, TableEntry *next);

void add_ht_entry(HashTable *ht, const char *key, Payload payload);
TableEntry *find_ht_entry(HashTable *ht, const char *key);

void free_ht(HashTable *ht);
void free_entry(TableEntry *e);
void free_payload(Payload p);

#endif
