/*----------------------------------------------------------
This Source Code Form is subject to the terms of the 
Mozilla Public License, v.2.0. If a copy of the MPL 
was not distributed with this file, You can obtain one 
at http://mozilla.org/MPL/2.0/.
*/

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <algorithm>
#include <cstdint>
#include <random>
#include "../version.h"
#include "versions.hxx"
#include <string>

using namespace std;

int version()
{
	cout << V8P_VERSION << endl;
	return 0;
}

int usage()
{
	cout << "V8 Versions tool. V " << V8P_VERSION << endl;
	cout << "Usage:" << endl;
	cout << "\tv8versions get    <filename>" << endl;
	cout << "\tv8versions set    <filename> <version>" << endl;
	cout << "\tv8versions update <filename>" << endl;
	return 0;
}

string new_version(uniform_int_distribution<int> &dis, mt19937_64 &gen)
{
	const static char hex[] = "0123456789abcdef";
	string result("00000000-0000-0000-0000-000000000000");
	for (auto index = 0; index < result.size(); index++) {
		
		if (result[index] == '-') {
			continue;
		}

		result[index] = hex[dis(gen) & 0xF];
	}

	return result;
}

int main(int argc, char **argv)
{

	if (argc < 2) {
		return usage();
	}

	string command = argv[1];
	transform(command.begin(), command.end(), command.begin(), ::tolower);

	shared_ptr<istream> in_file;
	shared_ptr<ostream> out_file;

	int arg_base = 2;
	if (arg_base < argc) {

		string arg2(argv[arg_base]);
		if (arg2 == "--versions-file") {

			// Переопределяем входной файл

			++arg_base;
			if (arg_base >= argc) {
				return usage();
			}

			string versions_file_name(argv[arg_base]);
			
			if (versions_file_name == "-") {

				in_file.reset (&cin,  [](...){} );
				out_file.reset(&cout, [](...){} );

			} else {

				boost::filesystem::path file_path(versions_file_name);
				in_file.reset(new boost::filesystem::ifstream(file_path));
			}

			++arg_base;
		}

	}

	// Команды, не требующие работы с файлом
	if (command == "version") {
		return version();
	}

	if (command == "usage" || command == "help") {
		return usage();
	}

	// будет работа с файлом версий
	if (in_file == nullptr) {
		
		// по-умолчанию считаем, что работаем с файлом "versions" в текущем каталоге
		in_file.reset(new boost::filesystem::ifstream("versions"));

	}

	VersionsFile versions;
	versions.LoadFile(*in_file);
	in_file.reset();

	if (command == "get") {
		
		if (arg_base >= argc) {
			return usage();
		}

		string inner_file_name(argv[arg_base]);
		++arg_base;

		cout << versions.Get(inner_file_name) << endl;
		return 0;
	}

	// Команды, которым потребуется запись в файл
	if (out_file == nullptr) {

		// по-умолчанию считаем, что работаем с файлом "versions" в текущем каталоге
		out_file.reset(new boost::filesystem::ofstream("versions"));

	}


	if (command == "set") {

		if (arg_base >= argc) {
			return usage();
		}

		string inner_file_name(argv[arg_base]);
		++arg_base;

		if (arg_base >= argc) {
			return usage();
		}

		string new_version(argv[arg_base]);
		++arg_base;

		versions.Set(inner_file_name, new_version);
		versions.SaveToFile(*out_file);

		return 0;
	}

	if (command == "update") {

		if (arg_base >= argc) {
			return usage();
		}

		string inner_file_name(argv[arg_base]);
		++arg_base;

		random_device rd;
		mt19937_64 gen(rd());
		uniform_int_distribution<int> dis;

		versions.Set(inner_file_name, new_version(dis, gen));
		versions.SaveToFile(*out_file);
		
		return 0;
	}


	return usage();
}
