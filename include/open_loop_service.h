/**
* @file open_loop_service.h
 * @brief Defines the data structures for the Common Service Area (CSA) of an open-loop transit card.
 * @details This header provides a complete, self-contained C++ interface for parsing, manipulating,
 *          and serializing the 96-byte Common Service Area as specified by NCMC and related standards.
 *          It is designed with a focus on data integrity, type safety, and ease of use by modeling
 *          each distinct data block of the CSA as a dedicated, fully encapsulated class.
 *
 *          The library follows a traditional C++ design pattern using default constructors and
 *          validated setter methods, avoiding more complex patterns to ensure clarity and accessibility.
 *          All functionality is organized within the `open_loop` namespace, with CSA-specific
 *          components nested inside the `open_loop::csa` namespace.
 *
 * @author Govind Yadav
 * @version 1.1
 * @date 2025-09-08
 */

#pragma once
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

/**
 * @namespace open_loop
 * @brief Encapsulates all components related to the open-loop payment and transit system standards.
 * @details This namespace serves as the root for data structures and logic that conform to
 *          open-loop specifications. The "open-loop" paradigm allows a single payment card (like a
 *          debit or credit card) to be used across multiple, independent transit systems, promoting
 *          interoperability and convenience for the user. This library provides the foundational
 *          tools to read from and write to the standardized data areas on such a card.
 */
namespace open_loop {

    /**
     * @enum language_code
     * @brief Defines the 5-bit language codes as specified by the NCMC standard.
     */
    enum class language_code : uint8_t {
        English = 0b00000,
        Hindi = 0b00001,
        Bengali = 0b00010,
        Marathi = 0b00011,
        Telugu = 0b00100,
        Tamil = 0b00101,
        Gujarati = 0b00110,
        Urdu = 0b00111,
        Kannada = 0b01000,
        Odia = 0b01001,
        Malayalam = 0b01010,
        Punjabi = 0b01011,
        Sanskrit = 0b01100,
        Assamese = 0b01101,
        Maithili = 0b01110,
        Santali = 0b01111,
        Kashmiri = 0b10000,
        Nepali = 0b10001,
        Sindhi = 0b10010,
        Dogri = 0b10011,
        Konkani = 0b10100,
        Manipuri = 0b10101,
        Bodo = 0b10110,
        RFU_Start = 0b10111,
        RFU_End = 0b11111
    };

    /**
     * @enum txn_status
     * @brief Defines the 4-bit transaction status codes.
     */
    enum class txn_status : uint8_t {
        EXIT = 0x0,
        ENTRY = 0x1,
        PENALTY = 0x2,
        ONETAP = 0x3
    };

    /**
     * @namespace csa
     * @brief Contains all classes and structures related to the Common Service Area.
     * @details The Common Service Area (CSA) is a standardized 96-byte data block on a transit card
     *          that holds shared information accessible by any compliant terminal, regardless of the
     *          transit operator. This namespace provides a complete object-oriented model of the CSA,
     *          breaking it down into its logical components: `general`, `terminal`, `validation`,
     *          `log`, `history`, and a final `container` class to orchestrate them all.
     */
    namespace csa {

        /**
         * @class general
         * @brief Represents the first 2 bytes of a card's Common Service Area (CSA).
         *
         * @details This class provides a complete, type-safe interface for managing the general-purpose
         *          data block of a transit card. It acts as a high-level abstraction, hiding the underlying
         *          bit-level complexity from the user. It uses a default constructor and public setters for object
         *          creation and modification. All setter methods include strict validation to ensure that
         *          only valid data conforming to the standard can be assigned.
         *
         *          The 2-byte data is structured as follows:
         *          - **Byte 0**: [Major Version (3 bits)][Minor Version (3 bits)][Patch Version (2 bits)]
         *          - **Byte 1**: [Language Code (5 bits)][RFU (3 bits)]
         *
         * @usage
         * @code
         *     // Create a default object and populate it using setters
         *     csa::general gen;
         *     gen.set_version(1, 2, 3);
         *     gen.set_language(language_code::English);
         *     std::cout << gen;
         *
         *     // Serialize and parse
         *     std::vector<uint8_t> bytes = gen.to_bytes();
         *     csa::general parsed_gen = csa::general::parse(bytes);
         *
         *     // Verify equality
         *     if (gen == parsed_gen) {
         *         std::cout << "Objects are identical." << std::endl;
         *     }
         * @endcode
         */
        class general {
        public:

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 2;
            //! The maximum value for the major version (stored in 3 bits: 2^3 - 1).
            static constexpr uint8_t MAJOR_VERSION_MAX = 7;
            //! The maximum value for the minor version (stored in 3 bits: 2^3 - 1).
            static constexpr uint8_t MINOR_VERSION_MAX = 7;
            //! The maximum value for the patch version (stored in 2 bits: 2^2 - 1).
            static constexpr uint8_t PATCH_VERSION_MAX = 3;
            //! The maximum value for the RFU field (stored in 3 bits: 2^3 - 1).
            static constexpr uint8_t RFU_MAX = 7;

            /**
             * @brief Default constructor.
             * @details Creates a `general` object with all fields initialized to zero or their default values
             *          (e.g., version 0.0.0, language English, RFU 0). This ensures a valid default state.
             */
            general() = default;

            /**
             * @brief Sets the version of the data format.
             * @param major The major version. What to send: A value in the range [0, 7].
             * @param minor The minor version. What to send: A value in the range [0, 7].
             * @param patch The patch version. What to send: A value in the range [0, 3].
             * @throws std::out_of_range if any version component is outside its valid bit-field range.
             */
            void set_version(const uint8_t major, const uint8_t minor, const uint8_t patch) {
                // Validate that the major version fits within its allocated 3 bits.
                if (major > MAJOR_VERSION_MAX) throw std::out_of_range("Major version must be in the range [0, 7].");
                // Validate that the minor version fits within its allocated 3 bits.
                if (minor > MINOR_VERSION_MAX) throw std::out_of_range("Minor version must be in the range [0, 7].");
                // Validate that the patch version fits within its allocated 2 bits.
                if (patch > PATCH_VERSION_MAX) throw std::out_of_range("Patch version must be in the range [0, 3].");
                // If all checks pass, assign the values to the member variables.
                major_version_ = major;
                minor_version_ = minor;
                patch_version_ = patch;
            }

            /**
             * @brief Sets the card's preferred language.
             * @param code The language code. What to send: A valid `language_code` enum member.
             * @note This function is `noexcept` because all possible values of the `language_code` enum are considered valid.
             */
            void set_language(const language_code code) noexcept {
                language_ = code;
            }

            /**
             * @brief Sets the value for the Reserved for Future Use (RFU) field.
             * @param value The RFU value. What to send: A value in the range [0, 7].
             * @throws std::out_of_range if the value is outside the valid 3-bit range.
             */
            void set_rfu(const uint8_t value) {
                // Validate that the RFU value fits within its allocated 3 bits.
                if (value > RFU_MAX) throw std::out_of_range("RFU value must be in the range [0, 7].");
                rfu_ = value;
            }

            /**
             * @brief Parses a 2-byte data vector into a `general` object.
             * @param data A const reference to a `std::vector` containing exactly 2 bytes of data.
             * @return A `general` object populated with the parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 2 bytes.
             */
            static general parse(const std::vector<uint8_t>& data) {
                // Ensure the input data is the correct size before attempting to parse.
                if (data.size() != DATA_SIZE) {
                    throw std::invalid_argument("General data must be exactly 2 bytes.");
                }

                // Create a new object to hold the parsed data.
                general g;

                // --- Byte 0: Version ---
                // Example Byte 0: 001 010 11 (Binary)
                // To get Major Version (first 3 bits): Right-shift by 5 bits, then mask with 0x07 (0b111).
                // (00101011 >> 5) -> 00000001. 00000001 & 0b111 -> 1.
                g.major_version_ = (data[0] >> 5) & 0x07;
                // To get Minor Version (middle 3 bits): Right-shift by 2 bits, then mask with 0x07.
                // (00101011 >> 2) -> 00001010. 00001010 & 0b111 -> 2.
                g.minor_version_ = (data[0] >> 2) & 0x07;
                // To get Patch Version (last 2 bits): Mask with 0x03 (0b11).
                // 00101011 & 0b11 -> 3.
                g.patch_version_ = data[0] & 0x03;

                // --- Byte 1: Language and RFU ---
                // To get Language Code (first 5 bits): Right-shift by 3 bits. Masking is implicit due to the limited range of the enum.
                g.language_ = static_cast<language_code>(data[1] >> 3);
                // To get RFU (last 3 bits): Mask with 0x07 (0b111).
                g.rfu_ = data[1] & 0x07;

                return g;
            }

            /**
             * @brief Serializes the `general` object into a 2-byte vector.
             * @return A `std::vector<uint8_t>` containing the 2 bytes of serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const noexcept {
                // --- Assemble Byte 0 (Version) ---
                // Left-shift the major version by 5 to place it in the most significant 3 bits.
                // Left-shift the minor version by 2 to place it in the middle 3 bits.
                // The patch version already occupies the least significant 2 bits.
                // Combine all parts using bitwise OR.
                const auto first_byte = static_cast<uint8_t>(
                    (major_version_ << 5) | (minor_version_ << 2) | patch_version_
                );

                // --- Assemble Byte 1 (Language and RFU) ---
                // Cast the language enum to its underlying uint8_t type.
                // Left-shift the language code by 3 to place it in the most significant 5 bits.
                // The RFU value already occupies the least significant 3 bits.
                // Combine both parts using bitwise OR.
                const auto second_byte = static_cast<uint8_t>(
                    (static_cast<uint8_t>(language_) << 3) | rfu_
                );

                // Return the assembled bytes in a vector.
                return { first_byte, second_byte };
            }


            [[nodiscard]] uint8_t get_major_version() const noexcept { return major_version_; }
            [[nodiscard]] uint8_t get_minor_version() const noexcept { return minor_version_; }
            [[nodiscard]] uint8_t get_patch_version() const noexcept { return patch_version_; }
            [[nodiscard]] language_code get_language() const noexcept { return language_; }
            [[nodiscard]] uint8_t get_rfu() const noexcept { return rfu_; }

            /**
             * @brief Retrieves the full version number as a formatted string.
             * @return The version string in "major.minor.patch" format (e.g., "1.2.3").
             */
            [[nodiscard]] std::string get_version() const {
                return std::to_string(major_version_) + "." + std::to_string(minor_version_) + "." + std::to_string(patch_version_);
            }

            /**
             * @brief Retrieves the language as a human-readable string.
             * @return The name of the language (e.g., "English"). Returns "Unknown" for undefined or RFU codes.
             */
            [[nodiscard]] std::string get_language_string() const {
                switch (language_) {
                    case language_code::English: return "English";
                    case language_code::Hindi:   return "Hindi";
                    case language_code::Marathi: return "Marathi";
                    // Note: This is not an exhaustive list of all languages in the enum.
                    // The default case is crucial for handling all other defined and RFU language codes gracefully.
                    default: return "Unknown";
                }
            }

            /**
             * @brief Retrieves the full version number as a formatted string.
             * @return The version string in "major.minor.patch" format (e.g., "1.2.3").
             */
            [[nodiscard]] std::string get_version_string() const {
                return std::to_string(major_version_) + "." + std::to_string(minor_version_) + "." + std::to_string(patch_version_);
            }

            /**
             * @brief Stream insertion operator for easy printing of `general` objects.
             * @param os The output stream.
             * @param obj The `general` object to print.
             * @return A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& os, const general& obj) {
                return os << "------------------------ GENERAL DATA ------------------------" << std::endl
                          << "  VERSION                  : " << obj.get_version() << std::endl
                          // Also print the binary representation of the language code for debugging purposes.
                          << "  LANGUAGE                 : " << obj.get_language_string() << " (0b"
                          << std::bitset<5>(static_cast<uint8_t>(obj.get_language())) << ")" << std::endl
                          << "  RFU                      : " << static_cast<int>(obj.get_rfu()) << std::endl
                          << "------------------------------------------------------------";
            }

            /**
             * @brief Compares two `general` objects for equality.
             * @return True if all corresponding members are identical, false otherwise.
             */
            friend bool operator==(const general& lhs, const general& rhs) {
                // This performs a deep comparison of all data members.
                return lhs.major_version_ == rhs.major_version_ &&
                       lhs.minor_version_ == rhs.minor_version_ &&
                       lhs.patch_version_ == rhs.patch_version_ &&
                       lhs.language_ == rhs.language_ &&
                       lhs.rfu_ == rhs.rfu_;
            }

        private:

            //! The major version number, stored in the 3 most significant bits of the first byte.
            uint8_t major_version_{ 0 };
            //! The minor version number, stored in the 3 middle bits of the first byte.
            uint8_t minor_version_{ 0 };
            //! The patch version number, stored in the 2 least significant bits of the first byte.
            uint8_t patch_version_{ 0 };
            //! The preferred language, stored as a 5-bit enum in the second byte.
            language_code language_{ language_code::English };
            //! Reserved for Future Use field, stored in the 3 least significant bits of the second byte.
            uint8_t rfu_{ 0 };

        };

        // ------------------------------------------------------ TERMINAL DATA -------------------------------------------------------

        /**
         * @class terminal
         * @brief Represents 6 bytes of terminal identification data.
         *
         * @details This class provides a structured interface for handling terminal-specific information,
         *          such as the IDs of the acquirer, operator, and the physical terminal itself. It is a
         *          foundational, reusable data structure used within larger transaction records like
         *          `csa::validation` and `csa::log`.
         *
         *          The 6-byte data is structured in big-endian format (most significant byte first):
         *          - **Byte 0**: Acquirer ID (1 byte)
         *          - **Bytes 1-2**: Operator ID (2 bytes)
         *          - **Bytes 3-5**: Terminal ID (3 bytes)
         *
         * @usage
         * @code
         *     // Create a default object and populate it using setters
         *     csa::terminal term;
         *     term.set_acquirer_id(15);
         *     term.set_operator_id(1025); // 0x0401
         *     term.set_terminal_id("A1B2C3");
         *     std::cout << term;
         *
         *     // Serialize and parse
         *     std::vector<uint8_t> bytes = term.to_bytes(); // {0x0F, 0x04, 0x01, 0xA1, 0xB2, 0xC3}
         *     csa::terminal parsed_term = csa::terminal::parse(bytes);
         *
         *     // Verify equality
         *     if (term == parsed_term) {
         *         std::cout << "Terminal objects are identical." << std::endl;
         *     }
         * @endcode
         */
        class terminal {
        public:

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 6;
            //! The required length of a terminal ID hex string (2 chars per byte for 3 bytes).
            static constexpr size_t TERMINAL_ID_HEX_LENGTH = 6;
            //! The maximum value for the terminal ID (stored in 24 bits: 2^24 - 1).
            static constexpr uint32_t TERMINAL_ID_MAX = 0xFFFFFF;

            /**
             * @brief Default constructor.
             * @details Creates a `terminal` object with all ID fields initialized to zero.
             *          Using `= default` instructs the compiler to generate a simple, efficient constructor.
             */
            terminal() = default;

            /**
             * @brief Sets the Acquirer ID.
             * @param id The acquirer's unique identifier. What to send: A value in the range [0, 255].
             * @note This function is `noexcept` because any `uint8_t` value is valid.
             */
            void set_acquirer_id(const uint8_t id) noexcept {
                acquirer_id_ = id;
            }

            /**
             * @brief Sets the Operator ID.
             * @param id The operator's unique identifier. What to send: A value in the range [0, 65535].
             * @note This function is `noexcept` because any `uint16_t` value is valid.
             */
            void set_operator_id(const uint16_t id) noexcept {
                operator_id_ = id;
            }

