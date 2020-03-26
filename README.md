1.红黑树性质

1.节点是红色或者黑色
2.根节点是黑色
3.所有叶子节点（null）都为黑色
4.红色节点的子节点都是黑色
5.从根节点到叶子节点所有路径上黑色节点数相同

2.基础

2.1 节点结构

struct rb_node {
    unsigned long  __rb_parent_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
不难看出 rb_left 和 rb_right 是该节点左右子节点的指针，结构体后面的__attribute__((aligned(sizeof(long))))让这个结构体按照4字节对齐（64位是8字节）。 __rb_parent_color这个参数名字有点奇怪，父亲的颜色？啥意思？不过当我们理解了这个参数的意义之后发现还真就应该叫这个名字。这个名字的意思其实是 "父亲和颜色"，它即代表了父节点的地址，也代表了自身的颜色。有人可能要问了，就这一个参数，怎么代表了两个东西？这个其实也简单，之前我们不是提到了这个rb_node是按照4字节对齐的嘛，那么当我们声明一个rb_node实例的时候，它的首地址必然是4的整数倍。首地址是4的整数倍，那地址的0bit和1bit肯定都是0嘛，不管怎么玩它都是0，那不就没啥"意义"了么，不如拿过来放颜色。那我们就让这个__rb_parent_color 最低位，也就是第0bit来表示颜色，0代表红色，1代表黑色。当我们需要颜色的时候我们就看第0bit，当我们需要父节点地址的时候我们就把最低两位弄成0就行啦。

//颜色宏
#define    RB_RED        0
#define    RB_BLACK    1
2.2 左旋和右旋

对E进行左旋null对S进行右旋null当然图片中演示的是一个大致流程，因为红黑树节点中还包含父节点地址以及自身颜色等信息，所以在实际旋转的时候需要考虑更多。

2.3 辅助函数__rb_rotate_set_parents

这个函数的作用：将old的颜色和父节点给new，之后old将new作为新的父节点并且设置自己的颜色为color

static inline void
__rb_rotate_set_parents(struct rb_node *old, struct rb_node *new,
            struct rb_root *root, int color)
{
    struct rb_node *parent = rb_parent(old);
    new->__rb_parent_color = old->__rb_parent_color;//步骤1
    rb_set_parent_color(old, new, color);//步骤2
    __rb_change_child(old, new, parent, root);//步骤3
}
null在图中我们可以注意到一个点，就是old指向子节点的指针没有改变，在实际程序中是需要将它指向正确位置的，不过并不是由这个辅助函数完成。

3.插入操作

在插入时，我们默认插入的节点是红色，并且只有插入节点的父节点是红色的时候才需要调整（因为违反了性质4），并且我们也能保证每次调整过后，红黑树只会因为不满足性质4而需要再次调整，这点非常重要。在看代码之前，我们需要先讨论一下插入总共涉及到几种情况。插入后一共有6种情况（本质上3种），在逻辑上涉及到这几个节点，祖父节点G，父节点P，叔叔节点U和插入的节点N。其中P是G左孩子有3种情况，P是G右孩子有3种情况，它们是对称的，在逻辑上一致，所以我们可以认为插入操作本质上只有3种情况需要讨论，这里假设P是G的左孩子。

1.

U是红色，此时我们需要变色

2.

U是黑色，并且G、P、N不在一条直线上，折了一下，比如

                         G    
                        / \         
                       P   U 
                        \      
                         N
这种情况，我们需要对P进行左旋（对称情况是右旋），让G、P、N处于同一条直线上，进入情况3

3.

U是黑色，并且G、P、U在同一条直线上，比如

