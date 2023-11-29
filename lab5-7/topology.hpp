#ifndef TOPOLOGY_H
#define TOPOLOGY_H

typedef const char* Error;

template <typename T>
class Topology {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;

        explicit Node(const T& value) : data(value), left(nullptr), right(nullptr) {}
    };

    Node* root;

public:
    Topology() : root(nullptr) {}
    ~Topology() {
        destroyTree(root);
    }

    Error insert(const T& value) {
        bool have = searchNode(root, value);
        if (have) {
            return "Error: Already exists";
        }
        root = insertNode(root, value);
        return nullptr;
    }

    bool search(const T& value) {
        return searchNode(root, value);
    }

private:
    Node* insertNode(Node* node, const T& value) {
        if (node == nullptr) {
            return new Node(value);
        }

        if (value < node->data) {
            node->left = insertNode(node->left, value);
        } else if (value > node->data) {
            node->right = insertNode(node->right, value);
        }

        return node;
    }

    bool searchNode(Node* node, const T& value) {
        if (node == nullptr) {
            return false;
        }

        if (value == node->data) {
            return true;
        } else if (value < node->data) {
            return searchNode(node->left, value);
        } else {
            return searchNode(node->right, value);
        }
    }

    void destroyTree(Node* node) {
        if (node == nullptr) {
            return;
        }

        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
};

#endif //TOPOLOGY_H
