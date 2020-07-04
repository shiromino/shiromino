/**
 * This library is available as public domain; its author disclaims any
 * copyright to it, if you choose to license as public domain.
 *
 * You may credit the author, Brandon McGriff, when licensing as public domain.
 *
 * You may also license this library under the MIT license; this is intended as
 * an option for countries that have no laws for explicit submission to the
 * public domain:
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Brandon McGriff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once

#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <locale>

namespace PDINI {
    /**
     * Read, modify, and write INI files. Section names, key names, and values
     * when provided as arguments to member functions must not have leading nor
     * trailing white space. Section and key names must be identifiers, like in
     * C++, that start with an underscore or an alphabetic character, then zero
     * or more underscore/alphanumeric characters. The null section contains
     * all keys at the head of the INI, before any section definitions, or keys
     * after a "[]" section; to use the null section with member functions, use
     * "" as the section name.
     */
    class INI {
    public:
        /**
         * Construct the INI class using the default case-insensitive matching
         * for section and key names.
         */
        INI() : INI(true) {}

        /**
         * Construct the INI class using an explicit setting for case
         * sensitivity matching for section and key names.
         */
        INI(bool caseInsenstive) : caseInsenstive(caseInsenstive) {}

        /**
         * Read an INI file. All the key-value pairs are read in an INI, even
         * if invalid lines are encountered. Overwrites previous key-value
         * pairs. When the same key is used multiple times in one or more
         * sections of the same name, the last key-value pair in the last
         * section is used. Doesn't erase previous key-value pairs already
         * stored, so you can use this for multiple INI files, accumulating all
         * the key-value pairs. The first value of the returned pair is true if
         * there was no error opening the INI file. The second value of the
         * returned pair contains the number of lines successfully read up to
         * and including the first invalid line; if no invalid lines were read,
         * the second value is 0.
         */
        std::pair<bool, std::size_t> read(const std::string& filename) {
            std::ifstream file(filename);

            if (file.fail()) {
                return {false, 0};
            }

            std::string line;

            // Comments or empty lines:
            // ; example comment
            const std::regex commentOrEmptyRegex("\\s*(;.*|)");

            // Sections, which can have comments:
            // [example_section]
            // [example_section] ; comment
            const std::regex sectionRegex("^\\s*\\[\\s*([_a-zA-Z][_a-zA-Z0-9]*|)\\s*\\]\\s*(;.*|)$");

            // Key-value pairs; text after a semicolon is not parsed as a
            // comment:
            // example_key = example value ; this text after the semicolon isn't a comment, and the space(s) before the semicolon, the semicolon, and text after the semicolon will be part of the value
            const std::regex keyValueRegex("^\\s*([_a-zA-Z][_a-zA-Z0-9]*)\\s*=\\s*(.*[^\\s]|)");

            // Key-value pairs at the head of the INI, before any sections, go
            // into the "null section", whose name is the empty string. The
            // null section can also be explicitly used, via an empty "[]"
            // section line.
            std::string sectionName("");

            std::pair<bool, std::size_t> readStatus(true, 0);
            for (std::size_t invalidLineNum = 1; std::getline(file, line); invalidLineNum++) {
                if (std::regex_match(line, commentOrEmptyRegex)) {
                    continue;
                }

                bool matched = true;
                std::smatch matches;
                if (std::regex_search(line, matches, keyValueRegex)) {
                    if (matches[2] != "") {
                        sections[sectionName][toName(matches[1])] = matches[2];
                        rawKeyNames[toName(matches[1])] = matches[1];
                    }
                }
                else if (std::regex_search(line, matches, sectionRegex)) {
                    sectionName = toName(matches[1]);
                    rawSectionNames[toName(matches[1])] = matches[1];
                }
                else {
                    matched = false;
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
         * read, its comments aren't kept when writing. All section and key
         * names will be written as they were originally represented in INI
         * files that were read, or the names originally used to set values.
         * Returns true if the file was written successfully, otherwise false.
         */
        bool write(const std::string& filename) {
            std::ofstream file(filename);

            if (file.fail()) {
                return false;
            }

            // Write null section.
            decltype(sections)::node_type nullHandle;
            bool moveBackNull = false;
            if (sections.count("") && sections[""].size()) {
                for (const auto& keyValue : sections[""]) {
                    file << rawKeyNames[keyValue.first] << " = " << keyValue.second << '\n';
                }
                if (sections.size() > 1) {
                    nullHandle = sections.extract("");
                    moveBackNull = true;
                    file << '\n';
                }
            }

            // Write non-null sections.
            if (sections.size()) {
                const auto& section = *sections.begin();
                file << "[" << rawSectionNames[section.first] << "]" << '\n';
                for (const auto& keyValue : section.second) {
                    file << rawKeyNames[keyValue.first] << " = " << keyValue.second << '\n';
                }
                auto firstHandle = sections.extract(sections.begin());
                for (const auto& section : sections) {
                    file << "\n[" << rawSectionNames[section.first] << "]" << '\n';
                    for (const auto& keyValue : section.second) {
                        file << rawKeyNames[keyValue.first] << " = " << keyValue.second << '\n';
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
        bool get(const std::string& sectionName, const std::string& keyName, T& value, const std::ios_base::fmtflags flags = std::ios_base::dec) {
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
         * with all leading and trailing space removed. Returns true if the
         * value was successfully retrieved, otherwise returns false and
         * doesn't modify the value argument.
         */
        bool get(const std::string& sectionName, const std::string& keyName, std::string& value) {
            const std::string realSectionName = toName(sectionName);
            const std::string realKeyName = toName(keyName);
            if (sections.count(realSectionName) && sections[realSectionName].count(realKeyName)) {
                value = sections[realSectionName][realKeyName];
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
        void set(const std::string& sectionName, const std::string& keyName, const T& value, const std::ios_base::fmtflags flags = std::ios_base::dec) {
            std::ostringstream stream;
            stream.setf(flags);
            stream << value;
            set(sectionName, keyName, stream.str());
        }

        /**
         * Sets a value in the INI directly with a string. If the section
         * didn't already exist, it's created. To remove a key, set the key to
         * "" to remove it from the INI; if the key's section is now empty, the
         * section is also removed.
         */
        void set(const std::string& sectionName, const std::string& keyName, const std::string& value) {
            const std::regex nameRegex("[_a-zA-Z][_a-zA-Z0-9]*");
            const std::regex valueRegex("(?!\\s+).*[^\\s]");
            const std::string realSectionName = toName(sectionName);
            const std::string realKeyName = toName(keyName);
            if (std::regex_match(keyName, nameRegex) && std::regex_match(keyName, nameRegex) && std::regex_match(value, valueRegex)) {
                if (value != "") {
                    sections[realSectionName][realKeyName] = value;
                    rawSectionNames[realSectionName] = sectionName;
                    rawKeyNames[realKeyName] = keyName;
                }
                else {
                    sections[realSectionName].erase(realKeyName);
                    rawKeyNames.erase(realKeyName);
                    if (!sections[realSectionName].size()) {
                        sections.erase(realSectionName);
                        rawSectionNames.erase(realSectionName);
                    }
                }
            }
        }

        /**
         * Removes a section from the INI. Does nothing if the section didn't
         * exist.
         */
        void removeSection(const std::string& sectionName) {
            const std::regex nameRegex("[_a-zA-Z][_a-zA-Z0-9]*");
            const std::string realSectionName = toName(sectionName);
            if (std::regex_match(sectionName, nameRegex) && sections.count(sectionName)) {
                sections.erase(realSectionName);
                rawSectionNames.erase(realSectionName);
            }
        }

    private:
        /**
         * Converts raw name strings to all uppercase if the INI class was
         * created to match names case insensitively.
         */
        std::string toName(const std::string& rawName) {
            if (caseInsenstive) {
                std::string convertedName = rawName;
                for (auto& c : convertedName) {
                    c = std::toupper(c);
                }
                return convertedName;
            }
            else {
                return rawName;
            }
        }

        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
        std::unordered_map<std::string, std::string> rawSectionNames;
        std::unordered_map<std::string, std::string> rawKeyNames;
        const bool caseInsenstive;
    };
}