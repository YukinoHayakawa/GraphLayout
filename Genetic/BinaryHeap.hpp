#pragma once

#include <vector>

namespace usagi
{
/**
 * \brief With Comparator == less, this maintains the smallest element at the
 * top.
 * \tparam T Supposed to be a pointer to external structure.
 * \tparam Comparator Use < for min heap, > for max heap.
 */
template <
    typename T,
    typename Comparator
>
class BinaryHeap
{
    std::vector<T> mHeap;
    Comparator mComparator;

    static constexpr std::size_t parent(const std::size_t node_idx)
    {
        return (node_idx - 1) / 2;
    }

    static constexpr std::size_t leftChild(const std::size_t node_idx)
    {
        return 2 * node_idx + 1;
    }

    static constexpr std::size_t rightChild(const std::size_t node_idx)
    {
        return 2 * node_idx + 2;
    }

    void swap(const std::size_t a, const std::size_t b)
    {
        std::swap(mHeap[a], mHeap[b]);
        // maintain correct references from tiles to queue
        mHeap[a]->queue_index = a;
        mHeap[b]->queue_index = b;
    }

    void bubble(const std::size_t index)
    {
        assert(index < mHeap.size());
        for(size_t idx = index, parent_idx = parent(idx);
            idx > 0 /* has a parent */;
            idx = parent_idx, parent_idx = parent(idx))
        {
            if(mComparator(mHeap[idx], mHeap[parent_idx]))
                swap(idx, parent_idx); // bubble up
        }
    }

    void sink(const std::size_t index)
    {
        assert(index < mHeap.size());
        size_t cur = index, min_child;
        while((min_child = minChild(cur)) != INVALID_INDEX)
        {
            if(mComparator(mHeap[cur], mHeap[min_child]))
                break;
            swap(cur, min_child);
            cur = min_child;
        }
    }

    std::size_t minChild(const std::size_t index)
    {
        assert(index < mHeap.size());
        size_t left = leftChild(index), right = rightChild(index);
        if(left >= size())
            if(right >= size())
                return INVALID_INDEX; // index is a leaf node
            else
                return right; // only have right node
        else
            if(right >= size())
                return left; // only have left node
            else
                return mComparator(mHeap[left], mHeap[right])
                    ? left : right;
    }

public:
    constexpr static std::size_t INVALID_INDEX = -1;

    void reserve(const std::size_t capacity)
    {
        mHeap.reserve(capacity);
    }

    bool empty() const
    {
        return mHeap.empty();
    }

    std::size_t size() const
    {
        return mHeap.size();
    }

    void clear()
    {
        mHeap.clear();
    }

    void insert(T &&element)
    {
        std::size_t new_elem_pos = mHeap.size();
        mHeap.push_back(element);
        mHeap.back()->queue_index = new_elem_pos;
        bubble(new_elem_pos);
    }

    T & top()
    {
        assert(!mHeap.empty());
        return mHeap[0];
    }

    T pop()
    {
        assert(!mHeap.empty());

        // prepare the element to be returned
        auto ret = std::move(top());
        ret->queue_index = INVALID_INDEX;

        // we have more than one elements before pop
        if(size() > 1)
        {
            // move last to first
            mHeap.front() = std::move(mHeap.back());
            mHeap.front()->queue_index = 0;
            sink(0);
        }
        mHeap.pop_back();

        return std::move(ret);
    }

    void modifyKey(const std::size_t index)
    {
        // recover proper order. if the element is smaller than its parent
        // then bubble up. otherwise sink down.
        if(index > 0 && mComparator(mHeap[index], mHeap[parent(index)]))
            bubble(index);
        else
            sink(index);
    }
};
}
