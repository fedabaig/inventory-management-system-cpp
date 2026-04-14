#ifndef STACK_H
#define STACK_H

#include <cstddef>   // size_t
#include <stdexcept> // runtime_error

// A simple stack (Last In, First Out).
// Used for "Undo" in the inventory system.
template <typename T>
class Stack {
private:
    // Node holds one value and points to the next node
    struct Node {
        T data;
        Node* next = nullptr;
        explicit Node(const T& d) : data(d) {}
    };

    Node* topNode = nullptr; // Top of the stack
    std::size_t n = 0;       // Number of items

public:
    Stack() = default;

    // Free memory when stack is destroyed
    ~Stack() { clear(); }

    // Disable copying to keep it simple
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    // Delete all nodes in the stack
    void clear() {
        while (topNode) {
            Node* nxt = topNode->next;
            delete topNode;
            topNode = nxt;
        }
        topNode = nullptr; // (extra clear)
        n = 0;
    }

    // Return true if stack is empty
    bool empty() const { return topNode == nullptr; }

    // Return number of items in the stack
    std::size_t size() const { return n; }

    // Push a new item onto the stack
    void push(const T& t) {
        Node* node = new Node(t);
        node->next = topNode;
        topNode = node;
        ++n;
    }

    // Look at the top item without removing it (optional helper)
    const T& peek() const {
        if (empty()) throw std::runtime_error("Stack is empty.");
        return topNode->data;
    }

    // Pop the top item and return it
    T pop() {
        if (empty()) throw std::runtime_error("Undo stack is empty.");

        Node* node = topNode;
        T out = node->data;

        topNode = node->next;
        delete node;
        --n;

        return out;
    }
};

#endif
