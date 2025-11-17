#include <sway/webapp/application.h>

namespace sway::webapp {

Application::Application(const std::string & elementId) {
	_treeUpdater = new webcore::TreeUpdater();
	_tree = new core::Hierarchy();
	_tree->attachListener(this);
	_tree->setRootNode(
		_root = new webcore::TreeNodeElement(nullptr, core::NodeIndex({ 0 }),
			elementId, webcore::TreeNodeElementCreateInfo("div", elementId))
	);
}

Application::~Application() {
	SAFE_DELETE(_root);
	SAFE_DELETE(_tree);
	SAFE_DELETE(_treeUpdater);
}

void Application::onNodeAdded(const core::NodeIndex & nodeIndex) {
	webcore::TreeNodeElement * element = (webcore::TreeNodeElement *) _tree->find(nodeIndex.getParent());
	EM_ASM({console.log("NODE_ID " + UTF8ToString($0))}, nodeIndex.toString().c_str());

	if (element)
		element->accept(_treeUpdater);

	_treeUpdater->forceUpdate();
}

void Application::onNodeRemoved(core::NodePtr_t parent, core::NodePtr_t child) {
	// Empty
}

void Application::onNodeUpdated(const core::NodeIndex & nodeIndex) {
	webcore::TreeNodeElement * element = (webcore::TreeNodeElement *) _tree->find(nodeIndex.getParent());
	EM_ASM({console.log("NODE_UPDATE_ID " + UTF8ToString($0))}, nodeIndex.toString().c_str());

	if (element)
		element->accept(_treeUpdater);

	_treeUpdater->forceUpdate();
}

webcore::TreeNodeElement * Application::getRoot() {
	return _root;
}

} // namespace sway::webapp

#include <sway/webcore/mvc/itemmodel.h>
#include <sway/webcore/mvc/collectionmodel.h>
#include <sway/webcore/router/router.h>

#include <sway/webcore.h>
#include <sway/webcore/treeupdater.h>
#include <sway/webcore/region.h>
#include <sway/webcore/treenodeelement.h>
#include <sway/webcore/mvc/itemview.h>
#include <sway/webcore/mvc/collectionview.h>
#include <sway/webcore/mvc/view/advanced/stackview.h>
#include <sway/webcore/mvc/controller.h>
#include <sway/webui.h>

using namespace sway;

EMSCRIPTEN_BINDINGS(vector) {
	emscripten::register_vector<emscripten::val>("VectorVal");
	emscripten::register_vector<std::string>("VectorString");
	emscripten::register_vector<int>("VectorInt");
	emscripten::register_vector<webcore::SelectorSmartPtr_t>("SelectorSmartPtr_t");
	emscripten::register_vector<webcore::CnSelectorDescriptor>("VectorCnSelectorDescriptor");
} // vector

EMSCRIPTEN_BINDINGS(event_listener) {
	emscripten::class_<webcore::EventListener>("EventListener")
		.function<void>("handleEvent", &webcore::EventListener::handleEvent);
} // event_listener

EMSCRIPTEN_BINDINGS(models) {
	emscripten::class_<core::utilities::Observable>("Observable")
		.constructor()
		.function("registerObserver", &core::utilities::Observable::registerObserver, emscripten::allow_raw_pointers())
		.function("notify", &core::utilities::Observable::notify);

	webcore::mvc::AItemModel::registerEmscriptenClass("AItemModel");
	webcore::mvc::ACollectionModel::registerEmscriptenClass("ACollectionModel");
} // models

