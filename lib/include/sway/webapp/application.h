#ifndef _SWAY_WEBAPP_APPLICATION_H
#define _SWAY_WEBAPP_APPLICATION_H

#include <sway/core/containers/hierarchylistener.h>
#include <sway/webcore/dom/htmldocument.h>
#include <sway/webcore/dom/htmlelement.h>
#include <sway/webcore/treeupdater.h>
#include <sway/webcore/treenodeelement.h>
#include <sway/webcore/mvc/itemview.h>
#include <sway/webcore/prereqs.h>

namespace sway::webapp {

class Application
	: public core::HierarchyListener {

public:

	#pragma region "Constructor / Destructor"

	/*!
	 * \brief
	 *    Конструктор класса.
	 *    Выполняет инициализацию нового экземпляра класса.
	 */
	Application(const std::string & elementId);

	/*!
	 * \brief
	 *    Деструктор класса.
	 */
	~Application();

	#pragma endregion // Constructor / Destructor

	virtual void onNodeAdded(const core::NodeIndex & nodeIndex);

	virtual void onNodeRemoved(core::NodePtr_t parent, core::NodePtr_t child);

	virtual void onNodeUpdated(const core::NodeIndex & nodeIndex);

	webcore::TreeNodeElement * getRoot();

	void start();

private:
	core::Hierarchy * _tree;
	webcore::TreeNodeElement * _root;
	webcore::TreeUpdater * _treeUpdater;
};

} // namespace sway::webapp

#endif // _SWAY_WEBAPP_APPLICATION_H