            /**
             * @brief Sets the Terminal ID from a hexadecimal string.
             * @param hex_id The terminal's unique identifier as a hex string.
             *               What to send: A 6-character, case-insensitive string containing only
             *               hexadecimal characters (0-9, A-F). Example: "1122AA".
             * @throws std::invalid_argument if `hex_id` is not exactly 6 characters long.
             * @throws std::out_of_range if `hex_id` contains invalid characters or represents a value > 0xFFFFFF.
             */
            void set_terminal_id(const std::string& hex_id) {
                // First, perform a quick and inexpensive check on the string length.
                if (hex_id.size() != TERMINAL_ID_HEX_LENGTH)
                    throw std::invalid_argument("Terminal ID hex string must be exactly 6 characters.");

                // Use a stringstream for a robust conversion from hex string to an integer.
                unsigned long long value = 0;
                std::stringstream ss;
                ss << std::hex << hex_id; // Tell the stream to interpret the input as hexadecimal.

                // Try to extract the value and perform comprehensive validation.
                // 1. `!(ss >> value)`: Fails if the string contains non-hex characters (e.g., "A1B2G3").
                // 2. `ss.rdbuf()->in_avail() != 0`: Fails if there are leftover characters after a valid parse,
                //    which handles cases where the stream stops parsing early.
                // 3. `value > TERMINAL_ID_MAX`: Fails if the parsed number exceeds the 24-bit limit.
                if (!(ss >> value) || ss.rdbuf()->in_avail() != 0 || value > TERMINAL_ID_MAX)
                    throw std::out_of_range("Terminal ID string is invalid or its value is out of the 24-bit range.");
                // If all checks pass, safely cast and assign the value.
                terminal_id_ = static_cast<uint32_t>(value);
            }

            /**
             * @brief Parses a 6-byte data vector into a `terminal` object.
             * @param data A const reference to a `std::vector` containing exactly 6 bytes of data.
             * @return A `terminal` object populated with the parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 6 bytes.
             */
            static terminal parse(const std::vector<uint8_t>& data) {
                if (data.size() != DATA_SIZE) {
                    throw std::invalid_argument("Terminal data must be 6 bytes.");
                }

                terminal t;

                // Byte 0: Acquirer ID is a single byte, so a direct copy is enough.
                t.acquirer_id_ = data[0];

                // Bytes 1-2: Operator ID (Big-Endian)
                // Shift the first byte (MSB) left by 8 bits to make room for the second byte.
                // Then, bitwise OR it with the second byte (LSB) to combine them into a uint16_t.
                // Example: {0x04, 0x01} -> (0x04 << 8) | 0x01 -> 0x0400 | 0x01 -> 0x0401 (1025).
                t.operator_id_ = (static_cast<uint16_t>(data[1]) << 8) | data[2];

                // Bytes 3-5: Terminal ID (Big-Endian)
                // Similar logic, but with three bytes. Shift the first byte by 16, the second by 8,
                // and combine all three with the last byte using bitwise OR.
                // Example: {0xA1, 0xB2, 0xC3} -> (0xA1 << 16) | (0xB2 << 8) | 0xC3 -> 0xA1B2C3.
                t.terminal_id_ = (static_cast<uint32_t>(data[3]) << 16) | (static_cast<uint32_t>(data[4]) << 8) | data[5];

                return t;
            }

            /**
             * @brief Serializes the `terminal` object into a 6-byte vector.
             * @return A `std::vector<uint8_t>` containing the 6 bytes of serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const noexcept {
                return {
                    // Byte 0: Acquirer ID is already a single byte.
                    acquirer_id_,

                    // Bytes 1-2: Operator ID (Big-Endian)
                    // Get the MSB by shifting right by 8 bits.
                    // Get the LSB by casting to uint8_t (which implicitly truncates the upper bits).
                    static_cast<uint8_t>(operator_id_ >> 8),
                    static_cast<uint8_t>(operator_id_),

                    // Bytes 3-5: Terminal ID (Big-Endian)
                    // Get the MSB by shifting right by 16.
                    // Get the middle byte by shifting right by 8.
                    // Get the LSB by casting.
                    static_cast<uint8_t>(terminal_id_ >> 16),
                    static_cast<uint8_t>(terminal_id_ >> 8),
                    static_cast<uint8_t>(terminal_id_)
                };
            }

            [[nodiscard]] uint8_t get_acquirer_id() const noexcept { return acquirer_id_; }
            [[nodiscard]] uint16_t get_operator_id() const noexcept { return operator_id_; }

            /**
             * @brief Retrieves the Terminal ID as a zero-padded, uppercase hexadecimal string.
             * @return A 6-character string representing the terminal ID (e.g., "A1B2C3").
             */
            [[nodiscard]] std::string get_terminal_id() const {
                std::stringstream ss;
                // Configure the stream for hex output:
                // - std::uppercase: Use "A-F" instead of "a-f".
                // - std::hex: Use base 16.
                // - std::setw(6): Set the total width to 6 characters.
                // - std::setfill('0'): Pad with leading zeros if the number is smaller than 6 hex digits.
                ss << std::uppercase << std::hex << std::setw(TERMINAL_ID_HEX_LENGTH) << std::setfill('0') << terminal_id_;
                return ss.str();
            }

            /**
             * @brief Stream insertion operator for easy printing of `terminal` objects.
             * @param os The output stream.
             * @param obj The `terminal` object to print.
             * @return A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& os, const terminal& obj) {
                return os << "  [TN] ACQUIRER ID       : " << +obj.get_acquirer_id() << std::endl // Unary `+` promotes uint8_t to int for printing as a number.
                          << "  [TN] OPERATOR ID       : " << obj.get_operator_id() << std::endl
                          << "  [TN] TERMINAL ID       : " << obj.get_terminal_id();
            }

            /**
             * @brief Compares two `terminal` objects for equality.
             * @return True if all corresponding members are identical, false otherwise.
             */
            friend bool operator==(const terminal& lhs, const terminal& rhs) {
                return lhs.acquirer_id_ == rhs.acquirer_id_ &&
                       lhs.operator_id_ == rhs.operator_id_ &&
                       lhs.terminal_id_ == rhs.terminal_id_;
            }

        private:

            //! The ID of the acquirer, stored as a 1-byte integer.
            uint8_t acquirer_id_{ 0 };
            //! The ID of the transit operator, stored as a 2-byte integer.
            uint16_t operator_id_{ 0 };
            //! The ID of the physical terminal, stored as a 3-byte (24-bit) integer.
            //! A uint32_t is used for convenience, but the value is always constrained to 0xFFFFFF.
            uint32_t terminal_id_{ 0 };

        };
        // ------------------------------------------------------ VALIDATION DATA -------------------------------------------------------

        /**
         * @class validation
         * @brief Represents the 19-byte validation data block with all functionality fully encapsulated.
         *
         * @details This class holds information about the last validation event (e.g., a tap at a transit gate).
         *          It is designed to be instantiated and then configured using individual setter methods,
         *          each of which performs validation to ensure data integrity.
         *
         *          The 19-byte data is structured in big-endian format and is composed of several fields,
         *          including a nested `terminal` object.
         *
         * @warning The transaction time is stored as an offset from a `card_effective_date`. Therefore, you
         *          **must** call `set_card_effective_date()` before you can call `set_date_and_time()`.
         *
         * @usage
         * @code
         *     csa::validation val;
         *
         *     // CRITICAL: Set the effective date first! This is the base for all time calculations.
         *     time_t effective_date_in_minutes = 28399680; // Example date
         *     val.set_card_effective_date(effective_date_in_minutes);
         *
         *     // Now, set the absolute transaction time. The class handles the conversion to an offset.
         *     uint64_t current_time_ms = 1704067200000ULL; // Jan 1, 2024
         *     val.set_date_and_time(current_time_ms);
         *
         *     // Set other fields...
         *     val.set_fare_amount(250);
         *     val.set_txn_status(txn_status::ENTRY);
         *
         *     // Serialize to bytes to write to a card
         *     std::vector<uint8_t> bytes = val.to_bytes();
         *
         *     // Parse bytes read from a card
         *     csa::validation parsed_val = csa::validation::parse(bytes, effective_date_in_minutes);
         * @endcode
         */
        class validation {
        public:

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 19;
            //! The maximum value for the time offset (stored in 24 bits: 2^24 - 1).
            static constexpr uint32_t TIME_OFFSET_MAX = 0xFFFFFF;
            //! The maximum value for the service provider data (stored in 24 bits: 2^24 - 1).
            static constexpr uint32_t SERVICE_DATA_MAX = 0xFFFFFF;
            //! The maximum value for the RFU field (stored in 4 bits: 2^4 - 1).
            static constexpr uint8_t RFU_MAX = 0x0F;

            /**
             * @brief Default constructor. Creates a `validation` object with all fields initialized to zero or their defaults.
             */
            validation() = default;

            /**
             * @brief Sets the card's effective date, which is the base for time calculations.
             * @param date_in_minutes The card's effective date. What to send: A `std::time_t` value representing
             *                        the number of **minutes** since the Unix epoch.
             */
            void set_card_effective_date(std::time_t date_in_minutes) noexcept {
                card_effective_date_in_minutes_ = date_in_minutes;
            }

            /**
             * @brief Sets the absolute transaction time from a millisecond timestamp.
             * @param absolute_time_in_milliseconds The absolute time of the transaction. What to send: A `uint64_t`
             *                                        value representing milliseconds since the Unix epoch.
             * @throws std::logic_error if `set_card_effective_date()` has not been called first.
             * @throws std::out_of_range if the calculated time difference is negative or exceeds the 24-bit storage limit.
             */
            void set_date_and_time(const uint64_t absolute_time_in_milliseconds) {
                // This function's primary job is to convert an absolute timestamp into a relative
                // offset in minutes, which is how the data is stored on the card.

                // First, ensure the base date for the calculation has been set.
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date must be set before setting transaction time.");

                // Convert the absolute time from milliseconds to minutes. Use uint64_t for the intermediate
                // value to prevent overflow and maintain precision.
                const uint64_t absolute_time_in_minutes = absolute_time_in_milliseconds / 60000;
                const uint64_t effective_date_minutes = *card_effective_date_in_minutes_;

                // The transaction time must be on or after the effective date.
                if (absolute_time_in_minutes < effective_date_minutes)
                    throw std::out_of_range("Transaction time cannot be before the card effective date.");

                // Calculate the difference in minutes. This difference will be stored.
                const uint64_t time_diff = absolute_time_in_minutes - effective_date_minutes;

                // Ensure the calculated offset fits within the 24 bits allocated for it.
                if (time_diff > TIME_OFFSET_MAX)
                    throw std::out_of_range("Transaction time is out of the valid 24-bit range from effective date.");

                // Safely store the valid offset.
                date_and_time_offset_ = static_cast<uint32_t>(time_diff);
            }

            /**
             * @brief Sets the service provider specific data.
             * @param data A numeric value for use by the service provider. What to send: A value in the range [0, 0xFFFFFF].
             * @throws std::out_of_range if the data value exceeds the 24-bit limit.
             */
            void set_service_provider_data(const uint32_t data) {
                if (data > SERVICE_DATA_MAX) throw std::out_of_range("Service provider data exceeds 24-bit limit.");
                service_provider_data_ = data;
            }

            /**
             * @brief Sets the value for the Reserved for Future Use (RFU) field.
             * @param value The RFU value. What to send: A value in the range [0, 15].
             * @throws std::out_of_range if the value is outside the valid 4-bit range.
             */
            void set_rfu(const uint8_t value) {
                if (value > RFU_MAX) throw std::out_of_range("RFU value must be in the range [0, 15].");
                rfu_ = value;
            }

            void set_error_code(const uint8_t code) noexcept { error_code_ = code; }
            void set_product_type(const uint8_t type) noexcept { product_type_ = type; }
            void set_terminal_info(const terminal &info) noexcept { terminal_info_ = info; }
            void set_fare_amount(const uint16_t amount) noexcept { fare_amount_ = amount; }
            void set_route_number(const uint16_t number) noexcept { route_number_ = number; }
            void set_txn_status(const txn_status status) noexcept { status_ = status; }

            /**
             * @brief Parses a 19-byte data vector into a `validation` object.
             * @param data A const reference to a vector containing exactly 19 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch. This is
             *        **required** to correctly interpret the time offset later.
             * @return A `validation` object populated with parsed data.
             * @throws std::invalid_argument if the data vector is not 19 bytes.
             */
            static validation parse(const std::vector<uint8_t>& data, std::time_t card_effective_date_in_minutes) {
                if (data.size() != DATA_SIZE) throw std::invalid_argument("Validation data must be exactly 19 bytes.");

                validation v;
                // Store the provided effective date, as it's necessary to calculate the absolute time later.
                v.card_effective_date_in_minutes_ = card_effective_date_in_minutes;

                // Bytes 0-1: Single-byte fields, direct copy.
                v.error_code_ = data[0];
                v.product_type_ = data[1];

                // Bytes 2-7: Terminal Info (6 bytes).
                // Delegate parsing to the `terminal` class by passing it the relevant slice of the data vector.
                v.terminal_info_ = terminal::parse({data.begin() + 2, data.begin() + 8});

                // Bytes 8-10: Date and Time Offset (24-bit, Big-Endian).
                // Reconstruct the 24-bit integer from three bytes using bitwise shifts and ORs.
                v.date_and_time_offset_ = (static_cast<uint32_t>(data[8]) << 16) | (static_cast<uint32_t>(data[9]) << 8) | data[10];

                // Bytes 11-12: Fare Amount (16-bit, Big-Endian).
                v.fare_amount_ = (static_cast<uint16_t>(data[11]) << 8) | data[12];

                // Bytes 13-14: Route Number (16-bit, Big-Endian).
                v.route_number_ = (static_cast<uint16_t>(data[13]) << 8) | data[14];

                // Bytes 15-17: Service Provider Data (24-bit, Big-Endian).
                v.service_provider_data_ = (static_cast<uint32_t>(data[15]) << 16) | (static_cast<uint32_t>(data[16]) << 8) | data[17];

                // Byte 18: Transaction Status (upper 4 bits) and RFU (lower 4 bits).
                // Get status by shifting right by 4 bits.
                v.status_ = static_cast<txn_status>(data[18] >> 4);
                // Get RFU by masking with 0x0F (binary 00001111).
                v.rfu_ = data[18] & RFU_MAX;

                return v;
            }

            /**
             * @brief Serializes the `validation` object into a 19-byte vector.
             * @return A `std::vector<uint8_t>` containing the serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(DATA_SIZE); // Pre-allocate memory to avoid reallocations.

                // Bytes 0-1: Single-byte fields.
                data.push_back(error_code_);
                data.push_back(product_type_);

                // Bytes 2-7: Terminal Info.
                // Delegate serialization to the `terminal` class and insert its bytes.
                auto terminal_bytes = terminal_info_.to_bytes();
                data.insert(data.end(), terminal_bytes.begin(), terminal_bytes.end());

                // Bytes 8-10: Date and Time Offset (24-bit, Big-Endian).
                // Deconstruct the 24-bit integer into three separate bytes.
                data.push_back((date_and_time_offset_ >> 16) & 0xFF); // Most significant byte
                data.push_back((date_and_time_offset_ >> 8) & 0xFF);  // Middle byte
                data.push_back(date_and_time_offset_ & 0xFF);         // Least significant byte

                // Bytes 11-12: Fare Amount (16-bit, Big-Endian).
                data.push_back((fare_amount_ >> 8) & 0xFF);
                data.push_back(fare_amount_ & 0xFF);

                // Bytes 13-14: Route Number (16-bit, Big-Endian).
                data.push_back((route_number_ >> 8) & 0xFF);
                data.push_back(route_number_ & 0xFF);

                // Bytes 15-17: Service Provider Data (24-bit, Big-Endian).
                data.push_back((service_provider_data_ >> 16) & 0xFF);
                data.push_back((service_provider_data_ >> 8) & 0xFF);
                data.push_back(service_provider_data_ & 0xFF);

                // Byte 18: Transaction Status and RFU.
                // Shift status into the upper 4 bits and combine with the 4-bit RFU value.
                data.push_back((static_cast<uint8_t>(status_) << 4) | rfu_);

                return data;
            }

            /**
             * @brief Calculates and returns the absolute transaction date and time in milliseconds.
             * @return The transaction time as a `uint64_t` value (milliseconds since Unix epoch).
             * @throws std::logic_error if the card's effective date was not set.
             */
            [[nodiscard]] uint64_t get_date_and_time() const {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date is not set; cannot calculate absolute time.");
                // Reconstruct the absolute time by adding the stored offset to the base effective date.
                const uint64_t total_minutes = static_cast<uint64_t>(*card_effective_date_in_minutes_) + date_and_time_offset_;
                // Convert the total minutes back to milliseconds for the absolute timestamp.
                return total_minutes * 60000;
            }

            /**
             * @brief Gets the card effective date associated with this validation record.
             * @return The effective date in minutes since the Unix epoch.
             * @throws std::logic_error if the effective date has not been set.
             */
            [[nodiscard]] std::time_t get_card_effective_date() const {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date has not been set for this record.");
                return *card_effective_date_in_minutes_;
            }

