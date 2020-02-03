#pragma once

#include "visited_list_pool.h"
#include "hnswlib.h"
#include <stdlib.h>
#include <assert.h>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <mutex>

using namespace std;

#define HNSWALG

namespace hnswlib {
	typedef unsigned int tableint;
	typedef unsigned int linklistsizeint;
	///////////////////////////////////////////////////
#define MAX_NUM_NODES 1000
	
#define NEWPQ

#ifdef NEWPQ // Remove tail, prev
	
	StaticPriorityQueue::StaticPriorityQueue() {

		for (int i = 0; i < MAX_NUM_NODES; ++i) {
			InitNode(&nodes[i]);
			if (i < (MAX_NUM_NODES - 1)) 
				nodes[i].next = &nodes[i + 1];
			nodes[MAX_NUM_NODES - 1].next = NULL;
		}

		free_list.head = &nodes[0];
		data_list.head = NULL;
		num_free_nodes = MAX_NUM_NODES;
		num_data_nodes = 0;

	}
	
	void StaticPriorityQueue::InitNode(Node* node) {
		node->dist = 0;
		node->ep_id = 0;
	}

	Node* StaticPriorityQueue::GetNewNode() {
		Node* node;

		if (num_free_nodes == 0)
			return NULL;

		node = free_list.head;
		free_list.head = node->next;
		node->next = NULL;
		num_free_nodes--;
		
		return node;
	}

	void StaticPriorityQueue::Pop() {
		if (num_data_nodes == 0)
			return;
		Node* head = data_list.head;
		data_list.head = data_list.head->next;
		InitNode(head);
		head->next = free_list.head;
		free_list.head = head;
		num_free_nodes++;
		num_data_nodes--;
	}

	Node* StaticPriorityQueue::Top() {
		if (num_data_nodes == 0)
			return NULL;
		return data_list.head;
	}

	Node* StaticPriorityQueue::Emplace(int64_t dist, tableint ep_id) {
		Node* node = GetNewNode();

		if (node == NULL)			
			return NULL; // No available free nodes

		node->dist = dist;
		node->ep_id = ep_id;
		num_data_nodes++;

		Node* tmp = data_list.head;
		Node* prev = NULL;


		while (1) {
			if (tmp == NULL){
				node->next = NULL;
				if (prev == NULL)
					data_list.head = node;
				else
					prev->next = node;
				break;
			}
			if (dist >= tmp->dist) {
				node->next = tmp;
				if (prev == NULL)
					data_list.head = node;
				else
					prev->next = node;
				break;
			}
			prev = tmp;
			tmp = tmp->next;
		}

		return node;
	}

	void StaticPriorityQueue::Swap(StaticPriorityQueue& spq) {
		StaticPriorityQueue tmp_spq;

		while (!spq.Empty()) {
			Node* node = spq.Top();
			tmp_spq.Emplace(node->dist, node->ep_id);
			spq.Pop();
		}

		while (!Empty()) {
			Node* node = Top();
			spq.Emplace(node->dist, node->ep_id);
			Pop();
		}

		while (!tmp_spq.Empty()) {
			Node* node = tmp_spq.Top();
			Emplace(node->dist, node->ep_id);
			tmp_spq.Pop();
		}
	}
	
	//////////////////////////////////////////////////////

#else
	StaticPriorityQueue::StaticPriorityQueue() {

		for (int i = 0; i < MAX_NUM_NODES; ++i) {
			InitNode(&nodes[i]);

			if (i > 0) {
				nodes[i].prev = &nodes[i - 1];
			}

			if (i < (MAX_NUM_NODES - 1)) {
				nodes[i].next = &nodes[i + 1];
			}
		}

		free_list.head = &nodes[0];
		free_list.tail = &nodes[MAX_NUM_NODES - 1];

		data_list.head = NULL;
		data_list.tail = NULL;

		num_free_nodes = MAX_NUM_NODES;
		num_data_nodes = 0;

	}

	void StaticPriorityQueue::Debug() {
		assert(num_data_nodes + num_free_nodes == MAX_NUM_NODES);
	}

