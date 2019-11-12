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



/*
* Author:  David Robert Nadeau
* Site:    http://NadeauSoftware.com/
* License: Creative Commons Attribution 3.0 Unported License
*          http://creativecommons.org/licenses/by/3.0/deed.en_US
*/

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))

#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif


/**
* Returns the peak (maximum so far) resident set size (physical
* memory use) measured in bytes, or zero if the value cannot be
* determined on this OS.
*/
static size_t getPeakRSS() {
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
    return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
    /* AIX and Solaris ------------------------------------------ */
    struct psinfo psinfo;
    int fd = -1;
    if ((fd = open("/proc/self/psinfo", O_RDONLY)) == -1)
        return (size_t)0L;      /* Can't open? */
    if (read(fd, &psinfo, sizeof(psinfo)) != sizeof(psinfo))
    {
        close(fd);
        return (size_t)0L;      /* Can't read? */
    }
    close(fd);
    return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    /* BSD, Linux, and OSX -------------------------------------- */
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
    return (size_t)rusage.ru_maxrss;
#else
    return (size_t) (rusage.ru_maxrss * 1024L);
#endif

#else
    /* Unknown OS ----------------------------------------------- */
    return (size_t)0L;          /* Unsupported. */
#endif
}


/**
* Returns the current resident set size (physical memory use) measured
* in bytes, or zero if the value cannot be determined on this OS.
*/
static size_t getCurrentRSS() {
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
    return (size_t)info.WorkingSetSize;

#elif defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &infoCount) != KERN_SUCCESS)
        return (size_t)0L;      /* Can't access? */
    return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE *fp = NULL;
    if ((fp = fopen("/proc/self/statm", "r")) == NULL)
        return (size_t) 0L;      /* Can't open? */
    if (fscanf(fp, "%*s%ld", &rss) != 1) {
        fclose(fp);
        return (size_t) 0L;      /* Can't read? */
    }
    fclose(fp);
    return (size_t) rss * (size_t) sysconf(_SC_PAGESIZE);

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    return (size_t)0L;          /* Unsupported. */
#endif
}


static void
get_gt(unsigned int *massQA, float *massQ, float *mass, size_t vecsize, size_t qsize, L2SpaceI &l2space,
       size_t vecdim, vector<std::priority_queue<std::pair<int, labeltype >>> &answers, size_t k) {


    (vector<std::priority_queue<std::pair<int, labeltype >>>(qsize)).swap(answers);
    DISTFUNC<int> fstdistfunc_ = l2space.get_dist_func();
    cout << qsize << "\n";
    for (int i = 0; i < qsize; i++) {
        for (int j = 0; j < k; j++) {
            answers[i].emplace(0.0f, massQA[100 * i + j]);
        }
    }
}

static float
test_approx(float *massQ, size_t vecsize, size_t qsize, HierarchicalNSW<int> &appr_alg, size_t vecdim,
            vector<std::priority_queue<std::pair<int, labeltype >>> &answers, size_t k, size_t HW) {
    size_t correct = 0;
    size_t total = 0;
    //uncomment to test in parallel mode:
    //#pragma omp parallel for
    for (int i = 0; i < qsize; i++) {

        std::priority_queue<std::pair<int, labeltype >> result = appr_alg.searchKnn(massQ + vecdim * i, k);
        std::priority_queue<std::pair<int, labeltype >> gt(answers[i]);
        unordered_set<labeltype> g;
        total += gt.size();

        while (gt.size()) {
            g.insert(gt.top().second);
            gt.pop();
        }

        while (result.size()) {
            if (g.find(result.top().second) != g.end()) {

                correct++;
            } else {
            }
            result.pop();
        }

    }
    return 1.0f * correct / total;
}

