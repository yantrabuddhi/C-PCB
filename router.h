#ifndef ROUTER_H
#define ROUTER_H

#include "mymath.h"
#include "layer.h"
#include <map>
#include <unordered_map>
#include <set>

class pcb;
class net;

struct terminal
{
	bool operator==(const terminal &t) const {
		return std::tie(t.m_radius, t.m_gap, t.m_term, t.m_shape) == std::tie(m_radius, m_gap, m_term, m_shape); }
	float m_radius;
	float m_gap;
	point_3d m_term;
	points_2d m_shape;
};
typedef std::vector<terminal> terminals;

//grid node and collections
struct node
{
	bool operator<(const node &n) const {
		return std::tie(n.m_x, n.m_y, n.m_z) < std::tie(m_x, m_y, m_z); }
	bool operator==(const node &n) const {
		return std::tie(n.m_x, n.m_y, n.m_z) == std::tie(m_x, m_y, m_z); }
	int m_x;
	int m_y;
	int m_z;
};
typedef std::vector<node> nodes;
typedef std::vector<nodes> nodess;
typedef std::set<node> node_set;

namespace std {
	template <>
	struct hash<node>
	{
		auto operator()(const node& n) const
		{
			return (std::hash<int>()(n.m_x)
					^ std::hash<int>()(n.m_y)
					^ std::hash<int>()(n.m_z));
		}
	};
}

//netlist structures
typedef std::vector<point_3d> path;
typedef std::vector<path> paths;

struct track
{
	float m_radius;
	float m_via;
	float m_gap;
	terminals m_terms;
};
typedef std::vector<track> tracks;

struct output
{
	float m_radius;
	float m_via;
	float m_gap;
	terminals m_terms;
	paths m_paths;
};
typedef std::vector<output> outputs;

//sortable node
struct sort_node
{
	float m_mark;
	node m_node;
};
typedef std::vector<sort_node> sort_nodes;

typedef std::vector<net> nets;
typedef float (*dfunc_t)(const point_3d &, const point_3d &);

//net object
class net
{
public:
	net(const terminals &terms, float radius, float via, float gap, pcb *pcb);
	bool route();
	void print_net();
	void remove();
	void shuffle_topology();

	int m_area;
	float m_radius;
	nodess m_paths;
	terminals m_terminals;
	layer::aabb m_bbox;

private:
	void add_terminal_collision_lines();
	void sub_terminal_collision_lines();
	void add_paths_collision_lines();
	void sub_paths_collision_lines();
	nodess optimise_paths(const nodess &paths);
	std::pair<nodes, bool> backtrack_path(const node_set &vis, const node &end,
		 								float radius, float via, float gap);

	pcb *m_pcb;
	float m_via;
	float m_gap;
};

//pcb class
class pcb
{
public:
	//dimensions of pcb board in grid points/layers
	struct dims
	{
		int m_width;
		int m_height;
		int m_depth;
	};

	pcb(const dims &dims, const nodess &rfvs, const nodess &rpvs,
		dfunc_t dfunc, int res, int verb, int quant, int viascost);
	~pcb();
	auto get_node(const node &n);
	void add_track(track &t);
	bool route(float timeout);
	int cost();
	void increase_quantization();
	void print_pcb();
	void print_netlist();
	void print_stats();
	point_3d grid_to_space_point(const node &n);
	nodes &all_not_shorting(const nodes &gather, const node &n, float radius, float gap);
	nodes &all_nearer_sorted(const nodess &vec, const node &n, dfunc_t dfunc);
	void mark_distances(const nodess &vec, float radius, float via, float gap,
							const node_set &starts, const nodes &ends);
	void unmark_distances();

	int m_resolution;
	int m_quantization;
	int m_depth;
	std::map<node, point_3d> m_deform;
	layers m_layers;
	nodess m_routing_flood_vectors;
	nodess m_routing_path_vectors;
	dfunc_t m_dfunc;

private:
	void set_node(const node &n, unsigned int value);
	sort_nodes &all_marked(const nodess &vec, const node &n);
	nodes &all_not_marked(const nodess &vec, const node &n);
	void reset_areas();
	void shuffle_netlist();
	int hoist_net(int n);
	void remove_netlist();

	int m_width;
	int m_height;
	int m_stride;
	int m_verbosity;
	int m_viascost;
	nets m_netlist;
	std::vector<unsigned int> m_nodes;
};

#endif
