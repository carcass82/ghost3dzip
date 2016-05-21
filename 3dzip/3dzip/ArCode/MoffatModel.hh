#pragma once
#include <vector>

class MoffatModel
{
    std::vector<unsigned> sym_to_pos, pos_to_sym, c, tree_c;
    unsigned tot;

    static unsigned parent_of(unsigned node) {
	return (node - 1) >> 1;
    }
    static unsigned left_son_of(unsigned node) {
	return (node << 1) + 1;
    }
    static unsigned right_son_of(unsigned node) {
	return (node + 1) << 1;
    }
    static bool is_left(unsigned node) {
	return (node & 1) == 1;
    }
    static bool is_right(unsigned node) {
	return is_left(node) == false;
    }

    void reorganise(unsigned s) {
	unsigned node = sym_to_pos.at(s), swap = node;
	while (swap > 0 && (c[swap] == c[swap - 1]))
	    --swap;
	if (node == swap)
	    return;
	unsigned f = pos_to_sym[swap];
	sym_to_pos[f] = node;
	sym_to_pos[s] = swap;
	pos_to_sym[node] = f;
	pos_to_sym[swap] = s;
    }

public:
    MoffatModel(int n):
	sym_to_pos(n),
	pos_to_sym(n),
	c(n),
	tree_c(n),
	tot(0)
    {
	for (int i = 0; i < n; ++i) {
	    sym_to_pos[i] = i;
	    pos_to_sym[i] = i;
	}
	for (unsigned i = 0; i < n; ++i)
	    count(i);
    }
    unsigned freq(const unsigned s) {
	return c[sym_to_pos.at(s)];
    }
    void count(const unsigned s) {
	reorganise(s);
	unsigned node = sym_to_pos[s];
	++c[node];
	++tot;
	while (node > 0) {
	    unsigned parent = parent_of(node);
	    if (is_left(node))
		++tree_c[parent];
	    node = parent;
	}
    }
    unsigned size() const {
	return c.size();
    }
    unsigned low(unsigned s) const {
	unsigned node = sym_to_pos.at(s);
	unsigned lbnd = tree_c[node];
	while (node > 0) {
	    unsigned parent = parent_of(node);
	    if (is_right(node))
		lbnd += tree_c[parent] + c[parent];
	    node = parent;
	}
	return lbnd;
    }
    unsigned high(unsigned s) const {
	return low(s) + c[s];
    }
    unsigned range() const {
	return tot;
    }
    unsigned find(const unsigned code) const {
	unsigned node = 0, lbnd = 0;
	bool found = false;
	while (found == false) {
	    if (lbnd + tree_c[node] > code)
		node = left_son_of(node);
	    else if (code >= lbnd + tree_c[node] + c[node]) {
		lbnd += tree_c[node] + c[node];
		node = right_son_of(node);
	    } else {
		lbnd += tree_c[node];
		found = true;
	    }
	}
	return pos_to_sym[node];
    }
};
