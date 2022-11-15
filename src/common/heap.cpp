#include "heap.h"
#include <stdio.h>

namespace ctm {
    void HeapItem::Fix() 
    { 
        if (m_heap) m_heap->Fix(m_index); 
    }

    void Heap::Push(HeapItem* data)
    {
        data->SetHeap(this);
        m_items.push_back(data);
        int n = Len() - 1;
        data->SetIndex(n);
        Up(n);
    }

    HeapItem* Heap::Pop() 
    {
        HeapItem* tmp = m_items[0];
        
        int n = Len() - 1;
        Swap(0, n);
        Down(0, n);
        m_items.pop_back();

        tmp->SetHeap(NULL);

        return tmp;
    }

    HeapItem* Heap::Remove(int i)
    {
        int n = Len() - 1;
        HeapItem* tmp = m_items[i];
        if (i != n) {
            Swap(i, n);
            if (!Down(i, n)) Up(i);
        }

        m_items.pop_back();
        tmp->SetHeap(NULL);
        return tmp;
    }  

    void Heap::Init() 
    {
        int n = Len();
        for (int i = (n - 1) / 2; i >= 0; i--) {
            Down(i, n);
        }
    }


    bool Heap::Down(int i, int n)
    {
        int old = i; 
        int left, right, j;
        while(1) 
        {
            left = (2 * i) + 1;
            if (left >= n) {
                break;
            }

            j = left;
            right = left + 1;
            if (right < n && *m_items[right] < *m_items[left]) {
                j = right;
            }

            if (*m_items[i] < *m_items[j]) {
                break;
            }

            Swap(i, j);
            i = j;
        }

        return old != i;
    }

    void Heap::Up(int i)
    {
        int p;
        while (1)
        {
            p = (i - 1) / 2;
            if (p == i || *m_items[p] < *m_items[i]) {
                break;
            }
            Swap(p, i);
            i = p;
        }
    }


    void Heap::Swap(int i, int j) 
    {
        HeapItem* tmp = m_items[i];
        m_items[i] = m_items[j];
        m_items[j] = tmp;

        m_items[i]->SetIndex(i);
        m_items[j]->SetIndex(j);
    }

    class IntItem : public HeapItem
    {    
    public:
        IntItem(int a) : n(a) {}
        virtual bool Less(const HeapItem* o) 
        {
            return n < dynamic_cast<const IntItem*>(o)->n;
        }

        int Data() {  return n; }
        void Set(int m) {  n = m; Fix(); }
    private:
        int n;
    };
    
    void TestHeap()
    {
        Heap h;

        auto a1 = new IntItem(1);
        auto a2 = new IntItem(2);
        auto a3 = new IntItem(3);
        auto a4 = new IntItem(4);

        h.Push(a4);
        h.Push(a2);
        h.Push(a3);
        h.Push(a1);

        a2->Set(10);
        a3->Set(0);

        printf("index:%d\n", a3->Index());
        h.Remove(a3->Index());

        while (h.Len())
        {
            auto e = dynamic_cast<IntItem*>(h.Pop());
            printf("int index:%d, data:%d\n", e->Index(), e->Data());
        }
    }
}