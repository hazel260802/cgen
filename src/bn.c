/*
  (C) 2021 Nguyen Ba Ngoc (bangoc)
*/

#include "bn.h"

void bn_free_tree(bn_tree_t *tp, bn_free_node_t fn) {
  bn_tree_t t = *tp;
  bn_node_t cur, prev = NULL_PTR;
  bn_postorder_foreach_inline(cur, t) {
    if (prev) {
      fn(prev);
    }
    prev = cur;
  }
  if (prev) {
    fn(prev);
  }
  free(t);
  *tp = NULL_PTR;
}

bn_node_t bn_create_node() {
  return calloc(1, sizeof(struct bn_node));
}

bn_tree_t bn_create_tree(bn_node_t node) {
  bn_tree_t tree = malloc(sizeof(struct bn_tree));
  tree->root = node;
  return tree;
}

void bn_free(bn_tree_t t) {
  free(t);
}

bn_node_t bn_left_most(bn_node_t x) {
  bn_node_t y;
  #define bn_MOST(x, child, out) \
  out = x; \
  do { \
    while (out->child != NULL_PTR) { \
      out = out->child; \
    } \
  } while (0)
  bn_MOST(x, left, y);
  return y;
}

bn_node_t bn_right_most(bn_node_t x) {
  bn_node_t y;
  bn_MOST(x, right, y);
  return y;
}

bn_tree_t bn_transplant(bn_tree_t t, bn_node_t u, bn_node_t v) {
  if (u->top == NULL_PTR) {
    t->root = v;
  } else if (u == u->top->left) {
    u->top->left = v;
  } else {
    u->top->right = v;
  }
  if (v != NULL_PTR) {
    v->top = u->top;
  }
  return t;
}

bn_node_t bn_left_deepest_node(bn_node_t node) {
  for (;;) {
    if (node->left) {
      node = node->left;
    } else if (node->right) {
      node = node->right;
    } else {
      return node;
    }
  }
}

bn_node_t bn_next_postorder(bn_node_t node) {
  bn_node_t parent;
  if (!node)
    return NULL;
  parent = node->top;

  /* If we're sitting on node, we've already seen our children */
  if (parent && node == parent->left && parent->right) {
    /* If we are the parent's left node, go to the parent's right
     * node then all the way down to the left */
    return bn_left_deepest_node(parent->right);
  } else
    /* Otherwise we are the parent's right node, and the parent
     * should be next */
    return parent;
}

bn_node_t bn_first_postorder(bn_tree_t t) {
  if (!t->root)
    return NULL;

  return bn_left_deepest_node(t->root);
}

void bn_postorder_foreach(bn_tree_t t, bn_callback_t op, void *u) {
  bn_node_t n = bn_first_postorder(t);
  for (; n != NULL; n = bn_next_postorder(n)) {
    if (op(n, u)) {
      break;
    }
  }
}

bn_node_t bn_next_inorder(bn_node_t x)  {
  bn_node_t y;
#define BNS_NEAREST(x, left, right, out) \
  do { \
    if (x->right != NULL_PTR) { \
      out = bn_ ##left ##_most(x->right); \
    } else { \
      out = x->top; \
      while (out != NULL_PTR && x == out->right) {\
        x = out; \
        out = out->top; \
      } \
    } \
  } while (0)
  BNS_NEAREST(x, left, right, y);
  return y;
}

bn_node_t bn_prev_inorder(bn_node_t x) {
  bn_node_t y;
  BNS_NEAREST(x, right, left, y);
  return y;
}

void bn_inorder_lnr_foreach(bn_tree_t t, bn_callback_t op, void *u) {
  if (!t->root) {
    return;
  }
  bn_node_t nd = bn_left_most(t->root);
  for (; nd != NULL_PTR; nd = bn_next_inorder(nd)) {
    if (op(nd, u)) {
      break;
    }
  }
}

void bn_inorder_rnl_foreach(bn_tree_t t, bn_callback_t op, void *u) {
  if (!t->root) {
    return;
  }
  bn_node_t nd = bn_right_most(t->root);
  for (; nd != NULL_PTR; nd = bn_prev_inorder(nd)) {
    if (op(nd, u)) {
      break;
    }
  }
}

void bn_pprint_internal(bn_node_t root, bn_node_print_t nprt,
      int spaces, int step) {
  if (!root) {
    return;
  }
  if (root->right) {
    bn_pprint_internal(root->right, nprt, spaces + step, step);
  }
  for (int i = 0; i < spaces; ++i) {
    printf(" ");
  }
  nprt(root);
  if (root->left) {
    bn_pprint_internal(root->left, nprt, spaces + step, step);
  }
}

int g_bn_pprint_spaces_at_begin = 0;
int g_bn_pprint_step = 3;

void bn_pprint(bn_tree_t t, bn_node_print_t nprt) {
  bn_pprint_internal(t->root, nprt, g_bn_pprint_spaces_at_begin,
        g_bn_pprint_step);
}

#define BN_IMPL_ROTATION(t, x, left, right) \
bn_node_t bn_ ##left ##_rotate(bn_tree_t t, bn_node_t x) { \
  bn_node_t y = x->right; \
  x->right = y->left; \
  if (y->left != NULL_PTR) { \
    y->left->top = x; \
  } \
  y->top = x->top; \
  bn_change_child(x, y, x->top, t); \
  y->left = x; \
  x->top = y; \
  return y; \
}

BN_IMPL_ROTATION(t, x, left, right)
BN_IMPL_ROTATION(t, x, right, left)
#undef BN_IMPL_ROTATION