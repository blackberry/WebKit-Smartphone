/*
 * Copyright (C) 2009-2011 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformMicroBenchmark_h
#define BlackBerryPlatformMicroBenchmark_h

#include <BlackBerryPlatformLog.h>
#include <BlackBerryPlatformMisc.h>
#include <limits>

namespace BlackBerry {
namespace Platform {

class MicroBenchmark {
public:
    static const int defaultSampleSize = 30;

    MicroBenchmark(const char* info)
        : m_info(info)
        , m_sampleSize(defaultSampleSize)
        , m_cumulativeTicks(0.0)
        , m_measurementCount(0)
        , m_value(std::numeric_limits<double>::infinity())
    {
    }

    void setSampleSize(int size) { m_sampleSize = size; }

    // Returns a start time to be passed to endMeasurement()
    double beginMeasurement() { return tickCount(); }
    
    // Returns true if we have enough samples to generate a benchmark
    bool endMeasurement(double startTime)
    {
        m_cumulativeTicks += tickCount()-startTime;
        ++m_measurementCount;
        if (m_measurementCount == m_sampleSize) {
            m_value = m_cumulativeTicks/m_sampleSize;
            m_cumulativeTicks = 0.0;
            m_measurementCount = 0;
            return true;
        }
        return false;
    }
    
    // If we do not yet have enough measurements, value is infinity
    bool hasValue() const { return m_value != std::numeric_limits<double>::infinity(); }
    double value() const { return m_value; }
    void printValueToLog(MessageLogLevel level) { log(level, "Micro benchmark %s: %f s", m_info, m_value); }

private:
    const char* m_info;
    int m_sampleSize;
    double m_cumulativeTicks;
    double m_measurementCount;
    double m_value;
};

// RAII class to perform a measurement on a MicroBenchmark and log it if the value changed
class MicroBenchmarkMeasurement {
public:
    MicroBenchmarkMeasurement(MicroBenchmark& benchmark, MessageLogLevel logLevel = LogLevelInfo)
        : m_benchmark(benchmark)
        , m_logLevel(logLevel)
    {
        m_startTime = m_benchmark.beginMeasurement();
    }
    
    ~MicroBenchmarkMeasurement()
    {
        bool valueChanged = m_benchmark.endMeasurement(m_startTime);
        if (valueChanged)
            m_benchmark.printValueToLog(m_logLevel);
    }

private:
    double m_startTime;
    MicroBenchmark& m_benchmark;
    MessageLogLevel m_logLevel;
};

}
}

#endif // BlackBerryPlatformMicroBenchmark_h