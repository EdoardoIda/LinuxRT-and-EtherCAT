/////////////////////////////////////////////////////////////////////////////////
// HashTable.h



///////////////////////////////////////////////////////////////////////////////
//	CHashTable

#define	HASHTABLESIZE		97			// should be a prime

template <class KEY, class VALUE, int SIZE>
class CHashTable
{

typedef void (*PForeEachFunc)( PVOID pThis, VALUE* pValue );

protected:
	typedef struct HashEntry
	{
		HashEntry*	pNext;
		HashEntry*	pNextFree;
		HashEntry*	pNextGlob;
		KEY			key;
		VALUE			value;
	} *PHashEntry;

	HashEntry	m_aHashEntrys[SIZE];
	PHashEntry	m_pFreeEntrys;
	PHashEntry	m_pFirstEntry;
	PHashEntry	m_pEntrys[HASHTABLESIZE];
	unsigned int			m_nKeyCount;

	EC_DECLARE_CRITSEC

public:
	CHashTable()
	{
		int i;
		EC_INIT_CRITSEC

		memset( m_pEntrys, 0, HASHTABLESIZE*sizeof(PHashEntry) );

		memset( m_aHashEntrys, 0, SIZE*sizeof(HashEntry) );
		
		for ( i = 1; i < SIZE; i++ )
			m_aHashEntrys[i-1].pNextFree = &m_aHashEntrys[i];

		m_aHashEntrys[SIZE-1].pNextFree = NULL;

		m_pFreeEntrys	= &m_aHashEntrys[0];
		m_pFirstEntry	= NULL;

		m_nKeyCount		= 0;
	}

	CHashTable(const CHashTable& src)
	{
		EC_INIT_CRITSEC

		long nOffs = (long)m_aHashEntrys - (long)src.m_aHashEntrys;

		memcpy(m_aHashEntrys, src.m_aHashEntrys, SIZE*sizeof(HashEntry));
		for ( int i=0; i<HASHTABLESIZE; i++ )
			m_pEntrys[i] = src.m_pEntrys[i] != NULL ?
									(PHashEntry)((BYTE*)src.m_pEntrys[i] + nOffs): NULL;

		m_pFreeEntrys	= (PHashEntry)((BYTE*)src.m_pFreeEntrys+nOffs);
		m_pFirstEntry	= (PHashEntry)((BYTE*)src.m_pFirstEntry+nOffs);
		m_nKeyCount		= src.m_nKeyCount;

	}

	virtual ~CHashTable()
	{
		EC_DELETE_CRITSEC
	}
	int	Add( KEY key, VALUE newValue )
	{
		int	ret	= 0;
		int	nHash = GetHashValue( key );
		PHashEntry p, n;

		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		if ( p = KeyInUse( key, nHash ) )
		{
			p->value = newValue;
			ret		= 1;
		}
		else
		{
			if ( m_pFreeEntrys )
			{
				n						= m_pFreeEntrys;
				m_pFreeEntrys		= m_pFreeEntrys->pNextFree;

				EC_ENABLE_IRQS();
		
				n->value				= newValue;
				n->key				= key;

				EC_DISABLE_IRQS();
		
				n->pNext				= m_pEntrys[nHash];
				m_pEntrys[nHash]	= n;

				n->pNextGlob		= m_pFirstEntry;
				m_pFirstEntry		= n;
				
				ret = 1;
				m_nKeyCount++;
			}
		}

		EC_ENABLE_IRQS();

		return ret;
	}

	int	Lookup( KEY key, VALUE& rValue )
	{
		int	ret		= 0;
		int	nHash		= GetHashValue( key );
	
		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		PHashEntry p	= m_pEntrys[nHash];

		while ( p != NULL )
		{
			if ( p->key == key )
				break;
			p = p->pNext;
		}
		if ( p )
		{
			rValue	= p->value;
			ret		= 1;
		}

		EC_ENABLE_IRQS();

		return ret;
	}

