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
struct free_bloc {
	size_t size;
	struct free_bloc* next;
};

typedef struct free_bloc* pfree_bloc;

struct first_bloc {
	size_t sizeG;
	pfree_bloc begin;
};

typedef struct first_bloc first_bloc;

struct bloc_used_ {
	size_t sizeUsed;
};

typedef struct bloc_used_ bloc_used;


static inline size_t get_system_memory_size() {
	return *(size_t*)memory_addr;
}


size_t ALIGN_SIZE(size_t taille){
	return taille + (ALIGNMENT - taille % ALIGNMENT);
}

void mem_init(void* mem, size_t taille)
{
	/* Création du bloc jaune */
        memory_addr = mem;
        ((first_bloc*)memory_addr)->sizeG = taille;
        ((first_bloc*)memory_addr)->begin = memory_addr + sizeof(first_bloc);
        ((first_bloc*)memory_addr)->begin->next = NULL;
        ((first_bloc*)memory_addr)->begin->size = taille - sizeof(struct first_bloc);
	assert(mem == get_system_memory_adr());
	assert(taille == get_system_memory_size());
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	pfree_bloc free_bloc = ((first_bloc*)memory_addr)->begin;
	bloc_used* moving = memory_addr + sizeof(first_bloc);
	__uint8_t* end_mem = get_system_memory_adr() + get_system_memory_size();

	while((__uint8_t*)moving < end_mem){
		if ((pfree_bloc)moving != free_bloc){
			print(moving, moving->sizeUsed, 0);
		} else {
			print(moving, moving->sizeUsed, 1);
			free_bloc = free_bloc->next;
		}
		moving = (bloc_used*)(((__uint8_t*)moving) + moving->sizeUsed);
	}
}

static mem_fit_function_t *mem_fit_fn;

void mem_fit(mem_fit_function_t *f) {
	mem_fit_fn=f;
}

void *mem_alloc(size_t taille) {
	size_t alloc_size = ALIGN_SIZE(taille + sizeof(bloc_used));
	/* __attribute__((unused))  juste pour que gcc compile ce squelette avec -Werror */
																/* size for the user + the metadata needed */
	struct free_bloc *fb=mem_fit_fn(((first_bloc*)memory_addr)->begin, alloc_size);
	/* here i get either the address to allocate or NULL which means do nothing*/
	if(fb != NULL){
		pfree_bloc previous = ((first_bloc*)memory_addr)->begin;
		/* previous means the block that were pointing at that free bloc*/
		if (fb != ((first_bloc*)memory_addr)->begin){

			/* check if the first free bloc is the one returned */
			while(previous->next != fb){
				previous = previous->next;
			}
			
			/*check if there is enough space for the metadata of a free_bloc*/
			if ((fb->size - taille - sizeof(bloc_used)) > sizeof(struct free_bloc)){
				/* update the pointer of free bloc */

				/*tmp contain the next address of the free_bloc*/
				pfree_bloc tmp = fb->next;

				/* we allocate the fb bloc by casting it in bloc used and affecting its size*/
				((bloc_used*)fb)->sizeUsed = taille;
				previous->next = (pfree_bloc)(((__uint8_t*)fb) + alloc_size);

				((pfree_bloc)previous->next)->size -= alloc_size;	

				((pfree_bloc) (((__uint8_t*)fb) + alloc_size))->next = tmp;
				
				
				/* if there is not enough space for sizeof(free_bloc) (ie another free slot to allocate),
				 we allocate all the freebloc */
			} else {

				((bloc_used*)fb)->sizeUsed = ((pfree_bloc)fb)->size;
				previous->next = fb->next;

			}

		} else {
			
			__uint8_t* addr = (__uint8_t*)fb;
			pfree_bloc	next_free_bloc = (pfree_bloc)(addr+ alloc_size);
			first_bloc* meta_dataGlobal = get_system_memory_adr();
			meta_dataGlobal->begin = next_free_bloc;
			meta_dataGlobal->begin->size = fb->size - alloc_size;
			meta_dataGlobal->begin->next =NULL;

			((bloc_used*)fb)->sizeUsed = alloc_size;  

		}
		return fb + sizeof(bloc_used);
	}
	return fb;
}

void mem_free(void* mem) {
	__uint8_t* addr_mem = (__uint8_t*) mem - sizeof(bloc_used);
	size_t size_mem = ((bloc_used*)addr_mem)->sizeUsed;
	__uint8_t* next_bloc = addr_mem + size_mem;

	__uint8_t* free_previous = (__uint8_t*)((first_bloc*)get_system_memory_adr())->begin;
	__uint8_t* free_next = (__uint8_t*)(((first_bloc*)get_system_memory_adr())->begin->next);

	while(free_previous < addr_mem){
		size_t sfree_previous = ((pfree_bloc)free_previous)->size;
		size_t sfree_next = ((pfree_bloc)free_next)->size;
		
		if (sfree_previous + free_previous == addr_mem){
		
			((pfree_bloc)free_previous)->size = sfree_previous + size_mem;
		
			if(next_bloc == free_next + sfree_next){
				((pfree_bloc)free_previous)->size = sfree_previous + sfree_next;
				((pfree_bloc)free_previous)->next = ((pfree_bloc)free_next)->next;
			}
			return;
		} else if (next_bloc == free_next){
			((pfree_bloc)free_previous)->next = (pfree_bloc)addr_mem;
			((pfree_bloc)addr_mem)->next = ((pfree_bloc)free_next)->next;
			((pfree_bloc)addr_mem)->size = size_mem + sfree_next;
		}
		
	}



		/*		This block is dealing with the case of ZL ZO ZL
		size_t sfree_previous = ((pfree_bloc)free_previous)->size;
		if (sfree_previous + free_previous == addr_mem){
			((pfree_bloc)free_previous)->size = sfree_previous + size_mem;
		}
		if (((pfree_bloc)free_previous)->next != NULL && (__uint8_t*)((pfree_bloc)free_previous)->next == next_bloc){
			((pfree_bloc)free_previous)->size = sfree_previous + ((pfree_bloc)free_previous)->size;
			((pfree_bloc)free_previous)->next = ((pfree_bloc)free_previous)->next->next;
		}
		*/
}


struct free_bloc* mem_fit_first(struct free_bloc *list, size_t size) {
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
struct free_bloc* mem_fit_best(struct free_bloc *list, size_t size) {
	return NULL;
}

struct free_bloc* mem_fit_worst(struct free_bloc *list, size_t size) {
	return NULL;
}
