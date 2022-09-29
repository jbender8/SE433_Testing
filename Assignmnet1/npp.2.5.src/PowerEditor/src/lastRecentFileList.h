#ifndef LASTRECENTFILELIST_H
#define LASTRECENTFILELIST_H

#include <list>
#include <string>

typedef std::list<std::string> stringList;

class LastRecentFileList
{
public :
	LastRecentFileList() : _userMax(NB_MAX_LRF_FILE) {};
	void add(const char *fn) {
		if (int(_lrfl.size()) >= _userMax) return;
		_lrfl.push_back(fn);
	};
	void set(const char *fn) {
		if (find(fn)) 
			_lrfl.remove(fn);

		_lrfl.push_front(fn);

		if (int(_lrfl.size()) >= _userMax)
			_lrfl.pop_back();
	};
	void saveLRFL() const {
		NppParameters *pNppParams = NppParameters::getInstance();
		int nbMaxFile = pNppParams->getNbMaxFile();
		pNppParams->writeNbHistoryFile(nbMaxFile);

		int i = 0;
		for (stringList::const_iterator it = _lrfl.begin() ; 
			it != _lrfl.end() && (i < nbMaxFile) ; 
			it++, i++)
		{
			pNppParams->writeHistory(((const std::string)*it).c_str());
		}
	};

	bool find(const char *fn) const {
		for (stringList::const_iterator it = _lrfl.begin() ; it != _lrfl.end() ; it++)
		{
			if (*it == fn)
			{
				//_lrfl.remove(*it);
				return true;
			}
		}
		return false;
	};
private:
	stringList _lrfl;
	int _userMax;
};

#endif //LASTRECENTFILELIST_H
