#include "text.h"
#include "dmsg.h"
#include <algorithm>

#include <iostream>

// DMSGVertex implement 
DMSGVertex::DMSGVertex(long id, const BitSeq & vec, BitSeq::size_t deg)
	: vid(id), vector(vec), degree(deg) {}
DMSGVertex::~DMSGVertex() {}

// DMSGraph implement
DMSGraph::DMSGraph() : roots(), leafs(), vertices() {}
DMSGraph::~DMSGraph() { clear(); }
bool DMSGraph::has_vertex(long id) const {
	return vertices.count(id) > 0;
}
const DMSGVertex & DMSGraph::get_vertex(long id) const {
	if (vertices.count(id) > 0) {
		auto ptr = vertices.find(id);
		return *(ptr->second);
	}
	else throw "Undefined vertex: ", id;
}
void DMSGraph::add(DMSGVertex & node) {
	long vid = node.vid;
	if (vertices.count(vid) == 0) {
		vertices[vid] = &node;
		if(node.in_degree() == 0) roots.insert(vid);
		if(node.out_degree() == 0) leafs.insert(vid);
	}
	else throw "Duplicated vid: ", vid;
}
void DMSGraph::link(DMSGVertex & source, DMSGVertex & target) {
	long sid = source.get_id(), tid = target.get_id();
	if (sid != tid) {
		DMSGSubsume edge(source, target);
		source.ou_list.push_front(edge);
		target.in_list.push_front(edge);
	}
	else throw "Invalid connection: ", sid, " to ", tid;
}
void DMSGraph::clear() {
	roots.clear(); leafs.clear();
	auto beg = vertices.begin(), end = vertices.end();
	while (beg != end) {
		DMSGVertex * vertex = (beg++)->second;
		if (vertex != nullptr) delete vertex;
	}
	vertices.clear();
}
static std::set<long> remove_set;
void DMSGraph::update_roots() {
	/* initialization */
	remove_set.clear();

	/* collect invalid root */
	auto beg = roots.begin(), end = roots.end();
	while (beg != end) {
		long vid = *(beg++);
		auto iter = vertices.find(vid);
		DMSGVertex & vex = *(iter->second);
		if (vex.in_degree() > 0)
			remove_set.insert(vid);
	}

	/* remove invalid roots from this->roots */
	beg = remove_set.begin(), end = remove_set.end();
	while (beg != end) {
		long vid = *(beg++);
		roots.erase(vid);
	}

	/* return */
	return;
}
void DMSGraph::update_leafs() {
	/* initialization */
	remove_set.clear();

	/* collect invalid root */
	auto beg = leafs.begin(), end = leafs.end();
	while (beg != end) {
		long vid = *(beg++);
		auto iter = vertices.find(vid);
		DMSGVertex & vex = *(iter->second);
		if (vex.out_degree() > 0)
			remove_set.insert(vid);
	}

	/* remove invalid roots from this->roots */
	beg = remove_set.begin(), end = remove_set.end();
	while (beg != end) {
		long vid = *(beg++);
		leafs.erase(vid);
	}

	/* return */
	return;
}

// DMSGVexIndex implement
DMSGVexIndex::DMSGVexIndex() : mid_vex(), vid_vex() {}
DMSGVexIndex::~DMSGVexIndex() { this->clear(); }
void DMSGVexIndex::add(DMSGVertex & node) {
	long vid = node.get_id();
	if (vid_vex.count(vid) == 0)
		vid_vex[vid] = &node;
}
void DMSGVexIndex::add(long mid, DMSGVertex & node) {
	long vid = node.get_id();
	if (vid_vex.count(vid) == 0)
		vid_vex[vid] = &node;
	if (mid_vex.count(mid) == 0) 
		mid_vex[mid] = vid;
	else throw "Duplicated mutant: ", mid;
}
void DMSGVexIndex::clear() { mid_vex.clear(); vid_vex.clear(); }
bool DMSGVexIndex::has_vertex(long vid) const {
	return vid_vex.count(vid) > 0;
}
DMSGVertex & DMSGVexIndex::get_vertex(long vid) const {
	if (vid_vex.count(vid) > 0) {
		auto iter = vid_vex.find(vid);
		return *(iter->second);
	}
	throw "Invalid vid: ", vid;
}
bool DMSGVexIndex::has_mutant(long mid) const {
	return mid_vex.count(mid) > 0;
}
long DMSGVexIndex::vertex_of_mutant(long mid) const {
	if (mid_vex.count(mid) > 0) {
		auto iter = mid_vex.find(mid);
		return iter->second;
	}
	throw "Invalid mid: ", mid;
}

