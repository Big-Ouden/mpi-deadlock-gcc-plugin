#include "plugin.h"
#include "mpi_collectives.h"
#include <diagnostic-core.h>
#include <errors.h>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

using namespace std;

// Point d'entrée du plugin
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {
  my_pass pass(g);
  struct register_pass_info my_pass_info;
  my_pass_info.pass = &pass;
  my_pass_info.reference_pass_name = "cfg";
  my_pass_info.ref_pass_instance_number = 0;
  my_pass_info.pos_op = PASS_POS_INSERT_AFTER;
  register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL,
                    &my_pass_info);
  return 0;
}

void iter_each_bb(function *fun, void (*operand)(basic_block)) {
  basic_block bb;
  FOR_EACH_BB_FN(bb, cfun) { operand(bb); }
}
void iter_all_bb(function *fun, void (*operand)(basic_block)) {
  basic_block bb;
  FOR_ALL_BB_FN(bb, cfun) { operand(bb); }
}

bitmap_head *new_bmp() {
  basic_block bb;
  bitmap_head *bmp_head;
  bmp_head = XNEWVEC(bitmap_head, last_basic_block_for_fn(cfun));

  FOR_ALL_BB_FN(bb, cfun) {
    bitmap_initialize(&bmp_head[bb->index], &bitmap_default_obstack);
  }
  return bmp_head;
}

void reset_bb(basic_block bb) {
  bb->aux = NULL;
  bb->flags = 0;
}

// Regarde s'il y a plusieurs collectives MPI dans un basic_block
bool has_multiple_mpi_calls(basic_block bb) {
  int n = 0;
  FOR_EACH_BB_FN(bb, cfun) {
    for (auto gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
      gimple *stmt = gsi_stmt(gsi);
      enum mpi_collective_code code = get_mpi_call_code(stmt);
      if (LAST_AND_UNUSED_MPI_COLLECTIVE_CODE != code) {
        n++;
      }
      if (n > 1)
        return true;
    }
  }
  return false;
}

// Remplit la bitmap frontiers avec les  frontières de post-dominance de chaque
// noeuds.
void compute_post_dominance_frontiers(bitmap_head *frontiers) {
  calculate_dominance_info(CDI_POST_DOMINATORS);
  edge p;
  edge_iterator ei;
  basic_block b;
  FOR_EACH_BB_FN(b, cfun) {
    if (EDGE_COUNT(b->succs) >= 2) {
      basic_block domsb = get_immediate_dominator(CDI_POST_DOMINATORS, b);
      FOR_EACH_EDGE(p, ei, b->succs) {
        basic_block runner = p->dest;
        if (runner == ENTRY_BLOCK_PTR_FOR_FN(cfun))
          continue;
        while (runner != domsb) {
          if (!bitmap_set_bit(&frontiers[runner->index], b->index))
            break;
          runner = get_immediate_dominator(CDI_POST_DOMINATORS, runner);
        }
      }
    }
  }
  free_dominance_info(CDI_POST_DOMINATORS);
}

// Supprime les boucles du cfg et le copie dans une bitmap
void remove_loops_cfg(bitmap_head *cfg) {
  edge p;
  edge_iterator ei;
  basic_block b;
  mark_dfs_back_edges(cfun);
  FOR_ALL_BB_FN(b, cfun) {
    FOR_EACH_EDGE(p, ei, b->preds) {
      if (p != NULL && (p->flags & EDGE_DFS_BACK) != 0) {
        bitmap_clear_bit(&cfg[p->src->index], p->dest->index);
      }
    }
  }
}

// Sépare les basic_block avec plusieurs collectives MPI
void split_cfg() {
  basic_block bb;

  FOR_ALL_BB_FN(bb, cfun) {
    int n = 0;
    for (auto gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
      gimple *stmt = gsi_stmt(gsi);
      enum mpi_collective_code code = get_mpi_call_code(stmt);
      if (code != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE)
        n++;
      if (n > 1) {
        gsi_prev(&gsi);
        stmt = gsi_stmt(gsi);
        split_block(bb, stmt);
      }
    }
  }
}

bitmap_head *make_empty_cfun_bitmap() {
  bitmap_head *cfg;
  basic_block bb;
  cfg = XNEWVEC(bitmap_head, last_basic_block_for_fn(cfun));
  FOR_ALL_BB_FN(bb, cfun) {
    bitmap_initialize(&cfg[bb->index], &bitmap_default_obstack);
  }
  return cfg;
}

bitmap_head *make_cfg_bitmap() {
  bitmap_head *cfg = make_empty_cfun_bitmap();
  basic_block bb;
  edge p;
  edge_iterator ei;
  FOR_ALL_BB_FN(bb, cfun) {
    FOR_EACH_EDGE(p, ei, bb->succs) {
      basic_block src = p->src;
      basic_block dest = p->dest;
      bitmap_set_bit(&cfg[src->index], dest->index);
    }
  }
  return cfg;
}

// Récupère un basic_block à partir de son index
basic_block bb_of_index(int i) {
  basic_block bb;
  FOR_ALL_BB_FN(bb, cfun) {
    if (bb->index == i)
      return bb;
  }

  return bb;
}

