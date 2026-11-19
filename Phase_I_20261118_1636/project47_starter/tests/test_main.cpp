#include "project47/algorithms/job_order.hpp"
#include "project47/algorithms/merge_sort.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct TaggedValue {
    int key;
    std::string tag;
};

void test_merge_sort_integers() {
    std::vector<int> values{7, -2, 4, 4, 0, 19, 1};
    project47::merge_sort(values);
    assert((values == std::vector<int>{-2, 0, 1, 4, 4, 7, 19}));
}

void test_merge_sort_empty_and_single() {
    std::vector<int> empty;
    std::vector<int> single{42};
    project47::merge_sort(empty);
    project47::merge_sort(single);
    assert(empty.empty());
    assert((single == std::vector<int>{42}));
}

void test_merge_sort_is_stable() {
    std::vector<TaggedValue> values{{2, "first"}, {1, "middle"}, {2, "second"}};
    project47::merge_sort(values, [](const TaggedValue& a, const TaggedValue& b) {
        return a.key < b.key;
    });
    assert(values[0].tag == "middle");
    assert(values[1].tag == "first");
    assert(values[2].tag == "second");
}

void test_job_scheduling_order() {
    std::vector<project47::Job> jobs{
        {3, "t3", 2, 5.0, 100, project47::JobStatus::Pending},
        {2, "t2", 4, 8.0, 100, project47::JobStatus::Pending},
        {1, "t1", 4, 3.0, 100, project47::JobStatus::Pending},
        {4, "t4", 4, 3.0, 100, project47::JobStatus::Pending}
    };
    project47::sort_jobs_for_scheduling(jobs);
    assert(jobs[0].id == 1);
    assert(jobs[1].id == 4);
    assert(jobs[2].id == 2);
    assert(jobs[3].id == 3);
}

void test_invalid_job_is_rejected() {
    std::vector<project47::Job> jobs{
        {0, "tile", 1, 2.0, 100, project47::JobStatus::Pending}
    };
    bool threw = false;
    try {
        project47::sort_jobs_for_scheduling(jobs);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

} // namespace

int main() {
    test_merge_sort_integers();
    test_merge_sort_empty_and_single();
    test_merge_sort_is_stable();
    test_job_scheduling_order();
    test_invalid_job_is_rejected();
    std::cout << "All Project 47 tests passed.\n";
    return 0;
}
