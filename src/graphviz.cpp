#include "../include/graphviz.h"
#include "../include/mpi_collectives.h"

/* Build a filename (as a string) based on function name */
char *cfgviz_generate_filename(function *fun, const char *suffix) {
  char *target_filename;

  target_filename = (char *)xmalloc(2048 * sizeof(char));

  snprintf(target_filename, 1024, "%s_%s_%d_%s.dot", current_function_name(),
           LOCATION_FILE(fun->function_start_locus),
           LOCATION_LINE(fun->function_start_locus), suffix);

  return target_filename;
}

/* Dump the graphviz representation of function 'fun' in file 'out' */
void cfgviz_internal_dump(function *fun, FILE *out) {

  // Print the header line and open the main graph
  fprintf(out, "Digraph G{\n");

  basic_block bb;
  FOR_ALL_BB_FN(bb, fun) {
    int index = bb->index;
    edge e;
    edge_iterator ei;
    int *collective = (int *)bb->aux;
    int code = 5;
    if (collective != NULL) {
      code = *collective;
    }

    fprintf(out, "N%d [label=\"Node %d\n %s \" shape=ellipse]\n", index, index,
            mpi_collective_name[code]);
    FOR_EACH_EDGE(e, ei, bb->succs) {
      basic_block src = e->src;
      basic_block dst = e->dest;

      fprintf(out, "N%d -> N%d [color=red label=\"\"]\n", src->index,
              dst->index);
    }
  }

  // Close the main graph
  fprintf(out, "}\n");
}

void cfgviz_dump(function *fun, const char *suffix) {
  char *target_filename;
  FILE *out;

  target_filename = cfgviz_generate_filename(fun, suffix);

  printf("[GRAPHVIZ] Generating CFG of function %s in file <%s>\n",
         current_function_name(), target_filename);

  out = fopen(target_filename, "w");

  cfgviz_internal_dump(fun, out);

  fclose(out);
  free(target_filename);
}
