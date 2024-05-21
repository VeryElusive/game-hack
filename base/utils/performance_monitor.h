#pragma once
#pragma once
#include <utility>
#include <chrono>
#include <string>
#include <vector>
#include <functional>
#include <windows.h>

class CScopedPerformanceMonitor {
    int* m_pOutput{ };
    std::chrono::time_point<std::chrono::steady_clock> m_flTime{ };
public:
    explicit CScopedPerformanceMonitor( int* timer ) :
        m_pOutput( timer ), m_flTime( std::chrono::high_resolution_clock::now( ) )  {}

    ~CScopedPerformanceMonitor( ) {
        *m_pOutput = static_cast< int >( std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::high_resolution_clock::now( ) - m_flTime
            ).count( ) );
    }
};