	void StaticPriorityQueue::InitNode(Node* node) {
		node->is_free = 1;

		node->dist = 0;
		node->ep_id = 0;

		node->prev = NULL;
		node->next = NULL;
	}

	Node* StaticPriorityQueue::GetNewNode() {
		Node* node;

		if (num_free_nodes == 0) {
			return NULL;
		}

		node = free_list.head;
		free_list.head = node->next;

		assert(node->prev == NULL);
		node->next = NULL;

		num_free_nodes--;

		if (num_free_nodes == 0) {
			assert(free_list.head == NULL);
			free_list.tail = NULL;
		}
		else {
			free_list.head->prev = NULL;
		}

		return node;
	}

	void StaticPriorityQueue::FreeNode(Node* node) {

		Node* prev_node = node->prev;
		Node* next_node = node->next;

		if (prev_node) {
			prev_node->next = next_node;
		}
		if (next_node) {
			next_node->prev = prev_node;
		}

		if (data_list.head == node) {
			data_list.head = next_node;
		}
		if (data_list.tail == node) {
			data_list.tail = prev_node;
		}

		if (num_data_nodes == 0) {
			assert(data_list.head == NULL);
			assert(data_list.tail == NULL);
		}

		InitNode(node);

		if (num_free_nodes == 0) {
			free_list.head = node;
			free_list.tail = node;
		}
		else {
			free_list.tail->next = node;
			node->prev = free_list.tail;

			free_list.tail = node;
		}

		num_free_nodes++;
	}

	void StaticPriorityQueue::Pop() {
		if (num_data_nodes == 0) {
			return;
		}

		Node* head = data_list.head;
		data_list.head = data_list.head->next;

		num_data_nodes--;

		if (num_data_nodes == 0) {
			assert(data_list.head == NULL);
			data_list.tail = NULL;
		}

		FreeNode(head);
	}

	Node* StaticPriorityQueue::Top() {
		if (num_data_nodes == 0) {
			return NULL;
		}

		return data_list.head;
	}

	Node* StaticPriorityQueue::Emplace(int64_t dist, tableint ep_id) {
		Node* node = GetNewNode();

		if (node == NULL) {
			// No available free nodes
			return NULL;
		}

		node->dist = dist;
		node->ep_id = ep_id;
		node->is_free = 0;

		num_data_nodes++;

		if (num_data_nodes == 1) {
			// First entry
			data_list.head = node;
			data_list.tail = node;

			return node;
		}

		Node* head_prev = NULL;
		Node* head = data_list.head;
		Node* tail = data_list.tail;

		// Optimization - the smallest entry?
		if (dist <= tail->dist) {
			tail->next = node;
			node->prev = tail;

			data_list.tail = node;
			return node;
		}

		// Otherwise...
		while (head) {
			if (dist >= head->dist) {

				head_prev = head->prev;

				node->next = head;
				head->prev = node;

				if (head == data_list.head) {
					data_list.head = node;
				}
				else {
					head_prev->next = node;
					node->prev = head_prev;
				}

				break;
			}

			head = head->next;
		}

		assert(head);
		return node;
	}

	void StaticPriorityQueue::Swap(StaticPriorityQueue& spq) {
		StaticPriorityQueue tmp_spq;

		while (!spq.Empty()) {
			Node* node = spq.Top();
			tmp_spq.Emplace(node->dist, node->ep_id);
			spq.Pop();
		}

		while (!Empty()) {
			Node* node = Top();
			spq.Emplace(node->dist, node->ep_id);
			Pop();
		}

		while (!tmp_spq.Empty()) {
			Node* node = tmp_spq.Top();
			Emplace(node->dist, node->ep_id);
			tmp_spq.Pop();
		}
	}
#endif

	///////////////////////////////////////////////////
	template<typename dist_t>
	class HierarchicalNSW : public AlgorithmInterface<dist_t> {
	public:

		HierarchicalNSW(SpaceInterface<dist_t>* s, const std::string& location, bool nmslib = false, size_t max_elements = 0) {
			loadIndex(location, s, max_elements);
		}