                         G    
                        / \         
                       P   U 
                      /       
                     N
这种情况，我们需要对G进行右旋（对称情况是左旋），旋转完成后红黑树调整完毕。




这里我们可以注意到一个特点，在每次调整完成之后，node总是指向局部满足的子树的根节点。 由此除了case3调整完直接结束之外，我们还能得到另外两个结束条件。

1.

Parent为空，这证明Node已经是根节点了，局部满足即为整体满足，调整结束，另外为了满足根节点为黑色，需要将Node（根节点）设置成黑色。

2.

Parent为黑色，之前我们有说过 "每次调整过后，红黑树只会因为不满足性质4而需要再次调整"，既然Parent为黑色，那无论Node是什么颜色，Node和Parent都不可能为红色，也就必然不会违反性质4，所以调整结束。

static __always_inline void
__rb_insert(struct rb_node *node, struct rb_root *root,
     void (*augment_rotate)(struct rb_node *old, struct rb_node *new))
{
 struct rb_node *parent = rb_red_parent(node), *gparent, *tmp;

 while (true) {
     /*
      * Loop invariant: node is red
      *
      * If there is a black parent, we are done.
      * Otherwise, take some corrective action as we don't
      * want a red root or two consecutive red nodes.
      */
     if (!parent) {
         rb_set_parent_color(node, NULL, RB_BLACK);
         break;
     } else if (rb_is_black(parent))
         break;

     gparent = rb_red_parent(parent);

     tmp = gparent->rb_right;
     if (parent != tmp) {    /* parent == gparent->rb_left */
         if (tmp && rb_is_red(tmp)) {
             /*
              * Case 1 - color flips
              *
              *       G            g
              *      / \          / \
              *     p   u  -->   P   U
              *    /            /
              *   n            n
              *
              * However, since g's parent might be red, and
              * 4) does not allow this, we need to recurse
              * at g.
              */
             rb_set_parent_color(tmp, gparent, RB_BLACK);
             rb_set_parent_color(parent, gparent, RB_BLACK);
             node = gparent;
             parent = rb_parent(node);
             rb_set_parent_color(node, parent, RB_RED);
             continue;
         }

         tmp = parent->rb_right;
         if (node == tmp) {
             /*
              * Case 2 - left rotate at parent
              *
              *      G             G
              *     / \           / \
              *    p   U  -->    n   U
              *     \           /
              *      n         p
              *
              * This still leaves us in violation of 4), the
              * continuation into Case 3 will fix that.
              */
             tmp = node->rb_left;
             WRITE_ONCE(parent->rb_right, tmp);
             WRITE_ONCE(node->rb_left, parent);
             if (tmp)
                 rb_set_parent_color(tmp, parent,
                             RB_BLACK);
             rb_set_parent_color(parent, node, RB_RED);
             augment_rotate(parent, node);
             parent = node;
             tmp = node->rb_right;
         }

         /*
          * Case 3 - right rotate at gparent
          *
          *        G           P
          *       / \         / \
          *      p   U  -->  n   g
          *     /                 \
          *    n                   U
          */
         WRITE_ONCE(gparent->rb_left, tmp); /* == parent->rb_right */
         WRITE_ONCE(parent->rb_right, gparent);
         if (tmp)
             rb_set_parent_color(tmp, gparent, RB_BLACK);
         __rb_rotate_set_parents(gparent, parent, root, RB_RED);
         augment_rotate(gparent, parent);
         break;
     } else {
         tmp = gparent->rb_left;
         if (tmp && rb_is_red(tmp)) {
             /* Case 1 - color flips */
             rb_set_parent_color(tmp, gparent, RB_BLACK);
             rb_set_parent_color(parent, gparent, RB_BLACK);
             node = gparent;
             parent = rb_parent(node);
             rb_set_parent_color(node, parent, RB_RED);
             continue;
         }

         tmp = parent->rb_left;
         if (node == tmp) {
             /* Case 2 - right rotate at parent */
             tmp = node->rb_right;
             WRITE_ONCE(parent->rb_left, tmp);
             WRITE_ONCE(node->rb_right, parent);
             if (tmp)
                 rb_set_parent_color(tmp, parent,
                             RB_BLACK);
             rb_set_parent_color(parent, node, RB_RED);
             augment_rotate(parent, node);
             parent = node;
             tmp = node->rb_left;
         }

         /* Case 3 - left rotate at gparent */
         WRITE_ONCE(gparent->rb_right, tmp); /* == parent->rb_left */
         WRITE_ONCE(parent->rb_left, gparent);
         if (tmp)
             rb_set_parent_color(tmp, gparent, RB_BLACK);
         __rb_rotate_set_parents(gparent, parent, root, RB_RED);
         augment_rotate(gparent, parent);
         break;
     }
 }
}