	VALUE* Lookup( KEY key )
	{
		int	nHash		= GetHashValue( key );

		KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		PHashEntry p	= m_pEntrys[nHash];

		while ( p != NULL )
		{
			if ( p->key == key )
				break;
			p = p->pNext;
		}

		EC_ENABLE_IRQS();

		if ( p )
			return &p->value;
		else
			return NULL;
	}

	int	Remove( KEY key )
	{
		int	ret		= 0;
		int	nHash		= GetHashValue( key );
		PHashEntry l	= NULL;
		PHashEntry p	= m_pEntrys[nHash];

		KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		while ( p )										// searching right entry
		{
			if ( p->key == key )
				break;									// entry found

			l = p;										// l is previous entry
			p = p->pNext;
		}
		
		if ( p )
		{
			//	removing from hash-list
			if ( l )										// if not first entry
				l->pNext				= p->pNext;		//	entry removed
			else											//	if first entry
				m_pEntrys[nHash]	= p->pNext;		//	entry removed

			// removing from global-list
			if ( p == m_pFirstEntry )
				m_pFirstEntry = p->pNextGlob;
			else
			{
				l = m_pFirstEntry;
				while ( l->pNextGlob != p )
					l = l->pNextGlob;

				l->pNextGlob = p->pNextGlob;
			}
			
			m_nKeyCount--;

			EC_ENABLE_IRQS();
			EC_DISABLE_IRQS();
	
			p->pNextFree	= m_pFreeEntrys;
			m_pFreeEntrys	= p;

			ret				= 1;
		}

		EC_ENABLE_IRQS();

		return ret;
	}

	int	RemoveFirstEntry( VALUE& rValue )
	{
		if ( m_pFirstEntry )
		{
			rValue = m_pFirstEntry->value;
			
			return Remove(m_pFirstEntry->key);
		}
		else
			return 0;
	}

	void	RemoveAll()
	{
		int i;

		memset( m_pEntrys, 0, HASHTABLESIZE*sizeof(PHashEntry) );

		memset( m_aHashEntrys, 0, SIZE*sizeof(HashEntry) );
		
		for ( i = 1; i < SIZE; i++ )
			m_aHashEntrys[i-1].pNextFree = &m_aHashEntrys[i];

		m_aHashEntrys[SIZE-1].pNextFree = NULL;

		m_pFreeEntrys	= &m_aHashEntrys[0];
		m_pFirstEntry	= NULL;

		m_nKeyCount		= 0;
	}

	int GetFirstEntry( VALUE& rValue, PVOID& pVoid )
	{
		pVoid = m_pFirstEntry;

		if ( pVoid )
		{
			rValue = ((PHashEntry)pVoid)->value;
			return 1;
		}
		else
			return 0;
	}

	int GetNextEntry( VALUE& rValue, PVOID& pVoid )
	{
		if ( !pVoid )
			pVoid = m_pFirstEntry;
		else
			pVoid = ((PHashEntry)pVoid)->pNextGlob;

		if ( pVoid )
		{
			rValue = ((PHashEntry)pVoid)->value;
			return 1;
		}
		else
			return 0;
	}

	int GetNextEntry( KEY& rKey, VALUE*& pValue, PVOID& pVoid )
	{
		if ( !pVoid )
			pVoid = m_pFirstEntry;
		else
			pVoid = ((PHashEntry)pVoid)->pNextGlob;

		if ( pVoid )
		{
			pValue	= &(((PHashEntry)pVoid)->value);
			rKey		= ((PHashEntry)pVoid)->key;
			return 1;
		}
		else
			return 0;
	}

	int KeyInUse( KEY key ) const
	{
		int			nHash	= GetHashValue( key );

		KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		PHashEntry	p		= m_pEntrys[nHash];
		while ( p )
		{
			if ( p->key == key )
			{
				EC_ENABLE_IRQS();
				return 1;
			}
			p = p->pNext;
		}

		EC_ENABLE_IRQS();

		return 0;
	}

