#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "veclor.h"

typedef struct city_s {
  char* name;
  int population;
  float per_capita_income;
} city;

void city_copy_constructor(city* dest, const city* src) {
  if(dest->name = malloc(1 + strlen(src->name)))
    strcpy(dest->name, src->name);
  dest->population = src->population;
  dest->per_capita_income = src->per_capita_income;
}

void godzilla(city* dest) { // godzilla, aka city destructor
  free(dest->name);
}

int main() {
  veclor japanese_cities = VECLOR(city, city_copy_constructor, godzilla);
}