// Effectue un parcours en profondeur permettant de calculer le rang
unordered_map<int, unordered_map<int, set<int>>>
dfs(bitmap_head *cfg, int v, unordered_map<int, bool> discovered,
    unordered_map<int, unordered_map<int, set<int>>> map) {
  vector<vector<int>> s;
  s.push_back({0, v});
  while (s.size() > 0) {
    int v = s[0][1];
    int rank = s[0][0];
    s.erase(s.begin());
    if (discovered.find(v) == discovered.end()) {
      discovered[v] = true;
      bitmap_head *v_head = &cfg[v];
      bitmap_iterator bi;
      unsigned bit_no;
      //  Début du code préfixe
      //  Calcul du rang
      enum mpi_collective_code code = LAST_AND_UNUSED_MPI_COLLECTIVE_CODE;
      basic_block bb = bb_of_index(v);
      bool check = false;
      for (auto gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
        gimple *stmt = gsi_stmt(gsi);
        code = get_mpi_call_code(stmt);
        if (code != LAST_AND_UNUSED_MPI_COLLECTIVE_CODE) {
          check = true;
          // Est-ce-que MAP[RANK] existe ?
          if (map.find(rank) == map.end()) {
            unordered_map<int, set<int>> m2;
            map[rank] = m2;
          }
          map[rank][code].insert(v);
        }
      }

      // Fin code préfixe
      EXECUTE_IF_SET_IN_BITMAP(v_head, 0, bit_no, bi) {
        s.push_back({check ? rank + 2 : rank, (int)bit_no});
      }
    }
  }
  return map;
}

unordered_map<int, unordered_map<int, set<int>>>
make_mpi_rank_set(bitmap_head *cfg) {
  unordered_map<int, unordered_map<int, set<int>>> map;
  unordered_map<int, bool> discovered;
  map = dfs(cfg, 0, discovered, map);
  return map;
}

// Calcule la frontière de post-dominance d'un ensemble (cf. algo1 dans rapport)
set<int> union_pdf(set<int> v, bitmap_head *frontiers, bitmap_head *cfg) {
  set<int> res;
  set<int> all_nodes;
  queue<int> a_traiter;
  for (int x : v) {
    all_nodes.insert(x);
    edge e;
    edge_iterator ei;
    basic_block bb = bb_of_index(x);
    FOR_EACH_EDGE(e, ei, bb->preds) { a_traiter.push(e->src->index); }
  }

  while (!a_traiter.empty()) {
    int x = a_traiter.front();
    basic_block parent = bb_of_index(x);
    a_traiter.pop();
    // On vérifie que tous les successeurs de parent sont dans all_nodes
    edge e;
    edge_iterator ei;
    bool check = true;
    FOR_EACH_EDGE(e, ei, parent->succs) {
      basic_block enfant = e->dest;
      if (all_nodes.find(enfant->index) == all_nodes.end())
        check = false;
    }
    if (check) {
      all_nodes.insert(parent->index);
      // On ajoute les parents du parent dans a_traiter
      FOR_EACH_EDGE(e, ei, parent->preds) { a_traiter.push(e->src->index); }

    } else {
      res.insert(parent->index);
    }
  }

  return res;
}

// Calcule la pdf itéré d'un ensemble
set<int> pdf_itere(set<int> v, bitmap_head *frontiers, bitmap_head *cfg) {
  set<int> prev;
  v = union_pdf(v, frontiers, cfg);
  // On itere tant que la pdf n'est pas stable
  while (prev != v) {
    prev = v;
    set<int> temp = union_pdf(v, frontiers, cfg);
    v.insert(temp.begin(), temp.end());
  }
  return v;
}

// Affiche des warnings pour chaque lignes source de deadlock potentiel
void print_warning(const unordered_map<int, unordered_map<int, set<int>>> &map,
                   bitmap_head *frontiers, bitmap_head *cfg) {
  for (const auto &[rank, m2] : map) {

    for (const auto &[code, v] : m2) {

      set<int> pdf = union_pdf(v, frontiers, cfg);

      set<int> pdfi = pdf_itere(v, frontiers, cfg);

      for (int x : pdf) {
        basic_block bb = bb_of_index(x);
        const gimple_stmt_iterator gsi = gsi_last_bb(bb);
        const gimple *stmt = gsi_stmt(gsi);
        const location_t loc = gimple_location(stmt);

        warning_at(loc, OPT_fplugin_,
                   "Line %<%d%> can cause an invalid fork when evaluating MPI "
                   "collectives"
                   "",
                   gimple_lineno(stmt));
      }
    }
  }
}

// Fonction principale du plugin
// Corps de la passe
unsigned int my_pass::execute(function *fun) {
  basic_block bb;
  bitmap_head *mpi_calls;
  bitmap_head *df_list;
  split_cfg();
  mpi_calls = XNEWVEC(bitmap_head, last_basic_block_for_fn(cfun));
  df_list = XNEWVEC(bitmap_head, last_basic_block_for_fn(cfun));

  // Initialisation des bitmaps
  FOR_ALL_BB_FN(bb, cfun) {
    bitmap_initialize(&mpi_calls[bb->index], &bitmap_default_obstack);
    bitmap_initialize(&df_list[bb->index], &bitmap_default_obstack);
  }

  // Calcul des frontières de post-dominance
  compute_post_dominance_frontiers(df_list);
  free_dominance_info(CDI_POST_DOMINATORS);

  // On retire les back-edge du cfg
  bitmap_head *cfg = make_cfg_bitmap();
  remove_loops_cfg(cfg);

  // Calcul du rang des collectives
  unordered_map<int, unordered_map<int, set<int>>> map = make_mpi_rank_set(cfg);
  // Affichages des warnings (s'il y en a)
  print_warning(map, df_list, cfg);

#ifdef DEBUG
  // Génération du fichier Graphviz
  cfgviz_dump(fun, "_cfg");
  iter_all_bb(fun, reset_bb);

#endif

  return 0;
}

bool my_pass::gate(function *fun) { return true; }
