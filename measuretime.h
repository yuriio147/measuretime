/*
 * ------------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Yurii Olenych <yolenych@gmail.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ------------------------------------------------------------------------------
 */

#ifndef MEASURETIME_H
#define MEASURETIME_H

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>

namespace mt
{
    using clock = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;

    template<typename D>
    constexpr const char* duration_to_str(D);

    template<>
    constexpr const char* duration_to_str(std::chrono::nanoseconds)
    {
        return "ns";
    }
    template<>
    constexpr const char* duration_to_str(std::chrono::microseconds)
    {
        return "us";
    }
    template<>
    constexpr const char* duration_to_str(std::chrono::milliseconds)
    {
        return "ms";
    }
    template<>
    constexpr const char* duration_to_str(std::chrono::seconds)
    {
        return "s";
    }
    template<>
    constexpr const char* duration_to_str(std::chrono::minutes)
    {
        return "m";
    }
    template<>
    constexpr const char* duration_to_str(std::chrono::hours)
    {
        return "h";
    }

    template<typename Scope, typename Stream, typename Precision = std::chrono::microseconds>
    class scope_time
    {
    public:
        scope_time(const scope_time&) = delete;
        scope_time& operator=(const scope_time&) = delete;
        scope_time(Scope scope, Stream& stream) : mScope{scope}, mStream{stream}
        {
            mBegin = clock::now();
        }
        ~scope_time()
        {
            const auto end = clock::now();
            const auto duration = std::chrono::duration_cast<Precision>(end - mBegin);
            static std::map<Scope, unsigned long long> sCalls{};
            static std::map<Scope, double> sAvg{};
            static std::map<Scope, unsigned long long> sTotal{};
            sAvg[mScope] = (sCalls[mScope] * sAvg[mScope] + duration.count()) / (sCalls[mScope] + 1);
            sCalls[mScope]++;
            sTotal[mScope] += duration.count();
            const auto duration_str = duration_to_str(duration);
            mStream << "\'" << mScope << "\'"
                    << " " << duration.count() << " [" << duration_str << "]"
                    << " avg. " << sAvg[mScope] << " [" << duration_str << "]"
                    << " cnt. " << sCalls[mScope] << " ttl. " << sTotal[mScope] << " [" << duration_str << "]";
            if (std::is_base_of<std::ios_base, Stream>::value)
            {
                mStream << "\n";
            }
        }

    private:
        Stream& mStream;
        Scope mScope;
        time_point mBegin;
    };

    template<typename Scope, typename Stream, typename Precision = std::chrono::microseconds>
    auto make_scope_time(Scope scope, Stream& stream)
    {
        return std::make_unique<scope_time<Scope, Stream, Precision>>(scope, stream);
    }

    template<typename Key, typename Stream, typename Precision = std::chrono::microseconds>
    struct time_tracker
    {
        static std::map<std::pair<Key, std::size_t>, time_point> sBeginPointByKey;
        static auto begin(Key key, Stream& stream)
        {
            const auto mapKey = std::pair{key, typeid(stream).hash_code()};
            const auto now = clock::now();
            sBeginPointByKey[mapKey] = now;
            return std::chrono::duration_cast<Precision>(now.time_since_epoch()).count();
        }

        static auto log(Key key, Stream& stream)
        {
            const auto mapKey = std::pair{key, typeid(stream).hash_code()};
            const auto now = clock::now();
            const auto it = sBeginPointByKey.find(mapKey);
            if (it != sBeginPointByKey.cend())
            {
                const auto elapsed = std::chrono::duration_cast<Precision>(now - it->second);
                stream << "\'" << key << "\'"
                       << " log: " << elapsed.count() << " [" << duration_to_str(elapsed) << "]";
                if (std::is_base_of<std::ios_base, Stream>::value)
                {
                    stream << "\n";
                }
                return elapsed.count();
            }
            return decltype(std::chrono::duration_cast<Precision>(now.time_since_epoch()).count())();
        }

        static auto end(Key key, Stream& stream)
        {
            const auto mapKey = std::pair{key, typeid(stream).hash_code()};
            const auto now = clock::now();
            const auto it = sBeginPointByKey.find(mapKey);
            if (it != sBeginPointByKey.cend())
            {
                const auto elapsed = std::chrono::duration_cast<Precision>(now - it->second);
                stream << "\'" << key << "\'"
                       << " end: " << elapsed.count() << " [" << duration_to_str(elapsed) << "]";
                if (std::is_base_of<std::ios_base, Stream>::value)
                {
                    stream << "\n";
                }
                sBeginPointByKey.erase(it);
                return elapsed.count();
            }
            return decltype(std::chrono::duration_cast<Precision>(now.time_since_epoch()).count())();
        }
    };