            [[nodiscard]] uint8_t get_error_code() const noexcept { return error_code_; }
            [[nodiscard]] uint8_t get_product_type() const noexcept { return product_type_; }
            [[nodiscard]] const terminal& get_terminal_info() const noexcept { return terminal_info_; }
            [[nodiscard]] uint16_t get_fare_amount() const noexcept { return fare_amount_; }
            [[nodiscard]] uint16_t get_route_number() const noexcept { return route_number_; }
            [[nodiscard]] txn_status get_txn_status() const noexcept { return status_; }

            /**
             * @brief Retrieves the service provider data as a zero-padded, uppercase hex string.
             * @return A 6-character string representing the data (e.g., "1A2B3C").
             */
            [[nodiscard]] std::string get_service_provider_data() const {
                std::stringstream ss;
                ss << std::uppercase << std::hex << std::setw(6) << std::setfill('0') << service_provider_data_;
                return ss.str();
            }

            /**
             * @brief Retrieves the transaction status as a human-readable string.
             * @return A `std::string` like "ENTRY", "EXIT", etc., or "UNKNOWN" for invalid values.
             */
            [[nodiscard]] std::string get_txn_status_string() const {
                switch (status_) {
                    case txn_status::ENTRY:   return "ENTRY";
                    case txn_status::EXIT:    return "EXIT";
                    case txn_status::ONETAP:  return "ONETAP";
                    case txn_status::PENALTY: return "PENALTY";
                    default:                  return "UNKNOWN";
                }
            }

            /**
             * @brief Retrieves the RFU value as a 4-character binary string.
             * @return A `std::string` like "1101". Useful for debugging.
             */
            [[nodiscard]] std::string get_rfu() const {
                return std::bitset<4>(rfu_).to_string();
            }

            /**
             * @brief Stream insertion operator for easy printing of `validation` objects.
             * @param os The output stream.
             * @param obj The `validation` object to print.
             * @return A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& os, const validation& obj) {
                os << "-------------------------- VALIDATION DATA -------------------------" << std::endl;
                os << obj.get_terminal_info() << std::endl;
                os << "  ERROR CODE             : " << static_cast<int>(obj.get_error_code()) << std::endl;
                os << "  PRODUCT TYPE           : " << static_cast<int>(obj.get_product_type()) << std::endl;
                try {
                    // Convert the millisecond timestamp from get_date_and_time() to seconds
                    // before passing it to std::gmtime, which expects seconds.
                    const std::time_t t_sec = obj.get_date_and_time() / 1000;
                    char time_str[100];
                    // Format the time as a human-readable UTC string.
                    if (std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::gmtime(&t_sec))) {
                        os << "  DATE AND TIME          : " << time_str << " (UTC)" << std::endl;
                    }
                } catch (const std::logic_error& e) {
                    os << "  DATE AND TIME          : " << "[Not available: " << e.what() << "]" << std::endl;
                }
                os << "  FARE AMOUNT            : " << obj.get_fare_amount() << std::endl;
                os << "  ROUTE NUMBER           : " << obj.get_route_number() << std::endl;
                os << "  SERVICE PROVIDER DATA  : 0x" << obj.get_service_provider_data() << std::endl;
                os << "  TRANSACTION STATUS     : " << obj.get_txn_status_string() << std::endl;
                os << "  RFU (BINARY)           : " << obj.get_rfu() << std::endl;
                os << "--------------------------------------------------------------------";
                return os;
            }

            /**
             * @brief Compares two `validation` objects for equality.
             * @return True if all corresponding members are identical, false otherwise.
             */
            friend bool operator==(const validation& lhs, const validation& rhs) {
                return lhs.error_code_ == rhs.error_code_ &&
                       lhs.product_type_ == rhs.product_type_ &&
                       lhs.terminal_info_ == rhs.terminal_info_ &&
                       lhs.date_and_time_offset_ == rhs.date_and_time_offset_ &&
                       lhs.fare_amount_ == rhs.fare_amount_ &&
                       lhs.route_number_ == rhs.route_number_ &&
                       lhs.service_provider_data_ == rhs.service_provider_data_ &&
                       lhs.status_ == rhs.status_ &&
                       lhs.rfu_ == rhs.rfu_ &&
                       lhs.card_effective_date_in_minutes_ == rhs.card_effective_date_in_minutes_;
            }

        private:

            uint8_t error_code_{ 0 };
            uint8_t product_type_{ 0 };
            //! A nested object containing terminal identification data.
            terminal terminal_info_{};
            //! The transaction time, stored as a 24-bit offset in minutes from the card's effective date.
            uint32_t date_and_time_offset_{ 0 };
            uint16_t fare_amount_{ 0 };
            uint16_t route_number_{ 0 };
            //! A 24-bit field for operator-specific data.
            uint32_t service_provider_data_{ 0 };
            //! The status of the transaction (e.g., ENTRY, EXIT), stored in 4 bits.
            txn_status status_{ txn_status::ENTRY };
            //! A 4-bit field Reserved for Future Use.
            uint8_t rfu_{ 0 };
            //! The base date for time calculations, stored in minutes since epoch. This is not part of the
            //! serialized data but is essential for interpreting the `date_and_time_offset_`.
            std::optional<std::time_t> card_effective_date_in_minutes_;

        };

        // ------------------------------------------------------ LOG DATA -------------------------------------------------------

        /**
         * @class log
         * @brief Represents a single 17-byte transaction log entry with all functionality fully encapsulated.
         *
         * @details This class encapsulates all data for one historical transaction, such as the amount,
         *          balance, terminal, and time. It is a key component of the `csa::history` class. The most
         *          complex feature is the packing of a 20-bit card balance into three bytes.
         *
         *          The 17-byte data layout includes (all multi-byte values are big-endian):
         *          - **Bytes 0-5**: Terminal Info (6 bytes)
         *          - **Bytes 6-8**: Date and Time Offset (24-bit)
         *          - **Bytes 9-10**: Transaction Amount (16-bit)
         *          - **Bytes 11-12**: Transaction Sequence Number (16-bit)
         *          - **Bytes 13-15**: Card Balance (20-bit, spanning 3 bytes)
         *          - **Byte 16**: Transaction Status (4-bit, upper nibble) and RFU (4-bit, lower nibble)
         *
         * @warning The transaction time is stored as an offset from a `card_effective_date`. Therefore, you
         *          **must** call `set_card_effective_date()` before you can call `set_date_and_time()`.
         */
        class log {
        public:
            // --- Public Constants ---

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 17;
            //! The maximum value for the time offset (stored in 24 bits: 2^24 - 1).
            static constexpr uint32_t TIME_OFFSET_MAX = 0xFFFFFF;
            //! The maximum value for the card balance (stored in 20 bits: 2^20 - 1).
            static constexpr uint32_t CARD_BALANCE_MAX = 0xFFFFF;
            //! The maximum value for the RFU field (stored in 4 bits: 2^4 - 1).
            static constexpr uint8_t RFU_MAX = 0x0F;

            /**
             * @brief Default constructor. Creates a `log` object with all fields initialized to zero or their defaults.
             */
            log() = default;

            // --- Setters ---

            /**
             * @brief Sets the card's effective date, which is the base for all time calculations.
             * @param date_in_minutes The card's effective date. What to send: A `std::time_t` value representing
             *                        the number of **minutes** since the Unix epoch.
             */
            void set_card_effective_date(std::time_t date_in_minutes) noexcept {
                card_effective_date_in_minutes_ = date_in_minutes;
            }

            /**
             * @brief Sets the absolute transaction time from a millisecond timestamp.
             * @param absolute_time_in_milliseconds The absolute time of the transaction. What to send: A `uint64_t`
             *                                        value representing milliseconds since the Unix epoch.
             * @throws std::logic_error if `set_card_effective_date()` has not been called first.
             * @throws std::out_of_range if the calculated time difference is negative or exceeds the 24-bit storage limit.
             */
            void set_date_and_time(const uint64_t absolute_time_in_milliseconds) {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date must be set before setting transaction time.");
                const uint64_t absolute_time_in_minutes = absolute_time_in_milliseconds / 60000;
                const uint64_t effective_date_minutes = *card_effective_date_in_minutes_;
                if (absolute_time_in_minutes < effective_date_minutes)
                    throw std::out_of_range("Transaction time cannot be before the card effective date.");
                const uint64_t time_diff = absolute_time_in_minutes - effective_date_minutes;
                if (time_diff > TIME_OFFSET_MAX)
                    throw std::out_of_range("Transaction time is out of the valid 24-bit range from effective date.");
                date_and_time_offset_ = static_cast<uint32_t>(time_diff);
            }

            /**
             * @brief Sets the card balance at the time of the transaction.
             * @param balance The card balance. What to send: A value in the range [0, 1048575] (0xFFFFF).
             * @throws std::out_of_range if the balance exceeds the 20-bit limit.
             */
            void set_card_balance(const uint32_t balance) {
                if (balance > CARD_BALANCE_MAX) throw std::out_of_range("Card balance exceeds 20-bit limit.");
                card_balance_ = balance;
            }

            /**
             * @brief Sets the value for the Reserved for Future Use (RFU) field.
             * @param value The RFU value. What to send: A value in the range [0, 15].
             * @throws std::out_of_range if the value is outside the valid 4-bit range.
             */
            void set_rfu(const uint8_t value) {
                if (value > RFU_MAX) throw std::out_of_range("RFU value must be in the range [0, 15].");
                rfu_ = value;
            }

            void set_terminal_info(const terminal& info) noexcept { terminal_info_ = info; }
            void set_txn_amount(const uint16_t amount) noexcept { txn_amount_ = amount; }
            void set_txn_sq_no(const uint16_t sq_no) noexcept { txn_sq_no_ = sq_no; }
            void set_txn_status(const txn_status status) noexcept { status_ = status; }

            // --- Serialization / Deserialization ---

            /**
             * @brief Parses a 17-byte data vector into a `log` object.
             * @param data A const reference to a vector containing exactly 17 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch.
             * @return A `log` object populated with parsed data.
             * @throws std::invalid_argument if the data vector is not 17 bytes.
             */
            static log parse(const std::vector<uint8_t>& data, std::time_t card_effective_date_in_minutes) {
                if (data.size() != DATA_SIZE) throw std::invalid_argument("Log data must be exactly 17 bytes.");
                log l;
                l.card_effective_date_in_minutes_ = card_effective_date_in_minutes;
                l.terminal_info_ = terminal::parse({data.begin(), data.begin() + 6});
                l.date_and_time_offset_ = (static_cast<uint32_t>(data[6]) << 16) | (static_cast<uint32_t>(data[7]) << 8) | data[8];
                l.txn_amount_ = (static_cast<uint16_t>(data[9]) << 8) | data[10];
                l.txn_sq_no_ = (static_cast<uint16_t>(data[11]) << 8) | data[12];
                l.card_balance_ = (static_cast<uint32_t>(data[13]) << 12) | (static_cast<uint32_t>(data[14]) << 4) | (data[15] >> 4);
                l.status_ = static_cast<txn_status>((data[16] >> 4) & 0x0F);
                l.rfu_ = data[16] & 0x0F;
                return l;
            }

            /**
             * @brief Serializes the `log` object into a 17-byte vector.
             * @return A `std::vector<uint8_t>` containing the serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(DATA_SIZE);

                // Bytes 0-5: Terminal Info.
                auto terminal_bytes = terminal_info_.to_bytes();
                data.insert(data.end(), terminal_bytes.begin(), terminal_bytes.end());

                // Bytes 6-8: Date and Time Offset (24-bit).
                data.push_back((date_and_time_offset_ >> 16) & 0xFF);
                data.push_back((date_and_time_offset_ >> 8) & 0xFF);
                data.push_back(date_and_time_offset_ & 0xFF);

                // Bytes 9-10: Transaction Amount (16-bit).
                data.push_back((txn_amount_ >> 8) & 0xFF);
                data.push_back(txn_amount_ & 0xFF);

                // Bytes 11-12: Transaction Sequence Number (16-bit).
                data.push_back((txn_sq_no_ >> 8) & 0xFF);
                data.push_back(txn_sq_no_ & 0xFF);

                // Bytes 13-15: Card Balance (20-bit).
                // Byte 13: The most significant 8 bits of the 20-bit balance.
                data.push_back((card_balance_ >> 12) & 0xFF);
                // Byte 14: The middle 8 bits of the 20-bit balance.
                data.push_back((card_balance_ >> 4) & 0xFF);
                // Byte 15: The least significant 4 bits of the balance, placed into the upper
                //          4 bits of this byte. The lower 4 unused bits are set to 1s (0x0F)
                //          to match the data specification.
                data.push_back(((card_balance_ & 0x0F) << 4) | 0x0F);

                // Byte 16: Transaction Status and RFU.
                data.push_back((static_cast<uint8_t>(status_) << 4) | rfu_);

                return data;
            }

            // --- Getters ---

            [[nodiscard]] uint64_t get_date_and_time() const {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date is not set; cannot calculate absolute time.");
                const uint64_t total_minutes = static_cast<uint64_t>(*card_effective_date_in_minutes_) + date_and_time_offset_;
                return total_minutes * 60000;
            }

            [[nodiscard]] std::time_t get_card_effective_date() const {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date has not been set for this log.");
                return *card_effective_date_in_minutes_;
            }

            [[nodiscard]] const terminal& get_terminal_info() const noexcept { return terminal_info_; }
            [[nodiscard]] uint16_t get_txn_amount() const noexcept { return txn_amount_; }
            [[nodiscard]] uint16_t get_txn_sq_no() const noexcept { return txn_sq_no_; }
            [[nodiscard]] uint32_t get_card_balance() const noexcept { return card_balance_; }
            [[nodiscard]] txn_status get_txn_status() const noexcept { return status_; }
            [[nodiscard]] std::string get_rfu() const noexcept { return std::bitset<4>(rfu_).to_string(); }

            [[nodiscard]] std::string get_txn_status_string() const {
                switch (status_) {
                    case txn_status::ENTRY:   return "ENTRY";
                    case txn_status::EXIT:    return "EXIT";
                    case txn_status::ONETAP:  return "ONETAP";
                    case txn_status::PENALTY: return "PENALTY";
                    default:                  return "UNKNOWN";
                }
            }

            // --- Operator Overloads ---

            friend std::ostream& operator<<(std::ostream& os, const log& obj) {
                os << "--------------------------- LOG ENTRY ----------------------------" << std::endl;
                os << obj.get_terminal_info() << std::endl;
                try {
                    const uint64_t time_ms = obj.get_date_and_time();
                    const std::time_t time_sec = time_ms / 1000;
                    char time_str[100];
                    if (std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::gmtime(&time_sec))) {
                        os << "  DATE AND TIME          : " << time_str << " (UTC)" << std::endl;
                    }
                } catch (const std::logic_error& e) {
                    os << "  DATE AND TIME          : " << "[Not available: " << e.what() << "]" << std::endl;
                }
                os << "  TRANSACTION SQ NO      : " << obj.get_txn_sq_no() << std::endl;
                os << "  TRANSACTION AMOUNT     : " << obj.get_txn_amount() << std::endl;
                os << "  CARD BALANCE           : " << obj.get_card_balance() << std::endl;
                os << "  TRANSACTION STATUS     : " << obj.get_txn_status_string() << std::endl;
                os << "  RFU (BINARY)           : " << obj.get_rfu() << std::endl;
                os << "--------------------------------------------------------------------";
                return os;
            }

            friend bool operator==(const log& lhs, const log& rhs) {
                return lhs.terminal_info_ == rhs.terminal_info_ &&
                       lhs.date_and_time_offset_ == rhs.date_and_time_offset_ &&
                       lhs.txn_amount_ == rhs.txn_amount_ &&
                       lhs.txn_sq_no_ == rhs.txn_sq_no_ &&
                       lhs.card_balance_ == rhs.card_balance_ &&
                       lhs.status_ == rhs.status_ &&
                       lhs.rfu_ == rhs.rfu_ &&
                       lhs.card_effective_date_in_minutes_ == rhs.card_effective_date_in_minutes_;
            }

        private:
            terminal terminal_info_{};
            uint32_t date_and_time_offset_{ 0 };
            uint16_t txn_amount_{ 0 };
            uint16_t txn_sq_no_{ 0 };
            uint32_t card_balance_{ 0 };
            txn_status status_{ txn_status::ENTRY };
            uint8_t rfu_{ 0 };
            std::optional<std::time_t> card_effective_date_in_minutes_;
        };

        // ------------------------------------------------------ HISTORY DATA -------------------------------------------------------

        /**
         * @class history
         * @brief Represents the 68-byte transaction history with all functionality fully encapsulated.
         *
         * @details This class manages the last four transaction `log` objects in a stateful, circular buffer
         *          fashion. When a new log is added, it is placed at the front (index 0), existing logs are
         *          shifted down, and the oldest log is discarded if the history is full. The object is
         *          intended to be instantiated once and then modified over its lifetime via its public methods.
         *
         *          The 68-byte data is structured as four consecutive 17-byte `csa::log` objects.
         *
         * @warning The history object is fundamentally tied to a `card_effective_date`. You **must** call
         *          `set_card_effective_date()` before you can add any logs via `add_log()`. This ensures
         *          that all logs within the history are consistent and can be correctly interpreted.
         *
         * @usage
         * @code
         *     // 1. Create a history object and set its master effective date.
         *     csa::history hist;
         *     time_t effective_date = 28399680;
         *     hist.set_card_effective_date(effective_date);
         *
         *     // 2. Create log objects that share the same effective date.
         *     csa::log log1, log2;
         *     log1.set_card_effective_date(effective_date);
         *     log1.set_txn_amount(100);
         *     log2.set_card_effective_date(effective_date);
         *     log2.set_txn_amount(150);
         *
         *     // 3. Add logs. log2 will be at index 0, log1 will be at index 1.
         *     hist.add_log(log1);
         *     hist.add_log(log2); // Pushes log1 down
         *
         *     // 4. Serialize and parse.
         *     std::vector<uint8_t> bytes = hist.to_bytes();
         *     csa::history parsed_hist = csa::history::parse(bytes, effective_date);
         * @endcode
         */
        class history {
        public:

