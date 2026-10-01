#include "SaveFile.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>

static std::string EscapeField(const std::string& text)
{
    std::string result = "";
    for (size_t i = 0; i < text.size(); i++)
    {
        char c = text[i];
        if (c == '\\')
        {
            result += "\\\\";
        }
        else if (c == '|')
        {
            result += "\\p";
        }
        else if (c == '\n')
        {
            result += "\\n";
        }
        else if (c == '\r')
        {
            // Dropped, line endings are handled by the file format itself
        }
        else
        {
            result += c;
        }
    }
    return result;
}

// ---------- Writer ----------

void SaveWriter::BeginRecord(const std::string& tag)
{
    _line = EscapeField(tag);
}

void SaveWriter::WriteInt(int value)
{
    _line += "|" + std::to_string(value);
}

void SaveWriter::WriteString(const std::string& value)
{
    _line += "|" + EscapeField(value);
}

void SaveWriter::EndRecord()
{
    _text += _line + "\n";
    _line = "";
}

bool SaveWriter::WriteToFile(const std::string& path) const
{
    std::string tempPath = path + ".tmp";

    std::ofstream out(tempPath.c_str(), std::ios::binary);
    if (!out)
    {
        return false;
    }
    out << _text;
    if (!out.good())
    {
        out.close();
        return false;
    }
    out.close();

    std::remove(path.c_str()); // Fails harmlessly on first save, when there is nothing to remove
    if (std::rename(tempPath.c_str(), path.c_str()) != 0)
    {
        return false;
    }
    return true;
}

// ---------- Reader ----------

bool SaveReader::LoadFromFile(const std::string& path)
{
    _lines.clear();
    _nextLine = 0;
    _fields.clear();
    _fieldIndex = 1;

    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in)
    {
        return false;
    }

    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r')
        {
            line.erase(line.size() - 1);
        }
        if (line.empty())
        {
            continue;
        }
        _lines.push_back(line);
    }
    return true;
}

bool SaveReader::NextRecord()
{
    if (_nextLine >= _lines.size())
    {
        return false;
    }

    const std::string& line = _lines[_nextLine];
    _nextLine++;

    _fields.clear();
    std::string current = "";
    for (size_t i = 0; i < line.size(); i++)
    {
        char c = line[i];
        if (c == '\\' && i + 1 < line.size())
        {
            i++;
            char escaped = line[i];
            if (escaped == 'p')
            {
                current += '|';
            }
            else if (escaped == 'n')
            {
                current += '\n';
            }
            else
            {
                current += escaped;
            }
        }
        else if (c == '|')
        {
            _fields.push_back(current);
            current = "";
        }
        else
        {
            current += c;
        }
    }
    _fields.push_back(current);

    _fieldIndex = 1;
    return true;
}

const std::string& SaveReader::GetTag() const
{
    if (_fields.empty())
    {
        return _emptyTag;
    }
    return _fields[0];
}

int SaveReader::ReadInt()
{
    if (_fieldIndex >= _fields.size())
    {
        return 0;
    }
    int value = std::atoi(_fields[_fieldIndex].c_str());
    _fieldIndex++;
    return value;
}

std::string SaveReader::ReadString()
{
    if (_fieldIndex >= _fields.size())
    {
        return "";
    }
    std::string value = _fields[_fieldIndex];
    _fieldIndex++;
    return value;
}