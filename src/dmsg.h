#pragma once
/*
	File: dmsg.h
	-Aim: to define data structure for DMSG
	-Dat: Mar 19th, 2017
	-Art: Lin Huan
	-Cls:
		[1] MSGVertex
		[2] MSGSubsume
		[3] MSGraph

		[4] MSGIterator
		[5] MSGVexIndex

		[6] DMSGVertex
		[7] DMSGHierarchy
*/

#include "bitseq.h"
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <fstream>

/*---- core data structure ----*/
class DMSGVertex;
class DMSGSubsume;
class DMSGraph;
class DMSGVexIndex;
/*---- algorithm data structure ----*/
class DMSGHierarchy;
class DMSGIterator;
	class DMSGIter_DownTop;
	class DMSGIter_TopDown;
	class DMSGIter_Random;
class DMSGraphBuilder;
/*---- IO data interfaces ----*/
class DMSGraphReader;
class DMSGraphWriter;

/* edge for MSG */
class DMSGSubsume {
private:
	/* node that edge comes from */
	DMSGVertex & source;
	/* node that edge comes to */
	DMSGVertex & target;
public:
	/* create a subsume edge by its source and target */
	DMSGSubsume(DMSGVertex & src, DMSGVertex & trg) : source(src), target(trg) {}
	/* create a subsume edge by copy */
	DMSGSubsume(const DMSGSubsume & edge) : source(edge.source), target(edge.target) {}
	/* deconstructor */
	~DMSGSubsume() {}
	/* get the source node where edge comes from */
	DMSGVertex & get_source() const { return source; }
	/* get the target node where edge comes to */
	DMSGVertex & get_target() const { return target; }
};
/* vertex for MSG */
class DMSGVertex {
protected:
	/* vertex identity */
	const long vid;
	/* list of edge to this node */
	std::list<DMSGSubsume> in_list;
	/* list of edge from this node */
	std::list<DMSGSubsume> ou_list;

	/* score vector of this node (cluster) */
	BitSeq vector;
	/* degree of this node (cluster) */
	BitSeq::size_t degree;

	/* create a distincted node by its id */
	DMSGVertex(long, const BitSeq &, BitSeq::size_t);
	/* deconstructor */
	~DMSGVertex();
public:
	/* get the id of this node */
	long get_id() const { return vid; }
	/* input degree */
	unsigned int in_degree() const { return in_list.size(); }
	/* output degree */
	unsigned int out_degree() const { return ou_list.size(); }
	/* get the input edges */
	const std::list<DMSGSubsume> & get_in_edges() const { return in_list; }
	/* get the output edges */
	const std::list<DMSGSubsume> & get_out_edges() const { return ou_list; }
	/* get score vector of this node */
	const BitSeq & get_vector() const { return vector; }
	/* get degree of this node vector */
	BitSeq::size_t get_degree() const { return degree; }

	friend class DMSGraphReader;	// create
	friend class DMSGraphBuilder;	// create
	friend class DMSGraph;			// delete
};
/* mutant subsumption graph */
class DMSGraph {
protected:
	/* set of roots (without ancestor) */
	std::set<long> roots;
	/* set of leafs (without children) */
	std::set<long> leafs;
	/* map from vertex id to their entity */
	std::map<long, DMSGVertex *> vertices;

	/* add a (new) vertex into the graph */
	void add(DMSGVertex &);
	/* link two vertex together, this will update roots and leafs */
	void link(DMSGVertex &, DMSGVertex &);
	/* clear all the nodes and edges in the graph */
	void clear();

	/* update the roots set */
	void update_roots();
	/* update the leafs set */
	void update_leafs();
public:
	/* create an empty graph */
	DMSGraph();
	/* clear the nodes in graph */
	~DMSGraph();

	/* get roots (without ancestor) */
	const std::set<long> & get_roots() const { return roots; }
	/* get leafs (without children) */
	const std::set<long> & get_leafs() const { return leafs; }
	/* get the number of vertices in graph */
	unsigned int number_of_vertices() const { return vertices.size(); }
	/* whether there is vertex referred by id */
	bool has_vertex(long id) const;
	/* get the vertex of id */
	const DMSGVertex & get_vertex(long id) const;

