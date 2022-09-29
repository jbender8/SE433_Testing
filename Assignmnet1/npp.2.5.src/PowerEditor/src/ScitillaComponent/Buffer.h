//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef BUFFER_H
#define BUFFER_H

#include <shlwapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Scintilla.h"
#include "Parameters.h"


const char UNTITLED_STR[] = "new ";
typedef sptr_t Document;


// Ordre important!! Ne le changes pas!
//SC_EOL_CRLF (0), SC_EOL_CR (1), or SC_EOL_LF (2).
enum formatType {WIN_FORMAT, MAC_FORMAT, UNIX_FORMAT};

const int CR = 0x0D;
const int LF = 0x0A;

enum docFileStaus{NEW_DOC, FILE_DELETED, NO_PROBLEM, MODIFIED_FROM_OUTSIDE};

// Related to Utf8_16::encodingType but with additional values at end
enum UniMode {uni8Bit=0, uni16BE=1, uni16LE=2, uniUTF8=3, uniCookie=4};

struct Position
{ 
	int _fistVisibleLine;
	int _startPos;
	int _endPos;
};

#define USER_LANG_CURRENT ""
const int userLangNameMax = 16;

static bool isInList(const char *token, const char *list) {
	if ((!token) || (!list))
		return false;
	char word[64];
	int i = 0;
	int j = 0;
	for (; i <= strlen(list) ; i++)
	{
		if ((list[i] == ' ')||(list[i] == '\0'))
		{
			if (j != 0)
			{
				word[j] = '\0';
				j = 0;
				
				if (!stricmp(token, word))
					return true;
			}
		}
		else 
		{
			word[j] = list[i];
			j++;
		}
	}
	return false;
};


class Buffer
{
friend class ScintillaEditView;
public :
	Buffer(Document doc, const char *fileName)
		: _isDirty(false), _doc(doc), _isReadOnly(false), _format(WIN_FORMAT),_unicodeMode(uni8Bit)
	{
		_pos._fistVisibleLine = 0;
		_pos._startPos = 0;
		_pos._endPos = 0;
		setFileName(fileName);
		//_userLangExt[0] = '\0';
	};

    Buffer(const Buffer & buf) : _isDirty(buf._isDirty),  _doc(buf._doc), _lang(buf._lang),
        _timeStamp(buf._timeStamp), _isReadOnly(buf._isReadOnly), _pos(buf._pos), _format(buf._format),_unicodeMode(buf._unicodeMode)
    {
        strcpy(_fullPathName, buf._fullPathName);
		strcpy(_userLangExt, buf._userLangExt);
    };

    Buffer & operator=(const Buffer & buf)
    {
        if (this != &buf)
        {
            this->_isDirty = buf._isDirty;
            this->_doc = buf._doc;
            this->_lang = buf._lang;
            this->_timeStamp = buf._timeStamp;
            this->_isReadOnly = buf._isReadOnly;
            this->_pos = buf._pos;
            this->_format = buf._format;
			this->_unicodeMode = buf._unicodeMode;

			strcpy(this->_fullPathName, buf._fullPathName);
            strcpy(this->_userLangExt, buf._userLangExt);
        }
        return *this;
    }

	LangType getLangFromExt(const char *ext)
	{
		int i = 0;
		Lang *l = NppParameters::getInstance()->getLangFromIndex(i++);
		while (l)
		{
			const char *defList = l->getDefaultExtList();
			const char *userList = NULL;

			LexerStylerArray &lsa = (NppParameters::getInstance())->getLStylerArray();
			const char *lName = l->getLangName();
			LexerStyler *pLS = lsa.getLexerStylerByName(lName);
			
			if (pLS)
				userList = pLS->getLexerUserExt();

			std::string list("");
			if (defList)
				list += defList;
			if (userList)
			{
				list += " ";
				list += userList;
			}
			if (isInList(ext, list.c_str()))
				return l->getLangID();
			l = (NppParameters::getInstance())->getLangFromIndex(i++);
		}
		return L_TXT;
	};