            //! The maximum number of log entries that can be stored.
            static constexpr size_t LOG_COUNT = 4;
            //! The size of a single serialized `log` object in bytes.
            static constexpr size_t LOG_SIZE_BYTES = 17; // Should be `log::DATA_SIZE`
            //! The total size of the history data block in bytes.
            static constexpr size_t TOTAL_SIZE = LOG_COUNT * LOG_SIZE_BYTES; // 4 * 17 = 68 bytes

            /**
             * @brief Default constructor.
             * @details Creates an empty `history` object with no logs and an uninitialized effective date.
             */
            history() = default;

            /**
             * @brief Sets the card's effective date, which is required for all subsequent operations.
             * @param date_in_minutes The card's effective date. What to send: A `std::time_t` value representing
             *                        the number of **minutes** since the Unix epoch.
             */
            void set_card_effective_date(std::time_t date_in_minutes) noexcept {
                card_effective_date_in_minutes_ = date_in_minutes;
            }

            /**
             * @brief Adds a new transaction log to the history using circular buffer logic.
             * @details This method implements "push-down" functionality. The new log is inserted
             *          at index 0. All existing logs are shifted one position to the right (e.g., the log at
             *          index 0 moves to index 1, etc.). If the history was already full (4 logs), the last
             *          log (at index 3) is discarded before the shift.
             * @param new_log The `log` object to add. What to send: A fully populated `log` object whose
             *                own effective date has been set and matches this history's effective date.
             * @throws std::logic_error if this history's effective date has not been set.
             * @throws std::invalid_argument if the `new_log`'s effective date does not match this history's.
             */
            void add_log(const log& new_log) {

                // Precondition: The history object must have its effective date set.
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Cannot add a log until the history's effective date is set.");

                // Precondition: The incoming log must be consistent with the history's effective date.
                if (new_log.get_card_effective_date() != *card_effective_date_in_minutes_)
                    throw std::invalid_argument("Log's effective date must match history's effective date.");

                // Determine how many existing elements need to be shifted to make space at the front.
                // If the array is not full, we shift `valid_log_count_` elements.
                // If the array is full, we only shift the first `LOG_COUNT - 1` elements, discarding the last one.
                const size_t elements_to_shift = std::min(valid_log_count_, LOG_COUNT - 1);

                // Perform the shift. Loop backwards from the end to avoid overwriting data prematurely.
                // e.g., for 3 elements: logs[3]=logs[2], logs[2]=logs[1], logs[1]=logs[0].
                for (size_t i = elements_to_shift; i > 0; --i)
                    logs_[i] = logs_[i - 1];

                // Insert the new log at the first position.
                logs_[0] = new_log;

                // Increment the count of valid logs but cap it at the maximum size.
                if (valid_log_count_ < LOG_COUNT)
                    valid_log_count_++;

            }

            /**
             * @brief Clears all log entries from the history, resetting its state to empty.
             * @details The card effective date is preserved, allowing the object to be reused.
             */
            void clear() noexcept {
                valid_log_count_ = 0;
            }

            /**
             * @brief Parses a 68-byte data vector into a `history` object.
             * @param data A const reference to a `std::vector` containing exactly 68 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch.
             * @return A `history` object populated with up to 4 logs from the data.
             * @throws std::invalid_argument if the data vector is not exactly 68 bytes.
             * @note This function assumes that log slots are contiguous from the start. It stops parsing
             *       if it encounters a log slot that is entirely filled with zeros.
             */
            static history parse(const std::vector<uint8_t>& data, const std::time_t card_effective_date_in_minutes) {

                if (data.size() != TOTAL_SIZE)
                    throw std::invalid_argument("History data must be exactly 68 bytes.");

                history h;
                h.set_card_effective_date(card_effective_date_in_minutes);

                // Iterate through the four possible log slots in the byte array.
                for (size_t i = 0; i < LOG_COUNT; ++i) {
                    // Get an iterator to the start of the current 17-byte log chunk.
                    const auto begin = data.begin() + (i * LOG_SIZE_BYTES);
                    const auto end = begin + LOG_SIZE_BYTES;

                    // Heuristic check: If a 17-byte chunk is all zeros, assume it's an empty log slot
                    // and that all subsequent slots are also empty. This prevents parsing invalid data.
                    if (std::all_of(begin, end, [](const uint8_t byte){ return byte == 0; }))
                        break; // Stop parsing.

                    // If the slot is not empty, delegate parsing to the `log` class.
                    h.logs_[i] = log::parse({begin, end}, card_effective_date_in_minutes);
                    h.valid_log_count_++;
                }
                return h;
            }

            /**
             * @brief Serializes the `history` object into a 68-byte vector.
             * @details Any unused log slots will be padded with zeros to ensure the output is always 68 bytes.
             * @return A `std::vector<uint8_t>` of the serialized history data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(TOTAL_SIZE); // Pre-allocate memory for efficiency.

                // Serialize each valid log entry and append its bytes to the main vector.
                for (size_t i = 0; i < valid_log_count_; ++i) {
                    auto log_bytes = logs_[i].to_bytes();
                    data.insert(data.end(), log_bytes.begin(), log_bytes.end());
                }

                // If there are fewer than 4 logs, the remaining space must be padded with zeros
                // to ensure the final output is exactly 68 bytes.
                if (const size_t padding_size = (LOG_COUNT - valid_log_count_) * LOG_SIZE_BYTES; padding_size > 0) {
                    data.insert(data.end(), padding_size, 0x00);
                }
                return data;
            }

            // --- Getters ---

            [[nodiscard]] const std::array<log, LOG_COUNT>& get_logs() const noexcept { return logs_; }
            [[nodiscard]] size_t get_valid_log_count() const noexcept { return valid_log_count_; }

            /**
             * @brief Gets the card effective date associated with this history.
             * @return The effective date in minutes since the Unix epoch.
             * @throws std::logic_error if the effective date has not been set.
             */
            [[nodiscard]] std::time_t get_card_effective_date() const {
                if (!card_effective_date_in_minutes_.has_value()) {
                    throw std::logic_error("Card effective date has not been set.");
                }
                return *card_effective_date_in_minutes_;
            }

            // --- Operator Overloads ---

            /**
             * @brief Stream insertion operator for easy printing of `history` objects.
             * @param os The output stream.
             * @param obj The `history` object to print.
             * @return A reference to the output stream.
             */
            friend std::ostream& operator<<(std::ostream& os, const history& obj) {
                os << "=========================== HISTORY DATA ===========================" << std::endl;
                try {
                    os << "  CARD EFFECTIVE DATE (MINS): " << obj.get_card_effective_date() << std::endl;
                } catch ([[maybe_unused]] const std::logic_error& e) {
                    // `[[maybe_unused]]` prevents a compiler warning if exceptions are turned off.
                    os << "  CARD EFFECTIVE DATE (MINS): [Not Set]" << std::endl;
                }
                os << "  VALID LOG COUNT           : " << obj.get_valid_log_count() << std::endl;

                if (obj.get_valid_log_count() > 0) {
                    // Print each valid log entry. The log's own stream operator will be used.
                    for (size_t i = 0; i < obj.get_valid_log_count(); ++i) {
                        os << obj.get_logs()[i] << std::endl;
                    }
                } else {
                    os << "  [No log entries]" << std::endl;
                }
                os << "==================================================================";
                return os;
            }

            /**
             * @brief Compares two `history` objects for equality.
             * @return True if all members and all valid logs are identical, false otherwise.
             */
            friend bool operator==(const history& lhs, const history& rhs) {
                // First, compare the cheap, non-array members.
                if (lhs.card_effective_date_in_minutes_ != rhs.card_effective_date_in_minutes_ ||
                    lhs.valid_log_count_ != rhs.valid_log_count_) {
                    return false;
                }
                // If those match, perform a more expensive comparison of the actual log data.
                // `std::equal` compares the contents of the logs in the range [begin, end).
                // We only compare the number of logs that are actually valid.
                return std::equal(lhs.logs_.begin(), lhs.logs_.begin() + lhs.valid_log_count_, rhs.logs_.begin());
            }

        private:
            // --- Private Member Variables ---

            //! A fixed-size array to hold up to four log entries.
            std::array<log, LOG_COUNT> logs_{};
            //! A counter for how many slots in the `logs_` array are currently filled with valid data.
            size_t valid_log_count_{ 0 };
            //! The base date for all logs within this history, stored in minutes since epoch. This is not
            //! serialized but is essential for consistency and time calculations.
            std::optional<std::time_t> card_effective_date_in_minutes_;
        };

       /**
         * @class container
         * @brief Represents the complete 96-byte Common Service Area with all functionality fully encapsulated.
         *
         * @details This class is the top-level wrapper that orchestrates the `general`, `validation`,
         *          and `history` data blocks, along with the final RFU bytes. It provides a single
         *          point of entry for parsing and serializing the entire 96-byte card data structure.
         *          Its primary responsibility is to ensure data consistency, especially for the time-sensitive
         *          `card_effective_date` that is shared across multiple child objects.
         *
         *          The 96-byte layout is as follows:
         *          - **Bytes 0-1**: `general` data (2 bytes)
         *          - **Bytes 2-20**: `validation` data (19 bytes)
         *          - **Bytes 21-88**: `history` data (4 logs, 68 bytes)
         *          - **Bytes 89-95**: RFU (Reserved for Future Use, 7 bytes)
         *
         * @warning An object of this class **must** be constructed with a `card_effective_date`, which
         *          governs all time-based calculations and is enforced for all child objects.
         *
         * @usage
         * @code
         *     // 1. Define the card's effective date in minutes since the Unix epoch.
         *     time_t effective_date = 28399680;
         *
         *     // 2. Create the main CSA container. The date is mandatory.
         *     csa::container my_csa(effective_date);
         *
         *     // 3. Set the general data.
         *     my_csa.get_general().set_version(1, 0, 0); // Modify the child object directly.
         *
         *     // 4. Set the validation data. Note that the container's constructor
         *     //    has already set the effective date on this object.
         *     my_csa.get_validation().set_date_and_time(1704067200000ULL);
         *     my_csa.get_validation().set_fare_amount(120);
         *
         *     // 5. Serialize the entire 96-byte structure to a byte vector.
         *     std::vector<uint8_t> bytes_to_write = my_csa.to_bytes();
         *
         *     // 6. Parse the byte vector back into an object.
         *     csa::container parsed_csa = csa::container::parse(bytes_to_write, effective_date);
         * @endcode
         */
        class container {
        public:

            //! The fixed total size of the entire CSA block in bytes.
            static constexpr size_t TOTAL_SIZE = 96;
            //! The size of the final RFU (Reserved for Future Use) block in bytes.
            static constexpr size_t RFU_SIZE = 7;

            // --- Offsets for Data Slicing ---
            // These constants define the starting position of each data block within the 96-byte array.
            // This makes the parsing and serialization logic clear and easy to maintain.

            //! The starting byte position of the `general` data block.
            static constexpr size_t GENERAL_OFFSET = 0;
            //! The starting byte position of the `validation` data block.
            static constexpr size_t VALIDATION_OFFSET = GENERAL_OFFSET + general::DATA_SIZE; // Offset 2
            //! The starting byte position of the `history` data block.
            static constexpr size_t HISTORY_OFFSET = VALIDATION_OFFSET + validation::DATA_SIZE; // Offset 21
            //! The starting byte position of the RFU data block.
            static constexpr size_t RFU_OFFSET = HISTORY_OFFSET + history::TOTAL_SIZE; // Offset 89

            /**
             * @brief Constructs a `container` object with a mandatory card effective date.
             * @param card_effective_date_in_minutes The card's effective date. What to send: A `std::time_t`
             *                                       value representing the number of **minutes** since the Unix epoch.
             *                                       This date is the single source of truth for the entire object.
             * @note The `explicit` keyword prevents implicit conversions (e.g., from an integer),
             *       which improves type safety.
             */
            explicit container(const std::time_t card_effective_date_in_minutes) noexcept
                : card_effective_date_(card_effective_date_in_minutes) {
                // The constructor immediately propagates the effective date to its time-sensitive
                // child objects. This ensures that the container and its parts are always in a
                // consistent state from the moment of creation.
                validation_.set_card_effective_date(card_effective_date_in_minutes);
                history_.set_card_effective_date(card_effective_date_in_minutes);
            }

            /**
             * @brief Sets the `general` data block for the CSA.
             * @param gen The `general` object to set.
             */
            void set_general(const general& gen) noexcept {
                general_ = gen;
            }

            /**
             * @brief Sets the `validation` data block for the CSA.
             * @param val The `validation` object to set. What to send: A `validation` object whose
             *            effective date has been set and matches this CSA's effective date.
             * @throws std::logic_error if the `validation` object's effective date does not match this CSA's.
             */
            void set_validation(const validation& val) {
                //  This check is now simple, efficient, and correct. It uses the public
                // getter on the `validation` object instead of a complex and buggy calculation.
                // This ensures that the child object is consistent with its parent container.
                if (val.get_card_effective_date() != card_effective_date_) {
                    throw std::logic_error("Validation object's effective date does not match CSA's.");
                }
                validation_ = val;
            }

            /**
             * @brief Sets the `history` data block for the CSA.
             * @param hist The `history` object to set. What to send: A `history` object whose
             *             effective date has been set and matches this CSA's effective date.
             * @throws std::logic_error if the `history` object's effective date does not match this CSA's.
             */
            void set_history(const history& hist) {
                // This check ensures data consistency between the container and its history object.
                if (hist.get_card_effective_date() != card_effective_date_)
                    throw std::logic_error("History object's effective date does not match CSA's.");
                history_ = hist;
            }

            /**
             * @brief Sets the 7 bytes for the Reserved for Future Use (RFU) field.
             * @param rfu_data The RFU data. What to send: A `std::array<uint8_t, 7>`.
             */
            void set_rfu(const std::array<uint8_t, RFU_SIZE>& rfu_data) noexcept {
                rfu_ = rfu_data;
            }

