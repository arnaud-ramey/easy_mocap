#include "DisjointSets.h"

/***************************************************************************//**
 * \file DisjointSets.cpp
 * 
 * \brief The implementation of DisjointSets.h
 * 
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 * 
 * \date May 2009
 *******************************************************************************/

DisjointSets::DisjointSets() {
	//	m_numElements = 0;
	//	m_numSets = 0;
}

DisjointSets::DisjointSets(int count) {
	//	m_numElements = 0;
	//	m_numSets = 0;
	AddElements(count);
}

DisjointSets::~DisjointSets() {
	//for(int i = 0; i < m_numElements; ++i)
	for (unsigned int i = 0; i < m_nodes.size(); ++i)
		delete m_nodes[i];
	m_nodes.clear();
	//	m_numElements = 0;
	//	m_numSets = 0;
}

DisjointSets::Node* DisjointSets::FindSet(DisjointSets::Node* element) { //const {
	//assert(elementId < m_numElements);

	Node *curNode = element, *next, *root;

	// Find the root element that represents the set which `elementId` belongs to
	while (curNode->parent != NULL)
		curNode = curNode->parent;
	root = curNode;

	// Walk to the root, updating the parents of `elementId`.
	// Make those elements the direct children of `root`.
	// This optimizes the tree for future FindSet invokations.
	curNode = element;
	while (curNode != root) {
		next = curNode->parent;
		curNode->parent = root;
		curNode = next;
	}

	return root;
}

DisjointSets::Node* DisjointSets::Union(DisjointSets::Node* set1,
		DisjointSets::Node* set2) {
	if (set1->index == set2->index)
		return set1;

	// Since two sets have fused into one, there is now one less set so update the set count.
	//	--m_numSets;

	// Determine which node representing a set has a higher rank. 
	// The node with the higher rank islikely to have a bigger subtree so 
	// in order to better balance the tree representing the union, 
	// the node with the higher rank is made the parent of the one with the 
	// lower rank and not the other way around.

	////////////////////////////////////////////////
	// IMPORTANT : change : now we don't use the higher rank, 
	// but the lower index.
	////////////////////////////////////////////////

	//if(set1->rank > set2->rank) {
	if (set1->index < set2->index) {
		set2->parent = set1;
		set1->size += set2->size;
		return set1;
	}

	//else if(set1->rank < set2->rank) {
	else if (set1->index > set2->index) {
		set1->parent = set2;
		set2->size += set1->size;
		return set2;
	}

	else {// set1->rank == set2->rank
		set2->parent = set1;
		//		++set1->rank; // update rank
		set1->size += set2->size;
		return set1;
	}

	return NULL;
}

inline void DisjointSets::AddElements(int numToAdd) {
	// insert and initialize the specified number of element nodes to the end of the `m_nodes` array
	//for(int i = 0; i < numToAdd; ++i)
	//m_nodes.push_back( new Node( m_numElements + i ) );
	m_nodes.resize(m_nodes.size() + numToAdd, NULL);

	// update element and set counts
	//	m_numElements += numToAdd;
	//	m_numSets += numToAdd;
}

int DisjointSets::NumElements() {
	//return m_numElements;
	return m_nodes.size();
}

//int DisjointSets::NumSets() const		{return m_numSets;}
