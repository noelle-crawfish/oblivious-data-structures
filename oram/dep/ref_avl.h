#pragma once

//  this implementation is just from geeksforgeeks
//  goal is to make sure the tree is consistent
struct node_ref {
	int key; 
	node_ref *left;
	node_ref *right; 
	int height; 
	node_ref(int k) {
		key = k; 
		left = nullptr; 
		right = nullptr; 
		height = 1;
	}
};

class ref_avl{
	public:
		ref_avl(); 
		void insert(int key); 
		void remove(int key); 
		void prefix_print();
	protected: 
		void prefix_print(node_ref *root);
		node_ref *insert(int key, node_ref *root); 
		node_ref *remove(int key, node_ref *root); 
		node_ref *left_rotate(node_ref *root); 
		node_ref *right_rotate(node_ref *root); 
		int height(node_ref *root); 
		int get_balance(node_ref *root); 
		node_ref *minValuenode_ref(node_ref* node_ref); 
	private: 
		node_ref *root;  
};