static void
test_vs_recall(float *massQ, size_t vecsize, size_t qsize, HierarchicalNSW<int> &appr_alg, size_t vecdim,
               vector<std::priority_queue<std::pair<int, labeltype >>> &answers, size_t k) {
    vector<size_t> efs;// = { 10,10,10,10,10 };
    for (int i = k; i < 30; i++) {
        efs.push_back(i);
    }
    for (int i = 30; i < 100; i += 10) {
        efs.push_back(i);
    }
    for (int i = 100; i < 500; i += 40) {
        efs.push_back(i);
    }
    for (size_t ef : efs) {
        appr_alg.setEf(ef);
        StopW stopw = StopW();

        float recall = test_approx(massQ, vecsize, qsize, appr_alg, vecdim, answers, k, 0);
        float time_us_per_query = stopw.getElapsedTimeMicro() / qsize;

        cout << ef << "\t" << recall << "\t" << time_us_per_query << " us\n";
        if (recall > 1.0) {
            cout << recall << "\t" << time_us_per_query << " us\n";
            break;
        }
    }
}

static void
test_HW_recall(float* massQ, size_t vecsize, size_t qsize, HierarchicalNSW<int>& appr_alg, size_t vecdim,
	vector<std::priority_queue<std::pair<int, labeltype >>>& answers, size_t k) {
	vector<size_t> efs;// = { 10,10,10,10,10 };

	for (int i = 0; i < 50; i += 10) {
		efs.push_back(i);
	}
	for (size_t ef : efs) {
		appr_alg.setEf(ef);
		StopW stopw = StopW();

		float recall = test_approx(massQ, vecsize, qsize, appr_alg, vecdim, answers, k, 0);
		float time_us_per_query = stopw.getElapsedTimeMicro() / qsize;
		cout << ef << "\t" << recall << "\t" << time_us_per_query << " us\n";

		stopw = StopW();
		float recall_hw = test_approx(massQ, vecsize, qsize, appr_alg, vecdim, answers, k, 1);
		time_us_per_query = stopw.getElapsedTimeMicro() / qsize;
		cout << ef << " HW\t" << recall_hw << "\t" << time_us_per_query << " us\n";

		if (recall != recall_hw) {
			cout << "HW Error!!\n";
			break;
		}
	}
}

inline bool exists_test(const std::string &name) {
    ifstream f(name.c_str());
    return f.good();
}


