#include <stdio.h>
#include "../includes/vector.h"
#include "../includes/structure.h"

int main() { 
  vec_pointer input = vector_create(sizeof(char));

  vec_pointer structure = create_structure();

  add_source(structure, vector_from_cstr("source1", -1));

  v_string current = v_get_vec(structure, 0);
  printf("Size: %d\n", structure->size);
  printf("Size: %d\n", current->size);

  v_string first = v_get_vec(current, 0);
  printf("First source: %s\n", first->value);

  
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest1", -1));
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest2", -1));
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest3", -1));
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest4", -1));
  add_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest5", -1));

  remove_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest2", -1));
  remove_destination(structure, vector_from_cstr("source1", -1), vector_from_cstr("dest4", -1));

  add_source(structure, vector_from_cstr("source2", -1));
  add_destination(structure,  vector_from_cstr("source2", -1), vector_from_cstr("dest3", -1));
  add_destination(structure,  vector_from_cstr("source2", -1), vector_from_cstr("dest4", -1));

  remove_source(structure, vector_from_cstr("source2", -1));

  for(int i = 0; i < structure->size; i++) {
    v_string source = v_get_vec(structure, i);
    printf("Source: %s\n", v_get_vec(source, 0)->value);

    for(int j = 1; j < source->size; j++) {
      v_string destination = v_get_vec(source, j);
      printf("  Destination: %s\n", destination->value);
    }
  }


  vector_delete(input);

  printf("Deleting structure\n");
  delete_structure(structure);
}
