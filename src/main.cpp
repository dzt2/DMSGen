#include "text.h"
#include "dmsg.h"
#include <time.h>
#include <iostream>

/*
	-File : main.cpp
	-Date : Mar 3rd
	-Arth : Huan Lin
	-Usage:
		To execute DMSGen.exe by the following commands.
			DMSGen [bias] [tnum] [input] [output]?
		The lines in input file must be:
			${mutant_id} '[' ${length} ']' ':' {'t'${test_id}}*
*/

unsigned int generateDMSG(BitSeq::size_t, int, const std::string &, const std::string &);
void summaryOfDMSG(unsigned int, const DMSGVexIndex &, const DMSGHierarchy &, const DMSGraph &, std::ostream &);

/* generate DMSG from specified score function from input file, write it to the output file and return how many mutants are killed */
unsigned int generateDMSG(BitSeq::size_t bias, int testnum, const std::string &input, const std::string &output) {
	/* inputs */
	LineReader reader(input);
	KillVectorProducer producer(testnum, bias);

	/* outputs */
	DMSGraph graph; DMSGVexIndex index; DMSGHierarchy hierarchy;
	DMSGraphBuilder builder(index, graph, hierarchy);

	/* intermediate */
	BitTrieTree & tree = *(new BitTrieTree());
	clock_t t0, t1, t2, t3;

	std::cout << "Classify: "; t0 = clock();
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
	delete &tree; t1 = clock();
	std::cout << (t1 - t0) << " ms.\n";

	/* parse II: sort the hierarchy */
	std::cout << "Sort: ";
	builder.sort_nodes();
	t2 = clock();
	std::cout << (t2 - t1) << " ms.\n";

	/* parse III: link the nodes and put them into graph */
	std::cout << "Link: ";
	builder.link_nodes(DMSGraphBuilder::Random);

	/* parse IV: end to parse */
	builder.close(); t3 = clock();
	std::cout << (t3 - t2) << " ms.\n";

	/* write DMSG */
	std::cout << "Writing DMSG to " << output << "......";
	DMSGraphWriter writer;
	writer.open(output);
	writer.write(graph, index);
	writer.close();
	std::cout << "\n Complete\n\n";

	/* print outputs */
	summaryOfDMSG(killed, index, hierarchy, graph, std::cout);
	std::cout << std::endl;

	/* return */
	return killed;
}
void summaryOfDMSG(unsigned int killed, const DMSGVexIndex & index, 
	const DMSGHierarchy & hierarchy, const DMSGraph & graph, std::ostream & out) {
	
	out << "All-Mutations   \t" << index.number_of_mutants() << "\n";
	out << "Killed-Mutants  \t" << killed << "\n";
	out << "Number-Cluster  \t" << graph.number_of_vertices() << "\n";
	out << "Length-Hierarchy\t" << hierarchy.number_of_levels() << "\n";

	long id = 0, vnum = graph.number_of_vertices(); unsigned int edges = 0;
	while (id < vnum) {
		const DMSGVertex & vertex = graph.get_vertex(id++);
		edges += vertex.out_degree();
	}
	out << "Direct-Subsume \t" << edges << "\n";
}

int main(int argc, char * argv[]) {
	BitSeq::size_t bias = 0; int testnum = 0;
	std::string input, output;
	if (argc < 4) throw "Invalid arguments: ", argc;

	bias = std::stoi(argv[1]);
	testnum = std::stoi(argv[2]);
	input = argv[3];
	if (argc == 4) output = input + "_graph.dat";
	else output = argv[4];

	generateDMSG(bias, testnum, input, output);

	return 0;
}
