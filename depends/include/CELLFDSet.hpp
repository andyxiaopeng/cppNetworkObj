#ifndef _CELL_FDSET_HPP_
#define _CELL_FDSET_HPP_

#include"CELL.hpp"

#define CELL_MAX_FD 10240

class CELLFDSet
{
public:
	CELLFDSet()
	{
		int nSocketNum = CELL_MAX_FD;
#ifdef _WIN32
		_nfdSize = sizeof(u_int) + (sizeof(SOCKET)*nSocketNum);
#else
		_nfdSize = nSocketNum / (8 * sizeof(char));
#endif // _WIN32
		_pfdset = (fd_set *)new char[_nfdSize];
		memset(_pfdset, 0, _nfdSize);
	}

	~CELLFDSet()
	{
		if (_pfdset)
		{
			delete[] _pfdset;
			_pfdset = nullptr;
		}
	}

	inline void add(SOCKET s)
	{
#ifdef _WIN32
		FD_SET(s, _pfdset);
#else
		if(s < CELL_MAX_FD)
		{
			FD_SET(s, _pfdset);
		}else{
			CELLLog_Error("CELLFDSet::add sock<%d>, CELL_MAX_FD<%d>",(int)s,CELL_MAX_FD);
		}
#endif // _WIN32
	}

	inline void del(SOCKET s)
	{
		FD_CLR(s, _pfdset);
	}

	inline void zero()
	{
#ifdef _WIN32
		FD_ZERO(_pfdset);
#else
		memset(_pfdset, 0, _nfdSize);
#endif // _WIN32
	}

	inline bool has(SOCKET s)
	{
		return FD_ISSET(s, _pfdset);
	}

	inline fd_set* fdset()
	{
		return _pfdset;
	}

	void copy(CELLFDSet& set)
	{
		memcpy(_pfdset, set.fdset(), set._nfdSize);
	}
private:
	fd_set * _pfdset = nullptr;
	size_t _nfdSize = 0;
};


#endif // !_CELL_FDSET_HPP_
