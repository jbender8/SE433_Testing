/*------------------------------------------------------------------------------------
this file is part of notepad++
Copyright (C)2003 Don HO < donho@altern.org >

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
----------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

#define KEYWORD_BOXHEADER 1
#define KEYWORD_FOLDCONTRACTED 2

static bool isInOpList(WordList & opList, char op)
{
	for (int i = 0 ; i < opList.len ; i++)
		if (op == *(opList[i]))
			return true;
	return false;
}

static void getRange(unsigned int start, unsigned int end, Accessor &styler, char *s, unsigned int len) 
{
	unsigned int i = 0;
	while ((i < end - start + 1) && (i < len-1)) 
	{
		s[i] = static_cast<char>(styler[start + i]);
		i++;
	}
	s[i] = '\0';
}

static inline bool isAWordChar(const int ch) {
	//return (ch < 0x80) && (isalnum(ch) || ch == '.' || ch == '_');
	return ((ch > 0x20) && (ch <= 0xFF) && (ch != ' ') && (ch != '\n'));
}

static inline bool isAWordStart(const int ch) {
	return isAWordChar(ch);
}

static void ColouriseUserDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],
                            Accessor &styler) {

	WordList &keywords = *keywordlists[0];
	WordList &blockOpenWords = *keywordlists[1];
	WordList &blockCloseWords = *keywordlists[2];
	WordList &symbols = *keywordlists[3];
	WordList &comments = *keywordlists[4];
	WordList &keywords5 = *keywordlists[5];
	WordList &keywords6 = *keywordlists[6];
    WordList &keywords7 = *keywordlists[7];
	WordList &keywords8 = *keywordlists[8];
	
	int chPrevNonWhite = ' ';
	int visibleChars = 0;

	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More(); sc.Forward()) 
	{
		// Determine if the current state should terminate.
		switch (sc.state)
		{			
			case SCE_USER_NUMBER :
			{
				//if (!isAWordChar(sc.ch))
				if (!IsADigit(sc.ch))
					sc.SetState(SCE_USER_DEFAULT);
				break;
			}

			case SCE_USER_IDENTIFIER : 
			{
				bool doDefault = true;
				//char aSymbol[2] = {sc.ch, '\0'};
				bool isSymbol = isInOpList(symbols, sc.ch);
				char s[100];
				sc.GetCurrent(s, sizeof(s));
				bool isFound = false;

				if (!isAWordChar(sc.ch)  || isSymbol)
				{
					bool doDefault = true;
					char s[100];
					sc.GetCurrent(s, sizeof(s));
					char commentLineStr[30] = "0";
					char *p = commentLineStr+1;
					strcpy(p, s);
					char commentOpen[30] = "1";
					p = commentOpen+1;
					strcpy(p, s);
					
					if (keywords5.InList(s))
						sc.ChangeState(SCE_USER_WORD1);
					else if (keywords6.InList(s)) 
						sc.ChangeState(SCE_USER_WORD2);
					else if (keywords7.InList(s))
						sc.ChangeState(SCE_USER_WORD3);
					else if (keywords8.InList(s)) 
						sc.ChangeState(SCE_USER_WORD4);
					else if (blockOpenWords.InList(s)) 
						sc.ChangeState(SCE_USER_BLOCK_OPERATOR_OPEN);
					else if (blockCloseWords.InList(s)) 
						sc.ChangeState(SCE_USER_BLOCK_OPERATOR_CLOSE);
					else if (comments.InList(commentLineStr)) 
					{
					      sc.ChangeState(SCE_USER_COMMENTLINE);
					      doDefault = false;
					}
					else if (comments.InList(commentOpen)) 
					{
					      sc.ChangeState(SCE_USER_COMMENT);
					      doDefault = false;
					}
					if (doDefault)
						sc.SetState(SCE_USER_DEFAULT);
				}
				break;
			}
			case SCE_USER_COMMENT :
			{
				char *pCommentClose = NULL;
				for (int i = 0 ; i < comments.len ; i++)
				{
					if (comments.words[i][0] == '2')
					{
						pCommentClose = comments.words[i] + 1;
						break;
					}
						//(commentClose);
				}
				if (pCommentClose)
				{
					int len = strlen(pCommentClose);
					if (len == 1)
					{
						if (sc.Match(pCommentClose[0])) 
						{
							sc.Forward();
							sc.SetState(SCE_USER_DEFAULT);
						}
					}
					else 
					{
						if (sc.Match(pCommentClose)) 
						{
							for (int i = 0 ; i < len ; i++)
								sc.Forward();
							sc.SetState(SCE_USER_DEFAULT);
						}
					}
				}
				break;
			} 
			case SCE_USER_COMMENTLINE :
			{
				if (sc.atLineEnd) 
				{
					sc.SetState(SCE_USER_DEFAULT);
					visibleChars = 0;
				}
				break;
			} 
			
			case SCE_USER_OPERATOR :
			{
				sc.SetState(SCE_USER_DEFAULT);
				break;
			} 
			
			default :
				break;
		}

		// Determine if a new state should be entered.
		if (sc.state == SCE_USER_DEFAULT) 
		{
			//char aSymbol[2] = {sc.ch, '\0'};

			if (IsADigit(sc.ch) /*&& IsADigit(sc.chNext)*/)
				sc.SetState(SCE_USER_NUMBER);
			//else if (symbols.InList(aSymbol))
			else if (isInOpList(symbols, sc.ch))
				sc.SetState(SCE_USER_OPERATOR);
			else if (isAWordStart(sc.ch)) 
				sc.SetState(SCE_USER_IDENTIFIER);
		}

		if (sc.atLineEnd) 
		{
			// Reset states to begining of colourise so no surprises
			// if different sets of lines lexed.
		   if (sc.state == SCE_USER_COMMENTLINE)
				sc.SetState(SCE_USER_DEFAULT);
			
			chPrevNonWhite = ' ';
			visibleChars = 0;
		}
		if (!IsASpace(sc.ch)) {
			chPrevNonWhite = sc.ch;
			visibleChars++;
		}

	}
	sc.Complete();
}


static void FoldUserDoc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],  Accessor &styler) 
{
	WordList &blockOpenWords = *keywordlists[1];
	WordList &blockCloseWords = *keywordlists[2];

	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	int style = initStyle;
	
	int lastStart = 0;
	
	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (stylePrev == SCE_USER_DEFAULT && (style == SCE_USER_BLOCK_OPERATOR_OPEN || style == SCE_USER_BLOCK_OPERATOR_CLOSE))
		{
			// Store last word start point.
			lastStart = i;
		}

		if(isAWordChar(ch) && !isAWordChar(chNext)) 
		{
			char s[30];
			getRange(lastStart, i, styler, s, sizeof(s));
	
			if (blockOpenWords.InList(s))  
			{
				levelCurrent++;
			} 
			if (blockCloseWords.InList(s)) 
			{
				levelCurrent--;
			}
		}
		

		if (atEOL) {
			int lev = levelPrev;

			if ((levelCurrent > levelPrev) && (visibleChars > 0))
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev = levelCurrent;
			visibleChars = 0;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelPrev | flagsNext);
}



static const char * const userDefineWordLists[] = {
            "Primary keywords and identifiers",
            "Secondary keywords and identifiers",
            "Documentation comment keywords",
            "Fold header keywords",
            0,
        };



LexerModule lmUserDefine(SCLEX_USER, ColouriseUserDoc, "user", FoldUserDoc, userDefineWordLists);
