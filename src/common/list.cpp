#include "log.h"
#include "list.h"

namespace ctm {
    void ShowListInt(List<int>& l) {
        DEBUG("size:%d", l.Size());
        List<int>::ListNode* node = l.Head();
        for (; node != NULL; node = node->Next()) {
            DEBUG("data = %d", node->Data());
        }
    }

    void TestList() 
    {
        List<int> l;
        List<int>::ListNode* node1 = l.InsertTail(1);
        List<int>::ListNode* node2 = l.InsertTail(2);
        List<int>::ListNode* node3 = l.InsertTail(3);
        List<int>::ListNode* node4 = l.InsertTail(4);
        List<int>::ListNode* node5 = l.InsertTail(5);
        DEBUG("insert tail 1 2 3 4 5");
        ShowListInt(l);

        l.MoveToHead(node5);
        DEBUG("move 5 to head");
        ShowListInt(l);

        l.MoveToTail(node1);
        DEBUG("move 1 to tail");
        ShowListInt(l);

        l.MoveToTail(node4);
        DEBUG("move 4 to tail");
        ShowListInt(l);

        l.Remove(node3);
        DEBUG("remove 3");
        ShowListInt(l);

        l.MoveToHead(node2);
        DEBUG("move 2 to head");
        ShowListInt(l);

        l.Clear();
        DEBUG("clear");
        ShowListInt(l);
    }
}