            /**
             * @brief Parses a 96-byte data vector into a complete `csa::container` object.
             * @param data A const reference to a `std::vector` containing exactly 96 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch, which is
             *        essential for interpreting all time offsets within the data.
             * @return A `container` object populated with all parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 96 bytes.
             */
            static container parse(const std::vector<uint8_t>& data, const std::time_t card_effective_date_in_minutes) {
                if (data.size() != TOTAL_SIZE)
                    throw std::invalid_argument("Input CSA data must be exactly 96 bytes.");

                // Create a new container with the mandatory effective date.
                container result(card_effective_date_in_minutes);

                // Delegate parsing for each block to its respective class, passing the correct slice of the data vector.
                // The use of iterators `{data.begin() + OFFSET, data.begin() + NEXT_OFFSET}` creates a temporary
                // `std::vector<uint8_t>` for each `parse` function.
                result.general_ = general::parse({data.begin() + GENERAL_OFFSET, data.begin() + VALIDATION_OFFSET});
                result.validation_ = validation::parse({data.begin() + VALIDATION_OFFSET, data.begin() + HISTORY_OFFSET}, card_effective_date_in_minutes);
                result.history_ = history::parse({data.begin() + HISTORY_OFFSET, data.begin() + RFU_OFFSET}, card_effective_date_in_minutes);

                // The final RFU block is a simple array of bytes, so we can copy it directly.
                std::copy(data.begin() + RFU_OFFSET, data.end(), result.rfu_.begin());

                return result;
            }

            /**
             * @brief Serializes the `csa::container` object into a 96-byte vector.
             * @return A `std::vector<uint8_t>` containing the complete 96 bytes of serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(TOTAL_SIZE); // Pre-allocate memory to avoid multiple reallocations.

                // Delegate serialization to each child object and append the resulting bytes in the correct order.
                auto general_data = general_.to_bytes();
                data.insert(data.end(), general_data.begin(), general_data.end());

                auto validation_data = validation_.to_bytes();
                data.insert(data.end(), validation_data.begin(), validation_data.end());

                auto history_data = history_.to_bytes();
                data.insert(data.end(), history_data.begin(), history_data.end());

                // Append the final RFU bytes.
                data.insert(data.end(), rfu_.begin(), rfu_.end());

                return data;
            }

            //! @brief Gets a mutable reference to the `general` object, allowing direct modification.
            [[nodiscard]] general& get_general() noexcept { return general_; }
            //! @brief Gets a const reference to the `general` object for read-only access.
            [[nodiscard]] const general& get_general() const noexcept { return general_; }

            //! @brief Gets a mutable reference to the `validation` object, allowing direct modification.
            [[nodiscard]] validation& get_validation() noexcept { return validation_; }
            //! @brief Gets a const reference to the `validation` object for read-only access.
            [[nodiscard]] const validation& get_validation() const noexcept { return validation_; }

            //! @brief Gets a mutable reference to the `history` object, allowing direct modification.
            [[nodiscard]] history& get_history() noexcept { return history_; }
            //! @brief Gets a const reference to the `history` object for read-only access.
            [[nodiscard]] const history& get_history() const noexcept { return history_; }

            //! @brief Gets a const reference to the 7-byte RFU data array.
            [[nodiscard]] const std::array<uint8_t, RFU_SIZE>& get_rfu() const noexcept { return rfu_; }
            //! @brief Gets the card effective date that this container was initialized with.
            [[nodiscard]] std::time_t get_card_effective_date() const noexcept { return card_effective_date_; }

            /**
             * @brief Stream insertion operator for easy printing of `container` objects.
             * @details This operator delegates the printing of each sub-block to that block's
             *          own stream operator, resulting in a clean, fully detailed output.
             */
            friend std::ostream& operator<<(std::ostream& os, const container& obj) {
                os << "======================= COMMON SERVICE AREA (CSA) =======================" << std::endl;
                os << obj.general_ << std::endl;
                os << obj.validation_ << std::endl;
                os << obj.history_ << std::endl;
                os << "-------------------------- RFU (7 Bytes) --------------------------" << std::endl << "  ";
                // Print the raw hex values for the RFU block.
                for (const auto& byte : obj.rfu_) {
                    os << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
                }
                os << std::dec << std::endl; // Reset stream to decimal for subsequent prints.
                os << "=======================================================================";
                return os;
            }

            /**
             * @brief Compares two `container` objects for equality.
             * @return True if all corresponding members and child objects are identical, false otherwise.
             */
            friend bool operator==(const container& lhs, const container& rhs) {
                // This deep comparison relies on the `operator==` implementation of each child class.
                return lhs.card_effective_date_ == rhs.card_effective_date_ &&
                       lhs.general_ == rhs.general_ &&
                       lhs.validation_ == rhs.validation_ &&
                       lhs.history_ == rhs.history_ &&
                       lhs.rfu_ == rhs.rfu_;
            }

        private:

            //! The general data block (2 bytes).
            general general_{};
            //! The last validation record (19 bytes).
            validation validation_{};
            //! The historical transaction log records (68 bytes).
            history history_{};
            //! The 7-byte reserved data block.
            std::array<uint8_t, RFU_SIZE> rfu_{};
            //! The single source of truth for all time calculations, stored in minutes since epoch.
            //! This member is NOT part of the serialized 96 bytes but is crucial for the object's logic.
            std::time_t card_effective_date_{};

        };

    }

    /**
     * @namespace osa
     * @brief Placeholder for components related to the Operator Service Area (OSA).
     * @details The Operator Service Area is a separate, proprietary data block on a transit card
     *          reserved for use by a specific transit operator. Its structure is vendor-defined
     *          and is not implemented in this library. This namespace is reserved for future
     *          extensions where a specific operator's OSA layout might be implemented.
     */
    namespace osa {
        /**
         * @class general
         * @brief Represents the 7-byte General Data block of the Operator Service Area (OSA).
         *
         * @details This class provides a structured interface for the OSA's general information. It has been
         *          refactored to use the same major.minor.patch versioning system as the `csa::general` class.
         *          The 10-digit phone number is stored efficiently in a 5-byte Binary-Coded Decimal (BCD) format.
         *
         *          The 7-byte data is structured as follows:
         *          - **Byte 0**: [Major (3 bits)][Minor (3 bits)][Patch (2 bits)]
         *          - **Bytes 1-5**: Phone Number (10 digits stored in 5 bytes as packed BCD)
         *          - **Byte 6**: [Language (5 bits)][Service Status (1 bit)][RFU (2 bits)]
         *
         * @usage
         * @code
         *     osa::general osa_gen;
         *     osa_gen.set_version(1, 2, 3);
         *     osa_gen.set_phone_number("9876543210"); // Must be 10 digits
         *     osa_gen.set_language(language_code::English);
         *     osa_gen.set_service_status(osa::general::service_status::active);
         *
         *     std::vector<uint8_t> bytes = osa_gen.to_bytes();
         *     osa::general parsed_gen = osa::general::parse(bytes);
         *
         *     std::cout << parsed_gen << std::endl;
         * @endcode
         */
        class general {
        public:

            //! Defines the service status as either active or inactive.
            enum class service_status : bool {
                inactive = false,
                active = true
            };

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 7;
            //! The fixed size of the BCD-encoded phone number in bytes.
            static constexpr size_t PHONE_NUMBER_BYTES = 5;
            //! The required number of digits for a valid phone number string.
            static constexpr size_t PHONE_NUMBER_DIGITS = 10;

            // --- Versioning Constants (mirrors csa::general) ---
            //! The maximum value for the major version (stored in 3 bits: 2^3 - 1).
            static constexpr uint8_t MAJOR_VERSION_MAX = 7;
            //! The maximum value for the minor version (stored in 3 bits: 2^3 - 1).
            static constexpr uint8_t MINOR_VERSION_MAX = 7;
            //! The maximum value for the patch version (stored in 2 bits: 2^2 - 1).
            static constexpr uint8_t PATCH_VERSION_MAX = 3;
            //! The maximum value for the RFU field (stored in 2 bits: 2^2 - 1).
            static constexpr uint8_t RFU_MAX = 3;

            /**
             * @brief Default constructor. Creates a `general` object with all fields initialized to their defaults.
             */
            general() = default;

            /**
             * @brief Sets the version of the data format.
             * @param major The major version. What to send: A value in the range [0, 7].
             * @param minor The minor version. What to send: A value in the range [0, 7].
             * @param patch The patch version. What to send: A value in the range [0, 3].
             * @throws std::out_of_range if any version component is outside its valid bit-field range.
             */
            void set_version(const uint8_t major, const uint8_t minor, const uint8_t patch) {
                if (major > MAJOR_VERSION_MAX) throw std::out_of_range("Major version must be in the range [0, 7].");
                if (minor > MINOR_VERSION_MAX) throw std::out_of_range("Minor version must be in the range [0, 7].");
                if (patch > PATCH_VERSION_MAX) throw std::out_of_range("Patch version must be in the range [0, 3].");
                major_version_ = major;
                minor_version_ = minor;
                patch_version_ = patch;
            }

            /**
             * @brief Sets the customer phone number from a 10-digit string using BCD encoding.
             * @details Binary-Coded Decimal (BCD) stores each decimal digit in a 4-bit "nibble". This method
             *          packs two digits into every byte of the `phone_number_` array.
             * @param number_str The phone number as a string of digits.
             *                   What to send: A string containing **exactly 10 digits** ('0'-'9').
             * @throws std::invalid_argument if the number string is not 10 characters or contains non-digits.
             */
            void set_phone_number(const std::string& number_str) {
                // Validate the input string format before processing.
                if (number_str.length() != PHONE_NUMBER_DIGITS)
                    throw std::invalid_argument("Phone number must be exactly 10 digits.");
                if (number_str.find_first_not_of("0123456789") != std::string::npos)
                    throw std::invalid_argument("Phone number must contain only digits.");

                // Iterate 5 times, processing two digits for each byte.
                for (size_t i = 0; i < PHONE_NUMBER_BYTES; ++i) {
                    // Convert the ASCII character of the first digit (e.g., '9') to its integer value (9).
                    const uint8_t high_nibble = number_str[i * 2] - '0';
                    // Convert the second digit.
                    const uint8_t low_nibble = number_str[i * 2 + 1] - '0';
                    // Pack the two 4-bit values into a single byte.
                    // e.g., for "98": high_nibble=9 (0b1001), low_nibble=8 (0b1000)
                    // (9 << 4) | 8  ->  0b10010000 | 0b00001000  ->  0b10011000 (0x98)
                    phone_number_[i] = (high_nibble << 4) | low_nibble;
                }
            }

            void set_language(const language_code code) noexcept { language_ = code; }
            void set_service_status(const service_status status) noexcept { status_ = status; }

            /**
             * @brief Sets the value for the Reserved for Future Use (RFU) field.
             * @param value The RFU value. What to send: A value in the range [0, 3].
             * @throws std::out_of_range if the value is outside the valid 2-bit range.
             */
            void set_rfu(const uint8_t value) {
                if (value > RFU_MAX) throw std::out_of_range("RFU value must be in the range [0, 3].");
                rfu_ = value;
            }

            /**
             * @brief Parses a 7-byte data vector into a `general` object.
             * @param data A const reference to a `std::vector` containing exactly 7 bytes.
             * @return A `general` object populated with the parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 7 bytes.
             */
            static general parse(const std::vector<uint8_t>& data) {
                if (data.size() != DATA_SIZE) throw std::invalid_argument("OSA General data must be exactly 7 bytes.");

                general g;

                // Byte 0: Version (Major/Minor/Patch).
                // Unpack the three version components from the first byte using bit shifts and masks.
                g.major_version_ = (data[0] >> 5) & 0x07;
                g.minor_version_ = (data[0] >> 2) & 0x07;
                g.patch_version_ = data[0] & 0x03;

                // Bytes 1-5: Phone Number (BCD).
                // Directly copy the 5 bytes of BCD data into the internal array.
                std::copy(data.begin() + 1, data.begin() + 1 + PHONE_NUMBER_BYTES, g.phone_number_.begin());

                // Byte 6: Packed Language, Status, and RFU.
                const uint8_t last_byte = data[6];
                // Extract the 5-bit language code.
                g.language_ = static_cast<open_loop::language_code>((last_byte >> 3) & 0x1F);
                // Extract the 1-bit service status.
                g.status_ = static_cast<service_status>((last_byte >> 2) & 0x01);
                // Extract the 2-bit RFU value.
                g.rfu_ = last_byte & 0x03;

                return g;
            }

            /**
             * @brief Serializes the `general` object into a 7-byte vector.
             * @return A `std::vector<uint8_t>` containing the serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(DATA_SIZE);

                // Byte 0: Version.
                // Pack the three version components into a single byte.
                data.push_back(
                    (major_version_ << 5) | (minor_version_ << 2) | patch_version_
                );

                // Bytes 1-5: Phone Number.
                // Insert the 5 bytes of raw BCD data.
                data.insert(data.end(), phone_number_.begin(), phone_number_.end());

                // Byte 6: Packed Language, Status, and RFU.
                // Combine the three fields into the last byte using bitwise shifts and ORs.
                const uint8_t last_byte = (static_cast<uint8_t>(language_) << 3) |
                                          (static_cast<uint8_t>(status_) << 2) |
                                          rfu_;
                data.push_back(last_byte);

                return data;
            }

            [[nodiscard]] uint8_t get_major_version() const noexcept { return major_version_; }
            [[nodiscard]] uint8_t get_minor_version() const noexcept { return minor_version_; }
            [[nodiscard]] uint8_t get_patch_version() const noexcept { return patch_version_; }
            [[nodiscard]] language_code get_language() const noexcept { return language_; }
            [[nodiscard]] service_status get_service_status() const noexcept { return status_; }
            [[nodiscard]] uint8_t get_rfu() const noexcept { return rfu_; }


            /**
             * @brief Retrieves the full version number as a formatted string.
             * @return The version string in "major.minor.patch" format (e.g., "1.2.3").
             */
            [[nodiscard]] std::string get_version_string() const {
                return std::to_string(major_version_) + "." + std::to_string(minor_version_) + "." + std::to_string(patch_version_);
            }

            /**
             * @brief Retrieves the customer phone number as a 10-digit string by decoding BCD.
             * @return A `std::string` containing the phone number, or an empty string if the number is all zeros.
             */
            [[nodiscard]] std::string get_phone_number() const {
                std::stringstream ss;
                // Unpack each byte into two digit characters.
                for (const auto& byte : phone_number_) {
                    // Get the high nibble (first digit) by shifting right.
                    ss << static_cast<char>('0' + ((byte >> 4) & 0x0F));
                    // Get the low nibble (second digit) by masking.
                    ss << static_cast<char>('0' + (byte & 0x0F));
                }
                std::string result = ss.str();
                // Treat an uninitialized (all-zeros) phone number as an empty string for convenience.
                if (result == "0000000000")
                    return "";
                return result;
            }

            [[nodiscard]] std::string get_service_status_string() const {
                return (status_ == service_status::active) ? "Active" : "Inactive";
            }

            /**
             * @brief Retrieves the language as a human-readable string.
             * @return The name of the language (e.g., "English"). Returns "Unknown" for undefined codes.
             */
            [[nodiscard]] std::string get_language_string() const {
                // This helper provides a more user-friendly output than just the raw enum value.
                switch (language_) {
                    case language_code::English: return "English";
                    case language_code::Hindi:   return "Hindi";
                    case language_code::Marathi: return "Marathi";
                    default: return "Unknown";
                }
            }

            /**
            * @brief Stream insertion operator for easy printing of `osa::general` objects.
            */
            friend std::ostream& operator<<(std::ostream& os, const general& obj) {
                os << "-------------------- OSA: GENERAL DATA ---------------------" << std::endl;
                os << "  VERSION                : " << obj.get_version_string() << std::endl;
                os << "  PHONE NUMBER           : " << obj.get_phone_number() << std::endl;
                os << "  LANGUAGE               : " << obj.get_language_string() << " (Code: " << static_cast<int>(obj.get_language()) << ")" << std::endl;
                os << "  SERVICE STATUS         : " << obj.get_service_status_string() << std::endl;
                os << "  RFU (BINARY)           : " << std::bitset<2>(obj.get_rfu()) << std::endl;
                os << "------------------------------------------------------------";
                return os;
            }

            /**
             * @brief Compares two `osa::general` objects for equality.
             */
            friend bool operator==(const general& lhs, const general& rhs) {
                return lhs.major_version_ == rhs.major_version_ &&
                       lhs.minor_version_ == rhs.minor_version_ &&
                       lhs.patch_version_ == rhs.patch_version_ &&
                       lhs.phone_number_ == rhs.phone_number_ &&
                       lhs.language_ == rhs.language_ &&
                       lhs.status_ == rhs.status_ &&
                       lhs.rfu_ == rhs.rfu_;
            }

        private:

            //! The major version number (3 bits).
            uint8_t major_version_{ 0 };
            //! The minor version number (3 bits).
            uint8_t minor_version_{ 0 };
            //! The patch version number (2 bits).
            uint8_t patch_version_{ 0 };

