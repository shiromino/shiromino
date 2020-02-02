/**
 * This library is public domain; its author disclaims any copyright to it.
 */

#pragma once

#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <sstream>

namespace PDINI {
    /**
     * Read, modify, and write INI files. Section and key names are
     * case-sensitive. Section names, key names, and values when provided as
     * arguments to member functions must not have leading nor trailing white
     * space. Section and key names must be identifiers, like in C++, that
     * start with an underscore or an alphabetic character, then zero or more
     * underscore/alphanumeric characters. The null section contains all keys
     * at the head of the INI, before any section definitions, or keys after a
     * "[]" section; to use the null section with member functions, use "" as
     * the section name.
     */
    class INI {
    public:
        /**
         * Read an INI file. All the key-value pairs are read in an INI, even
         * if invalid lines are encountered. Overwrites previous key-value
         * pairs. When the same key is used multiple times in one or more
         * sections, the last key-value pair in the last section is used.
         * Doesn't erase previous key-value pairs already stored, so you can
         * use this for multiple INI files, accumulating all the key-value
         * pairs. The first value of the returned pair is true if there was no
         * error opening the INI file, otherwise is false. The second value of
         * the returned pair contains the number of lines successfully read up
         * to and including the first invalid line; if no invalid lines were
         * read, the second value is 0.
         */
        std::pair<bool, std::size_t> read(const std::string filename) {
            std::ifstream file(std::move(filename));

            if (file.fail()) {
                return { false, 0 };
            }

            std::string line;

            const std::regex emptyRegex("\\s*");

            // Comments:
            // ; example comment
            const std::regex commentRegex("\\s*;.*");

            // Sections:
            // [example_section]
            const std::regex sectionRegex("^\\s*\\[\\s*([_a-zA-Z][_a-zA-Z0-9]*|)\\s*\\]\\s*$");

            // Key-value pairs:
            // example = key-value pair
            const std::regex keyValueRegex("^\\s*([_a-zA-Z][_a-zA-Z0-9]*)\\s*=\\s*(.*[^\\s]|)");

            // Key-value pairs at the head of the INI, before any sections, go into the
            // "null section", whose name is the empty string. The null section can
            // also be explicitly used, via a "[]" section line.
            std::string sectionName("");

            std::pair<bool, std::size_t> readStatus(true, 0);
            for (std::size_t invalidLineNum = 1; getline(file, line); invalidLineNum++) {
                std::smatch matches;
                bool matched = true;

                if (regex_match(line, emptyRegex)) {
                    continue;
                }
                else if (regex_search(line, matches, keyValueRegex)) {
                    if (matches[2] != "") {
                        sections[sectionName][matches[1]] = matches[2];
                    }
                }
                else if (regex_search(line, matches, sectionRegex)) {
                    sectionName = matches[1];
                }
                else {
                    matched = regex_match(line, commentRegex);
                }

                if (!matched && readStatus.second == 0) {
                    readStatus.second = invalidLineNum;
                }
            }
            return readStatus;
        }

        /**
         * Writes the INI to a file, truncating the file if it already existed.
         * Sections with no keys won't be written. If an INI was previously
         * read, its comments aren't kept when writing. Returns true if the
         * file was written successfully, otherwise false.
         */
        bool write(const std::string filename) {
            std::ofstream file(move(filename));

            if (file.fail()) {
                return false;
            }

            // Write null section.
            decltype(sections.extract("")) nullHandle;
            bool moveBackNull = false;
            if (sections.count("") && sections[""].size()) {
                for (auto keyValue : sections[""]) {
                    file << keyValue.first << " = " << keyValue.second << '\n';
                }
                if (sections.size() > 1) {
                    nullHandle = sections.extract("");
                    moveBackNull = true;
                    file << '\n';
                }
            }

            // Write non-null sections.
            if (sections.size()) {
                const auto section = *sections.begin();
                file << "[" << section.first << "]" << '\n';
                for (auto keyValue : section.second) {
                    file << keyValue.first << " = " << keyValue.second << '\n';
                }
                auto firstHandle = sections.extract(sections.begin());
                for (auto section : sections) {
                    file << "\n[" << section.first << "]" << '\n';
                    for (auto keyValue : section.second) {
                        file << keyValue.first << " = " << keyValue.second << '\n';
                    }
                }
                sections.insert(move(firstHandle));
            }
            if (moveBackNull) {
                sections.insert(move(nullHandle));
            }

            return !file.fail();
        }

