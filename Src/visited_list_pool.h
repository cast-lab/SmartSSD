#pragma once
#include <string.h>
#include "hnsw-myDeQ.h"

#define MAX_ELEMENTS 10000		//Same as hnsw-myDeQ MAX 

namespace hnswlib {
	typedef unsigned short int vl_type;

	class VisitedList {
	public:
		vl_type curV;
		vl_type mass[MAX_ELEMENTS];
		unsigned int numelements;
		VisitedList() {
			numelements = MAX_ELEMENTS;
		}
		VisitedList(int numelements1) {
			curV = -1;
			numelements = numelements1;
		}

		void setNumelements(int n1) {
			numelements = n1;
		}

		void reset() {
			curV++;
			if (curV == 0) {
				memset(mass, 0, sizeof(vl_type) * numelements);
				curV++;
			}
		};

		~VisitedList() { cout << "VLpool Done\n"; }
	};
	///////////////////////////////////////////////////////////
	//
	// Class for multi-threaded pool-management of VisitedLists
	//
	/////////////////////////////////////////////////////////

	class VisitedListPool {
		S_Deque<VisitedList*> pool;
		int numelements;
		VisitedList vlist[MAX_ELEMENTS];
	public:
		VisitedListPool(int initmaxpools, int numelements1) {
			numelements = numelements1;
			for (int i = 0; i < initmaxpools; i++) {
				vlist[i].setNumelements(numelements);
				pool.s_insertFront(&vlist[i]);
			}

		}

		VisitedList* getFreeVisitedList() {
			VisitedList* rez;
			{
				if (pool.s_getSize() > 0) {
					rez = pool.s_getFront();
					pool.s_deleteFront();
				}
				else {
					//We should never reach here, as we do static allocation
					rez = new VisitedList(numelements);
				}
			}
			rez->reset();
			return rez;
		};

		void releaseVisitedList(VisitedList* vl) {
			pool.s_insertFront(vl);
		};

		~VisitedListPool() {
			while (pool.s_getSize()) {
				pool.s_deleteFront();
			}
		};
	};
}