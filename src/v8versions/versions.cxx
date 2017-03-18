#include "versions.hxx"

using namespace std;

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