        /**
         * Removes all sections.
         */
        void clear() {
            sections.clear();
        }

        /**
         * Gets a value in the INI directly into the value argument. The flags
         * argument is used to control the input formatting when reading from
         * the INI; the default is for numeric types to be read as decimal
         * numbers. Supports everything that can be used to extract data with
         * istringstream::operator>>. Returns true if the value was
         * successfully retrieved, otherwise returns false and doesn't modify
         * the value argument.
         */
        template<typename T>
        bool get(const std::string sectionName, const std::string keyName, T& value, const std::ios_base::fmtflags flags = std::ios_base::dec) {
            std::string valueString;
            bool got = get(sectionName, keyName, valueString);
            if (got) {
                std::istringstream stream(valueString);
                stream.setf(flags);
                stream >> value;
            }
            return got;
        }

        /**
         * Gets a string value in the INI directly into the value argument,
         * with all leading and trailing space removed. The flags argument is
         * unused when getting strings, and is only present to ensure the
         * overload that gets std::string is used. Returns true if the value
         * was successfully retrieved, otherwise returns false and doesn't
         * modify the value argument.
         */
        bool get(const std::string sectionName, const std::string keyName, std::string& value, const std::ios_base::fmtflags flags) {
            return get(sectionName, keyName, value);
        }

        /**
         * Gets a string value in the INI directly into the value argument,
         * with all leading and trailing space removed. Returns true if the
         * value was successfully retrieved, otherwise returns false and
         * doesn't modify the value argument.
         */
        bool get(const std::string sectionName, const std::string keyName, std::string& value) {
            if (sections.count(sectionName) && sections[sectionName].count(keyName)) {
                value = sections[sectionName][keyName];
                return true;
            }
            else {
                return false;
            }
        }

        /**
         * Sets a value in the INI, converting the value argument as
         * appropriate. If the section didn't already exist, it's created. The
         * flags argument is used to control the output formatting when writing
         * to the INI; the default is for numeric types to be stored as decimal
         * numbers. Supports everything that can be used to write with
         * ostringstream::operator<<.
         */
        template<typename T>
        void set(const std::string sectionName, const std::string keyName, const T value, const std::ios_base::fmtflags flags = std::ios_base::dec) {
            std::ostringstream stream;
            stream.setf(flags);
            stream << std::move(value);
            set(std::move(sectionName), std::move(keyName), std::move(stream.str()));
        }

        /**
         * Sets a value in the INI directly with a string. If the section
         * didn't already exist, it's created. To remove a key, set the key to
         * "" to remove it from the INI; if the key's section is now empty, the
         * section is also removed. The flags argument is unused when setting
         * with strings, and is only present to ensure the overload that sets
         * with std::string is used.
         */
        void set(const std::string sectionName, const std::string keyName, const std::string value, const std::ios_base::fmtflags flags) {
            set(sectionName, keyName, value);
        }

        /**
         * Sets a value in the INI directly with a string. If the section
         * didn't already exist, it's created. To remove a key, set the key to
         * "" to remove it from the INI; if the key's section is now empty, the
         * section is also removed.
         */
        void set(const std::string sectionName, const std::string keyName, const std::string value) {
            const std::regex nameRegex("[_a-zA-Z][_a-zA-Z0-9]*");
            const std::regex valueRegex("(?!\\s+).*[^\\s]");
            if (regex_match(sectionName, nameRegex) && regex_match(keyName, nameRegex) && regex_match(value, valueRegex)) {
                if (value != "") {
                    sections[move(sectionName)][move(keyName)] = value;
                }
                else {
                    sections[sectionName].erase(keyName);
                    if (!sections[sectionName].size()) {
                        sections.erase(move(sectionName));
                    }
                }
            }
        }

        /**
         * Removes a section from the INI. Does nothing if the section didn't
         * exist.
         */
        void removeSection(const std::string sectionName) {
            const std::regex nameRegex("[_a-zA-Z][_a-zA-Z0-9]*");
            if (regex_match(sectionName, nameRegex) && sections.count(sectionName)) {
                sections.erase(std::move(sectionName));
            }
        }

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
    };
}