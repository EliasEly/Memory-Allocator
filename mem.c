#include "mem.h"
#include "common.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

// constante définie dans gcc seulement
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* La seule variable globale autorisée
 * On trouve à cette adresse la taille de la zone mémoire
 */
static void* memory_addr;

static inline void *get_system_memory_adr() {
	return memory_addr;
}


/* struct for free block */
struct fb {
	size_t size;
	struct fb* next;
	/* ... */
};

typedef struct fb* pfb;

struct first_bloc {
	size_t sizeG;
	pfb begin;
};


static inline size_t get_system_memory_size() {
	return ((struct first_bloc*)memory_addr)->sizeG;
}


void mem_init(void* mem, size_t taille)
{
	/* Création du bloc jaune */
        memory_addr = mem;
        ((struct first_bloc*)memory_addr)->sizeG = taille;
        ((struct first_bloc*)memory_addr)->begin = memory_addr + 1;
        ((struct first_bloc*)memory_addr)->begin->next = NULL;
        ((struct first_bloc*)memory_addr)->begin->size = taille - sizeof(struct first_bloc*);
	assert(mem == get_system_memory_adr());
	assert(taille == get_system_memory_size());

	/* ... */
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	/* ... */
	while (/* ... */ 0) {
		/* ... */
		print(/* ... */NULL, /* ... */0, /* ... */0);
		/* ... */
	}
}

static mem_fit_function_t *mem_fit_fn;

void mem_fit(mem_fit_function_t *f) {
	mem_fit_fn=f;
}

void *mem_alloc(size_t taille) {
	/* ... */
	/* __attribute__((unused))  juste pour que gcc compile ce squelette avec -Werror */
	struct fb *fb=mem_fit_fn( ((struct first_bloc*)memory_addr)->begin, taille);
	if(fb != NULL){
		struct fb temp;
		pfb previous = ((struct first_bloc*)memory_addr)->begin;

		/* finir d'update l'allocation */
		temp = *fb;
		fb->size = fb->size - taille;	

		/* cherche le bloc qui pointé sur l'adresse du bloc vide */
		while(previous->next != fb){
			previous = previous->next;
		}
	}
	/* ... */
	return NULL;
}


void mem_free(void* mem) {
}


struct fb* mem_fit_first(struct fb *list, size_t size) {
	while(size > list->size){
		if (list->next == NULL){
			return NULL;
		}
		list = list->next;
	}
	return list;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
	return 0;
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}
