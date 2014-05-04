#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QList>

namespace core {
namespace util {

template<class T>
class RingBuffer
{
public:
	RingBuffer(int max_size = 1024)
	: m_maxSize(max_size), m_head(0)
	{
	}
	int count() const
	{
		return m_data.size();
	}
	void append(const T &item)
	{
		if(m_data.size() < m_maxSize) {
			m_data.append(item);
		}
		else {
			m_data[m_head] = item;
		}
		m_head++;
		if(m_head >= m_maxSize)
			m_head -= m_maxSize;
	}
	T& at(int ix)
	{
		return m_data[bufferIndex(ix)];
	}
	T value(int ix) const
	{
		return m_data.value(bufferIndex(ix));
	}
private:
	int bufferIndex(int logical_index) const
	{
		if(m_maxSize == m_data.size()) {
			logical_index += m_head;
			logical_index %= m_maxSize;
		}
		return logical_index;
	}
private:
	QList<T> m_data;
	int m_maxSize;
	int m_head;
};

}
}

#endif // RINGBUFFER_H