	int AnyFreeEntries()
	{
		return (m_nKeyCount < SIZE);
	}

	void ForEach( PForeEachFunc pFunc, PVOID pUser ) const
	{
		int i;
		PHashEntry	p;
		for ( i = 0; i < HASHTABLESIZE; i++ )
		{
			p = m_pEntrys[i];
			while ( p )
			{
				pFunc( pUser, &p->value );
				p = p->pNext;
			}
		}
	}

	unsigned int KeyCount() const
	{
		return m_nKeyCount;
	}

protected:
	PHashEntry KeyInUse( KEY key, int nHash ) const
	{
		PHashEntry p = m_pEntrys[nHash];
		while ( p )
		{
			if ( p->key == key )
				return p;
			p = p->pNext;
		}
		return NULL;
	}

	int GetHashValue( KEY key ) const
		{ return ( key % HASHTABLESIZE ); }

};


///////////////////////////////////////////////////////////////////////////////
//	CHashTableDyn
template <class KEY, class VALUE>
class CHashTableDyn
{

typedef void (*PForeEachFunc)( PVOID pThis, VALUE* pValue );

protected:
	typedef struct HashEntry
	{
		HashEntry*	pNext;
		HashEntry*	pNextFree;
		HashEntry*	pNextGlob;
		KEY			key;
		VALUE			value;
	} *PHashEntry;

	unsigned long			m_size;
	PHashEntry	m_pHashEntrys;
	PHashEntry	m_pFreeEntrys;
	PHashEntry	m_pFirstEntry;
	PHashEntry	m_pEntrys[HASHTABLESIZE];
	unsigned int			m_nKeyCount;

	EC_DECLARE_CRITSEC

public:
	CHashTableDyn( unsigned long size )
	{
		EC_INIT_CRITSEC;

		m_size = size;

		memset( m_pEntrys, 0, HASHTABLESIZE*sizeof(PHashEntry) );

		m_pHashEntrys = new HashEntry[m_size];

		memset( m_pHashEntrys, 0, m_size*sizeof(HashEntry) );
		
		for ( unsigned long i = 1; i < m_size; i++ )
			m_pHashEntrys[i-1].pNextFree = &m_pHashEntrys[i];

		m_pHashEntrys[m_size-1].pNextFree = NULL;

		m_pFreeEntrys	= &m_pHashEntrys[0];
		m_pFirstEntry	= NULL;

		m_nKeyCount		= 0;
	}

	~CHashTableDyn()
	{
		delete m_pHashEntrys;
		EC_DELETE_CRITSEC
	}

	int AnyFreeEntries()
	{
		return (m_nKeyCount < m_size);
	}

	int	Add( KEY key, VALUE newValue )
	{
		int	ret	= 0;
		int	nHash = GetHashValue( key );
		PHashEntry p, n;

		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		if ( p = KeyInUse( key, nHash ) )
		{
			p->value = newValue;
			ret		= 1;
		}
		else
		{
			if ( m_pFreeEntrys )
			{
				n						= m_pFreeEntrys;
				m_pFreeEntrys		= m_pFreeEntrys->pNextFree;

				EC_ENABLE_IRQS();
		
				n->value				= newValue;
				n->key				= key;

				EC_DISABLE_IRQS();
		
				n->pNext				= m_pEntrys[nHash];
				m_pEntrys[nHash]	= n;

				n->pNextGlob		= m_pFirstEntry;
				m_pFirstEntry		= n;
				
				ret = 1;
				m_nKeyCount++;
			}
		}

		EC_ENABLE_IRQS();

		return ret;
	}