	friend class DMSGraphBuilder;
	friend class DMSGraphWriter;
	friend class DMSGraphReader;
	friend class DMSGIter_Random;
};
/* map from vertex-id to mutant-id(s) */
class DMSGVexIndex {
protected:
	/* map from vertex id to mutant id(s) */
	std::map<long, long> mid_vex;
	/* map from vertex id to node */
	std::map<long, DMSGVertex *> vid_vex;

	/* only add vertex into the index without linked mutant */
	void add(DMSGVertex &);
	/* link a new mutant id with vertex */
	void add(long, DMSGVertex &);
	/* clear the index from mutant to node */
	void clear();
public:
	/* create an empty index */
	DMSGVexIndex();
	/* clear the index */
	~DMSGVexIndex();
	
	/* whether there is node referring to the id */
	bool has_vertex(long vid) const;
	/* get the vertex of specified id */
	DMSGVertex & get_vertex(long vid) const;
	/* whether there is mutant in the index */
	bool has_mutant(long mid) const;
	/* get the id of node for mutant */
	long vertex_of_mutant(long mid) const;

	unsigned int number_of_mutants() const { return mid_vex.size(); }
	unsigned int number_of_vertices() const { return vid_vex.size(); }

	friend class DMSGraphBuilder;
	friend class DMSGraphWriter;
	friend class DMSGraphReader;
};
/* hierarchy for MSG vertex by their degrees */
class DMSGHierarchy {
protected:
	std::map<BitSeq::size_t, std::list<long> *> degree_map;
	std::vector<BitSeq::size_t> sort_list;

	/* add a new vertex into the hierarchy */
	void add(const DMSGVertex & vertex);
	/* sort the degrees in sort_list */
	void sort();
	/* clear the nodes in hierarchy */
	void clear();
public:
	/* create an empty hierarchy */
	DMSGHierarchy();
	/* clear all dynamically allocated list in hierarchy */
	~DMSGHierarchy();
	
	/* get the list */
	const std::vector<BitSeq::size_t> & get_degrees() const { return sort_list; }
	/* whether there are nodes to the specified degree */
	bool has_vertices(BitSeq::size_t) const;
	/* get the number of vertices at specified degree */
	unsigned int number_of_vertices(BitSeq::size_t) const;
	/* get the list of vertex id(s) for specified degree */
	const std::list<long> & get_vertices(BitSeq::size_t) const;
	/* get the number of levels */
	int number_of_levels() const { return sort_list.size(); }
	/* get the list of id(s) for nodes at specified level (location) */
	const std::list<long> & get_vertices_at(int) const;

	/* get total number of vertices in hierarchy */
	unsigned int number_of_vertices() const;

	friend class DMSGraphBuilder;
	friend class DMSGraphReader;
};
/* virtual class for iterator */
class DMSGIterator {
protected:
	const DMSGraph & graph;
	std::set<long> records;
	const DMSGVertex * nextptr;
	virtual void roll_next() { nextptr = nullptr; }
public:
	DMSGIterator(const DMSGraph & g) : graph(g), records(), nextptr(nullptr) {}
	virtual ~DMSGIterator() { records.clear(); }
	bool has_next() { return nextptr != nullptr; }
	const DMSGVertex * next() {
		const DMSGVertex * ans = nextptr;
		roll_next();
		return ans;
	}
};
/* iterate from leaf to root */
class DMSGIter_DownTop : public DMSGIterator {
protected:
	std::queue<long> qlist;
	void roll_next();
public:
	DMSGIter_DownTop(const DMSGraph & g) : DMSGIterator(g), qlist() {
		const std::set<long> & list = g.get_leafs();
		auto beg = list.begin(), end = list.end();
		while (beg != end) qlist.push(*(beg++));
		roll_next();
	}
	~DMSGIter_DownTop() { 
		while (!qlist.empty()) qlist.pop();
		records.clear(); 
	}
};
/* iterate nodes in DMSG from root to leaf */
class DMSGIter_TopDown : public DMSGIterator {
protected:
	std::queue<long> qlist;
	void roll_next();
public:
	DMSGIter_TopDown(const DMSGraph & g) : DMSGIterator(g), qlist() {
		const std::set<long> & list = g.get_roots();
		auto beg = list.begin(), end = list.end();
		while (beg != end) qlist.push(*(beg++));
		roll_next();
	}
	~DMSGIter_TopDown() {
		while (!qlist.empty()) qlist.pop();
		records.clear();
	}
};
/* iterate nodes in DMSG randomly */
class DMSGIter_Random : public DMSGIterator {
protected:
	std::map<long, DMSGVertex *>::const_iterator cursor, end;
	void roll_next();
public:
	DMSGIter_Random(const DMSGraph & g) : DMSGIterator(g) {
		cursor = g.vertices.begin();
		end = g.vertices.end();
		roll_next();
	}
	~DMSGIter_Random() {
		records.clear();
	}
};