		~HierarchicalNSW() {

			free(data_level0_memory_);
			for (tableint i = 0; i < cur_element_count; i++) {
				if (element_levels_[i] > 0)
					free(linkLists_[i]);
			}
			free(linkLists_);
			//delete visited_list_pool_; //Part makes the underflow issue after the whole process is done
		}

		size_t max_elements_;
		size_t cur_element_count;
		size_t size_data_per_element_;
		size_t size_links_per_element_;

		size_t M_;
		size_t maxM_;
		size_t maxM0_;
		size_t ef_construction_;

		double mult_, revSize_;
		int maxlevel_;

		VisitedListPool* visited_list_pool_;
		std::mutex cur_element_count_guard_;

		std::vector<std::mutex> link_list_locks_;
		tableint enterpoint_node_;


		size_t size_links_level0_;
		size_t offsetData_, offsetLevel0_;


		char* data_level0_memory_;
		char** linkLists_;
		std::vector<int> element_levels_;

		size_t data_size_;

		bool has_deletions_;


		size_t label_offset_;
		DISTFUNC<dist_t> fstdistfunc_;
		void* dist_func_param_;
		std::unordered_map<labeltype, tableint> label_lookup_;

		inline labeltype getExternalLabel(tableint internal_id) const {
			labeltype return_label;
			memcpy(&return_label, (data_level0_memory_ + internal_id * size_data_per_element_ + label_offset_), sizeof(labeltype));
			return return_label;
		}

		inline labeltype* getExternalLabeLp(tableint internal_id) const {
			return (labeltype*)(data_level0_memory_ + internal_id * size_data_per_element_ + label_offset_);
		}

		inline char* getDataByInternalId(tableint internal_id) const {
			return (data_level0_memory_ + internal_id * size_data_per_element_ + offsetData_);
		}

		template <bool has_deletions>
		void searchBaseLayerST(tableint ep_id, const void* data_point, size_t ef, StaticPriorityQueue &top_candidates) const {
			VisitedList* vl = visited_list_pool_->getFreeVisitedList();
			vl_type* visited_array = vl->mass;
			vl_type visited_array_tag = vl->curV;

			StaticPriorityQueue candidate_set;

			dist_t lowerBound;

			if (!has_deletions || !isMarkedDeleted(ep_id)) {
				dist_t dist = fstdistfunc_(data_point, getDataByInternalId(ep_id), dist_func_param_);
				lowerBound = dist;
				top_candidates.Emplace(dist, ep_id);
				candidate_set.Emplace(-dist, ep_id);
			}
			else{
				lowerBound = std::numeric_limits<dist_t>::max();
				candidate_set.Emplace(-lowerBound, ep_id);
			}

			visited_array[ep_id] = visited_array_tag;

			int candidate_set_size = 0;
			int top_candidates_set_size = 0;
			int changed = 0;

			while (!candidate_set.Empty()) {

				if (candidate_set_size < candidate_set.Size()) {
					candidate_set_size = candidate_set.Size();
				}

				if (top_candidates_set_size < top_candidates.Size()) {
					top_candidates_set_size = top_candidates.Size();
				}

				//std::pair<dist_t, tableint> current_node_pair = candidate_set.top();
				Node* node = candidate_set.Top();

				if ((-node->dist) > lowerBound) {
					break;
				}

				tableint current_node_id = node->ep_id;
				candidate_set.Pop();

				int* data = (int*)get_linklist0(current_node_id);
				size_t size = getListCount((linklistsizeint*)data);

				for (size_t j = 1; j <= size; j++) {
					int candidate_id = *(data + j);

					if (!(visited_array[candidate_id] == visited_array_tag)) {

						visited_array[candidate_id] = visited_array_tag;

						char* currObj1 = (getDataByInternalId(candidate_id));
						dist_t dist = fstdistfunc_(data_point, currObj1, dist_func_param_);

						if (top_candidates.Size() < ef || lowerBound > dist) {
							candidate_set.Emplace(-dist, candidate_id);

							if (!has_deletions || !isMarkedDeleted(candidate_id))
								top_candidates.Emplace(dist, candidate_id);

							if (top_candidates.Size() > ef)
								top_candidates.Pop();

							if (!top_candidates.Empty()) {
								lowerBound = top_candidates.Top()->dist;
							}
						}
					}
				}
			}

			visited_list_pool_->releaseVisitedList(vl);

		}
		