// DMSGHierarchy implement 
DMSGHierarchy::DMSGHierarchy() : sort_list(), degree_map() {}
DMSGHierarchy::~DMSGHierarchy() { clear(); }
void DMSGHierarchy::add(const DMSGVertex & vec) {
	BitSeq::size_t degree = vec.get_degree();

	if (degree_map.count(degree) == 0) {
		degree_map[degree] = new std::list<long>();
		sort_list.push_back(degree);
	}

	auto iter = degree_map.find(degree);
	std::list<long> * list = iter->second;
	list->push_front(vec.get_id());
}
bool DMSGHierarchy::has_vertices(BitSeq::size_t degree) const {
	return degree_map.count(degree) > 0;
}
unsigned int DMSGHierarchy::number_of_vertices(BitSeq::size_t degree) const {
	if (degree_map.count(degree) == 0) return 0;
	else {
		auto iter = degree_map.find(degree);
		std::list<long> * list = iter->second;
		return list->size();
	}
}
const std::list<long> & DMSGHierarchy::get_vertices(BitSeq::size_t degree) const {
	if (degree_map.count(degree) == 0)
		throw "Undefined degree in hierarchy: ", degree;
	else {
		auto iter = degree_map.find(degree);
		return *(iter->second);
	}
}
void DMSGHierarchy::sort() {
	std::sort(sort_list.begin(), sort_list.end());
}
void DMSGHierarchy::clear() {
	auto beg = degree_map.begin(), end = degree_map.end();
	while (beg != end) {
		std::list<long> * list = (beg++)->second;
		if (list != nullptr) {
			list->clear(); delete list;
		}
	}
	sort_list.clear(); degree_map.clear();
}
const std::list<long> & DMSGHierarchy::get_vertices_at(int i) const {
	if (i < 0 || i > sort_list.size())
		throw "Invalid index: ", i;
	else {
		BitSeq::size_t degree = sort_list[i];
		auto iter = degree_map.find(degree);
		return *(iter->second);
	}
}
unsigned int DMSGHierarchy::number_of_vertices() const {
	unsigned int sum = 0; 
	auto beg = degree_map.begin(), end = degree_map.end();
	while (beg != end) {
		std::list<long> * list = (beg++)->second;
		sum += list->size();
	}
	return sum;
}

// implement iterator for DMSG
void DMSGIter_DownTop::roll_next() {
	nextptr = nullptr; long id;
	while (!qlist.empty() && nextptr == nullptr) {
		id = qlist.front(); qlist.pop();
		if (records.count(id) > 0) continue;
		else records.insert(id);

		nextptr = &(graph.get_vertex(id));

		const std::list<DMSGSubsume> & edges = nextptr->get_in_edges();
		auto beg = edges.begin(), end = edges.end();
		while (beg != end) {
			long sid = ((beg++)->get_source()).get_id();
			if (records.count(sid) > 0 
				|| !graph.has_vertex(sid))
				continue;
			else qlist.push(sid);
		}
	}
}
void DMSGIter_TopDown::roll_next() {
	nextptr = nullptr; long id;
	while (!qlist.empty() && nextptr == nullptr) {
		id = qlist.front(); qlist.pop();
		if (records.count(id) > 0) continue;
		else records.insert(id);

		nextptr = &(graph.get_vertex(id));

		const std::list<DMSGSubsume> & edges = nextptr->get_out_edges();
		auto beg = edges.begin(), end = edges.end();
		while (beg != end) {
			long tid = ((beg++)->get_target()).get_id();
			if (records.count(tid) > 0 || !graph.has_vertex(tid))
				continue;
			else qlist.push(tid);
		}
	}
}
void DMSGIter_Random::roll_next() {
	nextptr = nullptr;
	if (cursor != end) {
		long id = (cursor++)->first;
		nextptr = &graph.get_vertex(id);
	}
}

// builder for DMSG
DMSGraphBuilder::DMSGraphBuilder(DMSGVexIndex & index, DMSGraph & g, DMSGHierarchy & h)
	: useid(0L), index_lib(index), graph(g), hierarchy(h), state(END), compares(0) {}
