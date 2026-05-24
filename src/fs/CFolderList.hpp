/****************************************************************************
 * Copyright (C) 2011
 * by Dj_Skual
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * CFolderList.hpp
 *
 * for SaveGame Manager GX 2011
 ***************************************************************************/
#ifndef _CFOLDERLIST_HPP_
#define _CFOLDERLIST_HPP_

#include <vector>
#include <string>


class CFolderList
{
	public:
		CFolderList() { };
		~CFolderList() { Reset(); };
		
		int Get();
		void Reset();
		void AddFolder(const std::string& name, const std::string& path);
		int GetCount() { return Folders.size(); };
		int GetSelectedCount();
		std::string GetName(int ind);
		std::string GetPath(int ind);
		bool IsSelected(int ind);
		void Select(int ind);
		void UnSelect(int ind);
		void SelectAll();
		void UnSelectAll();
		int GetFirstSelected();
		
		void Click(int ind);
		
	protected:
		void ScanPath(const std::string & rootPath, bool recursive, const std::string & prefix = "");
		void AddSequence(int index);
		void RemoveSequence(int index);
		
		typedef struct _FolderStruct
		{
			std::string name;
			std::string path;
			bool selected;
			int sequence;
		} FolderStruct;
		
		std::vector<FolderStruct *> Folders;
};

#endif
