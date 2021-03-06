#pragma once

/*
	File: bitseq.h
	-Aim: to define basic interfaces for bit sequence
	-Dat: Mar 12th, 2017
	-Art: Lin Huan
	-Cls:
		[0] bit, byte;
		[1] class BitSeq;
		[2] class KillVector;
		[3] class BitTrie;
		[4] class BitTrieTree;
*/

#include <string>
#include <list>

/* a bit is either 0 (false) or 1 (true) */
typedef bool bit;
/* byte is a unsigned char (8bit) */
typedef unsigned char byte;

/* bit-0 is represented as false */
static const bit BIT_0 = false;
/* bit-1 is represented as true */
static const bit BIT_1 = true;
/* used to mask to retrieve bit from bit sequence */
static const byte BIT_LOC[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

// declarations
class BitSeq;
class KillVector;
class KillVectorProducer;
class BitTrie;
class BitTrieTree;

/* Sequence of bits */
class BitSeq {
public:
	/* integer to access bit in BitSeq */
	typedef unsigned int size_t;

	/* construct from another bit sequence */
	BitSeq(const BitSeq &);
	/* construct a all-zero bit sequence of specified length */
	BitSeq(size_t);
	/* release dynamically allocated bytes in the sequence */
	~BitSeq();

	/* get the number of bits occupied by the sequence */
	size_t bit_number() const;
	/* get the ith bit from sequence */
	bit get_bit(size_t) const;
	/* set the ith bit in sequence */
	void set_bit(size_t, bit);

	/* update this sequence to another conjunction */
	void conjunct(const BitSeq &);
	/* whether this sequence subsumes another */
	bool subsume(const BitSeq &) const;

	/* converse the sequence to string */
	std::string to_string() const;

	/* number of bytes occupied by this sequence */
	int byte_number() const;
	/* sequence of bytes to store bit-sequence */
	byte * get_bytes() const;
	/* set the bytes in sequence with specified bytes (from 0 to max(length, slength) */
	void set_bytes(const byte *, size_t); 

	/* get the sub-bit-sequence within given range */
	BitSeq subseq(size_t, size_t) const;
private:
	/* number of bits */
	size_t bit_num;
	/* length of bytes */
	size_t length;
	/* bytes where bits are maintained */
	byte * bytes;
};
/* vector to represent tests that kill mutant */
class KillVector {
public:
	/* create a kill-vector for mutant mid with length testnum */
	KillVector(long mid, BitSeq::size_t testnum, const std::list<BitSeq::size_t> & killset);
	/* release memory for vector in the kill-vector */
	~KillVector();

	/* get the mutant id */
	long get_mutant_ID() const;
	/* get the number of tests kill this mutant (number of bit-1 in sequence) */
	int get_quantity() const;
	/* get the number of tests (or number of bits in vector) */
	unsigned int get_test_number() const;
	/* get the vector of this mutant */
	const BitSeq & get_vector() const;

private:
	/* mutant identifier */
	long mid;
	/* number of bit-1 in vector */
	int quantity;
	/* bit-sequence to present the kill-set */
	BitSeq vector;
	
};
/* to produce kill-vector by interpreting line text from results */
class KillVectorProducer {
private:
	BitSeq::size_t bias;
	BitSeq::size_t testnum;
	std::list<BitSeq::size_t> killset;

	std::string nextToken(const std::string &, int &) const;
	long deriveMID(const std::string & line);
	bool deriveKillSet(const std::string & line);
public:
	//KillVectorProducer(BitSeq::size_t testnum);
	KillVectorProducer(BitSeq::size_t testnum, BitSeq::size_t bias);
	const KillVector * produce(const std::string & line);
};

/* Binary trie for bit-sequence */
class BitTrie {
protected:
	/* set left child */
	void set_left(BitTrie *);
	/* set right child */
	void set_right(BitTrie *);
public:
	/* construct a leaf node without data and children by specifying its start-index and key-sequence */
	BitTrie(BitSeq::size_t, const BitSeq &);
	/* release the key sequence (not data, left and right) */
	~BitTrie();

	/* get the index to the first bit which it's going to match */
	BitSeq::size_t get_bias() const;
	/* get the key in this sequence */
	const BitSeq & get_key() const;

	/* get the left child */
	BitTrie * get_left() const;
	/* get the right child */
	BitTrie * get_right() const;
	/* get parent of this node */
	BitTrie * get_parent() const;

	/* whether the node is leaf */
	bool is_leaf() const;

	/* get the data (null if non-leaf) */
	void * get_data() const;
	/* set the data */
	void set_data(void *);

	/* friend class */
	friend class BitTrieTree;
private:
	/* index to the first bit matched by this node */
	BitSeq::size_t bias;
	/* partial key to match bit-sequence from index to index + key.bit_number() - 1 */
	BitSeq * key;
	/* left child */
	BitTrie * left;
	/* right child */
	BitTrie * right;
	/* parent for this node */
	BitTrie * parent;
	/* data item referred by this node */
	void * data;
};
/* Tree for bit-trie */
class BitTrieTree {
public:
	/* construct an empty trie tree */
	BitTrieTree();
	/* release all trie nodes in the tree */
	~BitTrieTree();

	/* get the root of the tree (null when tree is empty) */
	BitTrie * get_root() const;
	/* 
	*  insert another bit-sequence into the tree
	*	1) if some leaf refers to the bits, then nothing occurs;
	*	2) otherwise, new leaf is created and inserted to locations.
	*/
	BitTrie * insert_vector(const BitSeq &);
	/* get the leaf referring to the bit-sequence */
	BitTrie * get_leaf(const BitSeq &) const;

private:
	/* the root of this tree */
	BitTrie * root;
	/* match to the maximum prefix of sequence in the trie */
	BitTrie * maximum_prefix_match(const BitSeq &, BitSeq::size_t &) const;
};