DMSGraphBuilder::~DMSGraphBuilder() {}
bool DMSGraphBuilder::open() {
	if (state != END)
		throw "Invalid access: ", state;
	else {
		useid = 0L;
		state = INP;
		index_lib.clear();
		graph.clear();
		hierarchy.clear();
	}
}
DMSGVertex * DMSGraphBuilder::add_node(const KillVector & vec) {
	DMSGVertex & vertex = *(new DMSGVertex(
		useid++, vec.get_vector(), vec.get_quantity()));
	index_lib.add(vec.get_mutant_ID(), vertex);
	hierarchy.add(vertex);
	return &vertex;
}
bool DMSGraphBuilder::add_index(long mid, DMSGVertex & vertex) {
	index_lib.add(mid, vertex);
	return true;
}
bool DMSGraphBuilder::sort_nodes() {
	if (state != INP)
		throw "Invalid access: ", state;
	else {
		hierarchy.sort();
		state = SRT;
		return true;
	}
}
bool DMSGraphBuilder::link_nodes(const char strategy) {
	// validation 
	if (state != SRT)
		throw "Invalid access: ", state;

	/* link node from down to top */
	int n = hierarchy.number_of_levels();
	for (int i = n - 1; i >= 0; i--)
		this->link_level(i, strategy);
	
	// final works to translate 
	state = CMP;
	return true;
}
bool DMSGraphBuilder::close() {
	if (state != CMP)
		throw "Invalid access: ", state;
	else {
		state = END; return true;
	}
}
DMSGIterator & DMSGraphBuilder::get_iterator(char strategy) {
	switch (strategy) {
	case DownTop:
		return *(new DMSGIter_DownTop(graph));
	case TopDown:
		return *(new DMSGIter_TopDown(graph));
	case Random:
		return *(new DMSGIter_Random(graph));
	default:
		throw "Invalid strategy: ", strategy;
	}
}

