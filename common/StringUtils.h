/*
	This file is part of CG4.

	Copyright (c) Inbar Donag and Maxim Levitsky

    CG4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CG4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STR_UTILITES_H
#define STRUTILITES_H

#include <utility>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>

static inline std::list<std::string> tokenize(std::string str, const char* delims = "\n\r\t ", bool includeempty = false )
{
	size_t token_end = 0, token_start = 0, token_len;
	bool first_iteration = true;
	std::list<std::string> result;

	do
	{
		/* find start of the token */
		if (!includeempty) {

			token_start = str.find_first_not_of(delims, token_end);
			if (token_start == std::string::npos)
				break;

		}
		else if (!first_iteration)
		{
			token_start = token_end + 1;

			if (token_start >= str.length())
				break;
		}

		/* find the end of the token*/
		token_end = str.find_first_of(delims, token_start);

		if (token_end == std::string::npos)
			token_len = std::string::npos;
		else
			token_len = token_end - token_start;

		/* and put it there */
		result.push_back(str.substr(token_start, token_len));

		first_iteration = false;
	}

	while (token_end != std::string::npos);

	return result;
}


#endif
