#include "CodeFormatServer/Service/Indexs/ModuleIndex.h"
#include "CodeFormatServer/LanguageClient.h"


ModuleIndex::ModuleIndex()
{
}

void ModuleIndex::BuildIndex(const std::vector<std::string>& files)
{
	for (auto& filename : files)
	{
		auto options = LanguageClient::GetInstance().GetOptions(filename);
		auto relativePath = std::filesystem::relative(filename, options->export_root);
		std::string modulePath = relativePath.replace_extension("").string();
		if (modulePath.starts_with("."))
		{
			continue;
		}

		for (auto& c : modulePath)
		{
			if (c == '/' || c == '\\')
			{
				c = '.';
			}
		}

		if (modulePath.ends_with(".init"))
		{
			modulePath = modulePath.substr(0, modulePath.size() - 5);
		}

		if (_moduleIndex.count(options->export_root) == 0)
		{
			_moduleIndex.insert({options->export_root, std::vector<Module>()});
		}

		_moduleIndex.at(options->export_root).emplace_back(modulePath, filename);
	}
}

void ModuleIndex::ClearFile(std::string_view filePath)
{
	auto options = LanguageClient::GetInstance().GetOptions(filePath);
	if (_moduleIndex.count(options->export_root) > 0)
	{
		auto& vec = _moduleIndex.at(options->export_root);
		for(auto it = vec.begin(); it != vec.end(); ++it)
		{
			if(it->FilePath == filePath)
			{
				vec.erase(it);
				return;
			}
		}
	}
}

void ModuleIndex::RebuildIndex(const std::vector<std::string>& files)
{
	_moduleIndex.clear();
	BuildIndex(files);
}

std::vector<ModuleIndex::Module>& ModuleIndex::GetModules(std::shared_ptr<LuaCodeStyleOptions> options)
{

	for (auto& from : options->import_from)
	{
		auto it = _moduleIndex.find(from);

		if (it != _moduleIndex.end())
		{
			return it->second;
		}
	}
	return _emptyModules;
}
