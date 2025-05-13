#include "ref_avl.h"
#include <iostream>
ref_avl::ref_avl() {
    this->root = nullptr; 
}
void ref_avl::insert(int key){
    this->root = insert(key, this->root); 
}
void ref_avl::remove(int key){
    this->root = remove(key, this->root); 
}
void ref_avl::prefix_print(){
    prefix_print(this->root); 
}
node_ref * ref_avl::minValuenode_ref(node_ref* node) {
    node_ref* current = node;

    // loop down to find the leftmost leaf
    while (current->left != nullptr)
        current = current->left;

    return current;
}
node_ref *ref_avl::insert(int key, node_ref *root){
// Perform the normal BST insertion
    if (root == nullptr) 
        return new node_ref(key); 

    if (key < root->key) 
        root->left = insert(key, root->left); 
    else if (key > root->key) 
        root->right = insert(key, root->right); 
    else // Equal keys are not allowed in BST 
        return root; 

    // Update height of this ancestor root 
    root->height = 1 + std::max(height(root->left),
                           height(root->right)); 

    // Get the balance factor of this ancestor root 
    int balance = get_balance(root); 

    // If this root becomes unbalanced, 
    // then there are 4 cases 

    // Left Left Case 
    if (balance > 1 && key < root->left->key) 
        return right_rotate(root); 

    // Right Right Case 
    if (balance < -1 && key > root->right->key) 
        return left_rotate(root); 

    // Left Right Case 
    if (balance > 1 && key > root->left->key) { 
        root->left = left_rotate(root->left); 
        return right_rotate(root); 
    } 

    // Right Left Case 
    if (balance < -1 && key < root->right->key) { 
        root->right = right_rotate(root->right); 
        return left_rotate(root); 
    } 

    // Return the (unchanged) node pointer 
    return root; 
}
node_ref* ref_avl::remove(int key, node_ref* root) {

    if (root == nullptr) {
        return root;
    }
    if (key < root->key) {
        root->left = remove(key, root->left);
    } else if (key > root->key) {
        root->right = remove(key, root->right);
    } else {
        // Node to be deleted found
        if ((root->left == nullptr) || (root->right == nullptr)) {
            node_ref* temp = root->left ? root->left : root->right;

            if (temp == nullptr) {
                // No child
                delete root;
                return nullptr;
            } else {
                // One child
                node_ref* toDelete = root;
                root = temp;
                delete toDelete;
            }
        } else {
            // Two children: find in-order successor
            node_ref* temp = minValuenode_ref(root->right);
            root->key = temp->key;
            root->right = remove(temp->key, root->right);
        }
    }

    // Update height
    root->height = 1 + std::max(height(root->left), height(root->right));

    // Rebalance
    int balance = get_balance(root);

    // LL
    if (balance > 1 && get_balance(root->left) >= 0)
        return right_rotate(root);

    // LR
    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    // RR
    if (balance < -1 && get_balance(root->right) <= 0)
        return left_rotate(root);

    // RL
    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}


node_ref *ref_avl::left_rotate(node_ref *root) {
    node_ref *y = root->right; 
    // node_ref *T2 = y->left; 


    root->right = y->left; 
    y->left = root; 
    // Perform rotation 
    // y->left = root; 
    // root->right = T2; 

    // Update heights 
    root->height = 1 + std::max(height(root->left), 
                        height(root->right)); 
    y->height = 1 + std::max(height(y->left), 
                        height(y->right)); 

    // Return new root 
    return y; 
}
node_ref *ref_avl::right_rotate(node_ref *root) {
    node_ref *x = root->left; 
    node_ref *T2 = x->right; 

    // Perform rotation 
    x->right = root; 
    root->left = T2; 

    // Update heights 
    root->height = 1 + std::max(height(root->left), 
                    height(root->right)); 
    x->height = 1 + std::max(height(x->left), 
                        height(x->right)); 

    // Return new root 
    return x; 
}
int ref_avl::height(node_ref *root) {
    if (root == nullptr) 
        return 0; 
    return root->height; 
}
int ref_avl::get_balance(node_ref *root) {
    if (root == nullptr) {
        return 0; 
    }
    return height(root->left) - height(root->right);
}

void ref_avl::prefix_print(node_ref *root) {
  if(root == nullptr) {
    return;
  }
  std::cout<<root->key << " "; 
  prefix_print(root->left); // left
  prefix_print(root->right); // right
}