#ifndef _TREE_H_
#define _TREE_H_

template <class T>
struct STreeNode
{
	T *mData;
	STreeNode *mpFirstChild;
	STreeNode *mpNextBrother;

	STreeNode(T *val)
	{
		mData = val;
		mpFirstChild = NULL;
		mpNextBrother = NULL;
	}

	~STreeNode()
	{
		if(mData != NULL)
		{
			delete mData;
			mData = NULL;
		}
	}
};

template <class T>
class CTree
{
public:
	CTree() : m_pRoot(NULL)
	{

	}
	CTree(T *val)
	{
		m_pRoot = new STreeNode(val);
	}
	~CTree()
	{
		if(m_pRoot != NULL)
		{
			FreeMemory(m_pRoot);
		}
	}

	void Insert(T *parentVal, T *val)
	{
		if(m_pRoot == NULL)
		{
			return;
		}

		STreeNode pNode = Search(m_pRoot, parentVal);
		if(pNode == NULL)
		{
			return;
		}

		if(pNode.mpFirstChild == NULL)
		{
			pNode->mpFirstChild = new STreeNode(val);
		}
		else
		{
			InsertBrother(pNode->mpFirstChild, val);
		}
	}

	void InsertBrother(STreeNode *pNode, T *val)
	{
		if(pNode->mpNextBrother != NULL)
		{
			InsertBrother(pNode->mpNextBrother, val);
		}
		else
		{
			pNode.mpNextBrother = new STreeNode(val);
		}
	}

	STreeNode* Search(STreeNode *pNode, T *val)
	{
		if(pNode == NULL)
		{
			return NULL;
		}

		if(pNode->mData == val)
		{
			return pNode;
		}

		if(pNode->mpFirstChild == NULL && pNode->mpNextBrother == NULL)
		{
			return NULL;
		}
		else
		{
			if(pNode->mpFirstChild != NULL)
			{
				STreeNode *pNodeTemp = Search(pNode->mpFirstChild, val);
				if(pNodeTemp != NULL)
				{
					return pNodeTemp;
				}
				else
				{
					return Search(pNode->mpNextBrother, val);
				}
			}
			else
			{
				return Search(pNode->mpNextBrother, val);
			}
		}
	}

	template <class PreF>
	void PreOrder(STreeNode *pNode, PreF _pred)
	{
		if(pNode == NULL)
		{
			return;
		}

		_pred(pNode);

		PreOrder(pNode->mpFirstChild);
		PreOrder(pNode->mpNextBrother);
	}

	template <class PreF>
	void InOrder(STreeNode *pNode, PreF _pred)
	{
		if(pNode == NULL)
		{
			return;
		}

		InOrder(pNode->mpFirstChild);
		_pred(pNode);
		InOrder(pNode->mpNextBrother);
	}

	template <class PreF>
	void PostOrder(STreeNode *pNode, PreF _pred)
	{
		if(pNode == NULL)
		{
			return;
		}

		PostOrder(pNode->mpFirstChild);	
		PostOrder(pNode->mpNextBrother);
		_pred(pNode);
	}

	void FreeMemory(STreeNode *pNode)
	{
		if(pNode == NULL)
		{
			return;
		}

		if(pNode->mpFirstChild != NULL)
		{
			FreeMemory(pNode->mpFirstChild);
		}

		if(pNode->mpNextBrother != NULL)
		{
			FreeMemory(pNode->mpNextBrother);
		}

		delete pNode;
		pNode = NULL;
	}

private:
	STreeNode *m_pRoot;
};


#endif  //_TREE_H_