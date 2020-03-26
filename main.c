#include "stdio.h"
#include "stdlib.h"

/*���ڵ�*/
typedef struct node{
	int data;
	struct node * left; /*�ڵ���ߵ���֦*/
	struct node * right;/*�ڵ��ұߵ���֦*/
}Node;

/*����*/
typedef struct tree{
	Node * root;
}Tree;

/*���뺯�� ��һ���������������*/
void insert(Tree* tree, int value)
{
	/*����һ���ڵ�*/
    Node* node=(Node*)malloc(sizeof(Node));
    node->data = value;
    node->left = NULL;
    node->right = NULL;

    /*�ж����ǲ��ǿ���*/
    if (tree->root == NULL)
    {
        tree->root = node;
    }
    else /*���ǿ���*/
	{
        Node* temp = tree->root;/*��������ʼ*/
        while (temp != NULL)
        {
            if(value < temp->data)/*С�ھͽ������*/
            {
                if(temp->left == NULL)
                {
                    temp->left = node;
                    return;
                }
                else /*�����ж�*/
				{
                    temp = temp->left;
                }
            }
            else /*������Ҷ���*/
			{

                if(temp->right == NULL)
                {
                    temp->right = node;
                    return;
                }
                else /*�����ж�*/
				{
                    temp = temp->right;
                }
            }
        }
    }
}

/*
 ����һ������
 �������:����������
 */
void traverse(Node* node)
{
    if(node != NULL)
    {
        traverse(node->left);
        printf("node data:%d \n",node->data);
        traverse(node->right);
    }
}

/*����һ����*/
void distory_tree(Node* node)
{

	if(node != NULL)
	{
		distory_tree(node->left);
        distory_tree(node->right);
        printf("free node:%d\n",node->data);
		free(node);
		node = NULL;
	}
}

/*������*/
int main()
{
	int i = 0;
	Tree tree;
    tree.root = NULL;/*����һ������*/
    int n;
    printf("input total num:\n");
    /*����n���������������*/
    scanf("%d",&n);
    for(i = 0; i < n; i++)
    {
        int temp;
        scanf("%d",&temp);
        insert(&tree, temp);
    }
    /*����������*/
    traverse(tree.root);

	/*����һ����*/
	distory_tree(tree.root);
	return 0;
}
