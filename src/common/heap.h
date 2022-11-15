#ifndef CTM_COMMON_HEAP_H__
#define CTM_COMMON_HEAP_H__
#include <vector>
#include <stdio.h>

namespace ctm
{
    class Heap;

    class HeapItem 
    {
    public:
        HeapItem(): m_index(-1), m_heap(NULL) {}
        virtual ~HeapItem() {}
        virtual bool Less(const HeapItem* other) = 0;

        int Index() { return m_index; }
        void Fix();
        
        bool operator< (const HeapItem& other)
        {
            return Less(&other);
        }

    private:
        void SetIndex(int i) { m_index = i; }
        void SetHeap(Heap* heap) { m_heap = heap; }

    private:
        int m_index;
        Heap *m_heap;

        friend class  Heap;
    };

    class Heap
	{
    public:
        Heap() {}
        Heap(const std::vector<HeapItem*>& other) { m_items = other; Init(); }

        int Len() { return m_items.size(); }

        void Push(HeapItem* data);
        HeapItem* Top() { return m_items[0]; }
        HeapItem* Pop();
        HeapItem* Remove(int i);

        void Fix(int i) 
        {
            if (!Down(i, Len() - 1)) Up(i);
        }

        void Clear() { m_items.clear(); }

    private:
        void Init();
        void Up(int i);
        bool Down(int i, int n);
        void Swap(int i, int j);

    private:
        std::vector<HeapItem*> m_items;
	};

    void TestHeap();
}
#endif

