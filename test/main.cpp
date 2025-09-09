#include <iostream>
#include <vector>
#include <iomanip>
#include <cassert>
#include <stdexcept>
#include <functional>
#include "date_time.h"
#include "open_loop_service.h"
#ifdef _WIN32
#include <windows.h>
#endif

using namespace open_loop;

/**
 * @struct console_color
 * @brief A utility struct to hold ANSI escape codes for terminal colors.
 */
struct console_color {
    static constexpr auto RESET = "\033[0m";
    static constexpr auto BOLD = "\033[1m";
    static constexpr auto RED = "\033[31m";
    static constexpr auto GREEN = "\033[32m";
    static constexpr auto YELLOW = "\033[33m";
    static constexpr auto CYAN = "\033[36m";
};

/**
 * @brief Enables ANSI escape code processing on Windows consoles.
 * @details On modern Windows 10/11, this function enables the VIRTUAL_TERMINAL_PROCESSING
 *          flag, allowing the console to interpret ANSI color codes. On other platforms
 *          (Linux, macOS), this is typically enabled by default.
 */
void enable_virtual_terminal_processing() {
#ifdef _WIN32
    if (const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE); hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

int tests_passed = 0;
int tests_failed = 0;

/**
 * @brief Runs a single test case with professional, colorized, and animated reporting.
 * @param test_name The name of the test to be displayed.
 * @param test_func The function containing the test logic.
 */
void run_test(const std::string& test_name, const std::function<void()>& test_func) {
    // Print the initial "RUNNING" status without a newline
    std::cout << "  " << console_color::BOLD << std::left << std::setw(65) << test_name
              << console_color::RESET << "[" << console_color::CYAN << "RUNNING" << console_color::RESET << "]" << std::flush;
    try {
        test_func();
        // Use carriage return '\r' to go back to the start of the line and overwrite
        std::cout << "\r  " << console_color::BOLD << std::left << std::setw(65) << test_name
                  << console_color::RESET << "[" << console_color::GREEN << console_color::BOLD << " PASS " << console_color::RESET << "]" << std::endl;
        tests_passed++;
    } catch (const std::exception& e) {
        std::cout << "\r  " << console_color::BOLD << std::left << std::setw(65) << test_name
                  << console_color::RESET << "[" << console_color::RED << console_color::BOLD << " FAIL " << console_color::RESET << "]" << std::endl;
        std::cerr << "      " << console_color::RED << ">> EXCEPTION: " << e.what() << console_color::RESET << std::endl;
        tests_failed++;
    } catch (...) {
        std::cout << "\r  " << console_color::BOLD << std::left << std::setw(65) << test_name
                  << console_color::RESET << "[" << console_color::RED << console_color::BOLD << " FAIL " << console_color::RESET << "]" << std::endl;
        std::cerr << "      " << console_color::RED << ">> UNKNOWN EXCEPTION" << console_color::RESET << std::endl;
        tests_failed++;
    }
}

// --- Helper Function to Create Consistent CSA Test Data ---
std::vector<uint8_t> create_csa_golden_data(std::time_t effective_date) {
    csa::container c;
    c.set_card_effective_date(effective_date);
    c.get_general().set_version(1, 2, 3);
    csa::terminal term;
    term.set_acquirer_id(10); term.set_operator_id(1000); term.set_terminal_id("ABCDEF");
    c.get_validation().set_terminal_info(term);
    c.get_validation().set_date_and_time(1735689600000ULL);
    c.get_validation().set_fare_amount(1500);
    csa::log log1;
    log1.set_card_effective_date(effective_date);
    log1.set_terminal_info(term);
    log1.set_date_and_time(1735603200000ULL);
    log1.set_txn_sq_no(101);
    log1.set_card_balance(20000);
    c.get_history().add_log(log1);
    c.set_rfu({0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03});
    return c.to_bytes();
}

// --- CSA Test Cases ---

/**
 * @brief Tests the creation and printout of a fully populated CSA object.
 * @details This test serves as a visual verification. It constructs a `csa::container` and
 *          populates every child object with realistic data. It then prints the entire
 *          container to the console, demonstrating the `operator<<` functionality.
 * @note A passing result is a clean, well-formatted printout without exceptions.
 */
void csa_test_full_object_printout() {
    csa::container csa;
    csa.set_card_effective_date(28283400);
    csa.get_general().set_version(1, 2, 3);
    csa::terminal term;
    term.set_acquirer_id(15); term.set_operator_id(1025); term.set_terminal_id("A1B2C3");
    csa.get_validation().set_terminal_info(term);
    csa.get_validation().set_date_and_time(date_time::now());
    csa.get_validation().set_fare_amount(1250);
    csa::log log;
    log.set_card_effective_date(28283400);
    log.set_terminal_info(term);
    log.set_date_and_time(date_time::now() - 3600000);
    log.set_txn_sq_no(102); log.set_card_balance(8750);
    csa.get_history().add_log(log);
    csa.set_rfu({0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED, 0x00});
    std::cout << "\n--- Fully Populated CSA Object ---\n" << csa << std::endl;
}

/**
 * @brief Verifies that the library correctly throws exceptions for invalid data.
 * @details This test attempts to set out-of-range values for versions, invalid hex strings,
 *          and call time-sensitive functions before initialization. A pass indicates that
 *          all invalid operations were correctly caught and threw the expected exceptions.
 */
void csa_test_exceptions() {
    bool thrown = false;
    try { csa::general g; g.set_version(8, 0, 0); } catch (const std::out_of_range&) { thrown = true; } assert(thrown);
    thrown = false;
    try { csa::terminal t; t.set_terminal_id("12345G"); } catch (const std::out_of_range&) { thrown = true; } assert(thrown);
    thrown = false;
    try { csa::validation v; v.set_date_and_time(date_time::now()); } catch (const std::logic_error&) { thrown = true; } assert(thrown);
    thrown = false;
    try { csa::log l; l.set_card_balance(0x100000); } catch (const std::out_of_range&) { thrown = true; } assert(thrown);
}

/**
 * @brief Validates the "push-down" and cyclical logic of the `csa::history` buffer.
 * @details It adds 6 logs to a buffer of size 4 and asserts at each step that the
 *          newest log is at the front and the oldest logs are correctly discarded,
 *          proving the FIFO (First-In, First-Out) cyclical behavior.
 */
void csa_test_history_cyclical_buffer() {
    csa::history hist;
    hist.set_card_effective_date(1000);
    csa::log logs[6];
    for(int i=0; i<6; ++i) { logs[i].set_card_effective_date(1000); logs[i].set_txn_sq_no(i+1); }
    hist.add_log(logs[0]); hist.add_log(logs[1]); hist.add_log(logs[2]); hist.add_log(logs[3]);
    hist.add_log(logs[4]); // Buffer should be [5, 4, 3, 2]
    assert(hist.get_logs()[3].get_txn_sq_no() == 2);
    hist.add_log(logs[5]); // Buffer should be [6, 5, 4, 3]
    assert(hist.get_logs()[0].get_txn_sq_no() == 6);
    assert(hist.get_logs()[3].get_txn_sq_no() == 3);
}

/**
 * @brief Verifies the `Object -> Bytes -> Object` serialization round-trip.
 * @details This test creates an object, serializes it to bytes, parses those bytes back
 *          into a new object, and asserts that the original and parsed objects are identical.
 */
void csa_test_serialization_round_trip() {
    csa::container original;
    original.set_card_effective_date(28283400);
    original.get_validation().set_fare_amount(500);
    std::vector<uint8_t> bytes = original.to_bytes();
    csa::container parsed;
    parsed.set_card_effective_date(28283400);
    parsed.parse(bytes);
    assert(original == parsed);
}

/**
 * @brief Verifies the `Bytes -> Object -> Bytes` serialization integrity.
 * @details This is a critical "vice-versa" test. It starts with a known-good ("golden")
 *          byte array, parses it into an object, and then re-serializes that object back
 *          into a new byte array. It asserts that the original and re-serialized byte
 *          arrays are identical, proving perfect symmetry.
 */
void csa_test_raw_data_parse_and_reserialize() {
    constexpr std::time_t csa_date = 28399680;
    const std::vector<uint8_t> raw_data = create_csa_golden_data(csa_date);
    assert(raw_data.size() == 96);
    csa::container parsed_csa;
    parsed_csa.set_card_effective_date(csa_date);
    parsed_csa.parse(raw_data);
    std::vector<uint8_t> reserialized_bytes = parsed_csa.to_bytes();
    assert(raw_data == reserialized_bytes);
}

// --- OSA Test Cases ---

/**
 * @brief Tests the creation and printout of a fully populated OSA object.
 * @details Similar to the CSA version, this test serves as a visual verification of the
 *          entire `osa::container` and all its child objects.
 */
void osa_test_full_object_printout() {
    osa::container osa;
    osa.set_card_effective_date(28300000);
    osa.get_general().set_version(2, 0, 1);
    osa.get_general().set_phone_number("7977192875");
    osa.get_general().set_service_status(osa::general::service_status::active);
    osa.get_validation().set_date_and_time(date_time::now());
    osa.get_validation().set_station_id(505);
    osa::transaction_record rec;
    rec.set_card_effective_date(28300000);
    rec.set_fare(50);
    osa.get_history().add_record(rec);
    osa::trip_pass pass1;
    pass1.set_pass_id(101);
    pass1.set_pass_expiry(15552000000ULL); // June 1970
    pass1.set_trips_allotted(40);
    pass1.set_remaining_trips(35);
    osa.set_trip_pass(pass1, 0);
    std::cout << "\n--- Fully Populated OSA Object ---\n" << osa << std::endl;
}

/**
 * @brief Verifies that the OSA classes correctly throw exceptions for invalid data.
 * @details Tests invalid phone numbers, inconsistent trip counts, and out-of-range timestamps
 *          for the `trip_pass` class.
 */
void osa_test_exceptions() {
    bool thrown = false;
    try { osa::general g; g.set_phone_number("123"); } catch (const std::invalid_argument&) { thrown = true; } assert(thrown);
    thrown = false;
    try { osa::trip_pass p; p.set_trips_allotted(50); p.set_remaining_trips(51); } catch (const std::invalid_argument&) { thrown = true; } assert(thrown);
    thrown = false;
    try { osa::trip_pass p; p.set_pass_expiry(date_time::now()); } catch (const std::out_of_range&) { thrown = true; } assert(thrown);
}

/**
 * @brief Verifies the `Object -> Bytes -> Object` serialization round-trip for the OSA.
 */
void osa_test_serialization_round_trip() {
    osa::container original;
    original.set_card_effective_date(28300000);
    original.get_general().set_phone_number("7977192875");
    original.get_trip_pass(0).set_trips_allotted(40);
    original.get_trip_pass(0).set_remaining_trips(35);
    std::vector<uint8_t> bytes = original.to_bytes();
    assert(bytes.size() == 96);
    osa::container parsed;
    parsed.set_card_effective_date(28300000);
    parsed.parse(bytes);
    assert(original == parsed);
}

/**
 * @brief Validates the correctness of specific, complex data formats in the OSA.
 * @details This test performs byte-level verification of the Binary-Coded Decimal (BCD)
 *          phone number and the 24-bit absolute second timestamp in the `trip_pass`.
 */
void osa_test_bcd_and_time_formats() {
    osa::general gen;
    gen.set_phone_number("1234567890");
    std::vector<uint8_t> gen_bytes = gen.to_bytes();
    assert(gen_bytes[1] == 0x12 && gen_bytes[2] == 0x34 && gen_bytes[5] == 0x90);
    osa::trip_pass pass;
    pass.set_pass_expiry(1000000000); // 1,000,000 seconds
    std::vector<uint8_t> pass_bytes = pass.to_bytes();
    // 1,000,000 = 0x0F4240
    assert(pass_bytes[1] == 0x0F && pass_bytes[2] == 0x42 && pass_bytes[3] == 0x40);
}

// --- Main Test Runner ---
int main() {
    // Enable color support on Windows
    enable_virtual_terminal_processing();

    std::cout << console_color::BOLD << "========================================================================" << std::endl;
    std::cout << "                 RUNNING OPEN-LOOP LIBRARY TESTS" << std::endl;
    std::cout << "========================================================================" << console_color::RESET << std::endl;

    std::cout << "\n----- COMMON SERVICE AREA (CSA) -----" << std::endl;
    run_test("1. Full object printout (visual verification)", csa_test_full_object_printout);
    run_test("2. Exception handling for invalid data", csa_test_exceptions);
    run_test("3. History cyclical buffer logic", csa_test_history_cyclical_buffer);
    run_test("4. Full serialization/deserialization round-trip", csa_test_serialization_round_trip);
    run_test("5. Raw data parse -> reserialize integrity (vice-versa)", csa_test_raw_data_parse_and_reserialize);

    std::cout << "\n----- OPERATOR SERVICE AREA (OSA) -----" << std::endl;
    run_test("6. Full object printout (visual verification)", osa_test_full_object_printout);
    run_test("7. Exception handling for invalid data", osa_test_exceptions);
    run_test("8. Full serialization/deserialization round-trip", osa_test_serialization_round_trip);
    run_test("9. BCD phone number and absolute time format integrity", osa_test_bcd_and_time_formats);

    std::cout << "\n" << console_color::BOLD << "========================================================================" << std::endl;
    std::cout << "                           TEST REPORT" << std::endl;
    std::cout << "========================================================================" << std::endl;
    std::cout << "  TOTAL TESTS : " << (tests_passed + tests_failed) << std::endl;
    std::cout << "  " << console_color::GREEN << "PASSED      : " << tests_passed << console_color::RESET << std::endl;
    std::cout << "  " << (tests_failed > 0 ? console_color::RED : console_color::YELLOW)
              << "FAILED      : " << tests_failed << console_color::RESET << std::endl;
    std::cout << "========================================================================" << console_color::RESET << std::endl;

    return (tests_failed > 0) ? 1 : 0;
}