            //! The 10-digit phone number, stored as 5 bytes of packed BCD data.
            std::array<uint8_t, PHONE_NUMBER_BYTES> phone_number_{};
            //! The preferred language (5 bits).
            language_code language_{ language_code::English };
            //! The service status (1 bit).
            service_status status_{ service_status::inactive };
            //! Reserved for Future Use field (2 bits).
            uint8_t rfu_{ 0 };
        };

        /**
         * @class transaction_record
         * @brief Represents the 13-byte data block for a validation or log entry in the OSA.
         *
         * @details This class provides a structured interface for a single transaction event. The transaction
         *          time is stored internally as a 24-bit offset in minutes from a mandatory
         *          `card_effective_date`, ensuring efficient and consistent timekeeping. This class serves as the
         *          building block for both the OSA's `validation` record and its `history` entries.
         *
         *          The 13-byte data is structured in big-endian format as follows:
         *          - **Byte 0**: Error Code (8 bits)
         *          - **Byte 1**: Product Type (8 bits)
         *          - **Bytes 2-4**: Date & Time Offset (24 bits, in minutes)
         *          - **Bytes 5-6**: Station ID (16 bits)
         *          - **Bytes 7-8**: Fare (16 bits)
         *          - **Bytes 9-11**: Terminal ID (24 bits)
         *          - **Byte 12**: [Transaction Status (4 bits)][RFU (4 bits)]
         *
         * @warning The transaction time is stored as an offset. You **must** call
         *          `set_card_effective_date()` before you can call `set_date_and_time()`.
         *
         * @usage
         * @code
         *     osa::transaction_record record;
         *
         *     // CRITICAL: Set the effective date first!
         *     time_t effective_date = 28399680;
         *     record.set_card_effective_date(effective_date);
         *
         *     // Set the absolute transaction time and other properties.
         *     uint64_t current_ms = 1704067200000ULL; // Jan 1, 2024
         *     record.set_date_and_time(current_ms);
         *     record.set_fare(200);
         *     record.set_station_id(101);
         *
         *     std::cout << record << std::endl;
         * @endcode
         */
        class transaction_record {
        public:

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 13;
            //! The maximum value for the time offset (stored in 24 bits: 2^24 - 1).
            static constexpr uint32_t TIME_OFFSET_MAX = 0xFFFFFF;
            //! The maximum value for the terminal ID (stored in 24 bits: 2^24 - 1).
            static constexpr uint32_t TERMINAL_ID_MAX = 0xFFFFFF;
            //! The maximum value for the RFU field (stored in 4 bits: 2^4 - 1).
            static constexpr uint8_t RFU_MAX = 0x0F;

            /**
             * @brief Default constructor. Creates a record with all fields initialized to zero.
             */
            transaction_record() = default;

            /**
             * @brief Sets the card's effective date, which is the base for time calculations.
             * @param date_in_minutes The card's effective date. What to send: A `std::time_t` value
             *                        representing the number of **minutes** since the Unix epoch.
             */
            void set_card_effective_date(std::time_t date_in_minutes) noexcept {
                card_effective_date_in_minutes_ = date_in_minutes;
            }

            /**
             * @brief Sets the absolute transaction time from a millisecond timestamp.
             * @param absolute_time_in_milliseconds The absolute time of the transaction. What to send: A `uint64_t`
             *                                        value representing milliseconds since the Unix epoch.
             * @throws std::logic_error if `set_card_effective_date()` has not been called first.
             * @throws std::out_of_range if the calculated time difference is negative or exceeds the 24-bit storage limit.
             */
            void set_date_and_time(const uint64_t absolute_time_in_milliseconds) {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date must be set before setting transaction time.");

                // Use uint64_t for all intermediate calculations to prevent
                // signed/unsigned comparison issues and potential overflow.
                const uint64_t absolute_time_in_minutes = absolute_time_in_milliseconds / 60000;
                const uint64_t effective_date_minutes = *card_effective_date_in_minutes_;

                if (absolute_time_in_minutes < effective_date_minutes)
                    throw std::out_of_range("Transaction time cannot be before the card effective date.");

                const uint64_t time_diff = absolute_time_in_minutes - effective_date_minutes;

                if (time_diff > TIME_OFFSET_MAX)
                    throw std::out_of_range("Transaction time is out of the valid 24-bit range from effective date.");

                date_and_time_offset_ = static_cast<uint32_t>(time_diff);
            }

            /**
             * @brief Sets the Terminal ID.
             * @param id The terminal's unique identifier. What to send: A value in the range [0, 0xFFFFFF].
             * @throws std::out_of_range if the ID exceeds the 24-bit limit.
             */
            void set_terminal_id(const uint32_t id) {
                if (id > TERMINAL_ID_MAX) throw std::out_of_range("Terminal ID exceeds 24-bit limit.");
                terminal_id_ = id;
            }

            /**
             * @brief Sets the value for the Reserved for Future Use (RFU) field.
             * @param value The RFU value. What to send: A value in the range [0, 15].
             * @throws std::out_of_range if the value is outside the valid 4-bit range.
             */
            void set_rfu(const uint8_t value) {
                if (value > RFU_MAX) throw std::out_of_range("RFU value must be in the range [0, 15].");
                rfu_ = value;
            }

            void set_error_code(const uint8_t code) noexcept { error_code_ = code; }
            void set_product_type(const uint8_t type) noexcept { product_type_ = type; }
            void set_station_id(const uint16_t id) noexcept { station_id_ = id; }
            void set_fare(const uint16_t fare_amount) noexcept { fare_ = fare_amount; }
            void set_txn_status(const txn_status status) noexcept { status_ = status; }

            /**
             * @brief Parses a 13-byte data vector into a `transaction_record` object.
             * @param data A const reference to a `std::vector` containing exactly 13 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch,
             *        which is essential for interpreting the time offset.
             * @return A `transaction_record` object populated with the parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 13 bytes.
             */
            static transaction_record parse(const std::vector<uint8_t>& data, std::time_t card_effective_date_in_minutes) {
                if (data.size() != DATA_SIZE) throw std::invalid_argument("OSA Transaction Record data must be 13 bytes.");

                transaction_record rec;
                rec.card_effective_date_in_minutes_ = card_effective_date_in_minutes;

                // Bytes 0-1: Single-byte fields.
                rec.error_code_ = data[0];
                rec.product_type_ = data[1];

                // Bytes 2-4: Date & Time Offset (24-bit, Big-Endian). Reconstruct from 3 bytes.
                rec.date_and_time_offset_ = (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 8) | data[4];

                // Bytes 5-6: Station ID (16-bit, Big-Endian). Reconstruct from 2 bytes.
                rec.station_id_ = (static_cast<uint16_t>(data[5]) << 8) | data[6];

                // Bytes 7-8: Fare (16-bit, Big-Endian). Reconstruct from 2 bytes.
                rec.fare_ = (static_cast<uint16_t>(data[7]) << 8) | data[8];

                // Bytes 9-11: Terminal ID (24-bit, Big-Endian). Reconstruct from 3 bytes.
                rec.terminal_id_ = (static_cast<uint32_t>(data[9]) << 16) | (static_cast<uint32_t>(data[10]) << 8) | data[11];

                // Byte 12: Packed Status (upper 4 bits) and RFU (lower 4 bits).
                rec.status_ = static_cast<open_loop::txn_status>((data[12] >> 4) & 0x0F);
                rec.rfu_ = data[12] & 0x0F;

                return rec;
            }

            /**
             * @brief Serializes the `transaction_record` object into a 13-byte vector.
             * @return A `std::vector<uint8_t>` containing the serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(DATA_SIZE); // Pre-allocate memory for efficiency.

                // Bytes 0-1.
                data.push_back(error_code_);
                data.push_back(product_type_);

                // Bytes 2-4: Deconstruct 24-bit time offset into 3 bytes.
                data.push_back((date_and_time_offset_ >> 16) & 0xFF);
                data.push_back((date_and_time_offset_ >> 8) & 0xFF);
                data.push_back(date_and_time_offset_ & 0xFF);

                // Bytes 5-6: Deconstruct 16-bit station ID into 2 bytes.
                data.push_back((station_id_ >> 8) & 0xFF);
                data.push_back(station_id_ & 0xFF);

                // Bytes 7-8: Deconstruct 16-bit fare into 2 bytes.
                data.push_back((fare_ >> 8) & 0xFF);
                data.push_back(fare_ & 0xFF);

                // Bytes 9-11: Deconstruct 24-bit terminal ID into 3 bytes.
                data.push_back((terminal_id_ >> 16) & 0xFF);
                data.push_back((terminal_id_ >> 8) & 0xFF);
                data.push_back(terminal_id_ & 0xFF);

                // Byte 12: Pack 4-bit status and 4-bit RFU into one byte.
                data.push_back((static_cast<uint8_t>(status_) << 4) | rfu_);

                return data;
            }

            /**
             * @brief Calculates and returns the absolute transaction date and time in milliseconds.
             * @return The transaction time as a `uint64_t` value (milliseconds since Unix epoch).
             * @throws std::logic_error if the card's effective date was not set.
             */
            [[nodiscard]] uint64_t get_date_and_time() const {
                if (!card_effective_date_in_minutes_.has_value()) {
                    throw std::logic_error("Card effective date is not set; cannot calculate absolute time.");
                }
                // Reconstruct the absolute time by adding the stored offset back to the base date.
                const uint64_t total_minutes = static_cast<uint64_t>(*card_effective_date_in_minutes_) + date_and_time_offset_;
                // Convert the total minutes to milliseconds for the final timestamp.
                return total_minutes * 60000;
            }

            [[nodiscard]] uint8_t get_error_code() const noexcept { return error_code_; }
            [[nodiscard]] uint8_t get_product_type() const noexcept { return product_type_; }
            [[nodiscard]] uint16_t get_station_id() const noexcept { return station_id_; }
            [[nodiscard]] uint16_t get_fare() const noexcept { return fare_; }
            [[nodiscard]] uint32_t get_terminal_id() const noexcept { return terminal_id_; }
            [[nodiscard]] txn_status get_txn_status() const noexcept { return status_; }
            [[nodiscard]] uint8_t get_rfu() const noexcept { return rfu_; }
            [[nodiscard]] std::time_t get_card_effective_date() const {
                if (!card_effective_date_in_minutes_.has_value()) {
                    throw std::logic_error("Card effective date has not been set for this record.");
                }
                return *card_effective_date_in_minutes_;
            }

            /**
             * @brief Retrieves the transaction status as a human-readable string.
             * @return A `std::string` like "ENTRY", "EXIT", etc., or "UNKNOWN" for invalid values.
             */
            [[nodiscard]] std::string get_txn_status_string() const {
                switch (status_) {
                    case txn_status::ENTRY:   return "ENTRY";
                    case txn_status::EXIT:    return "EXIT";
                    case txn_status::ONETAP:  return "ONETAP";
                    case txn_status::PENALTY: return "PENALTY";
                    default:                             return "UNKNOWN";
                }
            }

            friend std::ostream& operator<<(std::ostream& os, const transaction_record& obj) {
                os << "---------------- OSA: TRANSACTION RECORD -----------------" << std::endl;
                os << "  ERROR CODE             : " << static_cast<int>(obj.error_code_) << std::endl;
                os << "  PRODUCT TYPE           : " << static_cast<int>(obj.product_type_) << std::endl;
                try {
                    // **BUG FIX**: Convert milliseconds from get_date_and_time() to seconds for std::gmtime.
                    const std::time_t t_sec = obj.get_date_and_time() / 1000;
                    char time_str[100];
                    if (std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::gmtime(&t_sec))) {
                        os << "  DATE AND TIME          : " << time_str << " (UTC)" << std::endl;
                    }
                } catch (const std::logic_error& e) {
                    os << "  DATE AND TIME          : " << "[Not available: " << e.what() << "]" << std::endl;
                }
                os << "  STATION ID             : " << obj.station_id_ << std::endl;
                os << "  FARE                   : " << obj.fare_ << std::endl;
                os << "  TERMINAL ID            : 0x" << std::hex << std::uppercase << std::setw(6) << std::setfill('0') << obj.terminal_id_ << std::dec << std::endl;
                // **IMPROVEMENT**: Use the helper method for a more readable status.
                os << "  TRANSACTION STATUS     : " << obj.get_txn_status_string() << std::endl;
                os << "  RFU (BINARY)           : " << std::bitset<4>(obj.rfu_) << std::endl;
                os << "------------------------------------------------------------";
                return os;
            }

            friend bool operator==(const transaction_record& lhs, const transaction_record& rhs) {
                return lhs.error_code_ == rhs.error_code_ &&
                       lhs.product_type_ == rhs.product_type_ &&
                       lhs.date_and_time_offset_ == rhs.date_and_time_offset_ &&
                       lhs.station_id_ == rhs.station_id_ &&
                       lhs.fare_ == rhs.fare_ &&
                       lhs.terminal_id_ == rhs.terminal_id_ &&
                       lhs.status_ == rhs.status_ &&
                       lhs.rfu_ == rhs.rfu_ &&
                       lhs.card_effective_date_in_minutes_ == rhs.card_effective_date_in_minutes_;
            }

        private:

            uint8_t error_code_{ 0 };
            uint8_t product_type_{ 0 };
            //! The transaction time, stored as a 24-bit offset in minutes from the card's effective date.
            uint32_t date_and_time_offset_{ 0 };
            uint16_t station_id_{ 0 };
            uint16_t fare_{ 0 };
            //! A 24-bit identifier for the physical terminal.
            uint32_t terminal_id_{ 0 };
            //! The status of the transaction, stored in 4 bits.
            txn_status status_{ open_loop::txn_status::ENTRY };
            //! A 4-bit field Reserved for Future Use.
            uint8_t rfu_{ 0 };
            //! The base date for time calculations, essential for interpreting the time offset. Not serialized.
            std::optional<std::time_t> card_effective_date_in_minutes_;
        };

        /**
         * @class history
         * @brief Represents the 26-byte transaction history for the OSA.
         *
         * @details This class manages the last two `transaction_record` objects in a circular buffer
         *          fashion. When a new record is added, it is placed at the front (index 0), the existing record
         *          is shifted down to index 1, and the oldest record is discarded if the history is full.
         *
         *          The 26-byte data is structured as two consecutive 13-byte `transaction_record` objects.
         *
         * @warning The history object is fundamentally tied to a `card_effective_date`. You **must** call
         *          `set_card_effective_date()` before you can add any logs via `add_record()`.
         *
         * @usage
         * @code
         *     // Create and populate
         *     osa::history hist;
         *     time_t effective_date = 28399680;
         *     hist.set_card_effective_date(effective_date);
         *
         *     osa::transaction_record rec1, rec2;
         *     rec1.set_card_effective_date(effective_date);
         *     rec2.set_card_effective_date(effective_date);
         *     // ... populate rec1 and rec2
         *
         *     hist.add_record(rec1);
         *     hist.add_record(rec2); // Pushes rec1 to index 1
         *
         *     // Serialize and parse
         *     std::vector<uint8_t> bytes = hist.to_bytes(); // A 26-byte vector
         *     osa::history parsed_hist = osa::history::parse(bytes, effective_date);
         * @endcode
         */
        class history {
        public:

            //! The maximum number of log entries that can be stored in the OSA history.
            static constexpr size_t LOG_COUNT = 2;
            //! The size of a single serialized `transaction_record` object in bytes.
            static constexpr size_t LOG_SIZE_BYTES = 13; // Should be `transaction_record::DATA_SIZE`
            //! The total size of the OSA history data block in bytes.
            static constexpr size_t TOTAL_SIZE = LOG_COUNT * LOG_SIZE_BYTES; // 2 * 13 = 26 bytes

            /**
             * @brief Default constructor. Creates an empty `history` object with no logs.
             */
            history() = default;

            /**
             * @brief Sets the card's effective date, which is required for all subsequent operations.
             * @param date_in_minutes The card's effective date. What to send: A `std::time_t` value representing
             *                        the number of **minutes** since the Unix epoch.
             */
            void set_card_effective_date(std::time_t date_in_minutes) noexcept {
                card_effective_date_in_minutes_ = date_in_minutes;
            }

