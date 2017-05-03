#pragma once
/**
* File: text.h
* 	-Aim: to provide interfaces for access to file text
* 	-Cls:
* 		[1] class LineReader;
* 		[2] class TextBuild;
* 	-Dat: March 10th, 2017
* 	-Art: Lin Huan
* **/

#include <string>
#include <vector>
#include <fstream>

class LineReader;
class TextBuild;

/*
* Reader to retrieve text from file line by line
* */
class LineReader {
public:
	/* maximum number of characters */
	static const int MaximumLineCharacters = 24 * 1024;

	/* constructor */
	LineReader(const std::string &);
	/* deconstructor */
	~LineReader();

	/* whether there has next line */
	bool hasNext();
	/* get next line (dynamically allocate) */
	std::string next();

private:
	/* input stream for file */
	std::ifstream in;
	/* pointer to the next line */
	std::string * line;
	/* buffer to cache characters from file */
	char buff[MaximumLineCharacters];

	/* update pointer to the next line */
	void roll();
};

/*
* Build for text to retrieve character by their lines
* */
class TextBuild {
public:
	/* constructor */
	TextBuild(LineReader &);
	/* deconstructor */
	~TextBuild();

	/* number of characters in text */
	int numberOfCharacters() const;
	/* number of lines in text */
	int numberOfLines() const;
	/* index to the first character in specific line (start from 1) */
	int indexOfLine(int line) const;
	/* line in which the character of index is located (index start from 0) */
	int lineOfIndex(int index) const;
	/* get the text retrieved from file */
	const std::string & getText() const;
private:
	/* line-reader to retrieve text from file */
	LineReader & reader;
	/* original text from source code */
	std::string text;
	/* list of indexes to the */
	std::vector<int> lines;

};