static std::queue<long> _node_queue;
static std::set<long> _node_records;
void DMSGraphBuilder::tag_ancestors(const DMSGVertex & x, std::set<long> & idset) {
	/* clear the queue for search */
	while (!_node_queue.empty())
		_node_queue.pop();
	_node_records.clear();

	/* put x into the queue */
	_node_queue.push(x.get_id());

	/* iterate the nodes from leafs to roots */
	while (!_node_queue.empty()) {
		long nid = _node_queue.front();
		_node_queue.pop(); 
		
		const DMSGVertex & vertex = graph.get_vertex(nid);
		const std::list<DMSGSubsume> & edges = vertex.get_in_edges();
		auto beg = edges.begin(), end = edges.end();
		while (beg != end) {
			const DMSGSubsume & edge = *(beg++);
			long sid = edge.get_source().get_id();

			if (_node_records.count(sid) > 0 
				|| !graph.has_vertex(sid)
				|| idset.count(sid) > 0)
				continue;
			else {
				idset.insert(sid); 
				_node_records.insert(sid);
				_node_queue.push(sid);
			}
		}
	}

	/* return */ return;
}
void DMSGraphBuilder::tag_descendants(const DMSGVertex & x, std::set<long> &idset) {
	/* clear the queue for search */
	while (!_node_queue.empty())
		_node_queue.pop();
	_node_records.clear();

	/* put x into the queue */
	_node_queue.push(x.get_id());

	/* iterate the nodes from leafs to roots */
	while (!_node_queue.empty()) {
		long nid = _node_queue.front();
		_node_queue.pop();

		const DMSGVertex & vertex = graph.get_vertex(nid);
		const std::list<DMSGSubsume> & edges = vertex.get_out_edges();
		auto beg = edges.begin(), end = edges.end();
		while (beg != end) {
			const DMSGSubsume & edge = *(beg++);
			long tid = edge.get_target().get_id();

			if (_node_records.count(tid) > 0 
				|| !graph.has_vertex(tid)
				|| idset.count(tid) > 0)
				continue;
			else {
				idset.insert(tid); 
				_node_records.insert(tid);
				_node_queue.push(tid);
			}
		}

	}

	/* return */ return;
}
static std::set<long> visitSpace;
static std::set<long> childSet;
void DMSGraphBuilder::find_direct_subsumed(const DMSGVertex & x, std::set<long> & DS, char strategy) {
	/* initialization */
	DMSGIterator & iter = this->get_iterator(strategy);
	visitSpace.clear(); DS.clear(); childSet.clear();

	/* iterate each node in graph*/
	const DMSGVertex * y; long yid; 
	while (iter.has_next()) {
		/* get next node in space */
		y = iter.next(); 
		if (y == nullptr) continue;

		/* record the node in visits */
		yid = y->get_id();
		if (visitSpace.count(yid) > 0) 
			continue;
		else visitSpace.insert(yid);

		/* count for the comparisons */ compares++; //cmps++;

		/* if x subsumes y, then remove its descendants from visitSpace and DS */
		if (x.get_vector().subsume(y->get_vector())) {
			/* get the descendants for y */
			childSet.clear(); 
			this->tag_descendants(*y, childSet);
			

			/* DS = DS - descendants; VS = VS + descendants; */
			auto beg = childSet.begin(), end = childSet.end();
			while (beg != end) {
				long cid = *(beg++);
				DS.erase(cid);
				visitSpace.insert(cid);
			}

			/* insert yid into DS temporarly */
			DS.insert(yid);
		}
		/* otherwise, remove its ancestors from visitSpace */
		else {
			this->tag_ancestors(*y, visitSpace);
		}
	} /* end while nodes in DMSG by now */

	//std::cout << "{" << x.get_id() << "}: " << its << "\t" << cmps << "\t" << ts << "\t" << fs << "\tat: " << graph.number_of_vertices() << "/" << index_lib.number_of_vertices() << std::endl;
	delete &iter;
	/* return */ return;
}
void DMSGraphBuilder::link_level(int i, char strategy) {
	/* initialization */
	const std::list<long> & level = hierarchy.get_vertices_at(i);
	auto beg = level.begin(), end = level.end();

	/* link nodes at level to the nodes in current graph */
	long xid; std::set<long> DS;
	while (beg != end) {
		/* get next node in level i */
		xid = *(beg++);
		DMSGVertex & x = index_lib.get_vertex(xid);

		/* compute its direct subsumed nodes in DMSG */
		find_direct_subsumed(x, DS, strategy);

		/* link x to directly subsumed nodes in DMSG */
		auto beg = DS.begin(), end = DS.end();
		while (beg != end) {
			DMSGVertex & y = index_lib.get_vertex(*(beg++));
			graph.link(x, y);
		}
	} /* end while link nodes */

	/* put nodes into the graph */
	beg = level.begin(), end = level.end();
	while (beg != end) {
		xid = *(beg++);
		DMSGVertex & x = index_lib.get_vertex(xid);
		graph.add(x);
	}

	/* update the roots in DMSG */
	graph.update_roots();

	/* return */ return;
}

