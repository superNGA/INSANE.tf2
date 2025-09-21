//=========================================================================
//                      STD DOUBLE BUFFER
//=========================================================================
// by      : INSANE
// created : 16/09/2025
// 
// purpose : A general purpose thread safe double buffer data structure. 
//           ( prefered for single writter, many readers. )
//-------------------------------------------------------------------------
#pragma once

#include <assert.h>
#include <atomic>
#include "../ConsoleLogging.h"


///////////////////////////////////////////////////////////////////////////
#define DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_NOSWAP(pBuf, pKeyData) Containers::BufferAutoRelease_t CONCAT(_temp_, __COUNTER__)(static_cast<IDoubleBuffer_t*>(pBuf), reinterpret_cast<void*>(pKeyData), true, false);
#define DOUBLEBUFFER_AUTO_RELEASE_WRITEBUFFER_SWAP(pBuf, pKeyData)   Containers::BufferAutoRelease_t CONCAT(_temp_, __COUNTER__)(static_cast<IDoubleBuffer_t*>(pBuf), reinterpret_cast<void*>(pKeyData), true, true);
#define DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(pBuf, pKeyData)         Containers::BufferAutoRelease_t CONCAT(_temp_, __COUNTER__)(static_cast<IDoubleBuffer_t*>(pBuf), reinterpret_cast<void*>(pKeyData), false, false);
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class IDoubleBuffer_t
{
public:
    virtual void ReturnReadBuffer(void* pDataGivenToCalle)                    = 0;
    virtual void ReturnWriteBuffer(void* pDataGivenToCalle, bool bSwapBuffer) = 0;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
namespace Containers
{
    template <typename T>
    class DoubleBuffer_t : public IDoubleBuffer_t
    {
    public:
        DoubleBuffer_t();
    
        T*   GetReadBuffer();
        void ReturnReadBuffer(void* pDataGivenToCalle) override final;
    
        T*   GetWriteBuffer();
        void ReturnWriteBuffer(void* pDataGivenToCalle, bool bSwapBuffer) override final;
    
        bool SwapBuffer();
    
    private:
        T                    m_data1;
        T                    m_data2;
                             
        T*                   m_pReadBuffer  = nullptr;
        T*                   m_pWriteBuffer = nullptr;
    
        std::atomic<int32_t> m_iReaderCounter;
        std::atomic<bool>    m_bWritingActive;
    };
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
namespace Containers
{
    class BufferAutoRelease_t
    {
    public:
        BufferAutoRelease_t(IDoubleBuffer_t* pBuf, void* pKeyData, bool bReleaseWriteBuffer, bool bSwap)
        {
            m_pBuf                = pBuf;
            m_bReleaseWriteBuffer = bReleaseWriteBuffer;
            m_bSwap               = bSwap;
            m_pKeyData            = pKeyData;
        }
        ~BufferAutoRelease_t()
        {
            if (m_bReleaseWriteBuffer == true)
            {
                m_pBuf->ReturnWriteBuffer(m_pKeyData, m_bSwap);
            }
            else
            {
                m_pBuf->ReturnReadBuffer(m_pKeyData);
            }
        }

    private:
        IDoubleBuffer_t* m_pBuf;
        void*            m_pKeyData            = nullptr;
        bool             m_bReleaseWriteBuffer = true;
        bool             m_bSwap               = false;
    };
}
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename T>
Containers::DoubleBuffer_t<T>::DoubleBuffer_t()
{
    m_iReaderCounter.store(0);
    m_bWritingActive.store(false);

    m_pReadBuffer  = &m_data1;
    m_pWriteBuffer = &m_data2;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline T* Containers::DoubleBuffer_t<T>::GetReadBuffer()
{
    assert(m_iReaderCounter.load() >= 0 && "Reader counter is negative. Programmer error");

    m_iReaderCounter.fetch_add(1);
    return m_pReadBuffer;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline void Containers::DoubleBuffer_t<T>::ReturnReadBuffer(void* pDataGivenToCalle)
{
    assert(m_iReaderCounter.load() >= 0 && "Reader counter is negative. Programmer error");

    if (pDataGivenToCalle == m_pReadBuffer)
    {
        m_iReaderCounter.fetch_sub(1);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline T* Containers::DoubleBuffer_t<T>::GetWriteBuffer()
{
    // Exchange returns the old value.
    // if already true then someone else writting to this shit, return nullptr. else return write buffer
    if (m_bWritingActive.exchange(true) == true)
        return nullptr;

    return m_pWriteBuffer;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline void Containers::DoubleBuffer_t<T>::ReturnWriteBuffer(void* pDataGivenToCalle, bool bSwapBuffer)
{
    // if caller can't return the write buffer pointer, that means we never
    // gave him the write buffer pointer. This prevents "someone else from setting m_bWritingActive to false"
    if (pDataGivenToCalle == m_pWriteBuffer)
    {
        // Swap the buffers if no one is reading from read buffer.
        if(m_iReaderCounter.load() == 0 && bSwapBuffer == true)
        {
            T* pTemp       = m_pWriteBuffer;
            m_pWriteBuffer = m_pReadBuffer;
            m_pReadBuffer  = pTemp;
        }

        m_bWritingActive.store(false);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline bool Containers::DoubleBuffer_t<T>::SwapBuffer()
{
    // if no one is reading and no one is writting to anything.
    if (m_iReaderCounter.load() == 0 && m_bWritingActive.load() == false)
    {
        T* pTemp       = m_pWriteBuffer;
        m_pWriteBuffer = m_pReadBuffer;
        m_pReadBuffer  = pTemp;
        return true;
    }

    return false;
}
