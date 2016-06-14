#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "veclor.h"

typedef struct city_s {
  char* name;
  int population;
} city;

int city_copy_constructor(city* dest, const city* src) {
  if(dest->name = malloc(1 + strlen(src->name)))
    strcpy(dest->name, src->name);
  else
    return -1; // define your own error codes less than zero.
  dest->population = src->population;
  return 0; // success code must be zero
}

void godzilla(city* dest) { // godzilla, aka city destructor
  free(dest->name);
}

int main() {
  veclor japanese_cities = VECLOR(city, city_copy_constructor, godzilla);
//veclor japanese_cities = VECLOR(city, NULL, NULL);

  char name[1000];
  city c;
  strcpy (c.name, "Tokyo");
  c.population = 13510000;
  veclor_push_back (&japanese_cities, &c);
  strcpy (c.name, "Kyoto");
  c.population = 1474000;
  veclor_push_back (&japanese_cities, &c);

  int i = 0;
  city* city_ptr;
  while (city_ptr = (city*) veclor_at(&japanese_cities, i++))
    printf("%s has population %d\n", city_ptr->name, city_ptr->population);
}