// writer for DMSG
void DMSGraphWriter::open(const std::string & path) {
	close();
	out.open(path, std::ios::binary);
}
void DMSGraphWriter::close() {
	if (out.is_open())
		out.close();
}
void DMSGraphWriter::write(const DMSGraph & graph, const DMSGVexIndex & index) {
	/* validation */
	if (!out.is_open())
		throw "Invalid access: not opened stream";

	/* output vertices in DMSG */
	begin_vertices(graph);
	write_vertices(graph);
	endin_vertices();

	/* output edges in DMSG */
	write_subsumes(graph);
	endin_subsumes();

	/* output indexlib */
	begin_indexlib(index);
	write_indexlib(index);
	endin_indexlib();

	/* return */
	return;
}
void DMSGraphWriter::begin_vertices(const DMSGraph & graph) {
	unsigned int vnum = graph.number_of_vertices();
	out.write((char *)(&vnum), sizeof(unsigned int));
}
void DMSGraphWriter::write_vertices(const DMSGraph & graph) {
	auto beg = graph.vertices.begin(), end = graph.vertices.end();
	while (beg != end) {
		/* get next vertex from DMSG */
		DMSGVertex * nodeptr = (beg++)->second;
		if (nodeptr == nullptr) continue;

		/* derive its basic information */
		long vid = nodeptr->get_id();
		BitSeq::size_t degree = nodeptr->get_degree();
		const BitSeq & vector = nodeptr->get_vector();
		BitSeq::size_t bitnum = vector.bit_number();
		const byte * bytes = vector.get_bytes();

		/* output elements in vertex */
		out.write((char *)(&vid), sizeof(long));
		out.write((char *)(&degree), sizeof(BitSeq::size_t));
		out.write((char *)(&bitnum), sizeof(BitSeq::size_t));
		out.write((char *)(bytes), sizeof(byte) * vector.byte_number());
	}
}
void DMSGraphWriter::endin_vertices() {
	long vid = -1L; 
	BitSeq::size_t degree = 0, bitnum = 0;
	out.write((char *)(&vid), sizeof(long));
	out.write((char *)(&degree), sizeof(BitSeq::size_t));
	out.write((char *)(&bitnum), sizeof(BitSeq::size_t));
}
void DMSGraphWriter::write_subsumes(const DMSGraph & graph) {
	auto beg = graph.vertices.begin(), end = graph.vertices.end();
	while (beg != end) {
		/* get next vertex from DMSG */
		DMSGVertex * nodeptr = (beg++)->second;
		if (nodeptr == nullptr) continue;
		else if (nodeptr->out_degree() == 0) continue;

		/* get source id and output */
		long sid = nodeptr->get_id();
		unsigned int out_degree = nodeptr->out_degree();
		out.write((char *)(&sid), sizeof(long));
		out.write((char *)(&out_degree), sizeof(unsigned int));

		/* get output edges */
		auto edges = nodeptr->get_out_edges();
		auto beg = edges.begin(), end = edges.end();
		/* write each edge into data file */
		while (beg != end) {
			const DMSGSubsume & edge = *(beg++);
			const DMSGVertex & target = edge.get_target();
			long tid = target.get_id();

			out.write((char *)(&tid), sizeof(long));
		}

		/* return */ return;
	}
}
void DMSGraphWriter::endin_subsumes() {
	long sid = -1; unsigned int out_degree = 0;
	out.write((char *)(&sid), sizeof(long));
	out.write((char *)(&out_degree), sizeof(unsigned int));
}
void DMSGraphWriter::begin_indexlib(const DMSGVexIndex & index) {
	unsigned int mnum = index.number_of_mutants();
	out.write((char *)(&mnum), sizeof(unsigned int));
}
void DMSGraphWriter::write_indexlib(const DMSGVexIndex & index) {
	auto beg = index.mid_vex.begin(), end = index.mid_vex.end();
	while (beg != end) {
		long mid = beg->first;
		long vid = beg->second;
		beg++;

		out.write((char *)(&mid), sizeof(long));
		out.write((char *)(&vid), sizeof(long));
	}
}
void DMSGraphWriter::endin_indexlib() {
	long mid = -1, vid = -1;
	out.write((char *)(&mid), sizeof(long));
	out.write((char *)(&vid), sizeof(long));
}

