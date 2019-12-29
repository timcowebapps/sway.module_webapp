#include <sway/webapp/application.h>

NAMESPACE_BEGIN(sway)
NAMESPACE_BEGIN(webapp)

Application::Application(const std::string & elementId) {
	_treeUpdater = new webcore::base::TreeUpdater();
	_tree = new core::containers::Hierarchy();
	_tree->attachListener(this);
	_tree->setRootNode(
		_root = new webcore::view::AItemView(nullptr, core::containers::HierarchyNodeIndex({ 0 }), elementId,
			(webcore::base::TreeNodeElementCreateInfo) {
				.tagname = "div",
				.id = elementId
			}
		)
	);
}

Application::~Application() {
	SAFE_DELETE(_root);
	SAFE_DELETE(_tree);
	SAFE_DELETE(_treeUpdater);
}

void Application::onNodeAdded(const core::containers::HierarchyNodeIndex & nodeIndex) {
	webcore::base::TreeNodeElement * element = (webcore::base::TreeNodeElement *) _tree->find(nodeIndex.getParent());
	EM_ASM({console.log("NODE_ID " + UTF8ToString($0))}, nodeIndex.toString().c_str());

	if (element)
		element->accept(_treeUpdater);

	_treeUpdater->forceUpdate();
}

void Application::onNodeRemoved(core::containers::HierarchyNodePtr_t parent, core::containers::HierarchyNodePtr_t child) {
	// Empty
}

void Application::onNodeUpdated(const core::containers::HierarchyNodeIndex & nodeIndex) {
	webcore::base::TreeNodeElement * element = (webcore::base::TreeNodeElement *) _tree->find(nodeIndex.getParent());
	EM_ASM({console.log("NODE_UPDATE_ID " + UTF8ToString($0))}, nodeIndex.toString().c_str());

	if (element)
		element->accept(_treeUpdater);

	_treeUpdater->forceUpdate();
}

webcore::base::TreeNodeElement * Application::getRoot() {
	return _root;
}

NAMESPACE_END(webapp)
NAMESPACE_END(sway)

#include <sway/webcore/model/abstractitemmodel.h>
#include <sway/webcore/model/abstractitemcollection.h>
#include <sway/webcore/router/router.h>

#include <sway/webcore.h>
#include <sway/webcore/base/treeupdater.h>
#include <sway/webcore/base/region.h>
#include <sway/webcore/base/treenodeelement.h>
#include <sway/webcore/view/itemview.h>
#include <sway/webcore/view/itemcollectionview.h>
#include <sway/webcore/view/advanced/stackview.h>
#include <sway/webcore/control/layout.h>
#include <sway/webcore/control/label.h>

using namespace sway;
using namespace sway::webcore;

EMSCRIPTEN_BINDINGS(vector) {
	emscripten::register_vector<emscripten::val>("VectorVal");
	emscripten::register_vector<std::string>("VectorString");
	emscripten::register_vector<int>("VectorInt");
} // vector

EMSCRIPTEN_BINDINGS(event_types) {
	emscripten::enum_<EventTypes_t>("EventTypes_t")
		.value("kClick", EventTypes_t::kClick)
		.value("kMouseOver", EventTypes_t::kMouseOver)
		.value("kMouseOut", EventTypes_t::kMouseOut);
} // event_types

EMSCRIPTEN_BINDINGS(event_listener) {
	emscripten::class_<EventListener>("EventListener")
		.function<void>("handleEvent", &EventListener::handleEvent);
} // event_listener

EMSCRIPTEN_BINDINGS(models) {
	emscripten::class_<core::utilities::Observable>("Observable")
		.constructor()
		.function("registerObserver", &core::utilities::Observable::registerObserver, emscripten::allow_raw_pointers())
		.function("notify", &core::utilities::Observable::notify);

	model::AbstractItemModel::registerEmscriptenClass("AbstractItemModel");
	model::AbstractItemCollection::registerEmscriptenClass("AbstractItemCollection");
} // models

