#pragma once
#include <string>
#include <vector>

const int SAVE_FORMAT_VERSION = 1;

// Writes a save as text, one record per line. A record is a tag followed by fields, separated by '|'.
// Text fields are escaped, so names and ids can hold any character. Nothing here knows about the game.
class SaveWriter
{
public:
    void BeginRecord(const std::string& tag);
    void WriteInt(int value);
    void WriteString(const std::string& value);
    void EndRecord();

    // Writes to a temp file first, then swaps it in, so a crash mid-write cannot leave half a save.
    bool WriteToFile(const std::string& path) const;

private:
    std::string _text;
    std::string _line;
};

class SaveReader
{
public:
    // False if the file is missing or unreadable (normal on first launch).
    bool LoadFromFile(const std::string& path);

    // Moves to the next record. False when there are no more.
    bool NextRecord();
    const std::string& GetTag() const;

    // Fields of the current record, in the order they were written. Missing fields read as 0 or "".
    int ReadInt();
    std::string ReadString();

private:
    std::vector<std::string> _lines;
    size_t _nextLine = 0;
    std::vector<std::string> _fields;
    size_t _fieldIndex = 1;
    std::string _emptyTag = "";
};