// reader for DMSG
void DMSGraphReader::open(const std::string & path) {
	close();
	in.open(path, std::ios::binary);
}
void DMSGraphReader::close() {
	if (in.is_open())
		in.close();
}
void DMSGraphReader::read(DMSGraph & graph, DMSGVexIndex & index, DMSGHierarchy & hierarchy) {
	/* validation */
	if (!in.is_open())
		throw "Input stream not opened";

	/* initialization */
	initial_graph(graph, index, hierarchy);

	/* read vertices, subsumption and index */
	read_vertices(index, hierarchy);
	read_subsumes(index, graph);
	read_indexlib(index);

	/* reconstruct the graph */
	rebuild_graph(index, graph);
}
void DMSGraphReader::initial_graph(DMSGraph & graph, DMSGVexIndex & index, DMSGHierarchy & hierarchy) {
	graph.clear();
	index.clear();
	hierarchy.clear();
}
void DMSGraphReader::read_vertices(DMSGVexIndex & index, DMSGHierarchy & hierarchy) {
	/* consume vertex number title */
	unsigned int vnum;
	in.read((char *)(&vnum), sizeof(unsigned int));

	long vid; BitSeq::size_t degree, bitnum;
	while ((vnum--) > 0) {
		/* consume header of vertex */
		in.read((char *)(&vid), sizeof(long));
		in.read((char *)(&degree), sizeof(BitSeq::size_t));
		in.read((char *)(&bitnum), sizeof(BitSeq::size_t));

		if (vid == -1L && degree == 0 && bitnum == 0)
			throw "Data file errors!";

		/* consume bytes for vector */
		BitSeq vector(bitnum);
		in.read((char *)(vector.get_bytes()), sizeof(byte) * vector.byte_number());

		/* create vertex and put into index and hierarchy */
		DMSGVertex * node = new DMSGVertex(vid, vector, degree);
		index.add(*node); hierarchy.add(*node);
	}
	/* consume tail of vertex */
	in.read((char *)(&vid), sizeof(long));
	in.read((char *)(&degree), sizeof(BitSeq::size_t));
	in.read((char *)(&bitnum), sizeof(BitSeq::size_t));
}
void DMSGraphReader::read_subsumes(DMSGVexIndex & index, DMSGraph & graph) {
	/* consume subsumption until (-1L, 0U) */
	long sid, tid; unsigned int degree;
	while (true) {
		/* consume subsume header */
		in.read((char *)(&sid), sizeof(long));
		in.read((char *)(&degree), sizeof(unsigned int));
		if (sid == -1L && degree == 0) break;

		/* consume targets id */
		DMSGVertex & x = index.get_vertex(sid);
		while ((degree--) > 0) {
			in.read((char *)(&tid), sizeof(long));
			DMSGVertex & y = index.get_vertex(tid);
			graph.link(x, y);
		}
	}

	/* return */ return;
}
void DMSGraphReader::read_indexlib(DMSGVexIndex & index) {
	/* consume index header */
	unsigned mnum; long mid, vid;
	in.read((char *)(&mnum), sizeof(unsigned int));

	/* consume index body */
	while ((mnum--) > 0) {
		in.read((char *)(&mid), sizeof(long));
		in.read((char *)(&vid), sizeof(long));
		if (mid == -1L && vid == -1L)
			throw "Data errors!";

		/* link mutant to the vertex */
		DMSGVertex & vertex = index.get_vertex(vid);
		index.add(mid, vertex);
	}

	/* consume index tail */
	in.read((char *)(&mid), sizeof(long));
	in.read((char *)(&vid), sizeof(long));
}
void DMSGraphReader::rebuild_graph(DMSGVexIndex & index, DMSGraph & graph) {
	auto beg = index.vid_vex.begin(), end = index.vid_vex.end();
	while (beg != end) {
		DMSGVertex & node = *(beg->second);
		graph.add(node);
	}
}

