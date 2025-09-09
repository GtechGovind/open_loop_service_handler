#include <iostream>
#include <vector>
#include <iomanip>
#include <cassert>
#include <stdexcept>
#include <functional>
#include "date_time.h"
#include "open_loop_service.h"

using namespace open_loop;

// --- Mini Testing Framework ---
int tests_passed = 0;
int tests_failed = 0;

void run_test(const std::string& test_name, const std::function<void()>& test_func) {
    std::cout << "  [RUNNING] " << std::left << std::setw(65) << test_name;
    try {
        test_func();
        std::cout << "[ PASS ]" << std::endl;
        tests_passed++;
    } catch (const std::exception& e) {
        std::cout << "[ FAIL ]" << std::endl;
        std::cerr << "      >> EXCEPTION: " << e.what() << std::endl;
        tests_failed++;
    } catch (...) {
        std::cout << "[ FAIL ]" << std::endl;
        std::cerr << "      >> UNKNOWN EXCEPTION" << std::endl;
        tests_failed++;
    }
}

// --- Helper Function to Create Consistent CSA Test Data ---
std::vector<uint8_t> create_csa_golden_data(std::time_t effective_date) {
    csa::container c(effective_date);
    c.get_general().set_version(1, 2, 3);
    c.get_general().set_language(language_code::English);
    
    csa::terminal term;
    term.set_acquirer_id(10);
    term.set_operator_id(1000);
    term.set_terminal_id("ABCDEF");
    
    c.get_validation().set_terminal_info(term);
    c.get_validation().set_date_and_time(1735689600000ULL); // Jan 1, 2025 00:00:00
    c.get_validation().set_fare_amount(1500);

    csa::log log1;
    log1.set_card_effective_date(effective_date);
    log1.set_terminal_info(term);
    log1.set_date_and_time(1735603200000ULL); // Dec 31, 2024 00:00:00
    log1.set_txn_sq_no(101);
    log1.set_card_balance(20000); // 0x4E20
    c.get_history().add_log(log1);
    
    c.set_rfu({0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03});

    return c.to_bytes();
}

// --- CSA Test Cases ---

void csa_test_full_object_printout() {
    csa::container csa(28283400);
    csa.get_general().set_version(1, 2, 3);
    csa.get_general().set_language(language_code::English);
    csa::terminal term;
    term.set_acquirer_id(15);
    term.set_operator_id(1025);
    term.set_terminal_id("A1B2C3");
    csa.get_validation().set_terminal_info(term);
    csa.get_validation().set_date_and_time(date_time::now());
    csa.get_validation().set_fare_amount(1250);
    csa::log log;
    log.set_card_effective_date(28283400);
    log.set_terminal_info(term);
    log.set_date_and_time(date_time::now() - 3600000);
    log.set_txn_sq_no(102);
    log.set_card_balance(8750);
    csa.get_history().add_log(log);
    csa.set_rfu({0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED, 0x00});
    std::cout << "\n--- Fully Populated CSA Object ---\n" << csa << std::endl;
}

void csa_test_exceptions() {
    bool thrown = false;
    try { csa::general g; g.set_version(8, 0, 0); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);

    thrown = false;
    try { csa::terminal t; t.set_terminal_id("12345G"); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);

    thrown = false;
    try { csa::validation v; v.set_date_and_time(date_time::now()); }
    catch (const std::logic_error&) { thrown = true; }
    assert(thrown);

    thrown = false;
    try { csa::log l; l.set_card_balance(0x100000); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);
}