EMSCRIPTEN_BINDINGS(views) {
	emscripten::class_<core::HierarchyListener>("HierarchyListener")
		.function("onNodeAdded", &core::HierarchyListener::onNodeAdded, emscripten::allow_raw_pointers())
		.function("onNodeRemoved", &core::HierarchyListener::onNodeRemoved, emscripten::allow_raw_pointers());

	emscripten::class_<core::Hierarchy>("Hierarchy")
		.constructor()
		.function("attachListener", &core::Hierarchy::attachListener, emscripten::allow_raw_pointers())
		.function("detachListener", &core::Hierarchy::detachListener, emscripten::allow_raw_pointers())
		.function("getRootNode", &core::Hierarchy::getRootNode, emscripten::allow_raw_pointers())
		.function("setRootNode", &core::Hierarchy::setRootNode, emscripten::allow_raw_pointers())
		.function("getListeners", &core::Hierarchy::getListeners, emscripten::allow_raw_pointers());

	emscripten::class_<core::NodeIndex>("NodeIndex")
		.constructor<std::vector<s32_t>>()
		.constructor<core::NodeIndex, s32_t>()
		.function("getParent", &core::NodeIndex::getParent, emscripten::allow_raw_pointers())
		.function("getDepth", &core::NodeIndex::getDepth)
		.function("isValid", &core::NodeIndex::isValid)
		.function("toString", &core::NodeIndex::toString);

	emscripten::class_<core::Node>("Node")
		.constructor<core::NodePtr_t, core::NodeIndex, std::string>()
		.function("addChild", &core::Node::addChild, emscripten::allow_raw_pointers())
		.function("findChild", &core::Node::findChild, emscripten::allow_raw_pointers())
		.function("getChild", &core::Node::getChild, emscripten::allow_raw_pointers())
		.function("hasChild", &core::Node::hasChild)
		.function("getParentNode", &core::Node::getParentNode, emscripten::allow_raw_pointers())
		.function("setParentNode", &core::Node::setParentNode, emscripten::allow_raw_pointers())
		.function("getNodeId", &core::Node::getNodeId)
		.function("setNodeId", &core::Node::setNodeId);

	emscripten::class_<webcore::ITreeVisitor>("ITreeVisitor")
		.function("visitOnEnter", &webcore::ITreeVisitor::visitOnEnter, emscripten::allow_raw_pointers())
		.function("visitOnLeave", &webcore::ITreeVisitor::visitOnLeave, emscripten::allow_raw_pointers());

	emscripten::class_<webcore::TreeUpdater, emscripten::base<webcore::ITreeVisitor>>("TreeUpdater")
		.constructor()
		.function("forceUpdate", &webcore::TreeUpdater::forceUpdate);

	emscripten::value_object<webcore::CnSelectorChain>("CnSelectorChain")
		.field("block", &webcore::CnSelectorChain::block)
		.field("elem", &webcore::CnSelectorChain::elem)
		.field("mods", &webcore::CnSelectorChain::mods);

	emscripten::value_object<webcore::CnSelectorDescriptor>("CnSelectorDescriptor")
		//.field("wrap", &webcore::CnSelectorDescriptor::wrap)
		.field("glob", &webcore::CnSelectorDescriptor::glob)
		.field("chain", &webcore::CnSelectorDescriptor::chain);

	emscripten::value_object<webcore::TreeNodeElementCreateInfo>("TreeNodeElementCreateInfo")
		.field("tagname", &webcore::TreeNodeElementCreateInfo::tagname)
		.field("id", &webcore::TreeNodeElementCreateInfo::id)
		.field("stylesheet", &webcore::TreeNodeElementCreateInfo::stylesheet)
		.field("classes", &webcore::TreeNodeElementCreateInfo::classes);

	emscripten::value_object<webcore::RegionCreateInfo>("RegionCreateInfo")
		.field("id", &webcore::RegionCreateInfo::id)
		.field("replace", &webcore::RegionCreateInfo::replace);

	webcore::Region::registerEmscriptenClass("Region");
	webcore::TreeNodeElement::registerEmscriptenClass("TreeNodeElement");
	webcore::StyleSheet::registerEmscriptenClass("StyleSheet");
	webcore::Selector::registerEmscriptenClass("Selector");
	webcore::IdSelector::registerEmscriptenClass("IdSelector");
	webcore::CnSelector::registerEmscriptenClass("CnSelector");
	webcore::mvc::AView::registerEmscriptenClass("AView");
	webcore::mvc::AItemView::registerEmscriptenClass("AItemView");
	webcore::mvc::ACollectionView::registerEmscriptenClass("ACollectionView");
	webcore::mvc::view::advanced::StackView::registerEmscriptenClass("StackView");
	webui::control::Label::registerEmscriptenClass("Label");
	webui::control::List::registerEmscriptenClass("List");
} // views

EMSCRIPTEN_BINDINGS(controllers) {
	emscripten::class_<webcore::mvc::AController>("AController")
		.constructor<webcore::mvc::AItemModel *>()
		.function("getModel", &webcore::mvc::AController::getModel, emscripten::allow_raw_pointers());
} // controllers

EMSCRIPTEN_BINDINGS(router) {
	webcore::router::Router::registerEmscriptenClass("Router");
} // router

EMSCRIPTEN_BINDINGS(application) {
	emscripten::class_<webapp::Application>("Application")
		.constructor<std::string>()
		.function("getRoot", &webapp::Application::getRoot, emscripten::allow_raw_pointers());
} // application

int main() {
	printf("Starting...\n");
}
