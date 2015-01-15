#pragma once

#include <string.h>
namespace server
{
	namespace net
	{
		class CNetSockBuff
		{
			public:

				CNetSockBuff(uint32_t MaxBlocks, uint32_t uBlocks):
				  uMaxBlock(MaxBlocks),
				  uBlockSize(uBlocks),
				  uBlocks(0),
				  uDataSize(0)
				  {
				  
				  }
				~CNetSockBuff()
				{
				
				}

				char* Tail()
				{
					return m_data + uDataSize;
				}

				char* Data()
				{
					return m_data;
				}

				bool CheckCapacity(uint32_t len)
				{
					uint32_t uFree = uBlocks * uMaxBlock - uDataSize;
					return uFree >= len;
				}
				
				bool AppendData(const char* data, uint32_t len)
				{
					uint32_t uFree = uBlocks * uBlockSize - uDataSize;
					uint32_t uNewBlocks = 0;
					while(uFree < len)
					{		
						if(uBlocks * 2 < uMaxBlock)
						{
							uNewBlocks = uMaxBlock;
						}

						uint32_t uFree = uNewBlocks * uBlockSize - uDataSize;
						if(uNewBlocks == uMaxBlock && uFree < len) return false;	
					}
					uBlocks = uNewBlocks;
					char* dataTmp = new char[uBlocks];
					memcpy(dataTmp, m_data, uDataSize);
					memcpy(dataTmp + uDataSize, data, len);
					delete[] m_data;
					m_data = dataTmp;
				}
				
				void Resize(uint32_t len)
				{
					if(uDataSize < len)
						return;
					
					memmove(m_data, m_data + len, uDataSize - len);

					uDataSize -= len; 
				}
				
				uint32_t Size()
				{
					return uDataSize;
				}
				
				void ResetSize(uint32_t uSize)
				{
					uDataSize = uSize;
				}
			private:

				uint32_t uMaxBlock;
				uint32_t uBlockSize;
				uint32_t uBlocks;

				uint32_t uDataSize;

				char* m_data;
		};
	}
}
