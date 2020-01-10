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

void get_gt(float* mass, float* massQ, size_t vecsize, size_t qsize, L2Space& l2space, size_t vecdim,
	vector<std::priority_queue<std::pair<float, labeltype >>>& answers, size_t k) {
	BruteforceSearch<float> bs(&l2space, vecsize);
	for (int i = 0; i < vecsize; i++) {
		bs.addPoint((void*)(mass + vecdim * i), (size_t)i);
	}
	(vector<std::priority_queue<std::pair<float, labeltype >>>(qsize)).swap(answers);

	for (int i = 0; i < qsize; i++) {
		std::priority_queue<std::pair<float, labeltype >> gt = bs.searchKnn_bf(massQ + vecdim * i, 10, 0);
//		StaticPriorityQueue gt = bs.searchKnn(massQ + vecdim * i, 10, 0);
		answers[i] = gt;
	}
}

void
get_gt(unsigned int* massQA, float* massQ, float* mass, size_t vecsize, size_t qsize, L2Space& l2space, size_t vecdim,
	vector<std::priority_queue<std::pair<float, labeltype >>>& answers, size_t k) {

	(vector<std::priority_queue<std::pair<float, labeltype >>>(qsize)).swap(answers);
	DISTFUNC<float> fstdistfunc_ = l2space.get_dist_func();
	cout << qsize << "\n";
	for (int i = 0; i < qsize; i++) {
		for (int j = 0; j < k; j++) {
			float other = fstdistfunc_(massQ + i * vecdim, mass + massQA[100 * i + j],
				l2space.get_dist_func_param());
			answers[i].emplace(other, massQA[100 * i + j]);
		}
	}
}

float test_approx(float* massQ, size_t vecsize, size_t qsize, HierarchicalNSW<float>& appr_alg, size_t vecdim,
	vector<std::priority_queue<std::pair<float, labeltype >>>& answers, size_t k) {
	size_t correct = 0;
	size_t total = 0;

	for (int i = 0; i < qsize; i++) {

//		std::priority_queue<std::pair<float, labeltype >> result = appr_alg.searchKnn(massQ + vecdim * i, 10, 0);
		StaticPriorityQueue result =  appr_alg.searchKnn(massQ + vecdim * i, 10, 0);
		std::priority_queue<std::pair<float, labeltype >> gt(answers[i]);
		unordered_set<labeltype> g;
		total += gt.size();
		while (gt.size()) {
			g.insert(gt.top().second);
			gt.pop();
		}

		/////////////////////
		Node* node = result.Top();
		tableint result_node_id = node->ep_id;
		/////////////////////
		while (result.Size()) {
			if (g.find(result_node_id) != g.end())
				correct++;
			result.Pop();
		}
	}
	return 1.0f * correct / total;
}

void test_vs_recall(float* massQ, size_t vecsize, size_t qsize, HierarchicalNSW<float>& appr_alg, size_t vecdim,
	vector<std::priority_queue<std::pair<float, labeltype >>>& answers, size_t k) {
	vector<size_t> efs;
	for (int i = 10; i < 60; i += 10) {
		efs.push_back(i);
	}
	for (int i = 100; i < 600; i += 100) {
		efs.push_back(i);
	}
	for (size_t ef : efs) {
		appr_alg.setEf(ef);
		StopW stopw = StopW();

		float recall = test_approx(massQ, vecsize, qsize, appr_alg, vecdim, answers, k);
		float time_us_per_query = stopw.getElapsedTimeMicro() / qsize;
		cout << ef << "\t" << recall << "\t" << time_us_per_query << " us\n";
		if (recall > 1.0) {
			cout << recall << "\t" << time_us_per_query << " us\n";
			break;
		}
	}
}


void sift_test() {
	size_t vecsize = 10000;
	size_t qsize = 1000;
	size_t vecdim = 128;

	float* mass = new float[vecsize * vecdim];
	ifstream input("../bigann/ANN_SIFT1M/sift_base.fvecs", ios::binary);
	input.read((char*)mass, vecsize * vecdim * sizeof(float));
	input.close();

	float* massQ = new float[qsize * vecdim];
	ifstream inputQ("../bigann/ANN_SIFT1M/sift_query.fvecs", ios::binary);
	inputQ.read((char*)massQ, qsize * vecdim * sizeof(float));
	inputQ.close();

	unsigned int* massQA = new unsigned int[qsize * 100];
	ifstream inputQA("../bigann/ANN_SIFT1M/sift_groundtruth.ivecs", ios::binary);
	inputQA.read((char*)massQA, qsize * 100 * sizeof(int));
	inputQA.close();

	int maxn = 16;

	L2Space l2space(vecdim);

	//#define LOAD_I
#ifdef LOAD_I

	HierarchicalNSW<float> appr_alg(&l2space, "../bigann/hnswlib_test", false);

#else
	HierarchicalNSW<float> appr_alg(&l2space, vecsize, 16, 40);

	cout << "Building index\n";
	StopW stopwb = StopW();
	for (int i = 0; i < 1; i++) {
		appr_alg.addPoint((void*)(mass + vecdim * i), (size_t)i);
	}
#pragma omp parallel for
	for (int i = 1; i < vecsize; i++) {
		appr_alg.addPoint((void*)(mass + vecdim * i), (size_t)i);
	}

	cout << "Index built, time=" << stopwb.getElapsedTimeMicro() * 1e-6 << "\n";
	appr_alg.saveIndex("../bigann/hnswlib_test");


#endif

	vector<std::priority_queue<std::pair<float, labeltype >>> answers;
	size_t k = 10;
	cout << "Loading gt\n";
	get_gt(mass, massQ, vecsize, qsize, l2space, vecdim, answers, k);

	cout << "Loaded gt\n";
	for (int i = 0; i < 1; i++)
		test_vs_recall(massQ, vecsize, qsize, appr_alg, vecdim, answers, k);

	return;
}