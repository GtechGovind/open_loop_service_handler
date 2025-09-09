#pragma once

#include <chrono>      // For the core C++ time library
#include <string>      // For std::string
#include <sstream>     // For std::stringstream
#include <iomanip>     // For std::setw, std::setfill
#include <stdexcept>   // For std::runtime_error, std::invalid_argument
#include <ctime>       // For std::tm, std::time_t, std::mktime
#include <vector>      // For internal buffer in format()
#include <cctype>      // For isdigit

/**
 * @file date_time.h
 * @version 4.0
 * @author Govind Yadav (Professionally Refactored)
 *
 * @brief A professional, self-contained C++17 utility library for high-performance date and time operations.
 *
 * @details This library provides a robust, thread-safe, and easy-to-use interface for common
 *          date and time tasks. The public API is intentionally simplified to exclusively use
 *          `long long` for millisecond timestamps and `std::string` for formatting.
 *
 *          This design abstracts away the complexity of the C++ `<chrono>` type system,
 *          providing a stable and straightforward interface for developers, while still
 *          leveraging the full power, type-safety, and precision of `<chrono>` for all
 *          internal calculations. All time-zone-sensitive functions come in two variants:
 *          one for the system's local time zone and one for UTC, promoting unambiguous code.
 *
 * @copyright Copyright (c) 2025
 */
namespace date_time {
    //================================================================================
    // Public Constants and Type Aliases
    //================================================================================

    //! @brief A type alias for a `long long` representing a timestamp in milliseconds since the Unix epoch.
    using timestamp = long long;

    inline const std::string ISO_8601_FORMAT = "%Y-%m-%dT%H:%M:%S.%msZ"; ///< ISO 8601 format with milliseconds, UTC.
    inline const std::string DEFAULT_DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S"; ///< e.g., "2025-09-02 15:45:00"
    inline const std::string DEFAULT_DATE_FORMAT = "%Y-%m-%d"; ///< e.g., "2025-09-02"
    inline const std::string DEFAULT_TIME_FORMAT = "%H:%M:%S"; ///< e.g., "15:45:00"

    constexpr timestamp MILLISECONDS_IN_SECOND = 1000LL;
    constexpr timestamp MILLISECONDS_IN_MINUTE = 60 * MILLISECONDS_IN_SECOND;
    constexpr timestamp MILLISECONDS_IN_HOUR = 60 * MILLISECONDS_IN_MINUTE;
    constexpr timestamp MILLISECONDS_IN_DAY = 24 * MILLISECONDS_IN_HOUR;


    //================================================================================
    // Internal Implementation Details (Not intended for direct use)
    //================================================================================
    namespace internal {
        /** @internal @brief A type alias for the system clock. */
        using sys_clock = std::chrono::system_clock;
        /** @internal @brief A type alias for a system clock time point. */
        using time_point = sys_clock::time_point;
        /** @internal @brief A type alias for millisecond duration. */
        using milliseconds = std::chrono::milliseconds;
        /** @internal @brief A type alias for second duration. */
        using seconds = std::chrono::seconds;
        /** @internal @brief A type alias for minute duration. */
        using minutes = std::chrono::minutes;
        /** @internal @brief A type alias for hour duration. */
        using hours = std::chrono::hours;
        /** @internal @brief A type alias for a 24-hour day duration. */
        using days = std::chrono::duration<long long, std::ratio<86400> >;

        /**
         * @internal
         * @brief Converts a millisecond timestamp to a C++ time_point object.
         * @param ms The timestamp in milliseconds since epoch.
         * @return A std::chrono::system_clock::time_point object.
         */
        inline time_point to_time_point(timestamp ms) { return time_point{milliseconds(ms)}; }

        /**
         * @internal
         * @brief Converts a C++ time_point object to a millisecond timestamp.
         * @param tp The std::chrono::system_clock::time_point object.
         * @return A timestamp in milliseconds since epoch.
         */
        inline timestamp to_milliseconds(const time_point &tp) {
            return std::chrono::duration_cast<milliseconds>(tp.time_since_epoch()).count();
        }
    }


    //================================================================================
    // Public API: Time Point Acquisition
    //================================================================================