void csa_test_history_cyclical_buffer() {
    csa::history hist;
    hist.set_card_effective_date(1000);
    csa::log log1; log1.set_card_effective_date(1000); log1.set_txn_sq_no(1);
    csa::log log2; log2.set_card_effective_date(1000); log2.set_txn_sq_no(2);
    csa::log log3; log3.set_card_effective_date(1000); log3.set_txn_sq_no(3);
    csa::log log4; log4.set_card_effective_date(1000); log4.set_txn_sq_no(4);
    csa::log log5; log5.set_card_effective_date(1000); log5.set_txn_sq_no(5);
    csa::log log6; log6.set_card_effective_date(1000); log6.set_txn_sq_no(6);
    hist.add_log(log1); hist.add_log(log2); hist.add_log(log3); hist.add_log(log4);
    hist.add_log(log5); // Buffer: [5, 4, 3, 2]
    assert(hist.get_logs()[3].get_txn_sq_no() == 2);
    hist.add_log(log6); // Buffer: [6, 5, 4, 3]
    assert(hist.get_logs()[0].get_txn_sq_no() == 6);
    assert(hist.get_logs()[3].get_txn_sq_no() == 3);
}

void csa_test_serialization_round_trip() {
    csa::container original(28283400);
    original.get_validation().set_fare_amount(500);
    std::vector<uint8_t> bytes = original.to_bytes();
    csa::container parsed = csa::container::parse(bytes, 28283400);
    assert(original == parsed);
}

void csa_test_raw_data_parse_and_reserialize() {
    constexpr std::time_t csa_date = 28399680;
    const std::vector<uint8_t> raw_data = create_csa_golden_data(csa_date);
    assert(raw_data.size() == 96);
    auto parsed_csa = csa::container::parse(raw_data, csa_date);
    assert(parsed_csa.get_general().get_version_string() == "1.2.3");
    assert(parsed_csa.get_validation().get_fare_amount() == 1500);
    assert(parsed_csa.get_history().get_logs()[0].get_card_balance() == 20000);
    std::vector<uint8_t> reserialized_bytes = parsed_csa.to_bytes();
    assert(raw_data == reserialized_bytes);
}

// --- OSA Test Cases ---

void osa_test_full_object_printout() {
    osa::container osa(28300000);
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

void osa_test_exceptions() {
    bool thrown = false;
    try { osa::general g; g.set_phone_number("123"); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);

    thrown = false;
    try { osa::trip_pass p; p.set_trips_allotted(50); p.set_remaining_trips(51); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);

    thrown = false;
    try { osa::trip_pass p; p.set_pass_expiry(date_time::now()); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);
}

void osa_test_serialization_round_trip() {
    osa::container original(28300000);
    original.get_general().set_phone_number("7977192875");
    original.get_trip_pass(0).set_trips_allotted(40);
    original.get_trip_pass(0).set_remaining_trips(35);
    std::vector<uint8_t> bytes = original.to_bytes();
    assert(bytes.size() == 96);
    osa::container parsed = osa::container::parse(bytes, 28300000);
    assert(original == parsed);
}

void osa_test_bcd_and_time_formats() {
    osa::general gen;
    gen.set_phone_number("1234567890");
    std::vector<uint8_t> gen_bytes = gen.to_bytes();
    assert(gen_bytes[1] == 0x12 && gen_bytes[2] == 0x34 && gen_bytes[5] == 0x90);

    osa::trip_pass pass;
    pass.set_pass_expiry(1000000000); // 1,000,000 seconds in milliseconds
    std::vector<uint8_t> pass_bytes = pass.to_bytes();
    // 1,000,000 = 0x0F4240
    assert(pass_bytes[1] == 0x0F && pass_bytes[2] == 0x42 && pass_bytes[3] == 0x40);
}

// --- Main Test Runner ---
int main() {
    std::cout << "====================================================================" << std::endl;
    std::cout << "                 RUNNING OPEN-LOOP LIBRARY TESTS" << std::endl;
    std::cout << "====================================================================" << std::endl;

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
    
    std::cout << "\n====================================================================" << std::endl;
    std::cout << "                           TEST REPORT" << std::endl;
    std::cout << "====================================================================" << std::endl;
    std::cout << "  TOTAL TESTS : " << (tests_passed + tests_failed) << std::endl;
    std::cout << "  PASSED      : " << tests_passed << std::endl;
    std::cout << "  FAILED      : " << tests_failed << std::endl;
    std::cout << "====================================================================" << std::endl;

    return (tests_failed > 0) ? 1 : 0;
}