#include "text.h"
#include <iostream>

/** class LineReader: implement **/
LineReader::LineReader(const std::string & filename) : in(filename.c_str(), std::ios::in) {
	if (!in) {
		std::cerr << "Invalid filename: \"" << filename << "\"" << std::endl;
		exit(1);
	}
	else {
		line = new std::string();
		this->roll();
	}
}
LineReader::~LineReader() {
	in.close();
	if (line != nullptr) delete line;
	line = nullptr;
}
bool LineReader::hasNext() {
	return line != nullptr;
}
std::string LineReader::next() {
	std::string ret = *line;
	this->roll();
	return ret;
}
void LineReader::roll() {
	*line = "";
	if (!in.eof()) {
		in.getline(buff, MaximumLineCharacters);
		*line = buff;
	}
	else {
		delete line; line = nullptr;
	}
}

/** class TextBuild: implement **/
TextBuild::TextBuild(LineReader &r) : reader(r), text(), lines() {
	lines.push_back(text.length());
	while (reader.hasNext()) {
		std::string line = reader.next();

		text += line;
		text += "\n";
		lines.push_back(text.length());
	}
}
TextBuild::~TextBuild() {}
const std::string & TextBuild::getText() const { return text; }
int TextBuild::numberOfCharacters() const { return text.length(); }
int TextBuild::numberOfLines() const { return lines.size() - 1; }
int TextBuild::indexOfLine(int line) const {
	if (line <= 0 || line >= lines.size())
		return -1;
	else return lines[line - 1];
}
int TextBuild::lineOfIndex(int index) const {
	if (index < 0 || index >= text.length()) return -1;
	else {
		// declarations
		int beg, mid, end, head, tail;
		beg = 0; end = lines.size() - 2;

		while (beg <= end) {
			mid = (beg + end) / 2;

			head = lines[mid];
			tail = lines[mid + 1];

			if (index >= head && index < tail)
				return mid + 1;
			else if (index < head)
				end = mid - 1;
			else
				beg = mid + 1;
		} /** end while binary search **/

		return -1;
	}
}