	// this method 1. copies the file name
	//             2. determinates the language from the ext of file name
	//             3. gets the last modified time
	void setFileName(const char *fn) 
	{
		bool isExtSet = false;

		strcpy(_fullPathName, fn);
        if (PathFileExists(_fullPathName))
		{
			// for _lang
			char *ext = PathFindExtension(_fullPathName);
			
			if (*ext == '.') ext += 1;
			_lang = getLangFromExt(ext);

			if (_lang == L_TXT)
			{
				char *fileName = PathFindFileName(_fullPathName);
				const char *langName = NULL;

				if ((!_stricmp(fileName, "makefile")) || (!_stricmp(fileName, "GNUmakefile")))
					_lang = L_MAKEFILE;

				else if (langName = (NppParameters::getInstance())->getLangNameFromExt(ext))
				{
					_lang = L_USER;
					strcpy(_userLangExt, langName);
					isExtSet = true;
				}

				else
					_lang = L_TXT;
            }

			if (!isExtSet)
				_userLangExt[0] = '\0';
			// for _timeStamp
			updatTimeStamp();
		}
		else
		{
			_lang = L_TXT;
			_timeStamp = 0;
		}
	};


	const char * getFileName() {return _fullPathName;};

	void updatTimeStamp() {
		struct _stat buf;
		_timeStamp = (_stat(_fullPathName, &buf)==0)?buf.st_mtime:0;
	};

	docFileStaus checkFileState() {
		if (isUntitled(_fullPathName))
		{
			_isReadOnly = false;
			return NEW_DOC;
		}
        if (!PathFileExists(_fullPathName))
		{
			_isReadOnly = false;
			return FILE_DELETED;
		}
		struct _stat buf;
		if (!_stat(_fullPathName, &buf))
		{
			_isReadOnly = (bool)(!(buf.st_mode & _S_IWRITE));

			if (_timeStamp != buf.st_mtime)
				return MODIFIED_FROM_OUTSIDE;
		}
		return NO_PROBLEM;
	};

	// to use this method with open and save
	void checkIfReadOnlyFile() {
		struct _stat buf;
		if (!_stat(_fullPathName, &buf))
		{
			_isReadOnly = (bool)(!(buf.st_mode & _S_IWRITE));
		}
	};

    bool isDirty() const {
        return _isDirty;
    };

    bool isReadOnly() const {
        return _isReadOnly;
    };
    
    time_t getTimeStamp() const {
        return _timeStamp;
    };

    void synchroniseWith(const Buffer & buf) {
        _isDirty = buf.isDirty();
        _timeStamp = buf.getTimeStamp();
    };

	// that is : the prefix of the string is "new "
	static bool isUntitled(const char *str2Test) {
		return (strncmp(str2Test, UNTITLED_STR, sizeof(UNTITLED_STR)-1) == 0);
	}

	void setFormat(formatType format) {
		_format = format;
	};

	void determinateFormat(char *data) {
		int len = strlen(data);
		for (int i = 0 ; i < len ; i++)
		{
			if (data[i] == CR)
			{
				if (data[i+1] == LF)
				{
					_format = WIN_FORMAT;
					return;
				}
				else
				{
					_format = MAC_FORMAT;
					return;
				}
			}
			if (data[i] == LF)
			{
				_format = UNIX_FORMAT;
				return;
			}
		}
		_format = WIN_FORMAT;
	};

	formatType getFormat() const {
		return _format;
	};

	bool isUserDefineLangExt() const {
		return _userLangExt[0];
	};

	const char * getUserDefineLangName() {return _userLangExt;};

	void setUnicodeMode(UniMode mode) {
		if ((_unicodeMode != mode) && !((_unicodeMode == uni8Bit) && (mode == uniCookie)) && \
			!((_unicodeMode == uniCookie) && (mode == uni8Bit)))
			_isDirty = true;
		_unicodeMode = mode;
	};
	UniMode getUnicodeMode() {return _unicodeMode;};

private :
	bool _isDirty;
	Document _doc;
	LangType _lang;
	char _userLangExt[userLangNameMax]; // it's useful if only (_lang == L_USER)

	time_t _timeStamp; // O if it's a new doc
	bool _isReadOnly;
	Position _pos;
	char _fullPathName[MAX_PATH];
	formatType _format;
	UniMode _unicodeMode;
};

#endif //BUFFER_H
