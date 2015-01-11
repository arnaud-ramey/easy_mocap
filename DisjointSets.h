#ifndef DISJOINTSETS_H
#define DISJOINTSETS_H

/***************************************************************************//**
 * \class DisjointSets
 * 
 * \brief Disjoint Set Data Structure
 * Implementaton is as described in http://en.wikipedia.org/wiki/Disjoint-set_data_structure
 *
 * cf "Data structures and algorithms for disjoint set union problems"
 * of Zvi Galil and Giuseppe F. Italiano
 * 
 * \author Arnaud Ramey ( arnaud.ramey@m4x.org )
 * 
 * \date May 2009
 *******************************************************************************/

#include <vector>
#include <iostream>			// for cin, cout
#include <cassert>
#include <string>
#include <sstream>
using namespace std;

class DisjointSets {
public:
	/**
	 * \class DisjointSets::Node
	 * \brief Internal Node data structure used for representing an element
	 */
	struct Node {
		//! The parent node of the node
		Node* parent;
		//! This roughly represent the max height of the node in its subtree
		//		int rank;
		//! The index of the element the node represents
		int index;
		//! the number of sons
		int size;

		Node(int indx = 0) {
			index = indx;
			parent = NULL;
			//			rank = 0;
			size = 1;
		}
		
		inline string toString() {
			ostringstream rep;
			rep << "#" << index << ": size=" << size << ", parent=" << ( parent ? parent->index : -1);
			return rep.str();
		}
	};

	/*!
	 * \brief   Create an empty DisjointSets data structure
	 */
	DisjointSets();
	
	/*!
	 * \brief   Create a DisjointSets data structure with 
	 * a specified number of elements (with element id's from 0 to count-1)
	 */
	DisjointSets(int count);
	
	/*!
	 * \brief   Destructor
	 */
	~DisjointSets();
	
	inline Node* elementAt(int i) {
		return m_nodes.at(i);
	}

	/*!
	 * \brief   Find the set identifier that an element currently belongs to.
	 * Note: some internal data is modified for optimization even though this method is consant.
	 */
	
	//int FindSet(int element);// const;
	Node* FindSet(Node* element);
	/*!
	 * \brief   Combine two sets into one. 
	 * All elements in those two sets will share the same set id that can be gotten using FindSet.
	 */
	//Node* Union(int setId1, int setId2);
	Node* Union(Node* set1, Node* set2);
	/*!
	 * \brief   Add a specified number of elements to the DisjointSets data structure. 
	 * The element id's of the new elements are numbered
	 * consequitively starting with the first never-before-used elementId.
	 */
	void AddElements(int numToAdd);
	/*!
	 * \brief   Returns the number of elements currently in the DisjointSets data structure.
	 */
	int NumElements();
	/*!
	 * \brief   Returns the number of sets currently in the DisjointSets data structure.
	 */
	//	int NumSets() const;

	//! the number of sets currently in the DisjointSets data structure.
	//	int m_numSets; 

//private:
	//! the number of elements currently in the DisjointSets data structure.
	//	int m_numElements;
	//! the list of nodes representing the elements
	std::vector<Node*> m_nodes;
};

#endif