/* builder for DMSG */
class DMSGraphBuilder {
protected:
	long useid;
	DMSGVexIndex & index_lib;
	DMSGraph & graph;
	DMSGHierarchy & hierarchy;
	char state;

	unsigned int compares;
public:
	DMSGraphBuilder(DMSGVexIndex &, DMSGraph &, DMSGHierarchy &);
	~DMSGraphBuilder();

	/* closed engine */
	static const char END = 0;
	/* after open(), before sort_nodes(), available for input_node() */
	static const char INP = 1;
	/* after sort_nodes(), before link_nodes(), available for link_nodes() */
	static const char SRT = 2;
	/* after link_nodes(), before close() */
	static const char CMP = 3;
	/* get current state */
	char get_state() const { return state; }

	/* start the engine for building DMSG, this will clear all the nodes in graph, index, and hierarchy */
	bool open();
	/* add a new node for specified vector */
	DMSGVertex * add_node(const KillVector &);
	/* add mutant index to the node */
	bool add_index(long, DMSGVertex &);
	/* sort the nodes in hierarchy */
	bool sort_nodes();
	/* build up the edges between nodes in graph */
	bool link_nodes(const char);
	/* close the engine for building DMSG */
	bool close();

	/* to determine the iterator to link nodes from down (leafs) to top (roots) */
	static const char DownTop = 0;
	/* to determine the iterator to link nodes from top (roots) to down (leafs) */
	static const char TopDown = 1;
	/* to determine the iterator to link nodes randomly */
	static const char Random = 2;

	/* get the number of comparisons between clusters */
	unsigned int get_comparisons() const { return compares; }

private:
	/* get a new iterator for nodes in DMSG (by strategy) -- which is dynamicaly created */
	DMSGIterator & get_iterator(char);
	/* tag the id(s) of nodes subsuming x (its ancestors) in search space */
	void tag_ancestors(const DMSGVertex &, std::set<long> &);
	/* tag the id(s) of nodes subsumed by x (its children) in search space */
	void tag_descendants(const DMSGVertex &, std::set<long> &);
	/* find the nodes in current graph directly subsumed by x, and put their id(s) into set */
	void find_direct_subsumed(const DMSGVertex &, std::set<long> &, char);
	/* link the nodes at ith level in hierarchy to the nodes in current graph */
	void link_level(int, char);
};

/* writer for DMSG */
class DMSGraphWriter {
public:
	/* create an initial writer for DMSG */
	DMSGraphWriter() : out() {}
	/* deconstructor */
	~DMSGraphWriter() { close(); }

	/* open the writer to target file where DMSG and index are maintained */
	void open(const std::string &);
	/* write DMSG and its index to specific data file (error if not opened) */
	void write(const DMSGraph &, const DMSGVexIndex &);
	/* close output stream if it's opened */
	void close();

private:
	/* output stream for writing graph */
	std::ofstream out;

	void begin_vertices(const DMSGraph &);
	void write_vertices(const DMSGraph &);
	void endin_vertices();
	void write_subsumes(const DMSGraph &);
	void endin_subsumes();
	void begin_indexlib(const DMSGVexIndex &);
	void write_indexlib(const DMSGVexIndex &);
	void endin_indexlib();
};
/* reader for DMSG */
class DMSGraphReader {
public:
	DMSGraphReader() : in() {}
	~DMSGraphReader() { close(); }

	void open(const std::string &);
	void read(DMSGraph &, DMSGVexIndex &, DMSGHierarchy &);
	void close();

private:
	std::ifstream in;

	void initial_graph(DMSGraph &, DMSGVexIndex &, DMSGHierarchy &);
	void read_vertices(DMSGVexIndex &, DMSGHierarchy &);
	void read_subsumes(DMSGVexIndex &, DMSGraph &);
	void read_indexlib(DMSGVexIndex &);
	void rebuild_graph(DMSGVexIndex &, DMSGraph &);
};