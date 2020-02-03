#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>
#include "hnswlib.h"
#include <unordered_set>

using namespace std;
using namespace hnswlib;


class StopW {
	std::chrono::steady_clock::time_point time_begin;
public:
	StopW() {
		time_begin = std::chrono::steady_clock::now();
	}

	float getElapsedTimeMicro() {
		std::chrono::steady_clock::time_point time_end = std::chrono::steady_clock::now();
		return (std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_begin).count());
	}

	void reset() {
		time_begin = std::chrono::steady_clock::now();
	}

};

void test_approx(float* massQ, size_t vecsize, size_t qsize, HierarchicalNSW<float>& appr_alg, size_t vecdim, tableint searchKnn_out[][1000], int idx) {
	//searchKnn_out[][qsize]
	for (int i = 0; i < qsize; i++) {
		StaticPriorityQueue result;
		appr_alg.searchKnn(massQ + vecdim * i, 10, 0, result);

		Node* node = result.Top();
		tableint result_node_id = node->ep_id;

		searchKnn_out[idx][i] = result_node_id;
	}
}


void sift_test() {

	const size_t vecsize = 10000;
	const size_t qsize = 1000;
	const size_t vecdim = 128;
	L2Space l2space(vecdim);

	// Reading Query Vectors
	float* massQ = new float[qsize * vecdim];
	ifstream inputQ("../bigann/ANN_SIFT1M/sift_query.fvecs", ios::binary);
	inputQ.read((char*)massQ, qsize * vecdim * sizeof(float));
	inputQ.close();

	// HNSW load index
	HierarchicalNSW<float> appr_alg(&l2space, "../bigann/hnswlib_test", false);

	//Seach Knn
	vector<size_t> efs;
	for (int i = 10; i < 60; i += 10) {
		efs.push_back(i);
	}
	for (int i = 100; i < 600; i += 100) {
		efs.push_back(i);
	}

	const int size_efs = 25;		//size of efs

	cout << "Search Knn" << endl;

	int idx = 0;

	for (size_t ef : efs) {
		appr_alg.setEf(ef);
		StopW stopw = StopW();
		
		//Final output of search_Knn
		tableint searchKnn_out[size_efs][qsize];

		test_approx(massQ, vecsize, qsize, appr_alg, vecdim, searchKnn_out, idx);

		idx++;
	}
}
