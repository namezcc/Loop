#ifndef LOOP_HEAP_H
#define LOOP_HEAP_H

#include <vector>
#include <functional>

template<typename T, typename _pr = std::less<T>>
class MaxHeap
{
public:
	typedef std::function<void(const T&, const int32_t&)> _IndexCall;
	MaxHeap()
	{};
	MaxHeap(const int32_t& cash) :m_arr(cash)
	{};
	MaxHeap(const int32_t& cash, const _IndexCall& call) :m_arr(cash), m_call(call)
	{};
	~MaxHeap()
	{};

	void setIndexCall(const _IndexCall& _call) { m_call = _call; };

	void insert(const T& t)
	{
		m_arr.push_back(t);
		AddCall(static_cast<uint32_t>(m_arr.size()) - 1);
		filterUp(static_cast<uint32_t>(m_arr.size()) - 1);
		ChangeCall();
	}

	void deleten(const uint32_t& _index)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		if (_index == m_arr.size() - 1)
		{
			m_arr.pop_back();
			return;
		}
		std::swap(m_arr[_index], m_arr[m_arr.size() - 1]);
		m_arr.pop_back();
		filter(_index);
		ChangeCall();
	}

	void change(const uint32_t& _index, const T& t)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		m_arr[_index] = t;
		filter(_index);
		ChangeCall();
	}

	void update(const uint32_t& _index)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		filter(_index);
		ChangeCall();
	}

	size_t getSize()
	{
		return m_arr.size();
	}

	const T& getTop() const
	{
		if (m_arr.size() <= 0)
			throw std::exception("error empty size");
		return m_arr.front();
	}

	T& getIndex(const int32_t& _index)
	{
		if (_index < 0 || _index >= m_arr.size())
			throw std::exception("error range");
		return m_arr[_index];
	}

	const std::vector<T>& getHeap() const
	{
		return m_arr;
	}

	bool checkHeap()
	{
		auto s = m_arr.size();
		for (size_t i = 0; i < s; i++)
		{
			int32_t l = 2 * i + 1;
			int32_t r = l + 1;
			if (l < s && m_less(m_arr[i], m_arr[l]))
				return false;

			if (r < s && m_less(m_arr[i], m_arr[r]))
				return false;
		}
		return true;
	}

protected:
	void filterUp(const uint32_t& _index)
	{
		if (m_arr.size() <= 1 || _index == 0)
			return;
		auto index = _index;
		while (true)
		{
			auto f = (index - 1) / 2;
			if (index == f)
				return;
			if (m_less(m_arr[f], m_arr[index]))
			{
				std::swap(m_arr[f], m_arr[index]);
				index = f;
				AddCall(f);
			}
			else
				return;
		}
	}

	void filterDown(const uint32_t& _index)
	{
		auto size = m_arr.size();
		if (size <= 1)
			return;
		auto index = _index;
		while (true)
		{
			uint32_t l = 2 * index + 1;
			if (l >= size)
				return;
			if (l + 1 < size && m_less(m_arr[l], m_arr[l + 1]))
				++l;
			if (m_less(m_arr[index], m_arr[l]))
			{
				std::swap(m_arr[index], m_arr[l]);
				index = l;
				AddCall(l);
			}
			else
				return;
		}
		if (index != _index)
			AddCall(_index);
	}

	void filter(const uint32_t& _index)
	{
		if (_index == 0)
			filterDown(_index);
		else if (_index >= m_arr.size())
			return;
		else
		{
			uint32_t fi = (_index - 1) / 2;
			if (m_less(m_arr[fi], m_arr[_index]))
			{
				AddCall(_index);
				filterUp(_index);
			}
			else
				filterDown(_index);
		}
	}

	void ChangeCall()
	{
		if (m_call)
		{
			for (auto& idx : m_callIndex)
			{
				m_call(m_arr[idx], idx);
			}
			m_callIndex.clear();
		}
	}

	void AddCall(const int32_t& index)
	{
		if (m_call)
			m_callIndex.push_back(index);
	}

private:
	std::vector<T> m_arr;
	std::vector<int32_t> m_callIndex;
	_IndexCall m_call;
	_pr m_less;
};


