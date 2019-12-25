#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "common.h"

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

void allocate_mem_test(size_t size){
    void* result;
    while((result = mem_alloc(size)) != NULL){
    printf("%zu Bytes were allocated at %ld\n",size, (result - (void*)get_memory_adr()));
    }
}

void use_of_test(){
    printf("Choisissez un test :\n");
    printf("1 - Allocation\n");
    printf("2 - Allocation et libération\n");
    printf("q - Quitter\n");
}

void n_for_next(){
    printf("\n\npress n for next : ");
    char c = getchar();
    while (c != 'n'){
        c = getchar();
    }
    system("clear");
}

int main(int argc, char** argv){
    char buffer[TAILLE_BUFFER];
    char c;
    while(1){
        use_of_test();
        fflush(stdout);
        c = getchar();
        switch (c)
        {
        case '1':
            mem_init(get_memory_adr(), 512);
            allocate_mem_test(128);
            printf("\n");
            mem_show(afficher_zone);
            printf("\n");
            break;
        case '2':
            system("clear");
            /* free in ascending order of the address the case of ZO ZO ZO ZL then ZL ZO ZO ZL and so on.. */
            printf("TEST 1 WITH FREE : \n");
            mem_init(get_memory_adr(), 512);
            allocate_mem_test(128);
            printf("\n");
            mem_show(afficher_zone);
            printf("\n");
            for(int i = 0; i<3; i++){
                mem_free(get_memory_adr() + (16 + 144 *i));
                mem_show(afficher_zone);    
                printf("\n");
            }

            n_for_next();

            printf("TEST 2 WITH FREE : \n");
            /*memory full, and we free from the very end*/
            mem_init(get_memory_adr(), 512);
            allocate_mem_test(128);
            /*in fact 48 + the metadata represents the left space*/
            void* result = mem_alloc(48);
            printf("%d Bytes were allocated at %ld\n",48, (result - (void*)get_memory_adr()));
            printf("\n");
            mem_show(afficher_zone);
            printf("\n");
            mem_free(get_memory_adr()+512-64);
            for(int i = 3; i>=0; i--){
                mem_free(get_memory_adr() + (16 + 144 *i));
                mem_show(afficher_zone);    
                printf("\n");
            }
            n_for_next();
            break;
        case 'q':
            printf("Bye!\n");
            return 0;
        default:
            printf("Commande inconnue !\n");
            break;
        }
        /* vide ce qu'il reste de la ligne dans le buffer d'entree */
      fgets(buffer,TAILLE_BUFFER,stdin);
    }
    return 0;
}