	int	Lookup( KEY key, VALUE& rValue )
	{
		int	ret		= 0;
		int	nHash		= GetHashValue( key );

		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		PHashEntry p	= m_pEntrys[nHash];

		while ( p != NULL )
		{
			if ( p->key == key )
				break;
			p = p->pNext;
		}
		if ( p )
		{
			rValue	= p->value;
			ret		= 1;
		}

		EC_ENABLE_IRQS();

		return ret;
	}

	VALUE* Lookup( KEY key )
	{
		int	nHash		= GetHashValue( key );
		
		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();
		
		PHashEntry p	= m_pEntrys[nHash];

		while ( p != NULL )
		{
			if ( p->key == key )
				break;
			p = p->pNext;
		}

		EC_ENABLE_IRQS();

		if ( p )
			return &p->value;
		else
			return NULL;
	}

	int	Remove( KEY key )
	{
		int	ret		= 0;
		int	nHash		= GetHashValue( key );
		PHashEntry l	= NULL;
		PHashEntry p	= m_pEntrys[nHash];

		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		while ( p )										// searching right entry
		{
			if ( p->key == key )
				break;									// entry found

			l = p;										// l is previous entry
			p = p->pNext;
		}
		
		if ( p )
		{
			//	removing from hash-list
			if ( l )										// if not first entry
				l->pNext				= p->pNext;		//	entry removed
			else											//	if first entry
				m_pEntrys[nHash]	= p->pNext;		//	entry removed

			// removing from global-list
			if ( p == m_pFirstEntry )
				m_pFirstEntry = p->pNextGlob;
			else
			{
				l = m_pFirstEntry;
				while ( l->pNextGlob != p )
					l = l->pNextGlob;

				l->pNextGlob = p->pNextGlob;
			}
			
			m_nKeyCount--;

			EC_ENABLE_IRQS();
			EC_DISABLE_IRQS();
	
			p->pNextFree	= m_pFreeEntrys;
			m_pFreeEntrys	= p;

			ret				= 1;
		}

		EC_ENABLE_IRQS();

		return ret;
	}

	int	RemoveFirstEntry( VALUE& rValue )
	{
		if ( m_pFirstEntry )
		{
			rValue = m_pFirstEntry->value;
			
			return Remove(m_pFirstEntry->key);
		}
		else
			return 0;
	}

	void	RemoveAll()
	{
		memset( m_pEntrys, 0, HASHTABLESIZE*sizeof(PHashEntry) );

		memset( m_pHashEntrys, 0, m_size*sizeof(HashEntry) );
		
		for ( unsigned long i = 1; i < m_size; i++ )
			m_pHashEntrys[i-1].pNextFree = &m_pHashEntrys[i];

		m_pHashEntrys[m_size-1].pNextFree = NULL;

		m_pFreeEntrys	= &m_pHashEntrys[0];
		m_pFirstEntry	= NULL;

		m_nKeyCount		= 0;
	}

	int GetFirstEntry( VALUE& rValue, PVOID& pVoid )
	{
		pVoid = m_pFirstEntry;

		if ( pVoid )
		{
			rValue = ((PHashEntry)pVoid)->value;
			return 1;
		}
		else
			return 0;
	}

	int GetNextEntry( VALUE& rValue, PVOID& pVoid )
	{
		if ( !pVoid )
			pVoid = m_pFirstEntry;
		else
			pVoid = ((PHashEntry)pVoid)->pNextGlob;

		if ( pVoid )
		{
			rValue = ((PHashEntry)pVoid)->value;
			return 1;
		}
		else
			return 0;
	}

	int GetNextEntry( KEY& rKey, VALUE*& pValue, PVOID& pVoid )
	{
		if ( !pVoid )
			pVoid = m_pFirstEntry;
		else
			pVoid = ((PHashEntry)pVoid)->pNextGlob;

		if ( pVoid )
		{
			pValue	= &(((PHashEntry)pVoid)->value);
			rKey		= ((PHashEntry)pVoid)->key;
			return 1;
		}
		else
			return 0;
	}

