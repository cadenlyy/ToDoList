#pragma once
#include <string>
#include <vector>

struct Task
{
	std::string description;
	bool done;
};

void SaveTasksToFile(const std::vector<Task>& tasks, const std::string& fileName, int h, int m);
std::vector<Task>LoadTasksFromFile(const std::string& fileName);
std::pair<int, int> savedTime(const std::string& fileName);

