#ifndef SOCKET_BUFFER_H_
#define SOCKET_BUFFER_H_

#include <string.h>
#include <stdexcept>

namespace znb
{
	/** @defgroup ����Ӧ�û���������̬����
		* @author reborn-lys
		* @version 0.1
		* @date 2015.4.4
		* @{
	*/
	class buffer_overflow : public std::runtime_error
	{
	public:
		buffer_overflow(const std::string& _w): std::runtime_error(_w) {}
	};

	class SocketBuffer
	{
	public:
		SocketBuffer(size_t uBlkSize, int nMaxBlks)
		: m_uBlkSize(uBlkSize)
		, m_nMaxBlks(nMaxBlks)
		, m_nBlks(0)
		, m_data(0)
		, m_size(0)
		{
		}

		~SocketBuffer()
		{
			if (m_data)
			{
				delete[] m_data;
			}
		}

	public:
		size_t free()
		{
			return m_uBlkSize * m_nBlks - m_size;
		}

		size_t fotalFree()
		{
			return m_uBlkSize * m_nMaxBlks - m_size;
		}

		char* tail()
		{
			return m_data + m_size;
		}

		char* data()
		{
			return m_data;
		}

		size_t size()
		{
			return m_size;
		}

		bool append(const char* data, size_t size)
		{
			if (size == 0)
				return true;

			if (m_uBlkSize * m_nBlks - m_size < size)
			{
				// ʣ��ռ䲻����������Ҫ��block����
				int nNeedBlk = (m_size + size) / m_uBlkSize;
				if ((m_size + size) % m_uBlkSize != 0)
				{
					nNeedBlk++;
				}

				if (nNeedBlk > m_nMaxBlks)
					return false;

				m_nBlks = nNeedBlk;
				char* buf = new char[m_nBlks * m_uBlkSize];

				if (m_data)
				{
					// ���ƾ�����
					memmove(buf, m_data, m_size);

					// �ͷžɻ�����
					delete[] m_data;
				}

				m_data = buf;
			}

			memcpy(m_data + m_size, data, size);
			m_size += size;
			return true;
		}

		void checkSpace()
		{
			// ��ʣ��ռ䲻����block
			if (free() < m_uBlkSize >> 1)
			{
				// ��Ҫ����һ��block��С
				if (m_nBlks < m_nMaxBlks)
				{
					m_nBlks++;
					char* buf = new char[m_nBlks * m_uBlkSize];

					if (m_data)
					{
						// ���ƾ�����
						memmove(buf, m_data, m_size);

						// �ͷžɻ�����
						delete[] m_data;
					}

					m_data = buf;
				}
			}
		}

		void inCrement(size_t uBytes)
		{
			m_size += uBytes;
		}

		// ��ͷ��ʼ���Ƴ�uBytes�ֽڵ�����
		void erase(size_t uBytes)
		{
			if (uBytes == 0)
				return;

			if (uBytes == m_size)
			{
				// ��ջ�����
				m_size = 0;
			}
			else
			{
				memmove(m_data, m_data + uBytes, m_size - uBytes);
				m_size -= uBytes;
			}
		}

	private:
		// ���С
		size_t m_uBlkSize;

		// ����������
		int m_nMaxBlks;

		// �����
		int m_nBlks;

		// �������׵�ַ
		char* m_data;

		// ���������ݿ�Ĵ�С
		size_t m_size;
	};
}

#endif // SOCKET_BUFFER_H_
