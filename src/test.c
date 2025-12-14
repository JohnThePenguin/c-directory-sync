#include <stdio.h>
#include "../includes/vector.h"
#include "../includes/structure.h"

int test() { 
  vec_pointer input = vector_create(sizeof(char));

  vec_pointer structure = create_structure();

  add_source(structure, vector_from_cstr("source1", -1), 1, 1);

  source_t current = v_get_source(structure, 0);
  printf("Size: %d\n", structure->size);
  printf("Size: %d\n", current.destinations->size);
  printf("First source: %s\n", current.source->value);

  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest1", -1), 1);
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest2", -1), 1);
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest3", -1), 1);
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest4", -1), 1);
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest5", -1), 1);
  remove_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest2", -1));
  remove_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest4", -1));

  add_source(structure, vector_from_cstr("source2", -1), 2, 2);
  add_destination(structure,  vector_from_cstr("source2", -1), vector_from_cstr("dest3", -1), 2);
  add_destination(structure,  vector_from_cstr("source2", -1), vector_from_cstr("dest4", -1), 2);
  remove_source(structure, vector_from_cstr("source2", -1));

  for(int i = 0; i < structure->size; i++) {
    source_t source = v_get_source(structure, i);
    printf("Source: %s\n", source.source->value);

    for(int j = 0; j < source.destinations->size; j++) {
     dest_t destination = v_get_destination(source.destinations, j);
      printf("  Destination: %s\n", destination.destination->value);
    }
  }


  vector_delete(input);

  printf("Deleting structure\n");
  delete_structure(structure);
}
