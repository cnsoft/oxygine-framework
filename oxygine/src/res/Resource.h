#pragma once
#include <string>
#include "core/Object.h"
#include "pugixml/pugixml.hpp"

namespace oxygine
{
	class Resources;
	class Mem2Native;

	class LoadResourcesContext;

	class Resource: public ObjectBase
	{
	public:
		Resource();
		~Resource();

		/**Loads resource heavy data into memory*/
		void load(LoadResourcesContext *context = 0);
		/**Unloads heavy data from memory. 
		All internal Objects (fonts, textures, sounds etc) remain valid but not usable (can't be displayed, played etc).
		*/
		void unload();

		/**Returns any attribute from xml resource definition. You could use it for defining for example per animation constants like duration, specific offsets, delays.*/
		pugi::xml_attribute getAttribute(const char *attr) const;
		pugi::xml_node		getNode() const {return _node;}
		bool				getUseLoadCounter() const {return _useLoadCounter;}


		void setUseLoadCounter(bool v) {_useLoadCounter = v;}

		/**returns id from xml node. Function is helper*/
		static string extractID(const pugi::xml_node &node, const string &file, const string &def);

	protected:
		static void setNode(Resource *res, const pugi::xml_node &node){res->_node = node;}

		virtual void _load(LoadResourcesContext *context = 0) = 0;
		virtual void _unload() = 0;
		
		int _loadCounter;
		bool _useLoadCounter;

		pugi::xml_node _node;

	private:
		//non copyable
		Resource(const Resource&);
		const Resource& operator=(const Resource&);	
	};
}