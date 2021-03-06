/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
* fuzz_tests.cpp
*
* Fuzz tests for the JSON library.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#include "stdafx.h"
#include "cpprest/filestream.h"
#include "cpprest/containerstream.h"

using namespace web; 

namespace tests { namespace functional { namespace json_tests {

#ifdef _MS_WINDOWS

SUITE(json_fuzz_tests)
{
std::string get_fuzzed_file_path()
{
    std::string ipfile;

    if(UnitTest::GlobalSettings::Has("fuzzedinputfile"))
    {
        ipfile = UnitTest::GlobalSettings::Get("fuzzedinputfile");
    }

    return ipfile;
}

TEST(fuzz_json_parser, "Requires", "fuzzedinputfile")
{
    std::wstring ipfile = utility::conversions::to_utf16string(get_fuzzed_file_path());
    if (true == ipfile.empty())
    {
        VERIFY_IS_TRUE(false, "Input file is empty");
        return;
    }

    auto fs = Concurrency::streams::file_stream<uint8_t>::open_istream(ipfile).get();
    concurrency::streams::container_buffer<std::string> cbuf;
    fs.read_to_end(cbuf).get();
    fs.close().get();
    auto json_str = cbuf.collection();

    // Look for UTF-8 BOM
    if ((uint8_t)json_str[0] != 0xEF || (uint8_t)json_str[1] != 0xBB || (uint8_t)json_str[2] != 0xBF)
    {
        VERIFY_IS_TRUE(false, "Input file encoding is not UTF-8. Test will not parse the file.");
        return;
    }

    auto utf16_json_str = utility::conversions::utf8_to_utf16(json_str);
    // UTF8 to UTF16 conversion will retain the BOM, remove it.
    if (utf16_json_str.front() == 0xFEFF)
        utf16_json_str.erase(0, 1);

    try
    {
        json::value::parse(std::move(utf16_json_str));
        std::cout << "Input file parsed successfully.";
    }
    catch(const json::json_exception& ex)
    {
        std::cout << "json exception:" << ex.what();
    }
}
}

#endif
}}}