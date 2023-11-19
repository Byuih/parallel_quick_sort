#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <algorithm>
#include <deque>
#include <mutex>

// ������� ��� ���������� ������� ���������� � ��������� ������
template<typename RandomIt>
void parallel_quick_sort(std::shared_ptr<std::vector<int>> vec, RandomIt first, RandomIt last, std::shared_ptr<std::mutex> mtx) {
    if (last - first < 100000) {
        // ���� ���������� ��������� �� ����������� 100000, ��������� ���������� ����������
        std::sort(first, last);
    }
    else {
        // ����� ��������� �� ��������� � ��������� ����������� ����������
        RandomIt pivot = std::next(first, std::distance(first, last) / 2);
        std::iter_swap(pivot, last - 1);
        pivot = std::partition(first, last - 1, [last](const auto& elem) { return elem < *last - 1; });

        // ������� promise � future ��� ������������� � ������� �������
        std::promise<void> promise;
        auto future = promise.get_future();

        // ��������� ��������� � ���� �������
        std::thread([vec, first, pivot, mtx, promise = std::move(promise)]() mutable {
            parallel_quick_sort(vec, first, pivot, mtx);
            promise.set_value();
        }).detach(); // detach, ����� �� ����� ���������� � ������� ������

        parallel_quick_sort(vec, pivot, last - 1, mtx);

        // ���� ���������� ������ ���������
        future.wait();

        // ������� ����������
        std::lock_guard<std::mutex> lock(*mtx);
        std::inplace_merge(first, pivot, last);
    }
}

int main() {
    std::vector<int> data = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
    std::shared_ptr<std::vector<int>> vec = std::make_shared<std::vector<int>>(data);
    std::shared_ptr<std::mutex> mtx = std::make_shared<std::mutex>();

    // ��������� ����������� ���������� ������� ����������
    parallel_quick_sort(vec, vec->begin(), vec->end(), mtx);

    // ���� ���������� ����������
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // ���� ����� ��� ���������� ���� �������

    // ������� ��������������� ������
    for (const auto& element : *vec) {
        std::cout << element << " ";
    }

    return 0;
}