EMSCRIPTEN_BINDINGS(views) {
	emscripten::class_<core::containers::HierarchyListener>("HierarchyListener")
		.function("onNodeAdded", &core::containers::HierarchyListener::onNodeAdded, emscripten::allow_raw_pointers())
		.function("onNodeRemoved", &core::containers::HierarchyListener::onNodeRemoved, emscripten::allow_raw_pointers());

	emscripten::class_<core::containers::Hierarchy>("Hierarchy")
		.constructor()
		.function("attachListener", &core::containers::Hierarchy::attachListener, emscripten::allow_raw_pointers())
		.function("detachListener", &core::containers::Hierarchy::detachListener, emscripten::allow_raw_pointers())
		.function("getRootNode", &core::containers::Hierarchy::getRootNode, emscripten::allow_raw_pointers())
		.function("setRootNode", &core::containers::Hierarchy::setRootNode, emscripten::allow_raw_pointers())
		.function("getListeners", &core::containers::Hierarchy::getListeners, emscripten::allow_raw_pointers());

	emscripten::class_<core::containers::HierarchyNodeIndex>("HierarchyNodeIndex")
		.constructor<std::vector<s32_t>>()
		.constructor<core::containers::HierarchyNodeIndex, s32_t>()
		.function("getParent", &core::containers::HierarchyNodeIndex::getParent, emscripten::allow_raw_pointers())
		.function("getDepth", &core::containers::HierarchyNodeIndex::getDepth)
		.function("isValid", &core::containers::HierarchyNodeIndex::isValid)
		.function("toString", &core::containers::HierarchyNodeIndex::toString);

	emscripten::class_<core::containers::HierarchyNode>("HierarchyNode")
		.constructor<core::containers::HierarchyNodePtr_t, core::containers::HierarchyNodeIndex, std::string>()
		.function("addChild", &core::containers::HierarchyNode::addChild, emscripten::allow_raw_pointers())
		.function("findChild", &core::containers::HierarchyNode::findChild, emscripten::allow_raw_pointers())
		.function("getChild", &core::containers::HierarchyNode::getChild, emscripten::allow_raw_pointers())
		.function("hasChild", &core::containers::HierarchyNode::hasChild)
		.function("getParentNode", &core::containers::HierarchyNode::getParentNode, emscripten::allow_raw_pointers())
		.function("setParentNode", &core::containers::HierarchyNode::setParentNode, emscripten::allow_raw_pointers())
		.function("getNodeId", &core::containers::HierarchyNode::getNodeId)
		.function("setNodeId", &core::containers::HierarchyNode::setNodeId);

	emscripten::class_<base::ITreeVisitor>("ITreeVisitor")
		.function("visitOnEnter", &base::ITreeVisitor::visitOnEnter, emscripten::allow_raw_pointers())
		.function("visitOnLeave", &base::ITreeVisitor::visitOnLeave, emscripten::allow_raw_pointers());

	emscripten::class_<base::TreeUpdater, emscripten::base<base::ITreeVisitor>>("TreeUpdater")
		.constructor()
		.function("forceUpdate", &base::TreeUpdater::forceUpdate);

	emscripten::value_object<base::TreeNodeElementCreateInfo>("TreeNodeElementCreateInfo")
		.field("tagname", &base::TreeNodeElementCreateInfo::tagname)
		.field("id", &base::TreeNodeElementCreateInfo::id);

	emscripten::value_object<base::RegionCreateInfo>("RegionCreateInfo")
		.field("id", &base::RegionCreateInfo::id)
		.field("replace", &base::RegionCreateInfo::replace);

	base::RegionMixin::registerEmscriptenClass("RegionMixin");
	base::TreeNodeElement::registerEmscriptenClass("TreeNodeElement");
	view::AItemView::registerEmscriptenClass("AItemView");
	view::AItemCollectionView::registerEmscriptenClass("AItemCollectionView");
	view::advanced::StackView::registerEmscriptenClass("StackView");
	control::Layout::registerEmscriptenClass("Layout");
	control::Label::registerEmscriptenClass("Label");
} // views

EMSCRIPTEN_BINDINGS(controllers) {
	emscripten::class_<controller::AbstractController>("AbstractController")
		.constructor<model::AbstractItemModel *>()
		.function("getModel", &controller::AbstractController::getModel, emscripten::allow_raw_pointers());
} // controllers

EMSCRIPTEN_BINDINGS(router) {
	router::Router::registerEmscriptenClass("Router");
} // router

EMSCRIPTEN_BINDINGS(application) {
	emscripten::class_<webapp::Application>("Application")
		.constructor<std::string>()
		.function("getRoot", &webapp::Application::getRoot, emscripten::allow_raw_pointers());
} // application

int main() {
	printf("Starting...\n");
}