    /**
     * @brief Gets the current system time as milliseconds since the Unix epoch (UTC).
     * @return A `timestamp` representing the current moment.
     */
    [[nodiscard]] inline timestamp now() {
        return internal::to_milliseconds(internal::sys_clock::now());
    }


    //================================================================================
    // Public API: Formatting (Timestamp -> String)
    //================================================================================

    /**
     * @brief Formats a timestamp into a string based on the system's LOCAL time zone.
     * @param ms_timestamp The timestamp to format.
     * @param format The `strftime`-compatible format string. Supports `%ms` for milliseconds.
     * @return The formatted time string.
     * @note The output depends on the time zone settings of the machine running the code.
     *       For servers and logs, `format_utc` is strongly recommended.
     */
    [[nodiscard]] inline std::string format_local(timestamp ms_timestamp, const std::string &format);

    /**
     * @brief Formats a timestamp into a string based on UTC.
     * @param ms_timestamp The timestamp to format.
     * @param format The `strftime`-compatible format string. Supports `%ms` for milliseconds.
     * @return The formatted UTC time string.
     * @note This function is timezone-independent and is the preferred choice for APIs and logging.
     */
    [[nodiscard]] inline std::string format_utc(timestamp ms_timestamp, const std::string &format);


    //================================================================================
    // Public API: Parsing (String -> Timestamp)
    //================================================================================

    /**
     * @brief Parses a time string into a timestamp, assuming the string is in the LOCAL time zone.
     * @param time_str The time string to parse (e.g., "2025-09-02 10:30:00").
     * @param format The `strftime`-compatible format string. Supports `%ms` for fractional seconds.
     * @return The parsed timestamp.
     * @throws std::runtime_error if the string cannot be fully parsed according to the format.
     * @note The result depends on the time zone settings of the machine. Use `parse_utc` for consistency.
     */
    [[nodiscard]] inline timestamp parse_local(const std::string &time_str, const std::string &format);

    /**
     * @brief Parses a time string into a timestamp, assuming the string is in UTC.
     * @param time_str The UTC time string to parse (e.g., "2025-09-02T10:30:00.123Z").
     * @param format The `strftime`-compatible format string. Supports `%ms` for fractional seconds.
     * @return The parsed timestamp.
     * @throws std::runtime_error if the string cannot be fully parsed according to the format.
     * @note This is the preferred parsing function for unambiguous time representation.
     */
    [[nodiscard]] inline timestamp parse_utc(const std::string &time_str, const std::string &format);


    //================================================================================
    // Public API: Time Arithmetic
    //================================================================================

    /** @brief Adds a specified number of days to a timestamp. A day is exactly 24 hours. */
    [[nodiscard]] inline timestamp add_days(const timestamp ms, const int days) {
        return ms + static_cast<timestamp>(days) * MILLISECONDS_IN_DAY;
    }

    /** @brief Adds a specified number of hours to a timestamp. */
    [[nodiscard]] inline timestamp add_hours(const timestamp ms, const int hours) {
        return ms + static_cast<timestamp>(hours) * MILLISECONDS_IN_HOUR;
    }

    /** @brief Adds a specified number of minutes to a timestamp. */
    [[nodiscard]] inline timestamp add_minutes(const timestamp ms, const int minutes) {
        return ms + static_cast<timestamp>(minutes) * MILLISECONDS_IN_MINUTE;
    }

    /** @brief Adds a specified number of seconds to a timestamp. */
    [[nodiscard]] inline timestamp add_seconds(const timestamp ms, const int seconds) {
        return ms + static_cast<timestamp>(seconds) * MILLISECONDS_IN_SECOND;
    }


    //================================================================================
    // Public API: Time Truncation
    //================================================================================