		linklistsizeint* get_linklist0(tableint internal_id) const {
			return (linklistsizeint*)(data_level0_memory_ + internal_id * size_data_per_element_ + offsetLevel0_);
		};

		linklistsizeint* get_linklist0(tableint internal_id, char* data_level0_memory_) const {
			return (linklistsizeint*)(data_level0_memory_ + internal_id * size_data_per_element_ + offsetLevel0_);
		};

		linklistsizeint* get_linklist(tableint internal_id, int level) const {
			return (linklistsizeint*)(linkLists_[internal_id] + (level - 1) * size_links_per_element_);
		};
		
		size_t ef_;

		void setEf(size_t ef) {
			ef_ = ef;
		}

		void loadIndex(const std::string& location, SpaceInterface<dist_t>* s, size_t max_elements_i = 0) {


			std::ifstream input(location, std::ios::binary);

			if (!input.is_open())
				throw std::runtime_error("Cannot open file");

			// get file size:
			input.seekg(0, input.end);
			std::streampos total_filesize = input.tellg();
			input.seekg(0, input.beg);

			readBinaryPOD(input, offsetLevel0_);
			readBinaryPOD(input, max_elements_);
			readBinaryPOD(input, cur_element_count);

			size_t max_elements = max_elements_i;
			if (max_elements < cur_element_count)
				max_elements = max_elements_;
			max_elements_ = max_elements;
			readBinaryPOD(input, size_data_per_element_);
			readBinaryPOD(input, label_offset_);
			readBinaryPOD(input, offsetData_);
			readBinaryPOD(input, maxlevel_);
			readBinaryPOD(input, enterpoint_node_);

			readBinaryPOD(input, maxM_);
			readBinaryPOD(input, maxM0_);
			readBinaryPOD(input, M_);
			readBinaryPOD(input, mult_);
			readBinaryPOD(input, ef_construction_);


			data_size_ = s->get_data_size();
			fstdistfunc_ = s->get_dist_func();
			dist_func_param_ = s->get_dist_func_param();

			auto pos = input.tellg();


			/// Optional - check if index is ok:

			input.seekg(cur_element_count * size_data_per_element_, input.cur);
			for (size_t i = 0; i < cur_element_count; i++) {
				if (input.tellg() < 0 || input.tellg() >= total_filesize) {
					throw std::runtime_error("Index seems to be corrupted or unsupported");
				}

				unsigned int linkListSize;
				readBinaryPOD(input, linkListSize);
				if (linkListSize != 0) {
					input.seekg(linkListSize, input.cur);
				}
			}

			// throw exception if it either corrupted or old index
			if (input.tellg() != total_filesize)
				throw std::runtime_error("Index seems to be corrupted or unsupported");

			input.clear();

			/// Optional check end

			input.seekg(pos, input.beg);


			data_level0_memory_ = (char*)malloc(max_elements * size_data_per_element_);
			input.read(data_level0_memory_, cur_element_count * size_data_per_element_);




			size_links_per_element_ = maxM_ * sizeof(tableint) + sizeof(linklistsizeint);


			size_links_level0_ = maxM0_ * sizeof(tableint) + sizeof(linklistsizeint);
			std::vector<std::mutex>(max_elements).swap(link_list_locks_);


			visited_list_pool_ = new VisitedListPool(1, max_elements);


			linkLists_ = (char**)malloc(sizeof(void*) * max_elements);
			element_levels_ = std::vector<int>(max_elements);
			revSize_ = 1.0 / mult_;
			ef_ = 10;
			for (size_t i = 0; i < cur_element_count; i++) {
				label_lookup_[getExternalLabel(i)] = i;
				unsigned int linkListSize;
				readBinaryPOD(input, linkListSize);
				if (linkListSize == 0) {
					element_levels_[i] = 0;

					linkLists_[i] = nullptr;
				}
				else {
					element_levels_[i] = linkListSize / size_links_per_element_;
					linkLists_[i] = (char*)malloc(linkListSize);
					input.read(linkLists_[i], linkListSize);
				}
			}

			has_deletions_ = false;

			for (size_t i = 0; i < cur_element_count; i++) {
				if (isMarkedDeleted(i)) {
					has_deletions_ = true;
					break;
				}
			}

			input.close();

			return;
		}
		

