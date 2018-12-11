#ifndef KD_TREE_H
#define KD_TREE_H

#include "packed_heap.h"

template<typename T, unsigned int K>
struct KDTree
{
	struct TreeNode
	{
		T value;
		idx_t left, right;

	    constexpr const T& operator*() const
		{
			return value;
		}
	};

	struct TreeNodeRef
	{
		const KDTree& ref;
		idx_t allocID;

		constexpr TreeNodeRef(const KDTree& tree, idx_t idx) : ref(tree), allocID(idx)
		{

		}

		constexpr const TreeNode& operator*() const
		{
			return ref[allocID];
		}

		constexpr const TreeNode* operator->() const
		{
			return &ref[allocID];
		}

	};

  private:

	PackedHeapL<TreeNode> alloc;
	idx_t rootNode;
	idx_t treeSize;

  public:

	constexpr idx_t size()
	{
		return treeSize;
	}

    constexpr TreeNode& operator[](idx_t idx)
	{
		return alloc[idx];
	}

    constexpr const TreeNode& operator[](idx_t idx) const
	{
		return alloc[idx];
	}

	constexpr TreeNodeRef Insert(const T& entry)
	{
		idx_t idx = 0;

		Insert(rootNode, entry, 0, &idx);

		if (!rootNode)
			rootNode = idx;

		return TreeNodeRef(*this, idx);
	}

	constexpr TreeNodeRef Find(const T& entry)
	{
		idx_t idx = Find(rootNode, entry, 0);
		return TreeNodeRef(*this, idx);
	}

	constexpr void DeleteNode(const TreeNodeRef& ref)
	{
		DeleteNode(rootNode, ref->value, 0);
	}

	void Clear()
	{
		rootNode = 0;
		treeSize = 0;
		alloc.FreeAll();
	}

  private:

	idx_t Insert(idx_t root, const T& entry, unsigned int depth, idx_t* out)
	{
		if (!root) {
			root = alloc.Alloc();
		    treeSize++;
			alloc[root].value = entry;
			if (out)
				*out = root;
			return root;
		}

		unsigned int d = depth % K;
		auto rootNode = alloc + root;

		if (entry[d] < rootNode->value[d])
			rootNode->left = Insert(rootNode->left, entry, depth + 1, out);
		else
			rootNode->right = Insert(rootNode->right, entry, depth + 1, out);

		return root;
	}

    idx_t Find(idx_t root, const T& entry, unsigned int depth)
	{
		if (!root)
			return 0;

		TreeNode& rootNode = alloc[root];

		if (entry == rootNode.value)
			return root;

		unsigned int d = depth % K;

		if (entry[d] < rootNode.value[d])
			return Find(rootNode.left, entry, depth + 1);

		return Find(rootNode.right, entry, depth + 1);
	}

	idx_t MinNode(idx_t x, idx_t y, idx_t z, unsigned int d)
	{
		idx_t res = x;
		if (y && alloc[y].value[d] < alloc[res].value[d])
			res = y;
		if (z && alloc[z].value[d] < alloc[res].value[d])
			res = z;
		return res;
	}

	idx_t FindMin(idx_t root, int dim, unsigned int depth)
	{
		if (!root)
			return 0;

		unsigned int d = depth % K;

		TreeNode& rootNode = alloc[root];

		if (d == dim) {
			if (!rootNode.left)
				return root;
			return FindMin(rootNode.left, dim, depth + 1);
		}

		return MinNode(root, FindMin(rootNode.left, dim, depth + 1), FindMin(rootNode.right, dim, depth + 1));
	}

	idx_t DeleteNode(idx_t root, const T& entry, unsigned int depth)
	{
		if (!root)
			return 0;

		unsigned int d = depth % K;

		TreeNode& rootNode = alloc[root];

		if (entry == rootNode.value) {
			if (rootNode.right) {
				idx_t min = FindMin(rootNode.right, d, 0);
				rootNode.value = alloc[rootNode.right].value;
				rootNode.right = DeleteNode(rootNode.right, alloc[min].value, depth + 1);
			} else if (rootNode.left) {
				idx_t min = FindMin(rootNode.left, d, 0);
				rootNode.value = alloc[rootNode.left].value;
				rootNode.left = DeleteNode(rootNode.left, alloc[min].value, depth + 1);
			} else {
				alloc.Free(root);
				treeSize--;
				return 0;
			}
			return root;
		}

		if (entry[d] < rootNode.value[d])
			rootNode.left = DeleteNode(rootNode.left, entry, depth + 1);
		else
			rootNode.right = DeleteNode(rootNode.right, entry, depth + 1);

		return root;
	}
};


#endif