void sift_test1B() {
    int subset_size_milllions = 1;          //200;
    int efConstruction = 40;                //controls index search speed/build speed tradeoff
    int M = 16;                             //is tightly connected with internal dimensionality of the data. stongly affects the memory cosumption
                                            //Higher M leads to higher accuracy/run_time at fixed ef/efconstruction

    size_t vecsize = subset_size_milllions * 10000;

    size_t qsize = 10000;
    size_t vecdim = 128;
    char path_index[1024];
    char path_gt[1024];

    const char* path_q = "../bigann/ANN_SIFT1M/sift_query.fvecs";
    const char* path_data = "../bigann/ANN_SIFT1M/sift_base.fvecs";
    sprintf_s(path_index, "../bigann/sift1b_%dm_ef_%d_M_%d.bin", subset_size_milllions, efConstruction, M);
    sprintf_s(path_gt, "../bigann/ANN_SIFT1M/sift_groundtruth.ivecs");

    float *massb = new float[vecdim];

    cout << "Loading GT:\n";
    //-------------------------------------------------------------------------------------------------
    // Loading File(path_gt) : "siftsmall_groundtruth.ivecs" :
    // INFO :
    //      - ivecs file : The groundtruth file in is .ivecs format
    //      - Groundtruth: exhaustive search using exhaustive Euclidean squared distance calculation
    //-------------------------------------------------------------------------------------------------
    ifstream inputGT(path_gt, ios::binary);
    unsigned int *massQA = new unsigned int[qsize * 100];
    for (int i = 0; i < qsize; i++) {
        int t;
        inputGT.read((char *) &t, 4);
        inputGT.read((char *) (massQA + 100 * i), t * 4);
        // if you use big dataset(ANN_SIFT1B), you need to set "t !=1000"
        if (t != 100) {
            cout << "err";
            return;
        }
    }
    inputGT.close();

    cout << "Loading queries:\n";
    //-------------------------------------------------------------------------------------------------
    // Loading File(path_q) : "siftsmall_query.fvecs" :
    // INFO :
    //      - fvecs file : The float vector files (Each vector takes 4+d*4 bytes)
    //      - _query : query vectors
    //-------------------------------------------------------------------------------------------------
    ifstream inputQ(path_q, ios::binary);
    float* massQ = new float[qsize * vecdim];
    for (int i = 0; i < qsize; i++) {
        int t;
        inputQ.read((char*)&t, 4);
        inputQ.read((char*)(massQ + vecdim * i), t * 4);
        if (t != 128) {
            cout << "err";
            return;
        }
    }
    inputQ.close();

    float *mass = new float[vecdim];
    ifstream input(path_data, ios::binary);
    int in = 0;
    L2SpaceI l2space(vecdim);
    
    HierarchicalNSW<int> *appr_alg;
    if (exists_test(path_index)) {
        //-------------------------------------------------------------------------------------------------
        // Loading File (path_index) : "_sift1b_1m_ef_40_M_16.bin" : 
        // INFO : 
        //      - bin file : The vector Index files 
        //-------------------------------------------------------------------------------------------------
        cout << "Loading index from " << path_index << ":\n";
        appr_alg = new HierarchicalNSW<int>(&l2space, path_index, false);
        cout << "Actual memory usage: " << getCurrentRSS() / 1000000 << " Mb \n";
    } else {
        cout << "Building index:\n";
        appr_alg = new HierarchicalNSW<int>(&l2space, vecsize, M, efConstruction);
        //-------------------------------------------------------------------------------------------------
        // Loading File (path_data) : "siftsmall_base.fvecs" :
        // INFO :
        //      - fvecs file : The vector files (Each vector takes 4+d*4 bytes)
        //      - _base : the vectors in which the search is performed
        //-------------------------------------------------------------------------------------------------
        input.read((char*)&in, 4);
        input.read((char*)(massb), in * 4);
        if (in != 128) {
                cout << "err";
                return;
            }
        for (int j = 0; j < vecdim; j++) {
            mass[j] = massb[j] * (1.0f);
        }

        appr_alg->addPoint((void *) (massb), (size_t) 0);
        int j1 = 0;
        StopW stopw = StopW();
        StopW stopw_full = StopW();
        size_t report_every = 10000;
#pragma omp parallel for
        for (int i = 1; i < vecsize; i++) {
            float mass[128];
            int j2=0;
#pragma omp critical
            {
                input.read((char*)&in, 4);
                input.read((char*)(massb), in * 4);
                if (in != 128) {
                    cout << "err";
                    return;
                }
                for (int j = 0; j < vecdim; j++) {
                    mass[j] = massb[j] * (1.0f);
                }

                j1++;
                j2=j1;
                if (j1 % report_every == 0) {
                    cout << j1 / (0.01 * vecsize) << " %, "
                         << report_every / (1000.0 * 1e-6 * stopw.getElapsedTimeMicro()) << " kips " << " Mem: "
                         << getCurrentRSS() / 1000000 << " Mb \n";
                    stopw.reset();
                }
            }
            appr_alg->addPoint((void *) (mass), (size_t) j2);
        }
        input.close();
        cout << "Build time:" << 1e-6 * stopw_full.getElapsedTimeMicro() << "  seconds\n";
        appr_alg->saveIndex(path_index);
    }

    vector<std::priority_queue<std::pair<int, labeltype >>> answers;
    size_t k = 1;
    cout << "Parsing gt:\n";
    get_gt(massQA, massQ, mass, vecsize, qsize, l2space, vecdim, answers, k);
    cout << "Loaded gt\n";

#if 0
    for (int i = 0; i < 1; i++)
        test_vs_recall(massQ, vecsize, qsize, *appr_alg, vecdim, answers, k);
#else
	test_HW_recall(massQ, vecsize, qsize, *appr_alg, vecdim, answers, k);
#endif

    cout << "Actual memory usage: " << getCurrentRSS() / 1000000 << " Mb \n";
    return;

}