		template<typename data_t>
		std::vector<data_t> getDataByLabel(labeltype label)
		{
			tableint label_c;
			auto search = label_lookup_.find(label);
			if (search == label_lookup_.end() || isMarkedDeleted(search->second)) {
				throw std::runtime_error("Label not found");
			}
			label_c = search->second;

			char* data_ptrv = getDataByInternalId(label_c);
			size_t dim = *((size_t*)dist_func_param_);
			std::vector<data_t> data;
			data_t* data_ptr = (data_t*)data_ptrv;
			for (int i = 0; i < dim; i++) {
				data.push_back(*data_ptr);
				data_ptr += 1;
			}
			return data;
		}

		static const unsigned char DELETE_MARK = 0x01;
		
		/**
			* Marks an element with the given label deleted, does NOT really change the current graph.
			* @param label
			*/
		void markDelete(labeltype label)
		{
			has_deletions_ = true;
			auto search = label_lookup_.find(label);
			if (search == label_lookup_.end()) {
				throw std::runtime_error("Label not found");
			}
			markDeletedInternal(search->second);
		}

		/**
		 * Uses the first 8 bits of the memory for the linked list to store the mark,
		 * whereas maxM0_ has to be limited to the lower 24 bits, however, still large enough in almost all cases.
		 * @param internalId
		 */
		void markDeletedInternal(tableint internalId) {
			unsigned char* ll_cur = ((unsigned char*)get_linklist0(internalId)) + 2;
			*ll_cur |= DELETE_MARK;
		}

		/**
		 * Remove the deleted mark of the node.
		 * @param internalId
		 */
		void unmarkDeletedInternal(tableint internalId) {
			unsigned char* ll_cur = ((unsigned char*)get_linklist0(internalId)) + 2;
			*ll_cur &= ~DELETE_MARK;
		}

		/**
		 * Checks the first 8 bits of the memory to see if the element is marked deleted.
		 * @param internalId
		 * @return
		 */
		bool isMarkedDeleted(tableint internalId) const {
			unsigned char* ll_cur = ((unsigned char*)get_linklist0(internalId)) + 2;
			return *ll_cur & DELETE_MARK;
		}

		unsigned short int getListCount(linklistsizeint* ptr) const {
			return *((unsigned short int*)ptr);
		}

		void searchKnn(const void* query_data, size_t k, size_t HW, StaticPriorityQueue &results) const {
			tableint currObj = enterpoint_node_;
			dist_t curdist = fstdistfunc_(query_data, getDataByInternalId(enterpoint_node_), dist_func_param_);

			for (int level = maxlevel_; level > 0; level--) {
				bool changed = true;
				while (changed) {
					changed = false;
					unsigned int* data;

					data = (unsigned int*)get_linklist(currObj, level);
					int size = getListCount(data);
					tableint* datal = (tableint*)(data + 1);
					for (int i = 0; i < size; i++) {
						tableint cand = datal[i];
						if (cand < 0 || cand > max_elements_)
							throw std::runtime_error("cand error");
						dist_t d = fstdistfunc_(query_data, getDataByInternalId(cand), dist_func_param_);

						if (d < curdist) {
							curdist = d;
							currObj = cand;
							changed = true;
						}
					}
				}
			}

			StaticPriorityQueue top_candidates;
			if (has_deletions_) {
				searchBaseLayerST<true>(currObj, query_data, std::max(ef_, k), top_candidates);
			}
			else {
				searchBaseLayerST<false>(currObj, query_data, std::max(ef_, k), top_candidates);
			}

			while (top_candidates.Size() > k) {
				top_candidates.Pop();
			}
			while (top_candidates.Size() > 0) {
				auto rez = top_candidates.Top();
				results.Emplace(rez->dist, getExternalLabel(rez->ep_id));
				//results.push(std::pair<dist_t, labeltype>(rez->dist, getExternalLabel(rez->ep_id)));
				top_candidates.Pop();
			}

		};



	};

}
