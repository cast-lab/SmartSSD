#pragma once


#include <queue>
#include <string.h>

#define MAX_NUM_NODES 1000		//Same as hnswalg MAX_NUM_NODES

namespace hnswlib {

	typedef unsigned int tableint;
	struct Node {
		int64_t dist;
		tableint ep_id;

		struct Node* next;
		struct Node* prev;

		int8_t is_free;
	};

	struct NodePointers {
		Node* head;
		Node* tail;
	};

	class StaticPriorityQueue {
	public:
		StaticPriorityQueue();
		Node* GetNewNode();
		void FreeNode(Node* node);
		void InitNode(Node* node);

		uint32_t Size() { return num_data_nodes; }
		uint8_t Empty() { return num_data_nodes == 0; }
		Node* Emplace(int64_t dist, tableint ep_id);
		void Pop();
		Node* Top();
		void Swap(StaticPriorityQueue& spq);

		void Debug();

	private:
		Node nodes[MAX_NUM_NODES];

		NodePointers data_list;
		NodePointers free_list;

		uint32_t num_free_nodes;
		uint32_t num_data_nodes;
	};


	typedef size_t labeltype;

	template<typename T>
	static void writeBinaryPOD(std::ostream& out, const T& podRef) {
		out.write((char*)&podRef, sizeof(T));
	}

	template<typename T>
	static void readBinaryPOD(std::istream& in, T& podRef) {
		in.read((char*)&podRef, sizeof(T));
	}

	template<typename MTYPE>
	using DISTFUNC = MTYPE(*)(const void*, const void*, const void*);


	template<typename MTYPE>
	class SpaceInterface {
	public:
		//virtual void search(void *);
		virtual size_t get_data_size() = 0;

		virtual DISTFUNC<MTYPE> get_dist_func() = 0;

		virtual void* get_dist_func_param() = 0;

		virtual ~SpaceInterface() {}
	};

	template<typename dist_t>
	class AlgorithmInterface {
	public:
		virtual void searchKnn(const void*, size_t, size_t, StaticPriorityQueue &) const = 0;
		virtual ~AlgorithmInterface() {
		}
	};
}

#include "space_l2.h"
#include "space_ip.h"
#include "hnswalg.h"