	int KeyInUse( KEY key ) const
	{
		int			nHash	= GetHashValue( key );
		KIRQL	oldIrql;

		EC_DISABLE_IRQS();
		PHashEntry	p		= m_pEntrys[nHash];
		while ( p )
		{
			if ( p->key == key )
			{
				EC_ENABLE_IRQS();
				return 1;
			}
			p = p->pNext;
		}
		EC_ENABLE_IRQS();
		return 0;
	}

	void ForEach( PForeEachFunc pFunc, PVOID pUser ) const
	{
		int i;
		PHashEntry	p;
		for ( i = 0; i < HASHTABLESIZE; i++ )
		{
			p = m_pEntrys[i];
			while ( p )
			{
				pFunc( pUser, &p->value );
				p = p->pNext;
			}
		}
	}

	unsigned int KeyCount() const
	{
		return m_nKeyCount;
	}

protected:
	PHashEntry KeyInUse( KEY key, int nHash ) const
	{
		PHashEntry p = m_pEntrys[nHash];
		while ( p )
		{
			if ( p->key == key )
				return p;
			p = p->pNext;
		}
		return NULL;
	}

	int GetHashValue( KEY key ) const
		{ return ( key % HASHTABLESIZE ); }

};


/////////////////////////////////////////////////////////////////////////////
// CListR0<TYPE, ARG_TYPE>

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
class CListR0
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		TYPE data;
	};
public:
// Construction
	CListR0();

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	int IsEmpty() const;

	// peek at head or tail
	TYPE& GetHead();
	TYPE GetHead() const;
	TYPE& GetTail();
	TYPE GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list !
	TYPE RemoveHead();
	TYPE RemoveTail();

	// add before head or after tail
	POSITION AddHead(ARG_TYPE newElement);
	POSITION AddTail(ARG_TYPE newElement);

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	TYPE& GetNext(POSITION& rPosition); // return *Position++
	TYPE GetNext(POSITION& rPosition) const; // return *Position++
	TYPE& GetPrev(POSITION& rPosition); // return *Position--
	TYPE GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	TYPE& GetAt(POSITION position);
	TYPE GetAt(POSITION position) const;
	void SetAt(POSITION pos, ARG_TYPE newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, ARG_TYPE newElement);
	POSITION InsertAfter(POSITION position, ARG_TYPE newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(ARG_TYPE searchValue, POSITION startAfter = NULL) const;
		// defaults to starting at the HEAD, return NULL if not found
	POSITION FindIndex(int nIndex) const;
		// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode*	m_pNodeHead;
	CNode*	m_pNodeTail;
	CNode*	m_pNodeFree;
	int		m_nCount;
	CNode		m_arrNodeFree[SIZE];

	EC_DECLARE_CRITSEC
public:
	~CListR0();
//	void Serialize(CArchive&);
};