    /** @brief Truncates a timestamp to the beginning of its second (milliseconds become 0). */
    [[nodiscard]] inline timestamp to_second(const timestamp ms) {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds{ms}).count();
    }

    /** @brief Truncates a timestamp to the beginning of its minute (seconds and ms become 0). */
    [[nodiscard]] inline timestamp to_minute(const timestamp ms) {
        return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::milliseconds{ms}).count();
    }

    /** @brief Truncates a timestamp to the beginning of its hour (minutes, seconds, ms become 0). */
    [[nodiscard]] inline timestamp to_hour(const timestamp ms) {
        return std::chrono::duration_cast<std::chrono::hours>(std::chrono::milliseconds{ms}).count();
    }


    //================================================================================
    // Public API: Time Difference
    //================================================================================

    /** @brief Calculates the whole number of 24-hour periods between two timestamps. */
    [[nodiscard]] inline long long difference_in_days(const timestamp t1, const timestamp t2) {
        return std::chrono::duration_cast<internal::days>(internal::milliseconds(t1 - t2)).count();
    }

    /** @brief Calculates the whole number of hours between two timestamps. */
    [[nodiscard]] inline long long difference_in_hours(const timestamp t1, const timestamp t2) {
        return std::chrono::duration_cast<internal::hours>(internal::milliseconds(t1 - t2)).count();
    }

    /** @brief Calculates the whole number of minutes between two timestamps. */
    [[nodiscard]] inline long long difference_in_minutes(const timestamp t1, const timestamp t2) {
        return std::chrono::duration_cast<internal::minutes>(internal::milliseconds(t1 - t2)).count();
    }

    /** @brief Calculates the whole number of seconds between two timestamps. */
    [[nodiscard]] inline long long difference_in_seconds(const timestamp t1, const timestamp t2) {
        return std::chrono::duration_cast<internal::seconds>(internal::milliseconds(t1 - t2)).count();
    }


    //================================================================================
    // Function Implementations
    //================================================================================

    namespace internal {
        /**
         * @internal
         * @brief Core implementation for formatting a timestamp to a string.
         * @param ms_timestamp The timestamp to format.
         * @param format The format string.
         * @param is_utc True to format in UTC, false for local time.
         * @return The formatted string.
         */
        inline std::string format_impl(timestamp ms_timestamp, const std::string &format, bool is_utc) {
            const auto tp = to_time_point(ms_timestamp);
            const std::time_t time_t_value = sys_clock::to_time_t(tp);
            std::tm time_info{};
#if defined(_WIN32)
            if (is_utc) gmtime_s(&time_info, &time_t_value);
            else localtime_s(&time_info, &time_t_value);
#else
            if (is_utc) gmtime_r(&time_t_value, &time_info);
            else localtime_r(&time_t_value, &time_info);
#endif
            std::vector<char> buffer(256);
            size_t written_len = 0;
            while ((written_len = std::strftime(buffer.data(), buffer.size(), format.c_str(), &time_info)) == 0) {
                if (buffer.size() > 8192)
                    throw std::runtime_error("Exceeded buffer size limit during formatting.");
                buffer.resize(buffer.size() * 2);
            }
            std::string result(buffer.data(), written_len);
            if (const auto pos = result.find("%ms"); pos != std::string::npos) {
                const auto ms_part = ms_timestamp >= 0 ? ms_timestamp % 1000 : (-ms_timestamp) % 1000;
                std::stringstream ss;
                ss << std::setw(3) << std::setfill('0') << ms_part;
                result.replace(pos, 3, ss.str());
            }
            return result;
        }

#if defined(_WIN32)
        /**
         * @internal
         * @brief A self-contained implementation of strptime for Windows/MSVC, where it is not standard.
         * @note This avoids reliance on the buggy std::get_time in some toolchains.
         */
        inline char *strptime(const char *s, const char *f, struct tm *tm) {
            int i;
            for (; *f; ++f) {
                if (*f == '%') {
                    ++f;
                    switch (*f) {
                        case 'd':
                        case 'e':
                            if (!s[0] || !s[1] || !isdigit(s[0]) || !isdigit(s[1])) return nullptr;
                            tm->tm_mday = (s[0] - '0') * 10 + (s[1] - '0');
                            s += 2;
                            break;
                        case 'H':
                            if (!s[0] || !s[1] || !isdigit(s[0]) || !isdigit(s[1])) return nullptr;
                            tm->tm_hour = (s[0] - '0') * 10 + (s[1] - '0');
                            s += 2;
                            break;
                        case 'm':
                            if (!s[0] || !s[1] || !isdigit(s[0]) || !isdigit(s[1])) return nullptr;
                            tm->tm_mon = (s[0] - '0') * 10 + (s[1] - '0') - 1;
                            s += 2;
                            break;
                        case 'M':
                            if (!s[0] || !s[1] || !isdigit(s[0]) || !isdigit(s[1])) return nullptr;
                            tm->tm_min = (s[0] - '0') * 10 + (s[1] - '0');
                            s += 2;
                            break;
                        case 'S':
                            if (!s[0] || !s[1] || !isdigit(s[0]) || !isdigit(s[1])) return nullptr;
                            tm->tm_sec = (s[0] - '0') * 10 + (s[1] - '0');
                            s += 2;
                            break;
                        case 'Y':
                            if (!s[0] || !s[1] || !s[2] || !s[3] || !isdigit(s[0]) || !isdigit(s[1]) || !isdigit(s[2])
                                || !isdigit(s[3])) return nullptr;
                            tm->tm_year = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0') -
                                          1900;
                            s += 4;
                            break;
                        case '%':
                            if (*s++ != '%') return nullptr;
                            break;
                        default: return nullptr;
                    }
                } else {
                    if (*s++ != *f) return nullptr;
                }
            }
            return (char *) s;
        }
#endif

        /**
         * @internal
         * @brief Core implementation for parsing a string into a timestamp.
         * @param time_str The string to parse.
         * @param format The format string.
         * @param is_utc True to parse as UTC, false as local time.
         * @return The parsed timestamp.
         */
        inline timestamp parse_impl(const std::string &time_str, const std::string &format, const bool is_utc) {
            std::string main_format = format;
            std::string main_time_str = time_str;
            milliseconds fractional_ms(0);
            if (const auto pos = format.find("%ms"); pos != std::string::npos) {
                if (pos > 0 && pos <= time_str.length() && !isdigit(format[pos - 1])) {
                    const char separator = format[pos - 1];
                    if (const auto sep_pos = time_str.rfind(separator);
                        sep_pos != std::string::npos && sep_pos < time_str.length() - 1) {
                        try {
                            main_format = format.substr(0, pos - 1);
                            main_time_str = time_str.substr(0, sep_pos);
                            const long long ms_val = std::stoll(time_str.substr(sep_pos + 1));
                            fractional_ms = milliseconds(ms_val);
                        } catch (...) {
                            /* Ignore parse error, fallback to parsing without ms */
                        }
                    }
                }
            }
            std::tm time_info = {};
            if (const char *parse_end = strptime(main_time_str.c_str(), main_format.c_str(), &time_info);
                parse_end == nullptr || *parse_end != '\0')
                throw std::runtime_error(
                    "Failed to parse time string '" + main_time_str + "' with format '" + main_format +
                    "' or extra characters found.");
            std::time_t time_t_val;
            if (is_utc) {
#if defined(_WIN32)
                time_info.tm_isdst = 0;
                time_t_val = _mkgmtime(&time_info);
#else
                time_info.tm_isdst = 0;
                time_t_val = timegm(&time_info);
#endif
            } else {
                time_info.tm_isdst = -1; // Let mktime decide DST
                time_t_val = std::mktime(&time_info);
            }
            if (time_t_val == -1)
                throw std::runtime_error("Parsed time cannot be represented as a valid calendar time.");
            const auto parsed_tp = internal::sys_clock::from_time_t(time_t_val);
            return to_milliseconds(parsed_tp) + fractional_ms.count();
        }
    }

    inline std::string format_local(const timestamp ms_timestamp, const std::string &format) {
        return internal::format_impl(ms_timestamp, format, false);
    }

    inline std::string format_utc(const timestamp ms_timestamp, const std::string &format) {
        return internal::format_impl(ms_timestamp, format, true);
    }

    inline timestamp parse_local(const std::string &time_str, const std::string &format) {
        return internal::parse_impl(time_str, format, false);
    }

    inline timestamp parse_utc(const std::string &time_str, const std::string &format) {
        return internal::parse_impl(time_str, format, true);
    }
}
