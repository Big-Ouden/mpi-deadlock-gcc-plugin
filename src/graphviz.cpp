#include "../include/graphviz.h"
#include "../include/mpi_collectives.h"

/* Replace '/' by '-' from filename */
void sanitize(char* s){
  for (int i = 0 ; s[i] != '\0'; i++){
    if (s[i] == '/'){
      s[i] = '-';
    } 
  }
}


/* Build a filename (as a string) based on function name in a specific dir*/
char *cfgviz_generate_filename(function *fun, const char *suffix) {
  char *target_filename;
  char *tmp;

  target_filename = (char *)xmalloc(2048 * sizeof(char));
  tmp = (char *)xmalloc(2048 * sizeof(char));

  snprintf(tmp, 1024, "%s_%s_%d_%s.dot",current_function_name(),
           LOCATION_FILE(fun->function_start_locus),
           LOCATION_LINE(fun->function_start_locus), suffix);
  sanitize(tmp);
  snprintf(target_filename, 1024, "%s/%s", "build", tmp);

  return target_filename;
}

/* Dump the graphviz representation of function 'fun' in file 'out' */
// void cfgviz_internal_dump(function *fun, FILE *out) {
//
//   // Print the header line and open the main graph
//   fprintf(out, "Digraph G{\n");
//
//   basic_block bb;
//   FOR_ALL_BB_FN(bb, fun) {
//     int index = bb->index;
//     edge e;
//     edge_iterator ei;
//     int *collective = (int *)bb->aux;
//     int code = LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
//     if (collective != NULL) {
//       code = *collective;
//     }
//     fprintf(stdout, "[DEBUG] code: %d\n", code);
//
//     fprintf(out, "N%d [label=\"Node %d\n %s \" shape=ellipse]\n", index, index,
//             mpi_collective_name[code]);
//     FOR_EACH_EDGE(e, ei, bb->succs) {
//       basic_block src = e->src;
//       basic_block dst = e->dest;
//
//       fprintf(out, "N%d -> N%d [color=red label=\"\"]\n", src->index,
//               dst->index);
//     }
//   }
//
//   // Close the main graph
//   fprintf(out, "}\n");
// }


/* First MPI collective called in 'bb', or LAST_AND_UNUSED if there is none.
   split_cfg() guarantees at most one collective per block. */
static enum mpi_collective_code block_collective(basic_block bb) {
  for (auto gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
    enum mpi_collective_code code = get_mpi_call_code(gsi_stmt(gsi));
    if (code != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
      return code;
  }
  return LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
}

/* First source line of a statement in 'bb', or 0 if none has one */
static int block_line(basic_block bb) {
  for (auto gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
    int line = gimple_lineno(gsi_stmt(gsi));
    if (line > 0)
      return line;
  }
  return 0;
}

/* Print the outgoing edges of 'bb' */
static void dump_edges(basic_block bb, FILE *out) {
  edge e;
  edge_iterator ei;
  FOR_EACH_EDGE(e, ei, bb->succs) {
    fprintf(out, "N%d -> N%d [color=red label=\"\"]\n", e->src->index,
            e->dest->index);
  }
}

void cfgviz_internal_dump(function *fun, FILE *out) {

  // Print the header line and open the main graph
  fprintf(out, "Digraph G{\n");
  basic_block bb;

  // Entry and exit blocks are not part of FOR_ALL_BB_FN
  fprintf(out, "N%d [label=\"ENTRY\" shape=ellipse]\n", ENTRY_BLOCK_PTR_FOR_FN(fun)->index);
  fprintf(out, "N%d [label=\"EXIT\" shape=ellipse]\n", EXIT_BLOCK_PTR_FOR_FN(fun)->index);

  // One node per basic block, so each block is declared exactly once
  FOR_EACH_BB_FN(bb, fun) {
    int index = bb->index;
    enum mpi_collective_code code = block_collective(bb);
    if (code != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE) {
      fprintf(out, "N%d [label=\"Node %d\n %s \" shape=ellipse]\n", index, index,
              mpi_collective_name[code]);
    } else {
      fprintf(out, "N%d [label=\"Node %d\n line %d \" shape=ellipse]\n", index, index,
              block_line(bb));
    }
  }

  dump_edges(ENTRY_BLOCK_PTR_FOR_FN(fun), out);
  FOR_EACH_BB_FN(bb, fun) {
    dump_edges(bb, out);
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
