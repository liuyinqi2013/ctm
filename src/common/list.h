#ifndef H_RESOLVER_LIST_H_
#define H_RESOLVER_LIST_H_

namespace ctm
{
    template <typename T>
    class List
    {
    public:
        class ListNode {
        public:
            ListNode* Next() 
            {
                if (next && list && list->m_head != next) return next;
                return NULL;
            }
            ListNode* Prev() 
            {
                if (prev && list && list->m_head != prev) return prev;
                return NULL;
            }

            T& Data() { return data; }
            
        private:
            friend class List;

            ListNode() : prev(NULL), next(NULL) {
            }

            struct ListNode *prev;
            struct ListNode *next;
            T data;
            List *list;
        };

        friend class ListNode;

    public:
        List() : m_size(0), m_head(new ListNode) {
            m_head->prev = m_head;
            m_head->next = m_head;
        }

        ~List() 
        {
            Clear();
            delete m_head;
        }

        void Clear()
        {
            ListNode* node = m_head->next;
            ListNode* tmp; 
            while (node != m_head) {
                tmp = node;
                node = node->next;
                Cut(tmp);
                delete tmp;
            }
            m_size = 0;
        }

        bool Empty() 
        {
            return m_head == m_head->next;
        }

        int Size() 
        {
            return m_size;
        }

        ListNode* Head() 
        {
            if (Empty()) return NULL;
            return m_head->next;
        }

        ListNode* Tail() 
        {
            if (Empty()) return NULL;
            return m_head->prev;
        }

        ListNode* InsertHead(T data) 
        {
            return InsertAfter(m_head, data);
            
        }

        ListNode* InsertTail(T data) 
        {
            return InsertBefore(m_head, data);
        }

        ListNode* InsertBefore(ListNode* h, T data) 
        {
            ListNode* node = new ListNode;
            node->data = data;
            node->list = this;
            InsertBefore(h, node);
            m_size++;
            return node;
        }

        ListNode* InsertAfter(ListNode* h, T data) 
        {
            ListNode* node = new ListNode;
            node->data = data;
            node->list = this;
            InsertAfter(h, node);
            m_size++;
            return node;
        }

        void Remove(ListNode* node) 
        {
            Cut(node);
            delete node;
            m_size--;
        }

        void MoveToHead(ListNode* node) 
        {
            Move(m_head->next, node);
        }

        void MoveToTail(ListNode* node) 
        {
            Move(m_head, node);
        }

        void Move(ListNode* h, ListNode* node) 
        {
            Cut(node);
            InsertBefore(h, node);
        }

    private:
        void InsertBefore(ListNode* h, ListNode* node) 
        {
            node->prev = h->prev;
            node->next = h;
            h->prev->next = node;
            h->prev = node;
        }

        void InsertAfter(ListNode* h, ListNode* node) 
        {
            node->next = h->next;
            node->prev = h;
            h->next->prev = node;
            h->next = node;
        }

        void Cut(ListNode* node) 
        {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

    private:
        int m_size;
        ListNode* m_head;
    };

    void TestList();

}

#endif