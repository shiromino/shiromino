#if !defined(PDBMFONT_INCLUDE) || defined(PDBMFONT_DEFINE)
#define PDBMFONT_INCLUDE
/**
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <unordered_map>

#if defined(PDBMFONT_TEXT) || defined(PDBMFONT_BINARY) || defined(PDBMFONT_XML)
#include <fstream>
#include <iostream>
#include <cstdio>
#endif

#ifdef PDBMFONT_TEXT
#include <regex>
#include <sstream>
#endif

#ifdef PDBMFONT_XML
#include "tinyxml2.h"
#include <limits>
#include <regex>
#include <sstream>
#endif

#ifdef PDBMFONT_BINARY
#include <cstring>
#endif

namespace PDBMFont {
#ifdef PDBMFONT_DEFINE
	// This class doesn't close the FILE* passed into the constructor when
	// destructed, so it's your responsibility to close the FILE* when done using
	// this class. It's also your responsibility to open the FILE* with the right
	// openmode, and matching that openmode to the mode used to construct this
	// class, so that reading and/or writing will work.
	class CFileStreambuf : public std::streambuf {
	public:
		CFileStreambuf() = delete;

		CFileStreambuf(FILE* const file, const std::ios_base::openmode mode) : file(file), ch(0), mode(mode) {
			setg(nullptr, nullptr, nullptr);
			setp(nullptr, nullptr);
		}

	protected:
		pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
			int whence;
			switch (dir) {
			case std::ios_base::beg:
				whence = SEEK_SET;
				break;
			case std::ios_base::cur:
				whence = SEEK_CUR;
				break;
			case std::ios_base::end:
				whence = SEEK_END;
				break;
			default:
				whence = SEEK_SET;
				break;
			}
			int success = fseek(file, static_cast<long>(off), whence);
			if (success == 0) {
				return off;
			}
			else {
				return pos_type(off_type(-1));
			}
		}

		pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
			int success = fseek(file, static_cast<long>(pos), SEEK_SET);
			if (success == 0) {
				return pos;
			}
			else {
				return pos_type(off_type(-1));
			}
		}

		std::streamsize showmanyc() {
			return feof(file) ? -1 : 1;
		}

		int_type underflow() {
			if (!(mode & std::ios_base::in)) {
				setg(nullptr, nullptr, nullptr);
				return traits_type::eof();
			}

			size_t count = fread((void*)&ch, sizeof(char_type), 1u, file);

			if (count > 0u && !feof(file)) {
				setg(&ch, &ch, &ch + 1u);
				return ch;
			}
			else {
				setg(nullptr, nullptr, nullptr);
				return traits_type::eof();
			}
		}

		std::streamsize xsgetn(char_type* s, std::streamsize count) {
			std::streamsize readcount = 0u;
			if ((mode & std::ios_base::in) && (readcount = fread(s, sizeof(char_type), count, file)) > 0u) {
				ch = s[readcount - 1u];
				setg(&ch, &ch, &ch + 1u);
			}
			else {
				setg(nullptr, nullptr, nullptr);
			}
			return readcount;
		}

		int_type overflow(int_type outch = traits_type::eof()) {
			if (!(mode & std::ios_base::out) || outch == traits_type::eof()) {
				setp(nullptr, nullptr);
				return traits_type::eof();
			}

			ch = outch;
			size_t writecount = fwrite(&ch, sizeof(char_type), 1u, file);

			if (writecount > 0u && !feof(file)) {
				setp(&ch, &ch);
				return ~traits_type::eof();
			}
			else {
				setp(nullptr, nullptr);
				return traits_type::eof();
			}
		}

		std::streamsize xsputn(const char_type* s, std::streamsize count) {
			std::streamsize writecount = traits_type::eof();
			if ((mode & std::ios_base::out) && (writecount = fwrite(s, sizeof(char_type), count, file)) > 0u) {
				ch = s[writecount - 1u];
				setp(&ch, &ch);
			}
			else {
				setp(nullptr, nullptr);
			}
			return writecount;
		}

		// pbackfail can't be implemented with FILE* easily, because the stdio
		// functions don't support everything required to easily implement
		// pbackfail. So it's not implemented for this class.

	private:
		FILE* const file;
		char_type ch;
		const std::ios_base::openmode mode;
	};
#endif

	// When reading and writing the text format, size_t or signed int fields
	// are supported for numeric fields, except for 32-bit character IDs. The
	// text format read function also supports any order of the lines in the
	// file, and any order of fields in a line, returning with an error as soon
	// as the file was found to be internally inconsistent (say, the number of
	// pages in the common line is not the same as the number of page lines) or
	// an error in parsing occurred. When writing in the text format, the
	// output will have lines and fields in the same order as the AngelCode
	// Bitmap Font Generator, but with only one space between fields; the order
	// of char and kerning lines may differ from the AngelCode Bitmap Font
	// Generator, though, but the groups of char and kerning lines will be in
	// the same place as the generator writes. It may also happen that a text
	// format file read will successfully parse even if invalid data is in the
	// file, but all the required data was successfully read (so if a text
	// format font has non-standard fields from another generator, they'll be
	// ignored).
	//
	// When using the binary format for reading and writing, there are lower
	// size restrictions than size_t and signed long, and an error will occur
	// if those restrictions are exceeded when writing. The binary write
	// function checks if all the fields are in the bounds of the binary format
	// first, and doesn't write anything and returns false if a field was found
	// to be outside the bounds. The binary format reads and writes little
	// endian integers, to match the AngelCode Bitmap Font Generator.
	//
	// When reading and writing the XML format, size_t and signed int are used,
	// like in the text format. No guarantee of order of lines in written XML
	// files is given, because the tinyxml2 library's facility to write the XML
	// is used; it's assumed you'd use an XML library that isn't order
	// dependent, or this very same library, which isn't order dependent. If
	// there's extra data in a read XML file that isn't standard, it'll be
	// ignored, and no error will occur, though an error will occur if the file
	// isn't correctly written XML, of course; that extra data will also not be
	// written when a write function is called.

#ifndef PDBMFONT_DECLARE
#define PDBMFONT_DECLARE
	class BMFont {
	public:
		struct KerningHasher {
			std::size_t operator()(const std::pair<unsigned, unsigned>& p) const noexcept;
		};

		struct Char {
			unsigned x;
			unsigned y;
			unsigned width;
			unsigned height;
			int xoffset;
			int yoffset;
			unsigned xadvance;
			unsigned page;
			enum Chnl {
				blue = 1u,
				green = 2u,
				red = 4u,
				alpha = 8u,
				allChannels = 15u
			};
			unsigned chnl;
		};

		struct Info {
			std::string face;
			int size;
			// In the AngelCode format documentation, fixedHeight is defined as
			// a field in the binary format, but not the text format. It's
			// supported by the text and XML functions, though, but only for
			// reading, and defaults to false if not present.
			bool fixedHeight;
			bool bold;
			bool italic;
			std::string charsetText; // Used with the text and XML formats.
			std::uint8_t charsetBinary; // Used with the binary format.
			bool unicode;
			unsigned stretchH;
			bool smooth;
			bool aa;
			unsigned padding[4u];
			unsigned spacing[2u];
			unsigned outline;
		};

		struct Common {
			unsigned lineHeight;
			unsigned base;
			unsigned scaleW;
			unsigned scaleH;
			bool packed;
			enum class Chnl {
				glyph,
				outline,
				glyphOutline,
				zero,
				one
			};
			Chnl alphaChnl;
			Chnl redChnl;
			Chnl greenChnl;
			Chnl blueChnl;
		};

		// Either load a bitmap font to initialize these, or set them based on
		// data from your software, so the font can then be written. There's no
		// default initialization of some fields in info and common, so it's on
		// you to set everything correctly.
		Info info;
		Common common;
		std::vector<std::string> pages;
		std::unordered_map<unsigned, Char> chars;
		std::unordered_map<std::pair<unsigned, unsigned>, int, KerningHasher> kernings;

#if defined(PDBMFONT_TEXT) || defined(PDBMFONT_BINARY) || defined(PDBMFONT_XML)
		enum class Format {
			none,
#ifdef PDBMFONT_TEXT
			text,
#endif
#ifdef PDBMFONT_XML
			xml,
#endif
#ifdef PDBMFONT_BINARY
			binary
#endif
		};
		// Though you could set this externally in your own code, so the
		// write() functions will work, it's intended to be managed by the read
		// functions of BMFont.
		Format format = Format::none;

		// For these functions that automatically handle file type, it's
		// probably best to just open files with the binary open mode set. The
		// functions taking a filename do that.

		// These functions automatically detect the format of the file for
		// reading, returning false if none of the attempts to load the
		// supported file types succeeded. The file must be seekable for format
		// detection to work.
		bool read(std::istream& stream);
		bool read(FILE* const file);
		bool read(const std::string& filename);
		// These functions write in the file type set by the last read function
		// call. All read functions set the appropriate file type, even the
		// format-specific functions (such as readText setting format to
		// Format::text). No writing is performed if the format isn't set to a
		// format that was compiled in, and in that case false is returned.
		bool write(std::ostream& stream);
		bool write(FILE* const file);
		bool write(const std::string& filename);
#endif

#ifdef PDBMFONT_TEXT
		bool readText(std::istream& stream);
		bool readText(FILE* const file);
		bool readText(const std::string& filename);
		bool writeText(std::ostream& stream);
		bool writeText(FILE* const file);
		bool writeText(const std::string& filename);
#endif

#ifdef PDBMFONT_XML
		bool convertFromXMLDocument(tinyxml2::XMLDocument& document, tinyxml2::XMLError error);
		bool readXML(std::istream& stream);
		bool readXML(FILE* const file);
		bool readXML(const std::string& filename);
		void convertToXMLDocument(tinyxml2::XMLDocument& document);
		bool writeXML(FILE* const file);
		bool writeXML(std::ostream& stream);
		bool writeXML(const std::string& filename);
#endif

#ifdef PDBMFONT_BINARY
		// When using an ifstream or FILE* with readBinary, be sure it was
		// created with the binary open mode flag set, or reading may fail
		// unexpectedly; reading almost certainly will fail on Windows, if the
		// binary flag isn't set.
		bool readBinary(std::istream& stream);
		bool readBinary(FILE* const file);
		bool readBinary(const std::string& filename);
		// When using an ofstream or FILE* with writeBinary, be sure it was
		// created with the binary open mode flag set, or writing may write out
		// incorrect data on Windows, or possibly other platforms.
		bool writeBinary(std::ostream& stream);
		bool writeBinary(FILE* const file);
		bool writeBinary(const std::string& filename);
#endif

	private:
		static std::uint8_t toByte(const char data) {
			return static_cast<std::uint8_t>(data);
		}

		static std::int16_t toInt(const char* data) {
			return
				((static_cast<std::int16_t>(data[0]) << 0) & INT16_C(0x00FF)) |
				((static_cast<std::int16_t>(data[1]) << 8) & INT16_C(0xFF00));
		}

		static std::uint16_t toUint(const char* data) {
			return
				((static_cast<std::uint16_t>(data[0]) << 0) & UINT16_C(0x00FF)) |
				((static_cast<std::uint16_t>(data[1]) << 8) & UINT16_C(0xFF00));
		}

		static std::uint32_t toSize(const char* data) {
			std::uint32_t size = UINT32_C(0);

			for (std::uint32_t i = UINT32_C(0); i < sizeof(std::uint32_t); i++) {
				size |= (static_cast<std::uint32_t>(data[i]) & UINT32_C(0xFF)) << (i * UINT32_C(8));
			}
			return size;
		}

		template<typename T>
		static char* toBinField(const T data, char* out) {
			for (T i = T(0); i < sizeof(T); i++) {
				out[i] = (data >> (i * T(8))) & T(0xFF);
			}
			return out;
		}

		template<typename T>
		static void writeBinField(const T data, std::ostream& stream) {
			char outData[sizeof(T)];
			stream.write(toBinField(data, outData), sizeof(T));
		}

#if defined(PDBMFONT_TEXT) || defined(PDBMFONT_BINARY) || defined(PDBMFONT_XML)
		static constexpr bool (BMFont::* const readFunctions[])(std::istream&)
		= {
#ifdef PDBMFONT_TEXT
			&BMFont::readText,
#endif
#ifdef PDBMFONT_BINARY
			&BMFont::readBinary,
#endif
#ifdef PDBMFONT_XML
			&BMFont::readXML
#endif
		};
#endif
	};
#endif

#ifdef PDBMFONT_DEFINE
	std::size_t BMFont::KerningHasher::operator()(const std::pair<unsigned, unsigned>& p) const noexcept {
		if (sizeof(std::size_t) >= 8u) {
			// Guarantees no hash collisions on 64-bit or higher systems.
			// As of the time this library was last updated, Unicode's
			// range isn't even close to the full range of 32 bits.
			return (static_cast<std::size_t>(p.first) << 32u) | p.second;
		}
		else {
			// Should work fine on 32-bit systems.
			return (static_cast<std::size_t>(p.first) << 11u) ^ p.second;
		}
	}

#if defined(PDBMFONT_TEXT) || defined(PDBMFONT_BINARY) || defined(PDBMFONT_XML)
	bool BMFont::read(std::istream& stream){
		for (const auto& readFunction : readFunctions) {
			if ((this->*readFunction)(stream)) {
				return true;
			}
			stream.clear();
			stream.seekg(0, std::ios_base::beg);
		}
		return false;
	}

	bool BMFont::read(FILE* const file){
		CFileStreambuf buf(file, std::ios_base::in);
		std::istream stream(&buf);
		return read(stream);
	}

	bool BMFont::read(const std::string& filename){
		std::ifstream stream(filename, std::ios_base::binary);
		return read(stream);
	}

	bool BMFont::write(std::ostream& stream){
		switch (format) {
		default:
		case Format::none:
			return false;

#ifdef PDBMFONT_TEXT
		case Format::text:
			return writeText(stream);
#endif

#ifdef PDBMFONT_BINARY
		case Format::binary:
			return writeBinary(stream);
#endif

#ifdef PDBMFONT_XML
		case Format::xml:
			return writeXML(stream);
#endif
		}
	}

	bool BMFont::write(FILE* const file){
		CFileStreambuf buf(file, std::ios_base::out);
		std::ostream stream(&buf);
		return write(stream);
	}

	bool BMFont::write(const std::string& filename){
		std::ios_base::openmode mode = std::ios_base::out;
#ifdef PDBMFONT_BINARY
		if (format == Format::binary) {
			mode |= std::ios_base::binary;
		}
#endif
		std::ofstream stream(filename, mode);
		return write(stream);
	}
#endif

#ifdef PDBMFONT_TEXT
	bool BMFont::readText(std::istream& stream) {
		format = Format::none;

		// Default to not fixed height, if the fixedHeight field of info isn't specified.
		// The field isn't written when writing in the text format, because the
		// AngelCode Bitmap Font Generator doesn't write it out. But it is
		// read, and is available in the info.fixedHeight field.
		info.fixedHeight = false;

		// The binary format doesn't use a text field for charset, so just init
		// this to zero when reading in the text format.
		info.charsetBinary = 0u;

		const std::regex tagNameRegex("^\\s*(info|common|page|chars|char|kernings|kerning)\\s+");
		const std::regex fieldRegex("\\s*([a-zA-Z]+)\\s*=\\s*(((0|[1-9u][0-9u]*),(0|[1-9u][0-9u]*),(0|[1-9u][0-9u]*),(0|[1-9u][0-9u]*))|((0|[1-9u][0-9u]*),(0|[1-9u][0-9u]*))|(0|-?[1-9u][0-9u]*)|(\"([^\"]*)\"))\\s*");

		bool infoFound = false;

		bool commonFound = false;

		unsigned charsCount = 0u;

		bool pagesFound = false;
		unsigned pagesCount = 0u;

		bool charsCountFound = false;

		bool kerningsCountFound = false;
		unsigned kerningsCount = 0u;

		std::size_t i = 0u;
		const auto end = std::sregex_iterator();
		for (std::string line; std::getline(stream, line); i++) {
			std::smatch tag;
			if (std::regex_search(line, tag, tagNameRegex)) {
				const std::string tagName = tag[1u];
				if (tagName == "info") {
					if (infoFound) {
						return false;
					}
					bool
						faceFound = false,
						sizeFound = false,
						fixedHeightFound = false,
						boldFound = false,
						italicFound = false,
						charsetFound = false,
						unicodeFound = false,
						stretchHFound = false,
						smoothFound = false,
						aaFound = false,
						paddingFound = false,
						spacingFound = false,
						outlineFound = false;

					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "face") {
							if ((*field)[12u] == "" || faceFound) {
								return false;
							}
							info.face = (*field)[13u];
							faceFound = true;
						}
						else if (fieldName == "size") {
							if ((*field)[11u] == "" || sizeFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> info.size;
							sizeFound = true;
						}
						else if (fieldName == "fixedHeight") {
							if ((*field)[11u] == "" || fixedHeightFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int fixedHeight;
							iss >> fixedHeight;
							info.fixedHeight = !!fixedHeight;
							fixedHeightFound = true;
						}
						else if (fieldName == "bold") {
							if ((*field)[11u] == "" || boldFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int bold;
							iss >> bold;
							info.bold = !!bold;
							boldFound = true;
						}
						else if (fieldName == "italic") {
							if ((*field)[11u] == "" || italicFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int italic;
							iss >> italic;
							info.italic = !!italic;
							italicFound = true;
						}
						else if (fieldName == "charset") {
							if ((*field)[12u] == "" || charsetFound) {
								return false;
							}
							info.charsetText = (*field)[13u];
							charsetFound = true;
						}
						else if (fieldName == "unicode") {
							if ((*field)[11u] == "" || unicodeFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int unicode;
							iss >> unicode;
							info.unicode = !!unicode;
							unicodeFound = true;
						}
						else if (fieldName == "stretchH") {
							if ((*field)[11u] == "" || stretchHFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> info.stretchH;
							stretchHFound = true;
						}
						else if (fieldName == "smooth") {
							if ((*field)[11u] == "" || smoothFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int smooth;
							iss >> smooth;
							info.smooth = !!smooth;
							smoothFound = true;
						}
						else if (fieldName == "aa") {
							if ((*field)[11u] == "" || aaFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int aa;
							iss >> aa;
							info.aa = !!aa;
							aaFound = true;
						}
						else if (fieldName == "padding") {
							if ((*field)[3u] == "" || paddingFound) {
								return false;
							}
							for (unsigned i = 0u; i < 4u; i++) {
								std::istringstream iss((*field)[4u + i]);
								iss >> info.padding[i];
							}
							paddingFound = true;
						}
						else if (fieldName == "spacing") {
							if ((*field)[8u] == "" || spacingFound) {
								return false;
							}
							for (unsigned i = 0u; i < 2u; i++) {
								std::istringstream iss((*field)[9u + i]);
								iss >> info.spacing[i];
							}
							spacingFound = true;
						}
						else if (fieldName == "outline") {
							if ((*field)[11u] == "" || outlineFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> info.outline;
							outlineFound = true;
						}
					}
					if (
						!faceFound ||
						!sizeFound ||
						// fixedHeight isn't defined in the AngelCode text
						// format documentation, so it's not required. But it's
						// supported as an optional field, since it's specified
						// for the binary format. The default is false, if it's
						// not specified.
						!boldFound ||
						!italicFound ||
						!charsetFound ||
						!unicodeFound ||
						!stretchHFound ||
						!smoothFound ||
						!aaFound ||
						!paddingFound ||
						!spacingFound ||
						!outlineFound) {
						return false;
					}
					else {
						infoFound = true;
					}
				}
				else if (tagName == "common") {
					if (commonFound) {
						return false;
					}
					bool
						lineHeightFound = false,
						baseFound = false,
						scaleWFound = false,
						scaleHFound = false,
						packedFound = false,
						alphaChnlFound = false,
						redChnlFound = false,
						greenChnlFound = false,
						blueChnlFound = false;
					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "lineHeight") {
							if ((*field)[11u] == "" || lineHeightFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> common.lineHeight;
							lineHeightFound = true;
						}
						else if (fieldName == "base") {
							if ((*field)[11u] == "" || baseFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> common.base;
							baseFound = true;
						}
						else if (fieldName == "scaleW") {
							if ((*field)[11u] == "" || scaleWFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> common.scaleW;
							scaleWFound = true;
						}
						else if (fieldName == "scaleH") {
							if ((*field)[11u] == "" || scaleHFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> common.scaleH;
							scaleHFound = true;
						}
						else if (fieldName == "pages") {
							if ((*field)[11u] == "" || pagesFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> pagesCount;
							if (pages.size() > pagesCount || pagesCount == 0u) {
								return false;
							}
							pages.resize(pagesCount);
							pagesFound = true;
						}
						else if (fieldName == "packed") {
							if ((*field)[11u] == "" || packedFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							int packed;
							iss >> packed;
							common.packed = !!packed;
							packedFound = true;
						}
						else if (fieldName == "alphaChnl") {
							if ((*field)[11u] == "" || alphaChnlFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							unsigned alphaChnl;
							iss >> alphaChnl;
							if (alphaChnl > 4u) {
								return false;
							}
							common.alphaChnl = static_cast<Common::Chnl>(alphaChnl);
							alphaChnlFound = true;
						}
						else if (fieldName == "redChnl") {
							if ((*field)[11u] == "" || redChnlFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							unsigned redChnl;
							iss >> redChnl;
							if (redChnl > 4u) {
								return false;
							}
							common.redChnl = static_cast<Common::Chnl>(redChnl);
							redChnlFound = true;
						}
						else if (fieldName == "greenChnl") {
							if ((*field)[11u] == "" || greenChnlFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							unsigned greenChnl;
							iss >> greenChnl;
							if (greenChnl > 4u) {
								return false;
							}
							common.greenChnl = static_cast<Common::Chnl>(greenChnl);
							greenChnlFound = true;
						}
						else if (fieldName == "blueChnl") {
							if ((*field)[11u] == "" || blueChnlFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							unsigned blueChnl;
							iss >> blueChnl;
							if (blueChnl > 4u) {
								return false;
							}
							common.blueChnl = static_cast<Common::Chnl>(blueChnl);
							blueChnlFound = true;
						}
					}
					if (
						!lineHeightFound ||
						!baseFound ||
						!scaleWFound ||
						!scaleHFound ||
						!packedFound ||
						!alphaChnlFound ||
						!redChnlFound ||
						!greenChnlFound ||
						!blueChnlFound) {
						return false;
					}
					else {
						commonFound = true;
					}
				}
				else if (tagName == "page") {
					bool
						idFound = false,
						fileFound = false;
					std::size_t id;
					std::string file;
					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "id") {
							if ((*field)[11u] == "" || idFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> id;
							idFound = true;
						}
						else if (fieldName == "file") {
							if ((*field)[12u] == "" || fileFound) {
								return false;
							}
							file = (*field)[13u];
							fileFound = true;
						}
					}
					if (idFound && fileFound) {
						if (pagesFound && id > pagesCount - 1u) {
							return false;
						}
						else if (!pagesFound && id >= pages.size()) {
							pages.resize(id + 1u);
						}
						pages[id] = file;
					}
					else {
						return false;
					}
				}
				else if (tagName == "chars") {
					if (charsCountFound) {
						return false;
					}
					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "count") {
							if ((*field)[11u] == "" || charsCountFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> charsCount;
							charsCountFound = true;
						}
					}
					if (!charsCountFound || charsCount < chars.size()) {
						return false;
					}
				}
				else if (tagName == "char") {
					bool
						idFound = false,
						xFound = false,
						yFound = false,
						widthFound = false,
						heightFound = false,
						xoffsetFound = false,
						yoffsetFound = false,
						xadvanceFound = false,
						pageFound = false,
						chnlFound = false;
					unsigned id;
					Char currentChar;
					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "id") {
							if ((*field)[11u] == "" || idFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> id;
							if (chars.count(id)) {
								return false;
							}
							idFound = true;
						}
						else if (fieldName == "x") {
							if ((*field)[11u] == "" || xFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.x;
							xFound = true;
						}
						else if (fieldName == "y") {
							if ((*field)[11u] == "" || yFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.y;
							yFound = true;
						}
						else if (fieldName == "width") {
							if ((*field)[11u] == "" || widthFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.width;
							widthFound = true;
						}
						else if (fieldName == "height") {
							if ((*field)[11u] == "" || heightFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.height;
							heightFound = true;
						}
						else if (fieldName == "xoffset") {
							if ((*field)[11u] == "" || xoffsetFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.xoffset;
							xoffsetFound = true;
						}
						else if (fieldName == "yoffset") {
							if ((*field)[11u] == "" || yoffsetFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.yoffset;
							yoffsetFound = true;
						}
						else if (fieldName == "xadvance") {
							if ((*field)[11u] == "" || xadvanceFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.xadvance;
							xadvanceFound = true;
						}
						else if (fieldName == "page") {
							if ((*field)[11u] == "" || pageFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.page;
							if (pagesFound && currentChar.page > pagesCount - 1) {
								return false;
							}
							pageFound = true;
						}
						else if (fieldName == "chnl") {
							if ((*field)[11u] == "" || chnlFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> currentChar.chnl;
							if ((currentChar.chnl & Char::allChannels) != currentChar.chnl) {
								return false;
							}
							chnlFound = true;
						}
					}
					if (
						!idFound ||
						!xFound ||
						!yFound ||
						!widthFound ||
						!heightFound ||
						!xoffsetFound ||
						!yoffsetFound ||
						!xadvanceFound ||
						!pageFound ||
						!chnlFound) {
						return false;
					}
					else {
						chars[id] = currentChar;
					}
				}
				else if (tagName == "kernings") {
					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "count") {
							if ((*field)[11u] == "" || kerningsCountFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> kerningsCount;
							if (kerningsCount < kernings.size()) {
								return false;
							}
							kerningsCountFound = true;
						}
					}
					if (!kerningsCountFound) {
						return false;
					}
				}
				else if (tagName == "kerning") {
					unsigned first, second;
					long amount;
					bool
						firstFound = false,
						secondFound = false,
						amountFound = false;
					for (auto field = std::sregex_iterator(line.begin(), line.end(), fieldRegex); field != end; field++) {
						const std::string fieldName = (*field)[1u];
						if (fieldName == "") {
							return false;
						}
						else if (fieldName == "first") {
							if ((*field)[11u] == "" || firstFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> first;
							firstFound = true;
						}
						else if (fieldName == "second") {
							if ((*field)[11u] == "" || secondFound) {
								return false;
							}
							std::istringstream iss((*field)[11u]);
							iss >> second;
							secondFound = true;
						}
						else if (fieldName == "amount" || amountFound) {
							if ((*field)[11u] == "") {
								return false;
							}
							amountFound = true;
							std::istringstream iss((*field)[11u]);
							iss >> amount;
							amountFound = true;
						}
					}
					if (
						!firstFound ||
						!secondFound ||
						!amountFound ||
						(kerningsCountFound && kerningsCount < kernings.size() + 1)) {
						return false;
					}
					else {
						// The AngelCode BMFont documentation says there can be
						// duplicate kernings with some fonts, so the last
						// kerning in the file for a given pair will end up
						// being used, rather than giving an error there was a
						// duplicate.
						kernings[{first, second}] = amount;
					}
				}
			}
			else {
				return false;
			}
		}

		// Since it seems fine if there's no kernings count line, nor kernings,
		// in a font, don't fail if the kernings count is omitted. The kernings
		// count is initialized to zero, and the kernings map's size will be
		// zero, if no kernings count nor kerning lines are present. But all
		// other line types have to be present.
		const bool success =
			infoFound &&
			commonFound &&
			charsCountFound &&
			pagesCount == pages.size() &&
			charsCount == chars.size() &&
			kerningsCount == kernings.size();
		if (success) {
			format = Format::text;
		}
		return success;
	}

	bool BMFont::readText(FILE* const file) {
		CFileStreambuf buf(file, std::ios_base::in);
		std::istream stream(&buf);
		return readText(stream);
	}

	bool BMFont::readText(const std::string& filename) {
		std::ifstream stream(filename);
		return readText(stream);
	}

	bool BMFont::writeText(std::ostream& stream) {
		stream << "info" <<
			" face=" << '"' << info.face << '"' <<
			" size=" << info.size <<
			" bold=" << static_cast<int>(info.bold) <<
			" italic=" << static_cast<int>(info.italic) <<
			" charset=" << '"' << info.charsetText << '"' <<
			" unicode=" << static_cast<int>(info.unicode) <<
			" stretchH=" << info.stretchH <<
			" smooth=" << static_cast<int>(info.smooth) <<
			" aa=" << static_cast<int>(info.aa) <<
			" padding=" <<
				static_cast<unsigned>(info.padding[0u]) << "," <<
				static_cast<unsigned>(info.padding[1u]) << "," <<
				static_cast<unsigned>(info.padding[2u]) << "," <<
				static_cast<unsigned>(info.padding[3u]) <<
			" spacing=" <<
				static_cast<unsigned>(info.spacing[0u]) << "," <<
				static_cast<unsigned>(info.spacing[1u]) <<
			" outline=" << static_cast<unsigned>(info.outline) <<
			'\n';

		stream << "common" <<
			" lineHeight=" << common.lineHeight <<
			" base=" << common.base <<
			" scaleW=" << common.scaleW <<
			" scaleH=" << common.scaleH <<
			" pages=" << pages.size() <<
			" packed=" << static_cast<int>(common.packed) <<
			" alphaChnl=" << static_cast<unsigned>(common.alphaChnl) <<
			" redChnl=" << static_cast<unsigned>(common.redChnl) <<
			" greenChnl=" << static_cast<unsigned>(common.greenChnl) <<
			" blueChnl=" << static_cast<unsigned>(common.blueChnl) <<
			'\n';

		for (std::size_t p = 0u; p < pages.size(); p++) {
			stream << "page" <<
				" id=" << p <<
				" file=" << '"' << pages[p] << '"' <<
				'\n';
		}

		stream << "chars" <<
			" count=" << chars.size() <<
			'\n';

		for (auto& c : chars) {
			stream << "char" <<
				" id=" << c.first <<
				" x=" << c.second.x <<
				" y=" << c.second.y <<
				" width=" << c.second.width <<
				" height=" << c.second.height <<
				" xoffset=" << c.second.xoffset <<
				" yoffset=" << c.second.yoffset <<
				" xadvance=" << c.second.xadvance <<
				" page=" << static_cast<unsigned>(c.second.page) <<
				" chnl=" << static_cast<unsigned>(c.second.chnl) <<
				'\n';
		}

		stream << "kernings" <<
			" count=" << kernings.size() <<
			'\n';

		for (auto& k : kernings) {
			stream << "kerning" <<
				" first=" << k.first.first <<
				" second=" << k.first.second <<
				" amount=" << k.second <<
				'\n';
		}

		stream.flush();
		return true;
	}

	bool BMFont::writeText(FILE* const file) {
		CFileStreambuf buf(file, std::ios_base::out);
		std::ostream stream(&buf);
		return writeText(stream);
	}

	bool BMFont::writeText(const std::string& filename) {
		std::ofstream stream(filename);
		return writeText(stream);
	}
#endif

#ifdef PDBMFONT_BINARY
	bool BMFont::readBinary(std::istream& stream) {
		format = Format::none;

		// Version 3 format check
		char fileID[4];
		stream.read(fileID, sizeof(fileID));
		if (stream.fail() || fileID[0] != 'B' || fileID[1] != 'M' || fileID[2] != 'F' || fileID[3] != '\x3') {
			return false;
		}

		// Block type 1: info
		if (stream.get() != '\x01') {
			return false;
		}
		char infoSizeData[4];
		stream.read(infoSizeData, sizeof(infoSizeData));
		if (!stream.good()) {
			return false;
		}
		const std::uint32_t infoSize = toSize(infoSizeData);
		if (infoSize < 15u) {
			return false;
		}
		char* const infoData = new char[infoSize];
		stream.read(infoData, infoSize);
		if (!stream.good()) {
			delete[] infoData;
			return false;
		}
		constexpr std::size_t fontSize = 0u;
		constexpr std::size_t infoBitField = 2u;
		constexpr std::size_t charSet = 3u;
		constexpr std::size_t stretchH = 4u;
		constexpr std::size_t aa = 6u;
		constexpr std::size_t padding = 7u;
		constexpr std::size_t spacing = 11u;
		constexpr std::size_t outline = 13u;
		constexpr std::size_t fontName = 14u;
		info.size = toInt(&infoData[fontSize]);
		info.smooth = !!(infoData[infoBitField] & (1 << 0));
		info.unicode = !!(infoData[infoBitField] & (1 << 1));
		info.italic = !!(infoData[infoBitField] & (1 << 2));
		info.bold = !!(infoData[infoBitField] & (1 << 3));
		info.fixedHeight = !!(infoData[infoBitField] & (1 << 4));
		info.charsetBinary = toByte(infoData[charSet]);
		info.stretchH = toUint(&infoData[stretchH]);
		info.aa = !!infoData[aa];
		for (std::size_t i = 0u; i < 4u; i++) {
			info.padding[i] = toByte(infoData[padding + i]);
		}
		for (std::size_t i = 0u; i < 2u; i++) {
			info.spacing[i] = toByte(infoData[spacing + i]);
		}
		info.outline = toByte(infoData[padding]);
		info.face = &infoData[fontName];
		delete[] infoData;

		// Block type 2: common
		if (stream.get() != '\x02') {
			return false;
		}
		char commonSizeData[4];
		stream.read(commonSizeData, sizeof(commonSizeData));
		if (!stream.good()) {
			return false;
		}
		const std::uint32_t commonSize = toSize(commonSizeData);
		if (commonSize != 15u) {
			return false;
		}
		char commonData[15];
		stream.read(commonData, sizeof(commonData));
		if (!stream.good()) {
			return false;
		}
		constexpr std::size_t lineHeight = 0u;
		constexpr std::size_t base = 2u;
		constexpr std::size_t scaleW = 4u;
		constexpr std::size_t scaleH = 6u;
		constexpr std::size_t pagesCount = 8u;
		constexpr std::size_t commonBitField = 10u;
		constexpr std::size_t alphaChnl = 11u;
		constexpr std::size_t redChnl = 12u;
		constexpr std::size_t greenChnl = 13u;
		constexpr std::size_t blueChnl = 14u;
		common.lineHeight = toUint(&commonData[lineHeight]);
		common.base = toUint(&commonData[base]);
		common.scaleW = toUint(&commonData[scaleW]);
		common.scaleH = toUint(&commonData[scaleH]);
		pages.resize(toUint(&commonData[pagesCount]));
		common.packed = !!(commonData[commonBitField] & 0x80);
		if (
			commonData[alphaChnl] < 0 || commonData[alphaChnl] > 4 ||
			commonData[redChnl] < 0 || commonData[redChnl] > 4 ||
			commonData[greenChnl] < 0 || commonData[greenChnl] > 4 ||
			commonData[blueChnl] < 0 || commonData[blueChnl] > 4) {
			return false;
		}
		common.alphaChnl = static_cast<Common::Chnl>(commonData[alphaChnl]);
		common.redChnl = static_cast<Common::Chnl>(commonData[redChnl]);
		common.greenChnl = static_cast<Common::Chnl>(commonData[greenChnl]);
		common.blueChnl = static_cast<Common::Chnl>(commonData[blueChnl]);

		// Block type 3: pages
		if (stream.get() != '\x3') {
			return false;
		}
		char pagesSizeData[4];
		stream.read(pagesSizeData, sizeof(pagesSizeData));
		if (!stream.good()) {
			return false;
		}
		const std::uint32_t pagesSize = toSize(pagesSizeData);
		char* const pagesData = new char[pagesSize];
		stream.read(pagesData, pagesSize);
		if (!stream.good() || (pagesSize / pages.size()) * pages.size() != pagesSize) {
			delete[] pagesData;
			return false;
		}
		char* pageData = pagesData;
		const std::size_t pageSize = pagesSize / pages.size();
		for (auto& page : pages) {
			page = pageData;
			pageData += pageSize;
		}
		delete[] pagesData;

		// Block 4: chars
		if (stream.get() != '\x4') {
			return false;
		}
		char charsSizeData[4];
		stream.read(charsSizeData, sizeof(charsSizeData));
		if (!stream.good()) {
			return false;
		}
		const std::uint32_t charsSize = toSize(charsSizeData);
		if (charsSize % 20u) {
			return false;
		}
		if (charsSize > 0u) {
			char* const charsData = new char[charsSize];
			stream.read(charsData, charsSize);
			if (stream.gcount() != charsSize) {
				return false;
			}
			constexpr std::size_t id = 0u;
			constexpr std::size_t x = 4u;
			constexpr std::size_t y = 6u;
			constexpr std::size_t width = 8u;
			constexpr std::size_t height = 10u;
			constexpr std::size_t xoffset = 12u;
			constexpr std::size_t yoffset = 14u;
			constexpr std::size_t xadvance = 16u;
			constexpr std::size_t page = 18u;
			constexpr std::size_t chnl = 19u;
			for (char* charData = charsData; charData < charsData + charsSize; charData += 20u) {
				Char c;
				c.x = toUint(&charData[x]);
				c.y = toUint(&charData[y]);
				c.width = toUint(&charData[width]);
				c.height = toUint(&charData[height]);
				c.xoffset = toInt(&charData[xoffset]);
				c.yoffset = toInt(&charData[yoffset]);
				c.xadvance = toInt(&charData[xadvance]);
				c.page = toByte(charData[page]);
				if ((charData[chnl] & Char::allChannels) != charData[chnl]) {
					return false;
				}
				c.chnl = toByte(charData[chnl]);
				chars[toSize(&charData[id])] = c;
			}
			delete[] charsData;
		}

		if (!stream.good()) {
			// Kernings are optional, so return as successful without reading
			// any when they're not present.
			kernings.clear();
			return true;
		}

		// Block type 5: kerning pairs
		if (stream.get() != '\x5') {
			return false;
		}
		char kerningsSizeData[4];
		stream.read(kerningsSizeData, sizeof(kerningsSizeData));
		if (!stream.good()) {
			return false;
		}
		const std::uint32_t kerningsSize = toSize(kerningsSizeData);
		if (kerningsSize % 10u) {
			return false;
		}
		char* const kerningsData = new char[kerningsSize];
		constexpr std::size_t first = 0u;
		constexpr std::size_t second = 4u;
		constexpr std::size_t amount = 8u;
		stream.read(kerningsData, kerningsSize);
		if (stream.gcount() != kerningsSize) {
			return false;
		}
		for (char* kerningData = kerningsData; kerningData < kerningsData + kerningsSize; kerningData += 10u) {
			kernings[{toSize(&kerningData[first]), toSize(&kerningData[second])}] = toInt(&kerningData[amount]);
		}

		format = Format::binary;
		return true;
	}

	bool BMFont::readBinary(FILE* const file) {
		CFileStreambuf buf(file, std::ios_base::in);
		std::istream stream(&buf);
		return readBinary(stream);
	}

	bool BMFont::readBinary(const std::string& filename) {
		std::ifstream stream(filename, std::ios::binary);
		return readBinary(stream);
	}

	bool BMFont::writeBinary(std::ostream& stream) {
		// First, check all data is within the bounds of the binary format.

		if (
			// Block type 1: info
			info.size > INT16_MAX || info.size < INT16_MIN ||
			info.stretchH > UINT16_MAX ||
			info.padding[0] > UINT8_MAX || info.padding[1] > UINT8_MAX || info.padding[2] > UINT8_MAX || info.padding[3] > UINT8_MAX ||
			info.spacing[0] > UINT8_MAX || info.spacing[1] > UINT8_MAX ||
			info.outline > UINT8_MAX ||

			// Block type 2: common
			common.lineHeight > UINT16_MAX ||
			common.base > UINT16_MAX ||
			common.scaleW > UINT16_MAX ||
			common.scaleH > UINT16_MAX ||
			pages.size() > UINT16_MAX ||
			common.alphaChnl < Common::Chnl::glyph || common.alphaChnl > Common::Chnl::one ||
			common.redChnl < Common::Chnl::glyph || common.redChnl > Common::Chnl::one ||
			common.greenChnl < Common::Chnl::glyph || common.greenChnl > Common::Chnl::one ||
			common.blueChnl < Common::Chnl::glyph || common.blueChnl > Common::Chnl::one ||

			// Block type 4: chars
			chars.size() > UINT32_MAX ||

			// Block type 5: kerning pairs
			kernings.size() > UINT32_MAX
			) {
			return false;
		}
		else {
			for (auto& c : chars) {
				if (
					c.second.x > UINT16_MAX ||
					c.second.y > UINT16_MAX ||
					c.second.width > UINT16_MAX ||
					c.second.height > UINT16_MAX ||
					c.second.xoffset > INT16_MAX || c.second.xoffset < INT16_MIN ||
					c.second.page > UINT8_MAX ||
					(c.second.chnl & ~0xF)) {
					return false;
				}
			}

			for (auto& k : kernings) {
				if (k.second > INT16_MAX || k.second < INT16_MIN) {
					return false;
				}
			}
		}

		// Everything checks out, so just write out now.
		const char header[4] = { 'B', 'M', 'F', '\x03' };
		stream.write(header, sizeof(header));

		// Block type 1: info
		stream.put('\x01');
		writeBinField(uint32_t(14u + info.face.size() + 1u), stream);

		writeBinField(int16_t(info.size), stream);
		stream.put(0xC0 | (info.smooth << 0) | (info.unicode << 1) | (info.italic << 2) | (info.bold << 3) | (info.fixedHeight << 4));
		stream.put(info.charsetBinary);
		writeBinField(uint16_t(info.stretchH), stream);
		stream.put(info.aa);
		for (std::size_t i = 0u; i < 4u; i++) {
			stream.put(uint8_t(info.padding[i]));
		}
		for (std::size_t i = 0u; i < 2u; i++) {
			stream.put(uint8_t(info.spacing[i]));
		}
		stream.put(uint8_t(info.outline));
		stream.write(info.face.c_str(), info.face.size() + 1u);

		// Block type 2: common
		stream.put('\x02');
		writeBinField(uint32_t(15u), stream);

		writeBinField(uint16_t(common.lineHeight), stream);
		writeBinField(uint16_t(common.base), stream);
		writeBinField(uint16_t(common.scaleW), stream);
		writeBinField(uint16_t(common.scaleH), stream);
		writeBinField(uint16_t(pages.size()), stream);
		stream.put(uint8_t(common.packed) << 7);
		stream.put(uint8_t(common.alphaChnl));
		stream.put(uint8_t(common.redChnl));
		stream.put(uint8_t(common.greenChnl));
		stream.put(uint8_t(common.blueChnl));

		// Block type 3: pages
		stream.put('\x03');
		std::size_t pageDataSize = 0u;
		for (auto& page : pages) {
			if (page.size() + 1u > pageDataSize) {
				pageDataSize = page.size() + 1u;
			}
		}
		writeBinField(uint32_t(pageDataSize * pages.size()), stream);

		char* const pagesData = new char[pageDataSize * pages.size()];
		memset(pagesData, 0, pageDataSize * pages.size());
		char* pageData = pagesData;
		for (const auto& page : pages) {
			memcpy(pageData, page.c_str(), page.size() + 1u);
			pageData += pageDataSize;
		}
		stream.write(pagesData, pageDataSize * pages.size());
		delete[] pagesData;

		// Block type 4: chars
		stream.put('\x04');
		writeBinField(uint32_t(chars.size() * 20u), stream);

		for (const auto& c : chars) {
			writeBinField(uint32_t(c.first), stream);
			writeBinField(uint16_t(c.second.x), stream);
			writeBinField(uint16_t(c.second.y), stream);
			writeBinField(uint16_t(c.second.width), stream);
			writeBinField(uint16_t(c.second.height), stream);
			writeBinField(int16_t(c.second.xoffset), stream);
			writeBinField(int16_t(c.second.yoffset), stream);
			writeBinField(int16_t(c.second.xadvance), stream);
			writeBinField(uint8_t(c.second.page), stream);
			writeBinField(uint8_t(c.second.chnl), stream);
		}

		// Block type 5: kerning pairs
		// Kerning pairs are optional, and nothing is written for them if they're absent.
		if (kernings.size() == 0u) {
			return true;
		}
		else {
			// The format specification also says no kernings are written if
			// all the kernings' amounts are 0. In practice, this will probably
			// never be true.
			std::size_t numZeroKernings = 0u;
			for (auto& k : kernings) {
				if (k.second == 0) {
					numZeroKernings++;
				}
			}
			if (numZeroKernings == kernings.size()) {
				return true;
			}
		}
		stream.put('\x05');
		writeBinField(uint32_t(kernings.size() * 10u), stream);

		for (const auto& k : kernings) {
			writeBinField(k.first.first, stream);
			writeBinField(k.first.second, stream);
			writeBinField(int16_t(k.second), stream);
		}

		return true;
	}

	bool BMFont::writeBinary(FILE* const file) {
		CFileStreambuf buf(file, std::ios_base::out);
		std::ostream stream(&buf);
		return writeBinary(stream);
	}

	bool BMFont::writeBinary(const std::string& filename) {
		std::ofstream stream(filename, std::ios::binary);
		return writeBinary(stream);
	}
#endif

#ifdef PDBMFONT_XML
	#define QUERYATTRIBUTE(e, s, m, x) \
	if (e->QueryAttribute(#m, &s.m) != tinyxml2::XML_SUCCESS || (x)) { \
		return false; \
	}
	#define QUERYATTRIBUTETEMP(e, s, m, t, x) \
	if (e->QueryAttribute(#m, &t) != tinyxml2::XML_SUCCESS || (x)) { \
		return false; \
	} \
	s.m = static_cast<decltype(s.m)>(t)

	bool BMFont::convertFromXMLDocument(tinyxml2::XMLDocument& document, tinyxml2::XMLError error) {
		if (error != tinyxml2::XML_SUCCESS) {
			return false;
		}

		const auto fontElement = document.FirstChildElement("font");
		if (!fontElement) {
			return false;
		}

		const auto infoElement = fontElement->FirstChildElement("info");
		if (!infoElement) {
			return false;
		}
		auto face = infoElement->Attribute("face");
		if (!face) {
			return false;
		}
		info.face = face;
		QUERYATTRIBUTE(infoElement, info, size, false );
		int temp;
		QUERYATTRIBUTETEMP(infoElement, info, bold, temp, false);
		QUERYATTRIBUTETEMP(infoElement, info, bold, temp, false);
		const char* charset;
		if (!(charset = infoElement->Attribute("charset"))) {
			return false;
		}
		info.charsetText = charset;
		QUERYATTRIBUTETEMP(infoElement, info, unicode, temp, false );
		QUERYATTRIBUTE(infoElement, info, stretchH, false );
		QUERYATTRIBUTETEMP(infoElement, info, smooth, temp, false);
		QUERYATTRIBUTETEMP(infoElement, info, aa, temp, false);
		const char* paddingAttribute;
		if (!(paddingAttribute = infoElement->Attribute("padding"))) {
			return false;
		}
		const std::string padding = paddingAttribute;
		const std::regex paddingRegex("^(0|[1-9][0-9]*),(0|[1-9][0-9]*),(0|[1-9][0-9]*),(0|[1-9][0-9]*)$");
		std::smatch paddingMatches;
		if (!std::regex_match(padding, paddingMatches, paddingRegex)) {
			return false;
		}
		for (std::size_t i = 1u; i < paddingMatches.size(); i++) {
			std::istringstream stream(paddingMatches[i]);
			stream >> info.padding[i - 1u];
		}
		std::string spacing;
		const char* spacingAttribute;
		if (!(spacingAttribute = infoElement->Attribute("spacing"))) {
			return false;
		}
		spacing = spacingAttribute;
		const std::regex spacingRegex("^(0|[1-9][0-9]*),(0|[1-9][0-9]*)$");
		std::smatch spacingMatches;
		if (!std::regex_match(spacing, spacingMatches, spacingRegex)) {
			return false;
		}
		for (std::size_t i = 1u; i < spacingMatches.size(); i++) {
			std::istringstream stream(spacingMatches[i]);
			stream >> info.spacing[i - 1u];
		}
		QUERYATTRIBUTE(infoElement, info, outline, false);
		if (infoElement->QueryAttribute("fixedHeight", &temp) != tinyxml2::XML_SUCCESS) {
			// fixedHeight is optional, as the AngelCode documentation only
			// specifies it for the binary format. So default to false, without
			// an error, if it's absent.
			info.fixedHeight = false;
		}
		else {
			info.fixedHeight = temp;
		}

		const auto commonElement = fontElement->FirstChildElement("common");
		if (!commonElement) {
			return false;
		}
		QUERYATTRIBUTE(commonElement, common, lineHeight, false);
		QUERYATTRIBUTE(commonElement, common, base, false);
		QUERYATTRIBUTE(commonElement, common, scaleW, false);
		QUERYATTRIBUTE(commonElement, common, scaleH, false);
		unsigned pagesCount;
		if (commonElement->QueryAttribute("pages", &pagesCount) != tinyxml2::XML_SUCCESS || pagesCount == 0u) {
			return false;
		}
		pages.resize(pagesCount);
		QUERYATTRIBUTETEMP(commonElement, common, packed, temp, false);
		QUERYATTRIBUTETEMP(commonElement, common, alphaChnl, temp, temp > 4u);
		QUERYATTRIBUTETEMP(commonElement, common, redChnl, temp, temp > 4u);
		QUERYATTRIBUTETEMP(commonElement, common, greenChnl, temp, temp > 4u);
		QUERYATTRIBUTETEMP(commonElement, common, blueChnl, temp, temp > 4u);

		const auto pagesElement = fontElement->FirstChildElement("pages");
		if (!pagesElement) {
			return false;
		}
		std::size_t pagesFoundCount = 0u;
		for (auto pageElement = pagesElement->FirstChildElement("page"); pageElement; pagesFoundCount++, pageElement = pageElement->NextSiblingElement("page")) {
			unsigned id;
			if (pageElement->QueryAttribute("id", &id) != tinyxml2::XML_SUCCESS) {
				return false;
			}
			auto file = pageElement->Attribute("file");
			if (!file || file == std::string("")) {
				return false;
			}

			if (id >= pagesCount) {
				return false;
			}
			pages[id] = file;
		}
		if (pagesFoundCount != pagesCount) {
			return false;
		}

		const auto charsElement = fontElement->FirstChildElement("chars");
		if (!charsElement) {
			return false;
		}
		unsigned charsCount;
		if (charsElement->QueryAttribute("count", &charsCount) != tinyxml2::XML_SUCCESS) {
			return false;
		}
		for (auto charElement = charsElement->FirstChildElement("char"); charElement; charElement = charElement->NextSiblingElement("char")) {
			uint32_t id;
			// 0x10FFFF is the maximum Unicode code point. But just let any ID
			// pass, if it's not Unicode.
			if (charElement->QueryAttribute("id", &id) != tinyxml2::XML_SUCCESS || (info.unicode && id > 0x10FFFFu)) {
				return false;
			}

			Char c;
			QUERYATTRIBUTE(charElement, c, x, false);
			QUERYATTRIBUTE(charElement, c, y, false);
			QUERYATTRIBUTE(charElement, c, width, false);
			QUERYATTRIBUTE(charElement, c, height, false);
			QUERYATTRIBUTE(charElement, c, xoffset, false);
			QUERYATTRIBUTE(charElement, c, yoffset, false);
			QUERYATTRIBUTE(charElement, c, xadvance, false);
			QUERYATTRIBUTE(charElement, c, page, false);
			QUERYATTRIBUTE(charElement, c, chnl, c.chnl & ~Char::Chnl::allChannels);

			chars[id] = c;
		}
		if (chars.size() != charsCount) {
			return false;
		}

		const auto kerningsElement = fontElement->FirstChildElement("kernings");
		if (!kerningsElement) {
			return true;
		}
		unsigned kerningsCount;
		if (kerningsElement->QueryAttribute("count", &kerningsCount) != tinyxml2::XML_SUCCESS) {
			return false;
		}
		for (auto kerningElement = kerningsElement->FirstChildElement("kerning"); kerningElement; kerningElement = kerningElement->NextSiblingElement("kerning")) {
			struct {
				unsigned first, second;
				int amount;
			} kerning;
			QUERYATTRIBUTE(kerningElement, kerning, first, false);
			QUERYATTRIBUTE(kerningElement, kerning, second, false);
			QUERYATTRIBUTE(kerningElement, kerning, amount, false);
			kernings[{ kerning.first, kerning.second }] = kerning.amount;
		}
		if (kernings.size() != kerningsCount) {
			return false;
		}

		return true;
	}
	#undef QUERYATTRIBUTETEMP
	#undef QUERYATTRIBUTE

	bool BMFont::readXML(std::istream& stream) {
		stream.ignore(std::numeric_limits<std::streamsize>::max());
		auto size = stream.gcount();
		stream.clear();
		stream.seekg(0, std::ios_base::beg);
		auto text = new char[size + 1u];
		stream.read(text, size);
		text[size] = '\0';
		tinyxml2::XMLDocument document;
		auto error = document.Parse(text, size);
		delete[] text;

		if (convertFromXMLDocument(document, error)) {
			format = Format::xml;
			return true;
		}
		else {
			format = Format::none;
			return false;
		}
	}

	bool BMFont::readXML(FILE* const file) {
		tinyxml2::XMLDocument document;
		auto error = document.LoadFile(file);

		if (convertFromXMLDocument(document, error)) {
			format = Format::xml;
			return true;
		}
		else {
			format = Format::none;
			return false;
		}
	}

	bool BMFont::readXML(const std::string& filename) {
		tinyxml2::XMLDocument document;
		auto error = document.LoadFile(filename.c_str());

		if (convertFromXMLDocument(document, error)) {
			format = Format::xml;
			return true;
		}
		else {
			format = Format::none;
			return false;
		}
	}

	void BMFont::convertToXMLDocument(tinyxml2::XMLDocument& document) {
		std::ostringstream stream;
		document.InsertFirstChild(document.NewDeclaration("xml version=\"1.0\""));

		const auto fontElement = document.NewElement("font");
		document.InsertEndChild(fontElement);

		const auto infoElement = fontElement->InsertNewChildElement("info");
		infoElement->SetAttribute("face", info.face.c_str());
		infoElement->SetAttribute("size", info.size);
		infoElement->SetAttribute("bold", static_cast<unsigned>(info.bold));
		infoElement->SetAttribute("italic", static_cast<unsigned>(info.italic));
		infoElement->SetAttribute("charset", info.charsetText.c_str());
		infoElement->SetAttribute("unicode", static_cast<unsigned>(info.unicode));
		infoElement->SetAttribute("stretchH", info.stretchH);
		infoElement->SetAttribute("smooth", static_cast<unsigned>(info.smooth));
		infoElement->SetAttribute("aa", static_cast<unsigned>(info.aa));
		stream.str("");
		stream <<
			info.padding[0] << "," <<
			info.padding[1] << "," <<
			info.padding[2] << "," <<
			info.padding[3];
		infoElement->SetAttribute("padding", stream.str().c_str());
		stream.str("");
		stream <<
			info.spacing[0] << "," <<
			info.spacing[1];
		infoElement->SetAttribute("spacing", stream.str().c_str());
		infoElement->SetAttribute("outline", info.outline);

		const auto commonElement = fontElement->InsertNewChildElement("common");
		commonElement->SetAttribute("lineHeight", common.lineHeight);
		commonElement->SetAttribute("base", common.base);
		commonElement->SetAttribute("scaleW", common.scaleW);
		commonElement->SetAttribute("scaleH", common.scaleH);
		commonElement->SetAttribute("pages", static_cast<unsigned>(pages.size()));
		commonElement->SetAttribute("packed", static_cast<unsigned>(common.packed));
		commonElement->SetAttribute("alphaChnl", static_cast<unsigned>(common.alphaChnl));
		commonElement->SetAttribute("redChnl", static_cast<unsigned>(common.redChnl));
		commonElement->SetAttribute("greenChnl", static_cast<unsigned>(common.greenChnl));
		commonElement->SetAttribute("blueChnl", static_cast<unsigned>(common.blueChnl));

		const auto pagesElement = fontElement->InsertNewChildElement("pages");
		for (unsigned id = 0u; id < pages.size(); id++) {
			const auto pageElement = pagesElement->InsertNewChildElement("page");
			pageElement->SetAttribute("id", id);
			pageElement->SetAttribute("file", pages[id].c_str());
		}

		const auto charsElement = fontElement->InsertNewChildElement("chars");
		charsElement->SetAttribute("count", static_cast<unsigned>(chars.size()));
		for (auto& c : chars) {
			const auto charElement = charsElement->InsertNewChildElement("char");
			charElement->SetAttribute("id", c.first);
			charElement->SetAttribute("x", c.second.x);
			charElement->SetAttribute("y", c.second.y);
			charElement->SetAttribute("width", c.second.width);
			charElement->SetAttribute("height", c.second.height);
			charElement->SetAttribute("xoffset", c.second.xoffset);
			charElement->SetAttribute("yoffset", c.second.yoffset);
			charElement->SetAttribute("xadvance", c.second.xadvance);
			charElement->SetAttribute("page", c.second.page);
			charElement->SetAttribute("chnl", c.second.chnl);
		}

		if (kernings.size()) {
			const auto kerningsElement = fontElement->InsertNewChildElement("kernings");
			kerningsElement->SetAttribute("count", static_cast<unsigned>(kernings.size()));
			for (auto& k : kernings) {
				const auto kerningElement = kerningsElement->InsertNewChildElement("kerning");
				kerningElement->SetAttribute("first", k.first.first);
				kerningElement->SetAttribute("second", k.first.second);
				kerningElement->SetAttribute("amount", k.second);
			}
		}
	}

	bool BMFont::writeXML(FILE* const file) {
		tinyxml2::XMLDocument document;
		convertToXMLDocument(document);
		return document.SaveFile(file) == tinyxml2::XML_SUCCESS;
	}

	bool BMFont::writeXML(std::ostream& stream) {
		tinyxml2::XMLDocument document;
		tinyxml2::XMLPrinter printer;
		convertToXMLDocument(document);
		document.Print(&printer);
		return !stream.write(printer.CStr(), printer.CStrSize()).bad();
	}

	bool BMFont::writeXML(const std::string& filename) {
		tinyxml2::XMLDocument document;
		convertToXMLDocument(document);
		return document.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS;
	}
#endif
#endif
}
#endif