// testers 
void printDMSG(const DMSGVexIndex & index, const DMSGraph & graph, 
	const DMSGHierarchy & hierarchy, std::ostream & out) {
	/* title */
	out << "Number of mutants: " << index.number_of_mutants() << "\n";
	out << "Number of vertices: " << graph.number_of_vertices() << "\n";
	out << "Number of levels: " << hierarchy.number_of_levels() << "\n";

	int levels = hierarchy.number_of_levels();
	for (int i = 0; i < levels; i++) {
		/* get the ith level */
		auto list = hierarchy.get_vertices_at(i);
		auto beg = list.begin(), end = list.end();

		/* derive nodes */
		out << "Level (" << i << ")\n\t";
		while (beg != end) {
			long vid = *(beg++);
			out << vid << "; ";
		}
		out << "\n";
	}
	out << "\n";

	out << "Edges: \n"; unsigned int edges = 0;
	unsigned int vnum = graph.number_of_vertices();
	for (long i = 0; i < vnum; i++) {
		const DMSGVertex & x = graph.get_vertex(i);
		out << "\t" << x.get_id() << " --> ";

		auto out_edges = x.get_out_edges();
		auto beg = out_edges.begin(), end = out_edges.end();
		while (beg != end) {
			DMSGSubsume & edge = *(beg++);
			DMSGVertex & target = edge.get_target();
			out << target.get_id() << "; ";
			edges++;
		}
		
		out << "\n";
	}
}
void efficiencyAnalysis(const DMSGVexIndex & index, const DMSGHierarchy & hierarchy,
	const DMSGraph & graph, DMSGraphBuilder & builder) {
	std::cout << "Number of mutants: " << index.number_of_mutants() << std::endl;
	std::cout << "Number of vertice: " << graph.number_of_vertices() << std::endl;
	std::cout << "Number of levels:  " << hierarchy.number_of_levels() << std::endl;

	unsigned int T0, T1, T2, T3, T4;

	T0 = index.number_of_mutants();
	T0 = T0 * (T0 - 1) / 2;

	T1 = graph.number_of_vertices();
	T1 = T1 * (T1 - 1) / 2;

	T2 = 0; unsigned int sum = 0;
	int levels = hierarchy.number_of_levels();
	for (int i = levels - 1; i >= 0; i--) {
		auto list = hierarchy.get_vertices_at(i);
		T2 += sum * list.size();
		sum += list.size();
	}

	T3 = builder.get_comparisons();

	T4 = 0;
	unsigned int vnum = graph.number_of_vertices();
	for (long i = 0; i < vnum; i++) {
		const DMSGVertex & x = graph.get_vertex(i);
		auto edges = x.get_out_edges();
		T4 += edges.size();
	}

	std::cout << "Number of edges: " << T4 << std::endl;

	std::cout << "\tT0 = " << T0 << "\n";
	std::cout << "\tT1 = " << T1 << "\n";
	std::cout << "\tT2 = " << T2 << "\n";
	std::cout << "\tT3 = " << T3 << "\n";
	std::cout << "\tT4 = " << T4 << std::endl;
}
void printDMSGString(const DMSGVexIndex & index, DMSGraph & graph, std::ostream & out) {
	out << "Index-Map: \n";
	long mid = 0, end = index.number_of_mutants();
	while (mid++ < end) {
		if (index.has_mutant(mid)) {
			long vid = index.vertex_of_mutant(mid);
			out << "\t" << mid << "\t" << vid << "\n";
		}
	}
	out << "\n";

	out << "Graph Map: \n";
	long vnum = graph.number_of_vertices(), vid = 0;
	while (vid < vnum) {
		const DMSGVertex & vertex = graph.get_vertex(vid);
		out << vertex.get_id() << " : ";

		const std::list<DMSGSubsume> & out_edges = vertex.get_out_edges();
		auto beg = out_edges.begin(), end = out_edges.end();
		while (beg != end) {
			const DMSGSubsume & edge = *(beg++);
			out << edge.get_target().get_id() << " ";
		}
		out << "\n";

		vid++;
	}

	out << std::endl;
}
void genDMSG(BitSeq::size_t bias, int testnum, std::string & program) {
	/* inputs */
	std::string infile = program + "_result.txt";
	LineReader reader(infile); 
	KillVectorProducer producer(testnum, bias);

	/* outputs */
	DMSGraph graph; DMSGVexIndex index; DMSGHierarchy hierarchy;
	DMSGraphBuilder builder(index, graph, hierarchy);

	/* intermediate */
	BitTrieTree & tree = *(new BitTrieTree());

	std::cout << "Classify...\n";
	/* parse I: create nodes and index from mutants to them */
	builder.open(); unsigned int killed = 0;
	while (reader.hasNext()) {
		/* get the next line for kill-vector */
		std::string line = reader.next();
		const KillVector * vec = producer.produce(line);
		if (vec == nullptr) continue;

		/* calculate the killed mutants */
		if (vec->get_quantity() > 0) killed++;
		
		/* get the leaf for this vector */
		BitTrie * leaf = tree.insert_vector(vec->get_vector());
		if (leaf == nullptr) throw "Interpreting error!";
		/* The first time it is created */
		if (leaf->get_data() == nullptr) {
			DMSGVertex * vex = builder.add_node(*vec);
			leaf->set_data(vex);
		}
		/* The second or other time is only linked to mutant */
		else {
			DMSGVertex & vex = *((DMSGVertex *)(leaf->get_data()));
			builder.add_index(vec->get_mutant_ID(), vex);
		}

		/* continue for the next */
		delete vec;
	}
	/* release the trie tree */
	delete &tree; 

	/* parse II: sort the hierarchy */
	std::cout << "Sort...\n ";
	builder.sort_nodes();

	/* parse III: link the nodes and put them into graph */
	std::cout << "Link...\n ";
	builder.link_nodes(DMSGraphBuilder::Random);

	/* parse IV: end to parse */ 
	builder.close();

	/* print outputs */
	std::cout << "\nKilled Mutants: " << killed << std::endl;
	efficiencyAnalysis(index, hierarchy, graph, builder);
	

	/* write DMSG */
	DMSGraphWriter writer;
	writer.open(program + "_graph.dat");
	writer.write(graph, index);
	writer.close();

	/* return */return;
}
