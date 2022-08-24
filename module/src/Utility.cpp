#include "Utility.h"

string Utility::format_json(ptree json) {
    stringstream container;
    write_json(container, json);
    string text = container.str();
    text.erase(remove(text.begin(), text.end(), '\n'), text.end());
    return text;
}

int Utility::mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

bool Utility::file_exists(const string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}