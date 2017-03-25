#ifndef V8VERSIONS_VERSIONS_HXX_INCLUDED
#define V8VERSIONS_VERSIONS_HXX_INCLUDED

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace V8Unpack {

class DiffElement
{
	public:

		std::string filename;
		std::string version1;
		std::string version2;
};

class VersionsFile
{
	public:

		void LoadFile(std::istream &in);
		void SaveToFile(std::ostream &out) const;

		std::string Get(const std::string &filename) const
		{
			auto element = _data.find(filename);
			if (element == _data.end()) {
				return "";
			}

			return element->second;
		}

		void Set(const std::string &filename, const std::string &version)
		{
			_data[filename] = version;
		}

	std::vector<DiffElement> Diff(const VersionsFile &fileb) const;

	private:
		std::map<std::string, std::string> _data;
};

} // namespace V8Unpack
#endif
