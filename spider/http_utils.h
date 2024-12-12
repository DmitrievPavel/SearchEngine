#pragma once 
#include <vector>
#include <string>
#include <map>
#include "link.h"

std::string getHtmlContent(const Link& link);
std::map<std::string, int> getWordsMap(const std::string& html);
std::vector<Link> getLinks(const std::string& html, const Link& HomeLink);
