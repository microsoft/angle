// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "ThreadEmulation.h"

#include <assert.h>
#include <vector>
#include <set>
#include <map>
#include <mutex>

#include <wrl/client.h>
#include <wrl/async.h>
#include <Windows.System.Threading.h>

using namespace std;
using namespace Windows::Foundation;
using namespace ABI::Windows::System::Threading;

namespace ThreadEmulation
{
    // Thread local storage.
    typedef vector<void*> ThreadLocalData;

    static __declspec(thread) ThreadLocalData* currentThreadData = nullptr;
    static set<ThreadLocalData*> allThreadData;
    static DWORD nextTlsIndex = 0;
    static vector<DWORD> freeTlsIndices;
    static mutex tlsAllocationLock;

    _Use_decl_annotations_ void Sleep(unsigned long dwMilliseconds)
    {
        static HANDLE singletonEvent = nullptr;

        HANDLE sleepEvent = singletonEvent;

        // Demand create the event.
        if (!sleepEvent)
        {
            sleepEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

            if (!sleepEvent)
                return;

            HANDLE previousEvent = InterlockedCompareExchangePointerRelease(&singletonEvent, sleepEvent, nullptr);
            
            if (previousEvent)
            {
                // Back out if multiple threads try to demand create at the same time.
                CloseHandle(sleepEvent);
                sleepEvent = previousEvent;
            }
        }

        // Emulate sleep by waiting with timeout on an event that is never signalled.
        WaitForSingleObjectEx(sleepEvent, dwMilliseconds, false);
    }

    unsigned long TlsAlloc()
    {
        // Can we reuse a previously freed TLS slot?
        if (!freeTlsIndices.empty())
        {
            DWORD result = freeTlsIndices.back();
            freeTlsIndices.pop_back();
            return result;
        }

        // Allocate a new TLS slot.
        return nextTlsIndex++;
    }

    _Use_decl_annotations_ int TlsFree(unsigned long dwTlsIndex)
    {
        assert(dwTlsIndex < nextTlsIndex);
        assert(find(freeTlsIndices.begin(), freeTlsIndices.end(), dwTlsIndex) == freeTlsIndices.end());

        // Store this slot for reuse by TlsAlloc.
        try
        {
            freeTlsIndices.push_back(dwTlsIndex);
        }
        catch (...)
        {
            return false;
        }

        // Zero the value for all threads that might be using this now freed slot.
        for each (auto threadData in allThreadData)
        {
            if (threadData->size() > dwTlsIndex)
            {
                threadData->at(dwTlsIndex) = nullptr;
            }
        }

        return true;
    }

    _Use_decl_annotations_ void* TlsGetValue(unsigned long dwTlsIndex)
    {
        ThreadLocalData* threadData = currentThreadData;

        if (threadData && threadData->size() > dwTlsIndex)
        {
            // Return the value of an allocated TLS slot.
            return threadData->at(dwTlsIndex);
        }
        else
        {
            // Default value for unallocated slots.
            return nullptr;
        }
    }

    _Use_decl_annotations_ int TlsSetValue(unsigned long dwTlsIndex, void* lpTlsValue)
    {
        ThreadLocalData* threadData = currentThreadData;

        if (!threadData)
        {
            // First time allocation of TLS data for this thread.
            try
            {
                threadData = new ThreadLocalData(dwTlsIndex + 1, nullptr);
                
                //lock_guard<mutex> lock(tlsAllocationLock);

                allThreadData.insert(threadData);

                currentThreadData = threadData;
            }
            catch (...)
            {
                if (threadData)
                    delete threadData;

                return false;
            }
        }
        else if (threadData->size() <= dwTlsIndex)
        {
            // This thread already has a TLS data block, but it must be expanded to fit the specified slot.
            try
            {
                //lock_guard<mutex> lock(tlsAllocationLock);

                threadData->resize(dwTlsIndex + 1, nullptr);
            }
            catch (...)
            {
                return false;
            }
        }

        // Store the new value for this slot.
        threadData->at(dwTlsIndex) = lpTlsValue;

        return true;
    }

    // Called at thread exit to clean up TLS allocations.
    void TlsShutdown()
    {
        ThreadLocalData* threadData = currentThreadData;

        if (threadData)
        {
            {
                lock_guard<mutex> lock(tlsAllocationLock);

                allThreadData.erase(threadData);
            }

            currentThreadData = nullptr;

            delete threadData;
        }
    }
}
