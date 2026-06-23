#include <gcc-plugin.h>                                                          
#include <tree.h>                                                                
#include <basic-block.h>                                                         
#include <gimple.h>                                                              
#include <tree-pass.h>                                                           
#include <context.h>                                                             
#include <function.h>                                                            
#include <gimple-iterator.h>                                                     


/* Build a filename (as a string) based on function name */
char * 
cfgviz_generate_filename( function * fun, const char * suffix );

/* Dump the graphviz representation of function 'fun' in file 'out' */
void 
cfgviz_internal_dump( function * fun, FILE * out );

void 
cfgviz_dump( function * fun, const char * suffix );