            /**
             * @brief Adds a new transaction record to the history using circular buffer logic.
             * @details This method implements "push-down" functionality. The new record is inserted
             *          at index 0. The existing record at index 0 is shifted to index 1. If the history was
             *          already full (2 records), the record at index 1 is discarded.
             * @param new_record The `transaction_record` object to add. What to send: A fully populated
             *                   record whose own effective date matches this history's effective date.
             * @throws std::logic_error if this history's effective date has not been set.
             * @throws std::invalid_argument if the `new_record`'s effective date does not match this history's.
             */
            void add_record(const transaction_record& new_record) {

                // The history object must have its primary effective date set.
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Cannot add a record until the history's effective date is set.");

                // Precondition: The incoming record must be consistent with the history's date.
                if (new_record.get_card_effective_date() != *card_effective_date_in_minutes_)
                    throw std::invalid_argument("Record's effective date must match history's effective date.");

                // Determine how many elements to shift. For LOG_COUNT=2, this will be min(0, 1)=0 or min(1, 1)=1.
                const size_t elements_to_shift = std::min(valid_log_count_, LOG_COUNT - 1);

                // Perform the shift. Loop backwards from the end to avoid overwriting data.
                for (size_t i = elements_to_shift; i > 0; --i) {
                    logs_[i] = logs_[i - 1]; // Move log[0] to log[1]
                }

                // Insert the new record at the front.
                logs_[0] = new_record;

                // Increment the count of valid logs, capping it at the maximum size.
                if (valid_log_count_ < LOG_COUNT) {
                    valid_log_count_++;
                }

            }

            /**
             * @brief Clears all log entries from the history, resetting its state to empty.
             * @details The card effective date is preserved.
             */
            void clear() noexcept {
                valid_log_count_ = 0;
            }

            /**
             * @brief Parses a 26-byte data vector into a `history` object.
             * @param data A const reference to a `std::vector` containing exactly 26 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch.
             * @return A `history` object populated with up to 2 logs from the data.
             * @throws std::invalid_argument if the data vector is not exactly 26 bytes.
             */
            static history parse(const std::vector<uint8_t>& data, const std::time_t card_effective_date_in_minutes) {

                if (data.size() != TOTAL_SIZE)
                    throw std::invalid_argument("OSA History data must be exactly 26 bytes.");

                history h;
                h.set_card_effective_date(card_effective_date_in_minutes);

                // Iterate through the two possible log slots.
                for (size_t i = 0; i < LOG_COUNT; ++i) {
                    // Get iterators for the current 13-byte slice of data.
                    const auto begin = data.begin() + (i * LOG_SIZE_BYTES);
                    const auto end = begin + LOG_SIZE_BYTES;

                    // If a log slot is all zeros, assume it and all subsequent slots are empty.
                    if (std::all_of(begin, end, [](uint8_t byte){ return byte == 0; })) {
                        break; // Stop parsing.
                    }

                    // Delegate the 13-byte chunk to the transaction_record parser.
                    h.logs_[i] = transaction_record::parse({begin, end}, card_effective_date_in_minutes);
                    h.valid_log_count_++;
                }
                return h;
            }

            /**
             * @brief Serializes the `history` object into a 26-byte vector.
             * @details Any unused log slots will be padded with zeros to ensure the output is always 26 bytes.
             * @return A `std::vector<uint8_t>` of the serialized history data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(TOTAL_SIZE);

                // Serialize each valid log entry in order.
                for (size_t i = 0; i < valid_log_count_; ++i) {
                    auto log_bytes = logs_[i].to_bytes();
                    data.insert(data.end(), log_bytes.begin(), log_bytes.end());
                }

                // Calculate how much padding is needed to reach the full 26-byte size.
                if (const size_t padding_size = (LOG_COUNT - valid_log_count_) * LOG_SIZE_BYTES; padding_size > 0) {
                    data.insert(data.end(), padding_size, 0x00);
                }
                return data;
            }

            [[nodiscard]] const std::array<transaction_record, LOG_COUNT>& get_logs() const noexcept { return logs_; }
            [[nodiscard]] size_t get_valid_log_count() const noexcept { return valid_log_count_; }

            /**
             * @brief Gets the card effective date associated with this history.
             * @return The effective date in minutes since the Unix epoch.
             * @throws std::logic_error if the effective date has not been set.
             */
            [[nodiscard]] std::time_t get_card_effective_date() const {
                if (!card_effective_date_in_minutes_.has_value())
                    throw std::logic_error("Card effective date has not been set.");
                return *card_effective_date_in_minutes_;
            }

            friend std::ostream& operator<<(std::ostream& os, const history& obj) {
                os << "======================== OSA: HISTORY DATA ========================" << std::endl;
                try {
                    os << "  CARD EFFECTIVE DATE (MINS): " << obj.get_card_effective_date() << std::endl;
                } catch (const std::logic_error&) {
                    os << "  CARD EFFECTIVE DATE (MINS): [Not Set]" << std::endl;
                }
                os << "  VALID LOG COUNT           : " << obj.get_valid_log_count() << std::endl;

                if (obj.get_valid_log_count() > 0) {
                    // Iterate through the valid logs and print them, using the transaction_record's stream operator.
                    for (size_t i = 0; i < obj.get_valid_log_count(); ++i) {
                        // Add a newline between entries but not after the last one.
                        os << obj.get_logs()[i] << (i < obj.get_valid_log_count() - 1 ? "\n" : "");
                    }
                } else {
                    os << "  [No log entries]";
                }
                os << "\n=================================================================";
                return os;
            }

            friend bool operator==(const history& lhs, const history& rhs) {
                // First, compare the inexpensive, non-array members.
                if (lhs.card_effective_date_in_minutes_ != rhs.card_effective_date_in_minutes_ ||
                    lhs.valid_log_count_ != rhs.valid_log_count_) {
                    return false;
                    }
                // If those match, compare the contents of the valid log entries.
                return std::equal(lhs.logs_.begin(), lhs.logs_.begin() + lhs.valid_log_count_, rhs.logs_.begin());
            }

        private:

            //! A fixed-size array to hold the transaction records.
            std::array<transaction_record, LOG_COUNT> logs_{};
            //! A counter for how many slots in the array contain valid data.
            size_t valid_log_count_{ 0 };
            //! The base date for all records in this history, essential for interpreting time offsets.
            std::optional<std::time_t> card_effective_date_in_minutes_;

        };

        /**
         * @class trip_pass
         * @brief Represents the 20-byte Trip Pass data block in the OSA.
         *
         * @details This class provides a structured interface for managing a trip-based pass product,
         *          including its validity, total and remaining trips, route information, and usage counters.
         *          Unlike the `transaction_record`, the timestamps here are stored as absolute seconds since
         *          the Unix epoch, not as offsets.
         *
         *          The 20-byte data is structured in big-endian format as follows:
         *          - **Byte 0**: Pass ID (8 bits)
         *          - **Bytes 1-3**: Pass Expiry (24 bits, seconds since epoch)
         *          - **Byte 4**: Priority (8 bits)
         *          - **Bytes 5-6**: Trips Allotted (16 bits)
         *          - **Bytes 7-8**: Remaining Trips (16 bits)
         *          - **Bytes 9-10**: Source ID (16 bits)
         *          - **Bytes 11-12**: Destination ID (16 bits)
         *          - **Byte 13**: Flags (8 bits)
         *          - **Byte 14**: Daily Trip Counter (8 bits)
         *          - **Bytes 15-16**: Daily Trip Indicator (16 bits, e.g., days since an epoch)
         *          - **Bytes 17-19**: Start Date & Time (24 bits, seconds since epoch)
         *
         * @usage
         * @code
         *     osa::trip_pass pass;
         *     pass.set_pass_id(101);
         *     pass.set_trips_allotted(60);
         *     pass.set_remaining_trips(55); // Must be set after allotted trips
         *     pass.set_pass_expiry(1735689600000ULL); // A timestamp in 2025
         *
         *     std::vector<uint8_t> bytes = pass.to_bytes();
         *     osa::trip_pass parsed_pass = osa::trip_pass::parse(bytes);
         *     std::cout << parsed_pass << std::endl;
         * @endcode
         */
        class trip_pass {
        public:

            //! The fixed size of this data block in bytes.
            static constexpr size_t DATA_SIZE = 20;
            //! The maximum value for a 24-bit timestamp (in seconds since epoch).
            static constexpr uint32_t TIME_MAX = 0xFFFFFF;

            /**
             * @brief Default constructor. Creates a pass with all fields initialized to zero.
             */
            trip_pass() = default;

            /**
             * @brief Sets the expiry date and time of the pass from a millisecond timestamp.
             * @param time_in_milliseconds The expiry time. What to send: A `uint64_t` value representing
             *                             milliseconds since the Unix epoch.
             * @throws std::out_of_range if the corresponding second-level timestamp exceeds the 24-bit storage limit.
             */
            void set_pass_expiry(const uint64_t time_in_milliseconds) {
                // Convert the user-provided millisecond timestamp to seconds for storage.
                const uint64_t time_in_seconds = time_in_milliseconds / 1000;
                // Validate that the timestamp can fit within the 24 bits allocated for it.
                if (time_in_seconds > TIME_MAX) {
                    throw std::out_of_range("Pass expiry time exceeds 24-bit storage limit.");
                }
                pass_expiry_ = static_cast<uint32_t>(time_in_seconds);
            }

            /**
             * @brief Sets the start (activation) date and time of the pass from a millisecond timestamp.
             * @param time_in_milliseconds The start time. What to send: A `uint64_t` value representing
             *                             milliseconds since the Unix epoch.
             * @throws std::out_of_range if the corresponding second-level timestamp exceeds the 24-bit limit.
             */
            void set_start_date_and_time(const uint64_t time_in_milliseconds) {
                // This logic is identical to set_pass_expiry.
                const uint64_t time_in_seconds = time_in_milliseconds / 1000;
                if (time_in_seconds > TIME_MAX) {
                    throw std::out_of_range("Start time exceeds 24-bit storage limit.");
                }
                start_date_and_time_ = static_cast<uint32_t>(time_in_seconds);
            }

            /**
             * @brief Sets the number of remaining trips.
             * @param trips The number of trips left. What to send: A value less than or equal to
             *              the number of trips allotted.
             * @throws std::invalid_argument if remaining trips exceed allotted trips.
             * @note You must call `set_trips_allotted()` before calling this method to ensure the
             *       validation check works correctly.
             */
            void set_remaining_trips(const uint16_t trips) {
                if (trips > trips_allotted_) {
                    throw std::invalid_argument("Remaining trips cannot be greater than allotted trips.");
                }
                remaining_trips_ = trips;
            }

            void set_pass_id(const uint8_t id) noexcept { pass_id_ = id; }
            void set_priority(const uint8_t p) noexcept { priority_ = p; }
            void set_trips_allotted(const uint16_t trips) noexcept { trips_allotted_ = trips; }
            void set_source_id(const uint16_t id) noexcept { source_id_ = id; }
            void set_destination_id(const uint16_t id) noexcept { destination_id_ = id; }
            void set_flags(const uint8_t f) noexcept { flags_ = f; }
            void set_daily_trip_counter(const uint8_t count) noexcept { daily_trip_counter_ = count; }
            void set_daily_trip_indicator(const uint16_t indicator) noexcept { daily_trip_indicator_ = indicator; }

            /**
             * @brief Parses a 20-byte data vector into a `trip_pass` object.
             * @param data A const reference to a `std::vector` containing exactly 20 bytes.
             * @return A `trip_pass` object populated with the parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 20 bytes.
             */
            static trip_pass parse(const std::vector<uint8_t>& data) {
                if (data.size() != DATA_SIZE) throw std::invalid_argument("OSA Trip Pass data must be 20 bytes.");

                trip_pass pass;
                // Byte 0: Pass ID (8-bit).
                pass.pass_id_ = data[0];
                // Bytes 1-3: Pass Expiry (24-bit, Big-Endian).
                pass.pass_expiry_ = (static_cast<uint32_t>(data[1]) << 16) | (static_cast<uint32_t>(data[2]) << 8) | data[3];
                // Byte 4: Priority (8-bit).
                pass.priority_ = data[4];
                // Bytes 5-6: Trips Allotted (16-bit, Big-Endian).
                pass.trips_allotted_ = (static_cast<uint16_t>(data[5]) << 8) | data[6];
                // Bytes 7-8: Remaining Trips (16-bit, Big-Endian).
                pass.remaining_trips_ = (static_cast<uint16_t>(data[7]) << 8) | data[8];
                // Bytes 9-10: Source ID (16-bit, Big-Endian).
                pass.source_id_ = (static_cast<uint16_t>(data[9]) << 8) | data[10];
                // Bytes 11-12: Destination ID (16-bit, Big-Endian).
                pass.destination_id_ = (static_cast<uint16_t>(data[11]) << 8) | data[12];
                // Byte 13: Flags (8-bit).
                pass.flags_ = data[13];
                // Byte 14: Daily Trip Counter (8-bit).
                pass.daily_trip_counter_ = data[14];
                // Bytes 15-16: Daily Trip Indicator (16-bit, Big-Endian).
                pass.daily_trip_indicator_ = (static_cast<uint16_t>(data[15]) << 8) | data[16];
                // Bytes 17-19: Start Date & Time (24-bit, Big-Endian).
                pass.start_date_and_time_ = (static_cast<uint32_t>(data[17]) << 16) | (static_cast<uint32_t>(data[18]) << 8) | data[19];

                return pass;
            }

            /**
             * @brief Serializes the `trip_pass` object into a 20-byte vector.
             * @return A `std::vector<uint8_t>` containing the serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(DATA_SIZE); // Pre-allocate memory for efficiency.

                // Deconstruct each multibyte field into its big-endian byte representation.
                data.push_back(pass_id_);
                data.push_back((pass_expiry_ >> 16) & 0xFF);
                data.push_back((pass_expiry_ >> 8) & 0xFF);
                data.push_back(pass_expiry_ & 0xFF);
                data.push_back(priority_);
                data.push_back((trips_allotted_ >> 8) & 0xFF);
                data.push_back(trips_allotted_ & 0xFF);
                data.push_back((remaining_trips_ >> 8) & 0xFF);
                data.push_back(remaining_trips_ & 0xFF);
                data.push_back((source_id_ >> 8) & 0xFF);
                data.push_back(source_id_ & 0xFF);
                data.push_back((destination_id_ >> 8) & 0xFF);
                data.push_back(destination_id_ & 0xFF);
                data.push_back(flags_);
                data.push_back(daily_trip_counter_);
                data.push_back((daily_trip_indicator_ >> 8) & 0xFF);
                data.push_back(daily_trip_indicator_ & 0xFF);
                data.push_back((start_date_and_time_ >> 16) & 0xFF);
                data.push_back((start_date_and_time_ >> 8) & 0xFF);
                data.push_back(start_date_and_time_ & 0xFF);

                return data;
            }

            [[nodiscard]] uint8_t get_pass_id() const noexcept { return pass_id_; }

            /**
             * @brief Gets the pass expiry date and time in milliseconds.
             * @return The expiry time as a `uint64_t` value (milliseconds since Unix epoch).
             */
            [[nodiscard]] uint64_t get_pass_expiry() const noexcept {
                // Convert the stored seconds-based timestamp back to milliseconds for the user.
                return static_cast<uint64_t>(pass_expiry_) * 1000;
            }

            /**
             * @brief Gets the pass start date and time in milliseconds.
             * @return The start time as a `uint64_t` value (milliseconds since Unix epoch).
             */
            [[nodiscard]] uint64_t get_start_date_and_time() const noexcept {
                // Convert the stored seconds-based timestamp back to milliseconds.
                return static_cast<uint64_t>(start_date_and_time_) * 1000;
            }

            [[nodiscard]] uint8_t get_priority() const noexcept { return priority_; }
            [[nodiscard]] uint16_t get_trips_allotted() const noexcept { return trips_allotted_; }
            [[nodiscard]] uint16_t get_remaining_trips() const noexcept { return remaining_trips_; }
            [[nodiscard]] uint16_t get_source_id() const noexcept { return source_id_; }
            [[nodiscard]] uint16_t get_destination_id() const noexcept { return destination_id_; }
            [[nodiscard]] uint8_t get_flags() const noexcept { return flags_; }
            [[nodiscard]] uint8_t get_daily_trip_counter() const noexcept { return daily_trip_counter_; }
            [[nodiscard]] uint16_t get_daily_trip_indicator() const noexcept { return daily_trip_indicator_; }

