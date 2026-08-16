#pragma once
#include "const.h"

#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <map>
#include <iostream>

struct SectionInfo
{
	SectionInfo(){}
	~SectionInfo() { _section_datas.clear(); }

	//拷贝构造
	SectionInfo(const SectionInfo& src) {
		_section_datas = src._section_datas;
	}
	//拷贝赋值操作符
	SectionInfo& operator = (const SectionInfo& src) {
		if (&src == this) {
			return *this;
		}

		this->_section_datas = src._section_datas;
		return *this;
	}
	//数据存储
	std::map<std::string, std::string> _section_datas;
	//下标操作符
	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";
		}

		return _section_datas[key];
	}
};


class ConfigMgr
{
public:
	~ConfigMgr() {
		_config_map.clear();
	}

	SectionInfo operator[](const std::string& section) {
		if (_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}

		return _config_map[section];
	}

	static ConfigMgr& Inst() {
		static ConfigMgr cfg_mgr;
		return cfg_mgr;
	}

	//拷贝构造
	ConfigMgr(const ConfigMgr& src) {
		this->_config_map = src._config_map;
	}
	//拷贝赋值
	ConfigMgr& operator = (const ConfigMgr& src) {
		if (&src == this) {
			return *this;
		}

		this->_config_map = src._config_map;
		return *this;
	}

private:
	ConfigMgr();
	std::map<std::string, SectionInfo> _config_map;

};

