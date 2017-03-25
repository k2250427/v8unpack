#include "versions.hxx"

using namespace std;

namespace V8Unpack {

bool read_element(istream &in, string &result)
{
    char c;
    bool quoted = false;
    in >> c;

    if (!in) {
        return false;
    }
    if (c == '}') {
        return false;
    }

    result = "";
    while (in && !(!quoted && c == ',')) {

        if (c == '}') {
            break;
        }

        if (c == '"') {
            quoted = !quoted;
        } else {
            result += c;
        }

        in >> c;
    }
    
    return true;
}

void VersionsFile::LoadFile(std::istream &in)
{
    char c;
    in >> c; // c == '{'

    string element;
    while (read_element(in, element)) {
        string value;
        read_element(in, value);
        _data[element] = value;
    }
}

void VersionsFile::SaveToFile(std::ostream &out) const
{
    out << "{1,";
    out << _data.size();
    for (auto element : _data) {
        out << ",\"" << element.first << "\"," << element.second;
    }
    out << "}";
}

std::vector<DiffElement> VersionsFile::Diff(const VersionsFile &fileb) const
{
    map<string, DiffElement> diff_table;
    for (auto element1 : _data) {
        auto diff_el = diff_table[element1.first];
        diff_el.filename = element1.first;
        diff_el.version1 = element1.second;
        diff_table[element1.first] = diff_el;
    }
    for (auto element2 : fileb._data) {
        auto diff_el = diff_table[element2.first];
        diff_el.filename = element2.first;
        diff_el.version2 = element2.second;
        diff_table[element2.first] = diff_el;
    }

    vector<DiffElement> result;

    for (auto element : diff_table) {
        auto diff_el = element.second;
        if (diff_el.version1 != diff_el.version2) {
            result.push_back(diff_el);
        }
    }

    return result;
}

} // namespace V8Unpack