template<typename T, typename _pr = std::less<T>>
class MinHeap
{
public:
	typedef std::function<void(const T&, const int32_t&)> _IndexCall;
	MinHeap()
	{};
	MinHeap(const int32_t& cash) :m_arr(cash)
	{};
	MinHeap(const int32_t& cash, const _IndexCall& call) :m_arr(cash), m_call(call)
	{};
	~MinHeap()
	{};

	void setIndexCall(const _IndexCall& _call) { m_call = _call; };

	void insert(const T& t)
	{
		m_arr.push_back(t);
		AddCall(static_cast<uint32_t>(m_arr.size()) - 1);
		filterUp(static_cast<uint32_t>(m_arr.size()) - 1);
		ChangeCall();
	}

	void deleten(const uint32_t& _index)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		if (_index == m_arr.size() - 1)
		{
			m_arr.pop_back();
			return;
		}
		std::swap(m_arr[_index], m_arr[m_arr.size() - 1]);
		m_arr.pop_back();
		AddCall(_index);
		filter(_index);
		ChangeCall();
	}

	void change(const uint32_t& _index, const T& t)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		m_arr[_index] = t;
		filter(_index);
		ChangeCall();
	}

	void update(const uint32_t& _index)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		filter(_index);
		ChangeCall();
	}

	size_t getSize()
	{
		return m_arr.size();
	}

	const T& getTop() const
	{
		if (m_arr.size() <= 0)
			throw std::exception("error empty size");
		return m_arr.front();
	}

	T& getIndex(const uint32_t& _index)
	{
		if (_index >= m_arr.size())
			throw std::exception("error range");
		return m_arr[_index];
	}

	const std::vector<T>& getHeap() const
	{
		return m_arr;
	}

	bool checkHeap()
	{
		auto s = m_arr.size();
		for (size_t i = 0; i < s; i++)
		{
			int32_t l = 2 * i + 1;
			int32_t r = l + 1;
			if (l < s && m_less(m_arr[l], m_arr[i]))
				return false;

			if (r < s && m_less(m_arr[r], m_arr[i]))
				return false;
		}
		return true;
	}

protected:
	void filterUp(const uint32_t& _index)
	{
		if (m_arr.size() <= 1 || _index == 0)
			return;
		auto index = _index;
		while (true)
		{
			auto f = (index - 1) / 2;
			if (index == f)
				return;
			if (m_less(m_arr[index], m_arr[f]))
			{
				std::swap(m_arr[f], m_arr[index]);
				index = f;
				AddCall(f);
			}
			else
				return;
		}
	}

	void filterDown(const uint32_t& _index)
	{
		auto size = m_arr.size();
		if (size <= 1)
			return;
		auto index = _index;
		while (true)
		{
			uint32_t l = 2 * index + 1;
			if (l >= size)
				break;
			if (l + 1 < size && m_less(m_arr[l + 1], m_arr[l]))
				++l;
			if (m_less(m_arr[l], m_arr[index]))
			{
				std::swap(m_arr[index], m_arr[l]);
				index = l;
				AddCall(l);
			}
			else
				break;
		}
		if (index != _index)
			AddCall(_index);
	}

	void filter(const uint32_t& _index)
	{
		if (_index == 0)
			filterDown(_index);
		else if (_index >= m_arr.size())
			return;
		else
		{
			uint32_t fi = (_index - 1) / 2;
			if (m_less(m_arr[_index], m_arr[fi]))
			{
				AddCall(_index);
				filterUp(_index);
			}
			else
				filterDown(_index);
		}
	}

	void ChangeCall()
	{
		if (m_call)
		{
			for (auto& idx : m_callIndex)
			{
				m_call(m_arr[idx], idx);
			}
			m_callIndex.clear();
		}
	}

	void AddCall(const uint32_t& index)
	{
		if (m_call)
			m_callIndex.push_back(index);
	}

private:
	std::vector<T> m_arr;
	std::vector<int32_t> m_callIndex;
	_IndexCall m_call;
	_pr m_less;
};

#endif
