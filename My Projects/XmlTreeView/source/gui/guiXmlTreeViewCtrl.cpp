#include "guiXmlTreeViewCtrl.h"

IMPLEMENT_CONOBJECT( GuiXmlTreeViewCtrlNode );

int GuiXmlTreeViewCtrlNode::type() const
{
   assert(mNode);
   return mNode->Type();
}

const char* GuiXmlTreeViewCtrlNode::attribute(const char *name) const
{
   assert(mNode);
   const TiXmlElement* elem = mNode->ToElement();
   return elem ? elem->Attribute(name) : "";
}

const char* GuiXmlTreeViewCtrlNode::name() const
{
   assert(mNode);
   return mNode->Type() == TiXmlNode::TINYXML_ELEMENT ? mNode->Value() : "";
}

const char* GuiXmlTreeViewCtrlNode::text() const
{
   assert(mNode);
   if (mNode->Type() == TiXmlNode::TINYXML_TEXT)
      return mNode->Value();
   if (mNode->Type() == TiXmlNode::TINYXML_ELEMENT)
      return mNode->ToElement()->GetText();
   return nullptr;
}

DefineEngineMethod( GuiXmlTreeViewCtrlNode, empty, bool, (), ,
   "@brief Returns if the node is valid." )
{
   return object->empty();
}

DefineEngineMethod( GuiXmlTreeViewCtrlNode, type, int, (), ,
   "@brief Gets the type of the current node.\n\n"
   "@return The node type (integer, see TiXmlNode::NodeType).\n" )
{
   // XXX: does torque script support enums or smth?
   return object->empty() ? TiXmlNode::TINYXML_UNKNOWN : object->type();
}

DefineEngineMethod( GuiXmlTreeViewCtrlNode, attribute, const char*, ( const char *name ), ,
   "@brief Get a string attribute.\n\n"
   "@param name Name of attribute to retrieve.\n"
   "@return The attribute string if found. Otherwise returns an empty string.\n" )
{
   return object->empty() ? "" : object->attribute(name);
}

DefineEngineMethod( GuiXmlTreeViewCtrlNode, name, const char*, (), ,
   "@brief Gets the node name.\n\n"
   "@return String containing the text in the node.\n" )
{
   return object->empty() ? "" : object->name();
}

DefineEngineMethod( GuiXmlTreeViewCtrlNode, text, const char*, (), ,
   "@brief Gets the text inside the node.\n\n"
   "@return String containing the text in the node.\n" )
{
   const char *text = object->empty() ? nullptr : object->text();
   return text ? text : "";
}

IMPLEMENT_CONOBJECT( GuiXmlTreeViewCtrl );

GuiXmlTreeViewCtrl::GuiXmlTreeViewCtrl() :
   mShowRoot(false),
   mShowXmlComments(false),
   mShowXmlDeclarations(false)
{
   // disable some stuff on GuiTreeViewCtrl which we do not support
   mSupportMouseDragging = false;
   mMultipleSelections = false;
   mDeleteObjectAllowed = false;
   mDragToItemAllowed = false;
}

bool GuiXmlTreeViewCtrl::onWake()
{
   if (!Parent::onWake())
      return false;

   updateTree();

   return true;
}

bool GuiXmlTreeViewCtrl::onVirtualParentExpand(Item *item)
{
   if (!item || !item->isExpanded())
      return true;

   const S32 itemIndex = item->getID();
   const TiXmlNode* node = getItemNode(itemIndex);
   assert(node);
   for (auto child = node->FirstChild(); child; child = child->NextSibling())
      insertXmlNode(itemIndex, child);
   item->setVirtualParent(false);

   return true;
}

void GuiXmlTreeViewCtrl::initPersistFields()
{
   addGroup("XmlTreeView");

      addField("showXmlRoot", TypeBool, Offset(mShowRoot, GuiXmlTreeViewCtrl),
               "If true, the root item of the xml document is shown in the tree.");
      addField("showXmlComments", TypeBool, Offset(mShowXmlComments, GuiXmlTreeViewCtrl),
               "If true, the xml comment nodes are shown in the tree.");
      addField("showXmlDeclarations", TypeBool, Offset(mShowXmlDeclarations, GuiXmlTreeViewCtrl),
               "If true, the xml declaration nodes are shown in the tree.");

   endGroup("XmlTreeView");

   Parent::initPersistFields();

   // hide some of the base fields which we do not support
   for (auto field : { "showRoot", "mouseDragging", "multipleSelections", "deleteObjectAllowed", "dragToItemAllowed" })
      removeField(field);
}

bool GuiXmlTreeViewCtrl::openFile(const char *path)
{
   if (mXmlDoc)
      mXmlDoc->Clear();
   else
      mXmlDoc.reset(new TiXmlDocument);

   bool res = mXmlDoc->LoadFile(path);
   if (!res)
      return res;

   updateTree();
   return true;
}

