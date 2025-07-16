#pragma once

#include <string> // string, wstring

// string => wstring 文字列変換
std::wstring ConverString(const std::string& str);

// wstring => string 文字列変換
std::string ConverString(const std::wstring& str);