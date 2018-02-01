///////////////////////////////////////////////////////////////////////////////
// FiFo.h

#ifndef	__FIFO_H__
#define	__FIFO_H__

///////////////////////////////////////////////////////////////////////////////
// CFiFoList

template <class VALUE, int SIZE>
class CFiFoList
{
protected:

	VALUE		m_pFiFoEntrys[SIZE];
	long		m_nFirst;
	long		m_nLast;
	long		m_nCount;
	
 	EC_DECLARE_CRITSEC

public:
	CFiFoList()
	{
 		EC_INIT_CRITSEC;

		memset( m_pFiFoEntrys, 0, SIZE * sizeof(VALUE) );
		m_nFirst	= 0;
		m_nLast	= 0;
		m_nCount	= 0;
	}

	virtual ~CFiFoList()
	{
 		EC_DELETE_CRITSEC
	}

	int Add( VALUE newValue )
	{
		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		if ( m_nCount >= SIZE )
		{
			EC_ENABLE_IRQS();
			return 0;
		}

		m_pFiFoEntrys[m_nLast] = newValue;

		m_nLast++;
		if ( m_nLast == SIZE )
			m_nLast = 0;

		m_nCount++;

 		EC_ENABLE_IRQS();

		return 1;
	}

	int	Remove( VALUE& rValue )
	{
		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();

		if ( m_nCount == 0 )
		{
 			EC_ENABLE_IRQS();
			return 0;
		}

		rValue = m_pFiFoEntrys[m_nFirst];
		memset( &m_pFiFoEntrys[m_nFirst], 0, sizeof(VALUE) );

		m_nFirst++;
		if ( m_nFirst == SIZE )
			m_nFirst = 0;

		m_nCount--;

 		EC_ENABLE_IRQS();

		return 1;
	}

	long	Count()
	{
		return m_nCount;
	}

	int GetNextEntry( VALUE& rValue, PVOID& pVoid )
	{
		long lVoid = (long)pVoid;
		int bRet = 0;
		//KIRQL	oldIrql;

		EC_DISABLE_IRQS();
		if ( m_nCount == 0 )
		{
			EC_ENABLE_IRQS();
			return 0;
		}

		if ( lVoid == 0 )
		{
			lVoid = m_nFirst;
			bRet = 1;
		}
		else
		{
			lVoid--;
			if ( lVoid < 0 )
				lVoid += SIZE;
			
			if ( m_nLast > m_nFirst )
				bRet = lVoid < m_nFirst || lVoid >= m_nLast;
			else
				bRet = lVoid < m_nFirst && lVoid >= m_nLast;
		}

		if ( bRet )
			rValue = m_pFiFoEntrys[lVoid];

		pVoid = (PVOID)lVoid;

 		EC_ENABLE_IRQS();
		return bRet;
	}

	long	Size()
	{
		return SIZE;
	}

	long	FreeCount()
	{
		return SIZE - m_nCount;
	}

};

///////////////////////////////////////////////////////////////////////////////
// CFiFoListDyn

template <class VALUE>
class CFiFoListDyn
{
protected:

	VALUE*	m_pFiFoEntrys;
	long		m_nFirst;
	long		m_nLast;
	long		m_nCount;
	long		m_nSize;

	EC_DECLARE_CRITSEC

public:
	CFiFoListDyn( long nSize )
	{
		EC_INIT_CRITSEC

		m_nSize = nSize;
		m_pFiFoEntrys	= new VALUE[m_nSize];
		memset( m_pFiFoEntrys, 0, m_nSize * sizeof(VALUE) );
		m_nFirst	= 0;
		m_nLast	= 0;
		m_nCount	= 0;
	}

	~CFiFoListDyn()
	{
		delete m_pFiFoEntrys;
		EC_DELETE_CRITSEC
	}

	int	Add( VALUE newValue )
	{
		//KIRQL	oldIrql;
		EC_DISABLE_IRQS();


		if ( m_nCount >= m_nSize )
		{
			EC_ENABLE_IRQS();
			return 0;
		}

		m_pFiFoEntrys[m_nLast] = newValue;

		m_nLast++;
		if ( m_nLast == m_nSize )
			m_nLast = 0;

		m_nCount++;

		EC_ENABLE_IRQS();

		return 1;
	}

	int	Remove( VALUE& rValue )
	{
		//KIRQL	oldIrql;
		EC_DISABLE_IRQS();

		if ( m_nCount == 0 )
		{
			EC_ENABLE_IRQS();
			return 0;
		}

		rValue = m_pFiFoEntrys[m_nFirst];
		memset( &m_pFiFoEntrys[m_nFirst], 0, sizeof(VALUE) );

		m_nFirst++;
		if ( m_nFirst == m_nSize )
			m_nFirst = 0;

		m_nCount--;

		EC_ENABLE_IRQS();

		return 1;
	}

	int	GetEntryXBeforeHead( long X, VALUE& rValue )
	{
		KIRQL	oldIrql;
		EC_DISABLE_IRQS();

		if ( m_nCount < X )
		{
			EC_ENABLE_IRQS();
			return 0;
		}

		X += m_nFirst;
		if ( X >= m_nSize )
			X -= m_nSize;

		rValue = m_pFiFoEntrys[X];

		EC_ENABLE_IRQS();

		return 1;
	}

	long	Count()
	{
		return m_nCount;
	}

	long	Size()
	{
		return m_nSize;
	}

	long	FreeCount()
	{
		return m_nSize - m_nCount;
	}

	void	Clear()
	{
		VALUE value;

		while ( Remove( value ) )
			;
	}

	int GetNextEntry( VALUE& rValue, PVOID& pVoid )
	{
		int bRet = 0;
		KIRQL	oldIrql;
		EC_DISABLE_IRQS();
		if ( m_nCount == 0 )
		{
			EC_ENABLE_IRQS();
			return 0;
		}

		if ( !pVoid )
		{
			pVoid = (PVOID)m_nFirst;
			bRet = 1;
		}
		else
		{
			((long)pVoid)--;
			if ( ((long)pVoid) < 0 )
				((long)pVoid) += m_nSize;
			
			if ( m_nLast > m_nFirst )
				bRet = ((long)pVoid) < m_nFirst || ((long)pVoid) >= m_nLast;
			else
				bRet = ((long)pVoid) < m_nFirst && ((long)pVoid) >= m_nLast;
		}

		if ( bRet )
			rValue = m_pFiFoEntrys[(long)pVoid];

		EC_ENABLE_IRQS();
		return 1;
	}

};

#endif	// __FIFO_H__
