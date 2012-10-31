/*
Copyright (c) 2012, Esteban Pellegrino
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ESTEBAN PELLEGRINO BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>

#include "ACEBlock.hpp"
#include "../AceUtils.hpp"

using namespace std;
using namespace Ace;

GenericBlock::GenericBlock(const int nxs[nxs_size], const int jxs[jxs_size],const std::vector<double>& xss, const int table_begin, AceTable* ace_table)
: ACEBlock(xss,ace_table) {
	/* Begin of the block */
	std::vector<double>::const_iterator beg = xss.begin() + table_begin;
	int cnt = 0;
	while(cnt < 4) {
		string str = toString(*beg);
		raw_data.push_back(str);
		beg++;
		cnt++;
	}
}

void GenericBlock::dump(std::ostream& xss) {
	for(size_t i = 0 ; i < raw_data.size() ; i++) {
		if(raw_data[i].find(".") != std::string::npos)
			putXSS(fromString<double>(raw_data[i]),xss);
		else
			putXSS(fromString<int>(raw_data[i]),xss);
	}
}