bool GuiXmlTreeViewCtrl::parse(const char *text)
{
   if (mXmlDoc)
      mXmlDoc->Clear();
   else
      mXmlDoc.reset(new TiXmlDocument);

   // XXX: can we get a status from this?
   //      documentation is unclear on what it returns exactly
   mXmlDoc->Parse(text);

   updateTree();
   return true;
}

void GuiXmlTreeViewCtrl::clear()
{
   mXmlNodes.clear();
   if (mXmlDoc)
      mXmlDoc->Clear();
   updateTree();
}

// completely rebuilds the tree view
void GuiXmlTreeViewCtrl::updateTree()
{
   _destroyTree();

   if (!mXmlDoc)
      return;

   mXmlNodes.clear();

   const TiXmlElement *root = mXmlDoc->RootElement();
   if (!root)
      return;

   if (mShowRoot)
   {
      insertXmlNode(0, root);
   }
   else
   {
      for (auto child = root->FirstChild(); child; child = child->NextSibling())
         insertXmlNode(0, child);
   }
}

// returns the xml node for the given itemId
const TiXmlNode* GuiXmlTreeViewCtrl::getItemNode(S32 itemId) const
{
   return itemId <= mXmlNodes.size() ? mXmlNodes[itemId - 1] : nullptr;
}

DefineEngineMethod( GuiXmlTreeViewCtrl, openFile, bool, ( const char* path ), ,
   "@brief Open an xml file at the given path.\n\n"
   "@param path Path to the xml file.\n"
   "@return True if it was successful, false if not.")
{
   return object->openFile(path);
}

DefineEngineMethod( GuiXmlTreeViewCtrl, parse, bool, (const char* text), ,
   "@brief Parse an xml string.\n\n"
   "@param text xml string.\n"
   "@return True if it was successful, false if not.")
{
   return object->parse(text);
}

DefineEngineMethod( GuiXmlTreeViewCtrl, clear, void, (), ,
   "@brief Clear the xml file & tree.")
{
   object->clear();
}

DefineEngineMethod( GuiXmlTreeViewCtrl, getItemNode, GuiXmlTreeViewCtrlNode*, (S32 itemId), ,
   "@brief Returns the xml node for the given itemId.\n\n"
   "@param itemId The id of the item in the tree.\n"
   "@return Proxy structuor containg info about the node.")
{
   const TiXmlNode* node = object->getItemNode(itemId);
   auto proxy = new GuiXmlTreeViewCtrlNode(node);
   proxy->registerObject();
   return proxy;
}

void GuiXmlTreeViewCtrl::insertXmlNode(S32 parentId, const TiXmlNode *node)
{
   // XXX: we can add an actual script driven filter for this
   if ((node->Type() == TiXmlNode::TINYXML_COMMENT && !mShowXmlComments) ||
       (node->Type() == TiXmlNode::TINYXML_DECLARATION && !mShowXmlDeclarations))
      return;

   // XXX: this can be made customizable by adding a name filter callback
   const char *nodeName = "";
   switch (node->Type())
   {
   case TiXmlNode::TINYXML_ELEMENT:
      nodeName = node->Value();
      break;
   case TiXmlNode::TINYXML_COMMENT:
      nodeName = "[comment]";
      break;
   case TiXmlNode::TINYXML_TEXT:
      nodeName = "[text]";
      break;
   case TiXmlNode::TINYXML_DECLARATION:
      nodeName = "[declaration]";
      break;
   case TiXmlNode::TINYXML_UNKNOWN:
   default:
      nodeName = "[unknown]";
      break;
   }
   S32 itemIndex = insertItem(parentId, nodeName, "", "", 0, 0);
   mXmlNodes.push_back(node);
   assert(itemIndex == mXmlNodes.size());

   // mark expandable if it has any "children"
   if (hasChildren(node))
   {
      Item *item = getItem(itemIndex);
      item->setVirtualParent(true);
      item->setExpanded(false);
   }
}

// determines if should mark the node expandable
bool GuiXmlTreeViewCtrl::hasChildren(const TiXmlNode *node) const
{
   int textNodes = 0;
   for (auto child = node->FirstChild(); child; child = child->NextSibling())
   {
      switch (child->Type())
      {
      case TiXmlNode::TINYXML_COMMENT:
         if (mShowXmlComments)
            return true;
         break;
      case TiXmlNode::TINYXML_DECLARATION:
         if (mShowXmlDeclarations)
            return true;
         break;
      case TiXmlNode::TINYXML_TEXT:
         textNodes++;
         break;
      default:
         return true;
      }
   }
   return textNodes > 1;
}
