#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <cstddef>   // size_t
#include <stdexcept> // runtime_error

// A simple singly linked list.
// Used to store items for each department.
template <typename T>
class LinkedList {
private:
    // Node holds one item and a pointer to the next node
    struct Node {
        T data;
        Node* next = nullptr;
        explicit Node(const T& d) : data(d) {}
    };

    Node* head = nullptr;     // first node
    Node* tail = nullptr;     // last node (to pushBack fast)
    std::size_t n = 0;        // number of items

    // Split the list into two halves and return the head of the second half
    static Node* splitMid(Node* start) {
        if (!start || !start->next) return nullptr;

        Node* slow = start;
        Node* fast = start->next;

        // fast moves 2 steps, slow moves 1 step
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }

        // slow is at the middle
        Node* mid = slow->next;
        slow->next = nullptr; // cut the list
        return mid;
    }

    // Merge two sorted lists into one sorted list
    template <typename Comp>
    static Node* mergeSorted(Node* a, Node* b, Comp comp) {
        if (!a) return b;
        if (!b) return a;

        Node* result = nullptr;

        if (comp(a->data, b->data)) {
            result = a;
            result->next = mergeSorted(a->next, b, comp);
        } else {
            result = b;
            result->next = mergeSorted(a, b->next, comp);
        }

        return result;
    }

    // Merge sort for linked list (recursive)
    template <typename Comp>
    static Node* mergeSort(Node* start, Comp comp) {
        if (!start || !start->next) return start;

        Node* mid = splitMid(start);

        Node* left  = mergeSort(start, comp);
        Node* right = mergeSort(mid, comp);

        return mergeSorted(left, right, comp);
    }

    // After sorting, rebuild tail pointer and count again
    void rebuildTailAndCount() {
        n = 0;
        tail = nullptr;

        for (Node* cur = head; cur; cur = cur->next) {
            tail = cur;
            ++n;
        }
    }

public:
    LinkedList() = default;

    // Free memory when list is destroyed
    ~LinkedList() { clear(); }

    // Disable copy to keep it simple (no deep copy)
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    // Clear the whole list (delete all nodes)
    void clear() {
        Node* cur = head;
        while (cur) {
            Node* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        head = tail = nullptr;
        n = 0;
    }

    // Return number of items
    std::size_t size() const { return n; }

    // Return true if list is empty
    bool empty() const { return head == nullptr; }

    // Add item to the end of the list
    void pushBack(const T& item) {
        Node* node = new Node(item);

        if (!head) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }

        ++n;
    }

    // Find item using a condition (predicate).
    // Returns pointer to item or nullptr.
    template <typename Pred>
    T* findIf(Pred pred) {
        for (Node* cur = head; cur; cur = cur->next) {
            if (pred(cur->data)) return &cur->data;
        }
        return nullptr;
    }

    // Const version of findIf
    template <typename Pred>
    const T* findIf(Pred pred) const {
        for (Node* cur = head; cur; cur = cur->next) {
            if (pred(cur->data)) return &cur->data;
        }
        return nullptr;
    }

    // Run a function for every item in the list
    template <typename Fn>
    void forEach(Fn fn) const {
        for (Node* cur = head; cur; cur = cur->next) {
            fn(cur->data);
        }
    }

    // Sort the list using merge sort (good for linked lists)
    template <typename Comp>
    void sort(Comp comp) {
        head = mergeSort(head, comp);
        rebuildTailAndCount();
    }
};

#endif