    template<typename Key, typename Stream, typename Precision>
    std::map<std::pair<Key, std::size_t>, time_point> time_tracker<Key, Stream, Precision>::sBeginPointByKey{};

    template<typename Key, typename Stream, typename Precision = std::chrono::microseconds>
    auto time_tracker_begin(Key key, Stream& stream)
    {
        return time_tracker<Key, Stream, Precision>::begin(key, stream);
    }

    template<typename Key, typename Stream, typename Precision = std::chrono::microseconds>
    auto time_tracker_log(Key key, Stream& stream)
    {
        return time_tracker<Key, Stream, Precision>::log(key, stream);
    }

    template<typename Key, typename Stream, typename Precision = std::chrono::microseconds>
    auto time_tracker_end(Key key, Stream& stream)
    {
        return time_tracker<Key, Stream, Precision>::end(key, stream);
    }

} // namespace mt

#define COMBINE1(X, Y) X##Y // helper macro
#define COMBINE(X, Y) COMBINE1(X, Y)

#define debugScopeTime(Scope) auto COMBINE(__debugScopeTime, __LINE__){mt::make_scope_time(Scope, std::cout)};
#define debugTime(Key) mt::time_tracker_begin(Key, std::cout);
#define debugTimeLog(Key) mt::time_tracker_log(Key, std::cout);
#define debugTimeEnd(Key) mt::time_tracker_end(Key, std::cout);

#if __has_include(<QDebug>)
#include <QDebug>

#define qDebugScopeTime(Scope) \
    auto COMBINE(__qDebugScopeThis, __LINE__){qDebug()}; \
    auto COMBINE(__qDebugScopeTime, __LINE__){mt::make_scope_time(Scope, COMBINE(__qDebugScopeThis, __LINE__).nospace())};
#define qDebugTime(Key) \
    \auto COMBINE(__qDebugTimeThis, __LINE__){qDebug()}; \
    \ mt::time_tracker_begin(Key, COMBINE(__qDebugTimeThis, __LINE__).nospace());
#define qDebugTimeLog(Key) \
    \auto COMBINE(__qDebugTimeLogThis, __LINE__){qDebug()}; \
    mt::time_tracker_log(Key, COMBINE(__qDebugTimeLogThis, __LINE__).nospace());
#define qDebugTimeEnd(Key) \
    \auto COMBINE(__qDebugTimeEndThis, __LINE__){qDebug()}; \
    \mt::time_tracker_end(Key, COMBINE(__qDebugTimeEndThis, __LINE__).nospace());

#define qWarnScopeTime(Scope) \
    auto COMBINE(__qWarnScopeThis, __LINE__){qWarning()}; \
    auto COMBINE(__qWarnScopeTime, __LINE__){mt::make_scope_time(Scope, COMBINE(__qWarnScopeThis, __LINE__).nospace())};
#define qWarnTime(Key) \
    \auto COMBINE(__qWarnTimeThis, __LINE__){qWarning()}; \
    \ mt::time_tracker_begin(Key, COMBINE(__qWarnTimeThis, __LINE__).nospace());
#define qWarnTimeLog(Key) \
    \auto COMBINE(__qWarnTimeLogThis, __LINE__){qWarning()}; \
    mt::time_tracker_log(Key, COMBINE(__qWarnTimeLogThis, __LINE__).nospace());
#define qWarnTimeEnd(Key) \
    \auto COMBINE(__qWarnTimeEndThis, __LINE__){qWarning()}; \
    \mt::time_tracker_end(Key, COMBINE(__qWarnTimeEndThis, __LINE__).nospace());
#endif

/* Some snipets for Qt Creator:
 *    dst dscopetime -> debugScopeTime($__FUNCTION__$);
 *    dtb dtimebegin -> debugTime(std::string{$key$});
 *    dte dtimeend -> debugTimeEnd(std::string{$key$});
 *    
 *    qdst qdscopetime -> qDebugScopeTime($__FUNCTION__$);
 *    qdtb qdtimebegin -> qDebugTime(std::string{$key$});
 *    qdte qdtimeend -> qDebugTimeEnd(std::string{$key$});
 *    
 *    qwst qwscopetime -> qWarnScopeTime($__FUNCTION__$);
 *    qwtb qwtimebegin -> qWarnTime(std::string{$key$});
 *    qwte qwtimeend -> qWarnTimeEnd(std::string{$key$});
 */

#endif // MEASURETIME_H
