#include <stdio.h>
#include <stdlib.h>
/* word */
struct node {
    int data;
    struct node *left;
    struct node *right;
};

struct node* createNode(int data) {
    struct node* newNode = malloc(sizeof(struct node));
    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;

    return newNode;
}

void insert(struct node **root, int data) {
    if (*root == NULL) {
        *root = createNode(data);
        return;
    }

    if (data < (*root)->data) {
        insert(&(*root)->left, data);
    } else if (data > (*root)->data) {
        insert(&(*root)->right, data);
    }
}

void printInorder(struct node *root) {
    if (root != NULL) {
        printInorder(root->left);
        printf("%d ", root->data);
        printInorder(root->right);
    }
}

void printPreorder(struct node *root) {
    if (root != NULL) {
        printf("%d ", root->data);
        printPreorder(root->left);
        printPreorder(root->right);
    }
}

void printPostorder(struct node *root) {
    if (root != NULL) {
        printPostorder(root->left);
        printPostorder(root->right);
        printf("%d ", root->data);
    }
}

int main() {
    int arr[] = { 5, 3, 7, 1, 4, 6, 8 };
    int n = sizeof(arr)/sizeof(arr[0]);
    struct node *root = NULL;

    for (int i = 0; i < n; i++) {
        insert(&root, arr[i]);
    }

    printf("Inorder traversal of binary tree:\n");
    printInorder(root);

    printf("\nPreorder traversal of binary tree:\n");
    printPreorder(root);

    printf("\nPostorder traversal of binary tree:\n");
    printPostorder(root);
	printf("\n");
    return 0;
}