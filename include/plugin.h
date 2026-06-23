#include <gcc-plugin.h>
#include <tree-pass.h>
#include <context.h>
#include <tree.h>
#include <basic-block.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <bitmap.h>

#include "graphviz.h"
#include "mpi_collectives.h"

int plugin_is_GPL_compatible;

int plugin_init(struct plugin_name_args *plugin_info,
        struct plugin_gcc_version *versiovn);

const pass_data my_pass_data {
    GIMPLE_PASS,       // Type
    "my_pass",         // Pass name
    OPTGROUP_NONE,     // optinfo_flags
    TV_NONE,           // tv_id
    PROP_gimple_any,   // Properties required
    0,                 // Properties provided
    0,                 // Properties destroyed
    0,                 // todo_flags_start
    0,                 // todo_flags_finish
};

class my_pass : public gimple_opt_pass
{
public:
    my_pass(gcc::context *ctxt)
        : gimple_opt_pass (my_pass_data, ctxt)
    {}

    my_pass* clone()
    {
        return new my_pass(g);
    }

    bool gate(function *fun);
    unsigned int execute(function *fun);
};
