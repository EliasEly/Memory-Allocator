#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TAILLE_BUFFER 128
void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %lu, Taille : %lu\n", free?"libre":"occupee",
         adresse - get_memory_adr(), (unsigned long) taille);
}

void afficher_zone_libre(void *adresse, size_t taille, int free)
{
  if (free)
	  afficher_zone(adresse, taille, 1);
}

void afficher_zone_occupee(void *adresse, size_t taille, int free)
{
  if (!free)
	  afficher_zone(adresse, taille, 0);
}

int main(int argc, char *argv[]) {
	char buffer[TAILLE_BUFFER];
    char *adresse;
    int offset;
    int taille, i;
    mem_init(get_memory_adr(), get_memory_size());
    alloc_max(get_memory_size());
    fflush(stdout);

    /**
     * ********** CAS n°1 **********
     * ZL ZO ZL
     * devient
     *    ZL
    */
    void *ptr1, *ptr2, *ptr3;
    ptr1 = mem_alloc(50);
    printf("Memoire allouee en %d\n", (int) (ptr1-get_memory_adr()));
    ptr2 = mem_alloc(61);
    printf("Memoire allouee en %d\n", (int) (ptr2-get_memory_adr()));
    mem_free(ptr1); 
    mem_show(afficher_zone);

    printf("\n");
    mem_free(ptr2);
	printf("Memoire liberee\n");
    mem_show(afficher_zone);
    

    /**
     * ********** CAS n°1 **********
     * ZO ZL ZO
     *  devient
     *   ZL  ZO
     *   puis
     *    ZL
    */
    void *ptr1, *ptr2, *ptr3;
    ptr1 = mem_alloc(50);
    printf("Memoire allouee en %d\n", (int) (ptr1-get_memory_adr()));
    ptr2 = mem_alloc(78);
    printf("Memoire allouee en %d\n", (int) (ptr2-get_memory_adr()));
    mem_show(afficher_zone);
    printf("\n");

    mem_free(ptr1);
    mem_show(afficher_zone);

    printf("\n");
    mem_free(ptr2);
	printf("Memoire liberee\n");
    mem_show(afficher_zone);

	// TEST OK
	return 0;
}