            friend std::ostream& operator<<(std::ostream& os, const trip_pass& obj) {
                // A stateless lambda function to format a millisecond timestamp into a human-readable UTC string.
                // This avoids duplicating formatting code. `auto` deduces the lambda's type.
                auto format_time_ms = [](const uint64_t t_ms) {
                    char buf[100];
                    // Convert milliseconds to seconds for std::gmtime.
                    const std::time_t t_sec = t_ms / 1000;
                    // Format the time. The `gmtime` function is used for UTC.
                    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::gmtime(&t_sec));
                    return std::string(buf);
                };

                os << "--------------------- OSA: TRIP PASS ---------------------" << std::endl;
                // Cast 8-bit integers to `int` for printing as numbers instead of characters.
                os << "  PASS ID                : " << static_cast<int>(obj.pass_id_) << std::endl;
                os << "  PASS EXPIRY            : " << format_time_ms(obj.get_pass_expiry()) << " (UTC)" << std::endl;
                os << "  PRIORITY               : " << static_cast<int>(obj.priority_) << std::endl;
                os << "  TRIPS ALLOTTED         : " << obj.trips_allotted_ << std::endl;
                os << "  REMAINING TRIPS        : " << obj.remaining_trips_ << std::endl;
                os << "  SOURCE ID              : " << obj.source_id_ << std::endl;
                os << "  DESTINATION ID         : " << obj.destination_id_ << std::endl;
                os << "  FLAGS (BINARY)         : " << std::bitset<8>(obj.flags_) << std::endl;
                os << "  DAILY TRIP COUNTER     : " << static_cast<int>(obj.daily_trip_counter_) << std::endl;
                os << "  DAILY TRIP INDICATOR   : " << obj.daily_trip_indicator_ << std::endl;
                os << "  START DATE & TIME      : " << format_time_ms(obj.get_start_date_and_time()) << " (UTC)" << std::endl;
                os << "------------------------------------------------------------";
                return os;
            }

            friend bool operator==(const trip_pass& lhs, const trip_pass& rhs) {
                // Perform a deep comparison of all data members.
                return lhs.pass_id_ == rhs.pass_id_ &&
                       lhs.pass_expiry_ == rhs.pass_expiry_ &&
                       lhs.priority_ == rhs.priority_ &&
                       lhs.trips_allotted_ == rhs.trips_allotted_ &&
                       lhs.remaining_trips_ == rhs.remaining_trips_ &&
                       lhs.source_id_ == rhs.source_id_ &&
                       lhs.destination_id_ == rhs.destination_id_ &&
                       lhs.flags_ == rhs.flags_ &&
                       lhs.daily_trip_counter_ == rhs.daily_trip_counter_ &&
                       lhs.daily_trip_indicator_ == rhs.daily_trip_indicator_ &&
                       lhs.start_date_and_time_ == rhs.start_date_and_time_;
            }

        private:

            uint8_t pass_id_{ 0 };
            //! The pass expiry time, stored as a 24-bit integer representing seconds since the Unix epoch.
            uint32_t pass_expiry_{ 0 };
            uint8_t priority_{ 0 };
            uint16_t trips_allotted_{ 0 };
            uint16_t remaining_trips_{ 0 };
            uint16_t source_id_{ 0 };
            uint16_t destination_id_{ 0 };
            //! A bitmask for various boolean properties of the pass.
            uint8_t flags_{ 0 };
            uint8_t daily_trip_counter_{ 0 };
            uint16_t daily_trip_indicator_{ 0 };
            //! The pass start/activation time, stored as a 24-bit integer representing seconds since the Unix epoch.
            uint32_t start_date_and_time_{ 0 };
        };

        /**
         * @class container
         * @brief Represents the complete Operator Service Area (OSA) within a 96-byte block.
         *
         * @details This class is the top-level wrapper that orchestrates all OSA components:
         *          `general`, `validation` (a `transaction_record`), a reduced-size `history`,
         *          and multiple `trip_pass` slots. It manages the serialization to and parsing
         *          from a fixed 96-byte block, automatically handling the padding required to
         *          fill the unused space. This structure is optimized to carry multiple fare
         *          products (passes) at the cost of a shorter transaction history.
         *
         *          The 96-byte layout is as follows:
         *          - **Bytes 0-6**: `general` data (7 bytes)
         *          - **Bytes 7-19**: `validation` data (13 bytes)
         *          - **Bytes 20-45**: `history` data (2 logs, 26 bytes)
         *          - **Bytes 46-65**: `trip_pass` Slot 1 (20 bytes)
         *          - **Bytes 66-85**: `trip_pass` Slot 2 (20 bytes)
         *          - **Bytes 86-95**: Padding (10 bytes)
         *
         * @warning An object of this class **must** be constructed with a `card_effective_date`, which
         *          governs all time-based calculations and ensures data consistency across its child objects.
         *
         * @usage
         * @code
         *     // 1. Define the card's effective date in minutes since the Unix epoch.
         *     time_t effective_date = 28399680;
         *
         *     // 2. Create the main OSA container.
         *     osa::container my_osa(effective_date);
         *
         *     // 3. Set components by accessing them through getters and modifying them.
         *     my_osa.get_general().set_version(1, 0, 0);
         *     my_osa.get_validation().set_fare(150);
         *     my_osa.get_trip_pass(0).set_pass_id(101); // Set the first trip pass
         *
         *     // 4. Serialize, parse, and verify.
         *     std::vector<uint8_t> bytes = my_osa.to_bytes(); // a 96-byte vector
         *     osa::container parsed_osa = osa::container::parse(bytes, effective_date);
         * @endcode
         */
        class container {
        public:

            //! The fixed total size of the OSA block on the card.
            static constexpr size_t BLOCK_SIZE = 96;
            //! The number of trip pass slots available in this layout.
            static constexpr size_t NUM_TRIP_PASSES = 2;

            // --- Offsets for Data Slicing ---
            // These constants define the starting position of each data block within the 96-byte array,
            // making the parsing and serialization logic clear and easy to maintain.

            //! Starting byte of the 'general' data block.
            static constexpr size_t GENERAL_OFFSET = 0;
            //! Starting byte of the 'validation' data block.
            static constexpr size_t VALIDATION_OFFSET = GENERAL_OFFSET + general::DATA_SIZE; // Offset 7
            //! Starting byte of the 'history' data block.
            static constexpr size_t HISTORY_OFFSET = VALIDATION_OFFSET + transaction_record::DATA_SIZE; // Offset 20
            //! Starting byte of the first 'trip_pass' data block.
            static constexpr size_t TRIP_PASS_START_OFFSET = HISTORY_OFFSET + history::TOTAL_SIZE; // Offset 46

            //! The total size of all active data components. Used to calculate padding.
            static constexpr size_t ACTUAL_DATA_SIZE = general::DATA_SIZE +
                                                       transaction_record::DATA_SIZE +
                                                       history::TOTAL_SIZE +
                                                       (NUM_TRIP_PASSES * trip_pass::DATA_SIZE); // 7+13+26+(2*20) = 86 bytes
            //! The number of zero-bytes needed to pad the data to the full block size.
            static constexpr size_t PADDING_SIZE = BLOCK_SIZE - ACTUAL_DATA_SIZE; // 96 - 86 = 10 bytes

            /**
             * @brief Constructs an `osa::container` with a mandatory card effective date.
             * @details This is the only way to create a valid container. The provided date becomes the
             *          single source of truth for all time-sensitive child objects (`validation` and `history`),
             *          which are automatically initialized with this date upon construction.
             * @param card_effective_date_in_minutes The card's effective date. What to send: A `std::time_t`
             *                                       value representing minutes since the Unix epoch.
             */
            explicit container(const std::time_t card_effective_date_in_minutes) noexcept
                : card_effective_date_(card_effective_date_in_minutes) {
                // Immediately propagate the effective date to ensure a consistent state.
                validation_.set_card_effective_date(card_effective_date_in_minutes);
                history_.set_card_effective_date(card_effective_date_in_minutes);
            }

            void set_general(const general& gen) noexcept {
                general_ = gen;
            }

            /**
             * @brief Sets the `validation` data block for the OSA.
             * @param val The `transaction_record` object to set as the validation record.
             *            What to send: A record whose effective date has been set and matches this container's.
             * @throws std::logic_error if the validation record's effective date does not match this container's,
             *         which would indicate a critical data consistency issue.
             */
            void set_validation(const transaction_record& val) {
                if (val.get_card_effective_date() != card_effective_date_)
                    throw std::logic_error("Validation record's effective date does not match OSA container's.");
                validation_ = val;
            }

            /**
             * @brief Sets the `history` data block for the OSA.
             * @param hist The `history` object to set. What to send: A history object whose
             *             effective date has been set and matches this container's.
             * @throws std::logic_error if the history object's effective date does not match, which would
             *         indicate a critical data consistency issue.
             */
            void set_history(const history& hist) {
                if (hist.get_card_effective_date() != card_effective_date_)
                    throw std::logic_error("History object's effective date does not match OSA container's.");
                history_ = hist;
            }

            /**
             * @brief Sets a specific trip pass at the given index.
             * @param pass The `trip_pass` object to set.
             * @param index The slot for the pass. What to send: A value from 0 to (NUM_TRIP_PASSES - 1).
             * @throws std::out_of_range if the index is invalid.
             */
            void set_trip_pass(const trip_pass& pass, const size_t index) {
                if (index >= NUM_TRIP_PASSES)
                    throw std::out_of_range("Trip pass index is out of bounds.");
                trip_passes_[index] = pass;
            }

            /**
             * @brief Parses a 96-byte data vector into a complete `osa::container` object.
             * @details This function slices the 96-byte input vector according to the predefined
             *          offsets and delegates parsing to each respective child component. Padding
             *          at the end of the data block is ignored.
             * @param data A const reference to a `std::vector` containing exactly 96 bytes.
             * @param card_effective_date_in_minutes The card's effective date in minutes since epoch.
             * @return An `osa::container` object populated with all parsed data.
             * @throws std::invalid_argument if the data vector is not exactly 96 bytes.
             */
            static container parse(const std::vector<uint8_t>& data, const std::time_t card_effective_date_in_minutes) {
                if (data.size() != BLOCK_SIZE)
                    throw std::invalid_argument("Input OSA data must be exactly 96 bytes.");

                // Construct the result with the mandatory date. This also correctly initializes sub-objects.
                container result(card_effective_date_in_minutes);

                // Delegate parsing for each block to its respective class, passing the correct slice of data.
                result.general_ = general::parse({data.begin() + GENERAL_OFFSET, data.begin() + VALIDATION_OFFSET});
                result.validation_ = transaction_record::parse({data.begin() + VALIDATION_OFFSET, data.begin() + HISTORY_OFFSET}, card_effective_date_in_minutes);
                result.history_ = history::parse({data.begin() + HISTORY_OFFSET, data.begin() + TRIP_PASS_START_OFFSET}, card_effective_date_in_minutes);

                // Parse each trip pass slot individually.
                for(size_t i = 0; i < NUM_TRIP_PASSES; ++i) {
                    const auto begin = data.begin() + TRIP_PASS_START_OFFSET + (i * trip_pass::DATA_SIZE);
                    const auto end = begin + trip_pass::DATA_SIZE;
                    result.trip_passes_[i] = trip_pass::parse({begin, end});
                }

                return result;
            }

            /**
             * @brief Serializes the `osa::container` object into a 96-byte vector.
             * @details This method serializes all OSA components in order (`general`, `validation`,
             *          `history`, `trip_pass` array) and then appends zero-byte padding to ensure
             *          the final output is exactly 96 bytes long.
             * @return A `std::vector<uint8_t>` containing the complete 96 bytes of serialized data.
             */
            [[nodiscard]] std::vector<uint8_t> to_bytes() const {
                std::vector<uint8_t> data;
                data.reserve(BLOCK_SIZE);

                // Delegate serialization to each child object and append the results in order.
                auto general_data = general_.to_bytes();
                data.insert(data.end(), general_data.begin(), general_data.end());

                auto validation_data = validation_.to_bytes();
                data.insert(data.end(), validation_data.begin(), validation_data.end());

                auto history_data = history_.to_bytes();
                data.insert(data.end(), history_data.begin(), history_data.end());

                for(size_t i = 0; i < NUM_TRIP_PASSES; ++i) {
                    auto trip_pass_data = trip_passes_[i].to_bytes();
                    data.insert(data.end(), trip_pass_data.begin(), trip_pass_data.end());
                }

                // Add padding to ensure the final vector is exactly 96 bytes.
                if constexpr (PADDING_SIZE > 0) {
                    data.insert(data.end(), PADDING_SIZE, 0x00);
                }

                return data;
            }

            //! @brief Gets a mutable reference to the `general` object, allowing direct modification.
            [[nodiscard]] general& get_general() noexcept { return general_; }
            //! @brief Gets a const reference to the `general` object for read-only access.
            [[nodiscard]] const general& get_general() const noexcept { return general_; }

            //! @brief Gets a mutable reference to the `validation` record, allowing direct modification.
            [[nodiscard]] transaction_record& get_validation() noexcept { return validation_; }
            //! @brief Gets a const reference to the `validation` record for read-only access.
            [[nodiscard]] const transaction_record& get_validation() const noexcept { return validation_; }

            //! @brief Gets a mutable reference to the `history` object, allowing direct modification.
            [[nodiscard]] history& get_history() noexcept { return history_; }
            //! @brief Gets a const reference to the `history` object for read-only access.
            [[nodiscard]] const history& get_history() const noexcept { return history_; }

            //! @brief Gets the card effective date that this container was initialized with.
            [[nodiscard]] std::time_t get_card_effective_date() const noexcept { return card_effective_date_; }

            /**
             * @brief Gets a mutable reference to a specific trip pass.
             * @param index The slot of the pass to retrieve (from 0 to NUM_TRIP_PASSES - 1).
             * @return A reference to the `trip_pass` object at that index.
             * @throws std::out_of_range if the index is invalid.
             */
            [[nodiscard]] trip_pass& get_trip_pass(size_t index) {
                if (index >= NUM_TRIP_PASSES)
                    throw std::out_of_range("Trip pass index is out of bounds.");
                return trip_passes_[index];
            }

            /**
             * @brief Gets a const reference to a specific trip pass.
             * @param index The slot of the pass to retrieve (from 0 to NUM_TRIP_PASSES - 1).
             * @return A const reference to the `trip_pass` object at that index.
             * @throws std::out_of_range if the index is invalid.
             */
            [[nodiscard]] const trip_pass& get_trip_pass(size_t index) const {
                if (index >= NUM_TRIP_PASSES)
                    throw std::out_of_range("Trip pass index is out of bounds.");
                return trip_passes_[index];
            }

            friend std::ostream& operator<<(std::ostream& os, const container& obj) {
                os << "==================== OPERATOR SERVICE AREA (OSA) ====================" << std::endl;
                // Delegate printing to the stream operators of each child object.
                os << obj.general_ << std::endl;
                os << obj.validation_ << std::endl;
                os << obj.history_ << std::endl;
                for(size_t i = 0; i < container::NUM_TRIP_PASSES; ++i) {
                    os << obj.trip_passes_[i] << (i < container::NUM_TRIP_PASSES - 1 ? "\n" : "");
                }
                os << std::endl << "-------------------------- PADDING ---------------------------" << std::endl;
                os << "  " << container::PADDING_SIZE << " byte(s) of padding appended during serialization." << std::endl;
                os << "=================================================================";
                return os;
            }

            friend bool operator==(const container& lhs, const container& rhs) {
                // Delegate comparison to the equality operators of each child object.
                return lhs.general_ == rhs.general_ &&
                       lhs.validation_ == rhs.validation_ &&
                       lhs.history_ == rhs.history_ &&
                       lhs.trip_passes_ == rhs.trip_passes_ &&
                       lhs.card_effective_date_ == rhs.card_effective_date_;
            }

        private:

            //! The general data block (7 bytes).
            general general_{};
            //! The last validation record (13 bytes).
            transaction_record validation_{};
            //! The historical transaction records (26 bytes).
            history history_{};
            //! An array holding the trip pass products (2 slots * 20 bytes = 40 bytes).
            std::array<trip_pass, NUM_TRIP_PASSES> trip_passes_{};
            //! The single source of truth for all time calculations, stored in minutes since epoch.
            //! This member is NOT part of the serialized 96 bytes but is crucial for the object's logic.
            std::time_t card_effective_date_{};

        };

    }

}
