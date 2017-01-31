/////////////////////////////////////////////////////////////////////////////
//
//
//	Author:			disa_da
//	E-mail:			disa_da2@mail.ru
//
//
/////////////////////////////////////////////////////////////////////////////

/**
    2014-2017       dmpas       sergey(dot)batanov(at)dmpas(dot)ru
 */

// main.cpp : Defines the entry point for the console application.
//

#include "V8File.h"
#include "version.h"
#include <iostream>
#include <algorithm>

using namespace std;

int usage(char **argv)
{
	cout << endl;
	cout << "V8Upack Version " << V8P_VERSION
		 << " Copyright (c) " << V8P_RIGHT << endl;

	cout << endl;
	cout << "Unpack, pack, deflate and inflate 1C v8 file (*.cf)" << endl;
	cout << endl;
	cout << "V8UNPACK" << endl;
	cout << "  -U[NPACK]            in_filename.cf     out_dirname" << endl;
	cout << "  -PA[CK]              in_dirname         out_filename.cf" << endl;
	cout << "  -I[NFLATE]           in_filename.data   out_filename" << endl;
	cout << "  -D[EFLATE]           in_filename        filename.data" << endl;
	cout << "  -E[XAMPLE]" << endl;
	cout << "  -BAT" << endl;
	cout << "  -P[ARSE]             in_filename        out_dirname" << endl;
	cout << "  -B[UILD] [-N[OPACK]] in_dirname         out_filename" << endl;
	cout << "  -V[ERSION]" << endl;

	return 0;
}

int version(char ** argv)
{
	cout << V8P_VERSION << endl;
	return 0;
}

int inflate(char **argv)
{
	CV8File V8File;
	V8File.Inflate(std::string(argv[0]), std::string(argv[1]));
	return 0;
}

int deflate(char **argv)
{
	CV8File V8File;
	int ret = V8File.Deflate(std::string(argv[0]), std::string(argv[1]));
	return ret;
}

int unpack(char **argv)
{
	CV8File V8File;
	int ret = V8File.UnpackToFolder(std::string(argv[0]), std::string(argv[1]), argv[2], true);
	return ret;
}

int pack(char **argv)
{
	CV8File V8File;
	int ret = V8File.PackFromFolder(argv[0], argv[1]);
	return ret;
}

int parse(char **argv)
{
	CV8File V8File;
	int ret = V8File.Parse(argv[0], argv[1]);
	return ret;
}

int bat(char **argv)
{
	cout << "if %1 == P GOTO PACK" << endl;
	cout << "if %1 == p GOTO PACK" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":UNPACK" << endl;
	cout << "V8Unpack.exe -unpack      %2                              %2.unp" << endl;
	cout << "V8Unpack.exe -undeflate   %2.unp\\metadata.data            %2.unp\\metadata.data.und" << endl;
	cout << "V8Unpack.exe -unpack      %2.unp\\metadata.data.und        %2.unp\\metadata.unp" << endl;
	cout << "GOTO END" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":PACK" << endl;
	cout << "V8Unpack.exe -pack        %2.unp\\metadata.unp            %2.unp\\metadata_new.data.und" << endl;
	cout << "V8Unpack.exe -deflate     %2.unp\\metadata_new.data.und   %2.unp\\metadata.data" << endl;
	cout << "V8Unpack.exe -pack        %2.unp                         %2.new.cf" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":END" << endl;

	return 0;
}

int example(char **argv)
{
	cout << "" << endl;
	cout << "" << endl;
	cout << "UNPACK" << endl;
	cout << "V8Unpack.exe -unpack      1Cv8.cf                         1Cv8.unp" << endl;
	cout << "V8Unpack.exe -undeflate   1Cv8.unp\\metadata.data          1Cv8.unp\\metadata.data.und" << endl;
	cout << "V8Unpack.exe -unpack      1Cv8.unp\\metadata.data.und      1Cv8.unp\\metadata.unp" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << "PACK" << endl;
	cout << "V8Unpack.exe -pack        1Cv8.unp\\metadata.unp           1Cv8.unp\\metadata_new.data.und" << endl;
	cout << "V8Unpack.exe -deflate     1Cv8.unp\\metadata_new.data.und  1Cv8.unp\\metadata.data" << endl;
	cout << "V8Unpack.exe -pack        1Cv8.und                        1Cv8_new.cf" << endl;
	cout << "" << endl;
	cout << "" << endl;

	return 0;
}

int build(char **argv)
{
	const bool dont_pack = false;
	CV8File V8File;
	int ret = V8File.BuildCfFile(argv[0], argv[1], dont_pack);
	return ret;
}

int build_nopack(char **argv)
{
	const bool dont_pack = true;
	CV8File V8File;
	int ret = V8File.BuildCfFile(argv[0], argv[1], dont_pack);
	return ret;
}

typedef int (*handler_t)(char **argv);

handler_t getRunMode(char *argv[], int argc, int &arg_base)
{
	if (argc < 2) {
		return usage;
	}

	string cur_mode(argv[1]);
	transform(cur_mode.begin(), cur_mode.end(), cur_mode.begin(), ::tolower);

	arg_base = 2;
	if (cur_mode == "-version" || cur_mode == "-v") {
		return version;
	}

	if (cur_mode == "-inflate" || cur_mode == "-i" || cur_mode == "-und" || cur_mode == "-undeflate") {
		return inflate;
	}

	if (cur_mode == "-deflate" || cur_mode == "-d") {
		return deflate;
	}

	if (cur_mode == "-unpack" || cur_mode == "-u" || cur_mode == "-unp") {
		return unpack;
	}

	if (cur_mode == "-pack" || cur_mode == "-pa") {
		return pack;
	}

	if (cur_mode == "-parse" || cur_mode == "-p") {
		return parse;
	}

	if (cur_mode == "-build" || cur_mode == "-b") {

		bool dont_pack = false;

		if (argc > 2) {
			string arg2(argv[2]);
			transform(arg2.begin(), arg2.end(), arg2.begin(), ::tolower);
			if (arg2 == "-n" || arg2 == "-nopack") {
				arg_base++;
				dont_pack = true;
			}
		}
		return dont_pack ? build_nopack : build;
	}

	if (cur_mode == "-bat") {
		return bat;
	}

	if (cur_mode == "-example" || cur_mode == "-e") {
		return example;
	}

	return nullptr;
}

int main(int argc, char* argv[])
{
	int arg_base = 1;
	handler_t handler = getRunMode(argv, argc, arg_base);

	if (handler == nullptr) {
		usage(nullptr);
		return 1;
	}

	int ret = handler(&argv[arg_base]);
	if (ret == SHOW_USAGE) {
		usage(nullptr);
	}
	return ret;
}
