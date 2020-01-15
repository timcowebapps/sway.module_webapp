#include <sway/webapp/application.h>

NAMESPACE_BEGIN(sway)
NAMESPACE_BEGIN(webapp)

Application::Application(const std::string & elementId) {
	_treeUpdater = new webcore::TreeUpdater();
	_tree = new core::containers::Hierarchy();
	_tree->attachListener(this);
	_tree->setRootNode(
		_root = new webcore::TreeNodeElement(nullptr, core::containers::HierarchyNodeIndex({ 0 }),
			elementId, webcore::TreeNodeElementCreateInfo("div", elementId))
	);
}

Application::~Application() {
	SAFE_DELETE(_root);
	SAFE_DELETE(_tree);
	SAFE_DELETE(_treeUpdater);
}

void Application::onNodeAdded(const core::containers::HierarchyNodeIndex & nodeIndex) {
	webcore::TreeNodeElement * element = (webcore::TreeNodeElement *) _tree->find(nodeIndex.getParent());
	EM_ASM({console.log("NODE_ID " + UTF8ToString($0))}, nodeIndex.toString().c_str());

	if (element)
		element->accept(_treeUpdater);

	_treeUpdater->forceUpdate();
}

void Application::onNodeRemoved(core::containers::HierarchyNodePtr_t parent, core::containers::HierarchyNodePtr_t child) {
	// Empty
}

void Application::onNodeUpdated(const core::containers::HierarchyNodeIndex & nodeIndex) {
	webcore::TreeNodeElement * element = (webcore::TreeNodeElement *) _tree->find(nodeIndex.getParent());
	EM_ASM({console.log("NODE_UPDATE_ID " + UTF8ToString($0))}, nodeIndex.toString().c_str());

	if (element)
		element->accept(_treeUpdater);

	_treeUpdater->forceUpdate();
}

webcore::TreeNodeElement * Application::getRoot() {
	return _root;
}

NAMESPACE_END(webapp)
NAMESPACE_END(sway)

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
	emscripten::register_vector<webcore::css::SelectorSmartPtr_t>("SelectorSmartPtr_t");
	emscripten::register_vector<webcore::css::CnSelectorDescriptor>("VectorCnSelectorDescriptor");
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

	emscripten::class_<webcore::ITreeVisitor>("ITreeVisitor")
		.function("visitOnEnter", &webcore::ITreeVisitor::visitOnEnter, emscripten::allow_raw_pointers())
		.function("visitOnLeave", &webcore::ITreeVisitor::visitOnLeave, emscripten::allow_raw_pointers());

	emscripten::class_<webcore::TreeUpdater, emscripten::base<webcore::ITreeVisitor>>("TreeUpdater")
		.constructor()
		.function("forceUpdate", &webcore::TreeUpdater::forceUpdate);

	emscripten::value_object<webcore::css::CnSelectorChain>("CnSelectorChain")
		.field("block", &webcore::css::CnSelectorChain::block)
		.field("elem", &webcore::css::CnSelectorChain::elem)
		.field("mods", &webcore::css::CnSelectorChain::mods);

	emscripten::value_object<webcore::css::CnSelectorDescriptor>("CnSelectorDescriptor")
		//.field("wrap", &webcore::css::CnSelectorDescriptor::wrap)
		.field("glob", &webcore::css::CnSelectorDescriptor::glob)
		.field("chain", &webcore::css::CnSelectorDescriptor::chain);

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
	webcore::css::StyleSheet::registerEmscriptenClass("StyleSheet");
	webcore::css::Selector::registerEmscriptenClass("Selector");
	webcore::css::IdSelector::registerEmscriptenClass("IdSelector");
	webcore::css::CnSelector::registerEmscriptenClass("CnSelector");
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