#ifndef BTREE_H
#define BTREE_H

//Shared memory supporting binary tree. Mostly a cut down std::map

template<typename Key, typename T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, T>>>
class BTree
{
	using value_type = std::pair<const Key, T>;
	using key_type = Key;
	using size_type = size_t;
	using pointer = std::allocator_traits<Allocator>::pointer;

	using pointer3 = typename std::allocator_traits<Alloc2>::pointer;
	using TreeNode2 = TreeNode_t<T, pointer3>;
	using pointer2 = typename std::remove_const<typename TreeNode2::Pointer>::type;
	using TreeNode = TreeNode_t<T, pointer2>;
	using Alloc = typename Alloc2::template rebind<TreeNode>::other;
	using pointer = typename std::remove_const<typename TreeNode::Pointer>::type;

	void insert(const std::pair<const Key, T>& pair)
	{

	}
};

#endif