template<class TYPE, class ARG_TYPE, unsigned int SIZE>
inline int CListR0<TYPE, ARG_TYPE, SIZE>::GetCount() const
	{ return m_nCount; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
int CListR0<TYPE, ARG_TYPE, SIZE>::IsEmpty() const
	{ return m_nCount == 0; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE& CListR0<TYPE, ARG_TYPE, SIZE>::GetHead()
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::GetHead() const
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE& CListR0<TYPE, ARG_TYPE, SIZE>::GetTail()
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::GetTail() const
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE& CListR0<TYPE, ARG_TYPE, SIZE>::GetNext(POSITION& rPosition) // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::GetNext(POSITION& rPosition) const // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE& CListR0<TYPE, ARG_TYPE, SIZE>::GetPrev(POSITION& rPosition) // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::GetPrev(POSITION& rPosition) const // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE& CListR0<TYPE, ARG_TYPE, SIZE>::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
template<class TYPE, class ARG_TYPE, unsigned int SIZE>
void CListR0<TYPE, ARG_TYPE, SIZE>::SetAt(POSITION pos, ARG_TYPE newElement)
	{ CNode* pNode = (CNode*) pos;
		pNode->data = newElement; }

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
CListR0<TYPE, ARG_TYPE, SIZE>::CListR0()
{
	memset( m_arrNodeFree, 0, sizeof(m_arrNodeFree) );

	for ( unsigned int i=1; i < SIZE; i++ )
	{
		m_arrNodeFree[i].pPrev = &m_arrNodeFree[i-1];
		m_arrNodeFree[i-1].pNext = &m_arrNodeFree[i];
	}

	m_pNodeFree = m_arrNodeFree;
	m_pNodeHead	= NULL;
	m_pNodeTail	= NULL;

	m_nCount		= 0;

	EC_INIT_CRITSEC
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
CListR0<TYPE, ARG_TYPE, SIZE>::~CListR0()
{
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::AddHead(ARG_TYPE newElement)
{
	if ( !m_pNodeFree )
		return NULL;

	CNode* pNewNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNewNode->pPrev = NULL;
	pNewNode->pNext = m_pNodeHead;
	m_nCount++;

	pNewNode->data = newElement;

		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;

	EC_ENABLE_IRQS();

	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::AddTail(ARG_TYPE newElement)
{
	if ( !m_pNodeFree )
		return NULL;

		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	CNode* pNewNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNewNode->pPrev = m_pNodeTail;
	pNewNode->pNext = NULL;
	m_nCount++;

	pNewNode->data = newElement;

	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;

	EC_ENABLE_IRQS();

	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::RemoveHead()
{
	//ASSERT(m_pNodeHead != NULL);  // don't call on empty list !!!

		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	CNode* pOldNode = m_pNodeHead;
	TYPE returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;

	pOldNode->pNext = m_pNodeFree;
	m_pNodeFree = pOldNode;
	m_nCount--;

	EC_ENABLE_IRQS();

	//ASSERT(m_nCount >= 0);  // make sure we don't underflow

	return returnValue;
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
TYPE CListR0<TYPE, ARG_TYPE, SIZE>::RemoveTail()
{
	//ASSERT(m_pNodeHead != NULL);  // don't call on empty list !!!

		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	CNode* pOldNode = m_pNodeTail;
	TYPE returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;

	pOldNode->pNext = m_pNodeFree;
	m_pNodeFree = pOldNode;
	m_nCount--;

	EC_ENABLE_IRQS();

	//ASSERT(m_nCount >= 0);  // make sure we don't underflow

	return returnValue;
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::InsertBefore(POSITION position, ARG_TYPE newElement)
{
	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNewNode->pPrev = pOldNode->pPrev;
	pNewNode->pNext = pOldNode;
	m_nCount++;

	pNewNode->data = newElement;
	
	if (pOldNode->pPrev != NULL)
	{
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		//ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	
	EC_ENABLE_IRQS();

	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::InsertAfter(POSITION position, ARG_TYPE newElement)
{
	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNewNode->pPrev = pOldNode;
	pNewNode->pNext = pOldNode->pNext;
	m_nCount++;

	pNewNode->data = newElement;
	
	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		//ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	
	EC_ENABLE_IRQS();

	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
void CListR0<TYPE, ARG_TYPE, SIZE>::RemoveAt(POSITION position)
{
		KIRQL	oldIrql;

	EC_DISABLE_IRQS();

	CNode* pOldNode = (CNode*) position;

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	
	pOldNode->pNext = m_pNodeFree;
	m_pNodeFree = pOldNode;
	m_nCount--;

	EC_ENABLE_IRQS();

	//ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

template<class TYPE, class ARG_TYPE, unsigned int SIZE>
POSITION CListR0<TYPE, ARG_TYPE, SIZE>::FindIndex(int nIndex) const
{
	if (nIndex >= m_nCount || nIndex < 0)
		return NULL;  // went too far

	CNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (POSITION) pNode;
}

