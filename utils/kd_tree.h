#ifndef KD_TREE_H
#define KD_TREE_H

#include <stdint.h>
#include <memory>
#include <assert.h>

template<typename T, typename Pointer2 = uintptr_t*>
struct TreeNode_t
{
	using Pointer = typename std::pointer_traits<Pointer2>::template rebind<TreeNode_t>;
	T value;
	Pointer left = 0, right = 0;

	constexpr const T& operator*() const
	{
		return value;
	}
};

template<typename T, unsigned int K, typename Alloc2 = std::allocator<TreeNode_t<T>>>
struct KDTree
{
  private:
	//heck
	using pointer3 = typename std::allocator_traits<Alloc2>::pointer;
	using TreeNode2 = TreeNode_t<T, pointer3>;
	using pointer2 = typename std::remove_const<typename TreeNode2::Pointer>::type;
	using TreeNode = TreeNode_t<T, pointer2>;
	using Alloc = typename Alloc2::template rebind<TreeNode>::other;
	using pointer = typename std::remove_const<typename TreeNode::Pointer>::type;

	Alloc alloc;
	pointer rootNode;
	pointer freeNode;
	size_t treeSize;
	size_t freeSize = 0;

  public:

	~KDTree()
	{
		Free();
	}

	constexpr size_t size()
	{
		return treeSize;
	}

	constexpr pointer Insert(const T& entry)
	{
		pointer idx = 0;

		Insert(rootNode, entry, 0, &idx);

		if (!rootNode)
			rootNode = idx;

		return idx;
	}

	constexpr pointer Find(const T& entry)
	{
	    return Find(rootNode, entry, 0);
	}

	constexpr void DeleteNode(const pointer& ref)
	{
		DeleteNode(rootNode, ref->value, 0);
	}

	void WalkDelete(pointer root)
	{
		if (!root)
			return;

		WalkDelete(root->left);
		WalkDelete(root->right);
		Deallocate(root);
		treeSize--;
	}

	void Clear()
	{
		WalkDelete(rootNode);
		assert(!treeSize);
		rootNode = 0;
		treeSize = 0;
	}

	void WalkFree(pointer root)
	{
		if (!root)
			return;

		WalkFree(root->left);
		root->left = nullptr;
		alloc.deallocate(root, 1);
		freeSize--;
	}

	void Free()
	{
		WalkFree(freeNode);
		assert(!freeSize);
		freeNode = nullptr;
	}

  private:

	pointer Allocate()
	{
		if (freeNode) {
			pointer ret = freeNode;
			freeNode = freeNode->left;
			freeSize--;
			return ret;
		}
		return alloc.allocate(1);
	}

	void Deallocate(pointer ptr)
	{
		ptr->left = freeNode;
		freeNode = ptr;
		freeSize++;
	}

	pointer Insert(pointer root, const T& entry, unsigned int depth, pointer* out)
	{
		if (!root) {
			root = Allocate();
			*root = TreeNode();
		    treeSize++;
		    root->value = entry;
			if (out)
				*out = root;
			return root;
		}

		unsigned int d = depth % K;

		if (entry[d] < root->value[d])
			root->left = Insert((pointer)root->left, entry, depth + 1, out);
		else
			root->right = Insert((pointer)root->right, entry, depth + 1, out);

		return root;
	}

	pointer Find(pointer root, const T& entry, unsigned int depth)
	{
		if (!root)
			return 0;

		if (entry == root->value)
			return root;

		unsigned int d = depth % K;

		if (entry[d] < root->value[d])
			return Find((pointer)root->left, entry, depth + 1);

		return Find((pointer)root->right, entry, depth + 1);
	}

	pointer MinNode(pointer x, pointer y, pointer z, unsigned int d)
	{
		pointer res = x;
		if (y && z->value[d] < res->value[d])
			res = y;
		if (z && z->value[d] < res->value[d])
			res = z;
		return res;
	}

	pointer FindMin(pointer root, int dim, unsigned int depth)
	{
		if (!root)
			return 0;

		unsigned int d = depth % K;

		if (d == dim) {
			if (!root->left)
				return root;
			return FindMin(root->left, dim, depth + 1);
		}

		return MinNode(root, FindMin(root->left, dim, depth + 1), FindMin(root->right, dim, depth + 1));
	}

	pointer DeleteNode(pointer root, const T& entry, unsigned int depth)
	{
		if (!root)
			return 0;

		unsigned int d = depth % K;

		if (entry == root->value) {
			if (root->right) {
				pointer min = FindMin((pointer)root->right, d, 0);
				root->value = ((pointer)root->right)->value;
				root->right = DeleteNode((pointer)root->right, min->value, depth + 1);
			} else if (root->left) {
				pointer min = FindMin((pointer)root->left, d, 0);
				root->value = ((pointer)root->left)->value;
				root->left = DeleteNode((pointer)root->left, min->value, depth + 1);
			} else {
				Deallocate(root);
				treeSize--;
				return 0;
			}
			return root;
		}

		if (entry[d] < root->value[d])
		    root->left = DeleteNode((pointer)root->left, entry, depth + 1);
		else
		    root->right = DeleteNode((pointer)root->right, entry, depth + 1);

		return root;
	}
};


#endif
