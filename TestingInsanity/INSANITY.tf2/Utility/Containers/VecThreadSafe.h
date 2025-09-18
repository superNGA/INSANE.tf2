#pragma once

#include <vector>
#include <assert.h>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include "../ConsoleLogging.h"


///////////////////////////////////////////////////////////////////////////
#define VECTHREADSAFE_AUTO_RELEASE_READ_BUFFER(pBuf, pKeyData)  VecThreadAutoRelease_t CONCAT(_temp_, __COUNTER__)(pBuf, pKeyData, false)
#define VECTHREADSAFE_AUTO_RELEASE_WRITE_BUFFER(pBuf, pKeyData) VecThreadAutoRelease_t CONCAT(_temp_, __COUNTER__)(pBuf, pKeyData, true)
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
namespace Containers
{
    class IVecThreadSafe_t
    {
    public:
        virtual void ReturnReadBuffer(void* pKeyData)  = 0;
        virtual void ReturnWriteBuffer(void* pKeyData) = 0;
    };
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
namespace Containers
{
    template <typename T>
    class VecThreadSafe_t : public IVecThreadSafe_t
    {
    public:
        VecThreadSafe_t();

        void PushBack(T data);
        
        std::vector<T>* GetReadBuffer (bool bDumpProdQueue);
        std::vector<T>* GetWriteBuffer(bool bDumpProdQueue);
        void            ReturnReadBuffer(void* pKeyData)  override final;
        void            ReturnWriteBuffer(void* pKeyData) override final;

    private:
        bool _DumpProducerQueue();

        std::vector<T> m_vecMainBuffer;
        std::vector<T> m_vecProducerQueue;

        std::shared_mutex m_mtxMain;
        std::mutex        m_mtxTemp;
    };
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
class VecThreadAutoRelease_t
{
public:
    VecThreadAutoRelease_t(Containers::IVecThreadSafe_t* pObj, void* pKeyData, bool bWriteBuffer)
    {
        m_pObj         = pObj;
        m_pKeyObj      = pKeyData;
        m_bWriteBuffer = bWriteBuffer;
    }
    ~VecThreadAutoRelease_t()
    {
        if (m_bWriteBuffer == true)
        {
            m_pObj->ReturnWriteBuffer(m_pKeyObj);
        }
        else
        {
            m_pObj->ReturnReadBuffer(m_pKeyObj);
        }
    }

private:
    Containers::IVecThreadSafe_t* m_pObj;
    void*                         m_pKeyObj;
    bool                          m_bWriteBuffer;
};
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename T>
Containers::VecThreadSafe_t<T>::VecThreadSafe_t()
{
    m_vecMainBuffer.reserve(8);
    m_vecProducerQueue.reserve(8);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline void Containers::VecThreadSafe_t<T>::PushBack(T data)
{
    m_mtxTemp.lock();
    m_vecProducerQueue.push_back(data);
    m_mtxTemp.unlock();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::vector<T>* Containers::VecThreadSafe_t<T>::GetReadBuffer(bool bDumpProdQueue)
{
    if (bDumpProdQueue == true)
        _DumpProducerQueue();

    m_mtxMain.lock_shared();
    return &m_vecMainBuffer;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::vector<T>* Containers::VecThreadSafe_t<T>::GetWriteBuffer(bool bDumpProdQueue)
{
    if (bDumpProdQueue == true)
        _DumpProducerQueue();

    m_mtxMain.lock();
    return &m_vecMainBuffer;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline void Containers::VecThreadSafe_t<T>::ReturnReadBuffer(void* pKeyData)
{
    if (pKeyData == &m_vecMainBuffer)
    {
        m_mtxMain.unlock_shared();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline void Containers::VecThreadSafe_t<T>::ReturnWriteBuffer(void* pKeyData)
{
    if (pKeyData == &m_vecMainBuffer)
    {
        m_mtxMain.unlock();
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
template<typename T>
inline bool Containers::VecThreadSafe_t<T>::_DumpProducerQueue()
{
    m_mtxTemp.lock(); m_mtxMain.lock();

    // If nothing to dump then leave early.
    if (m_vecProducerQueue.size() <= 0)
    {
        m_mtxMain.unlock(); m_mtxTemp.unlock();
        return false;
    }

    // Dump & release locks.
    for (T& data : m_vecProducerQueue)
    {
        m_vecMainBuffer.push_back(data);
    }
    LOG("Dumped [ %d ] objects from producers queue to main buffer { size : %d }", m_vecProducerQueue.size(), m_vecMainBuffer.size());
    m_vecProducerQueue.clear();

    m_mtxMain.unlock(); m_mtxTemp.unlock();
    return true;
}
