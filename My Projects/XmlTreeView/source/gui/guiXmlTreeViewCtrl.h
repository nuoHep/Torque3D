
#ifndef _GUI_XMLTREEVIEWCTRL_H_
#define _GUI_XMLTREEVIEWCTRL_H_

#include <memory>

#include "gui\controls\guiTreeViewCtrl.h"

// helper class for the onNodeSelect callback on GuiXmlTreeViewCtrl
class GuiXmlTreeViewCtrlNode : public SimObject
{
   using Parent = SimObject;

public:
   GuiXmlTreeViewCtrlNode() : mNode(nullptr) {}
   GuiXmlTreeViewCtrlNode(const TiXmlNode *node) : mNode(node) {}

   bool empty() const { return !mNode; }
   // node info accessors, valid only when !empty()
   int type() const;
   const char* attribute(const char *name) const;
   const char* name() const;
   const char* text() const;

   DECLARE_CONOBJECT(GuiXmlTreeViewCtrlNode);

private:
   const TiXmlNode *mNode;
};

class GuiXmlTreeViewCtrl : public GuiTreeViewCtrl
{
   using Parent = GuiTreeViewCtrl;

public:
   GuiXmlTreeViewCtrl();

   bool onWake() override;
   bool onVirtualParentExpand(Item *item) override;

   static void initPersistFields();

   bool openFile(const char *path);
   bool parse(const char *text);
   void clear();
   void updateTree();
   const TiXmlNode* getItemNode(S32 itemId) const;

   DECLARE_CONOBJECT(GuiXmlTreeViewCtrl);
   DECLARE_DESCRIPTION("A control that displays a hierarchical tree view of a an xml 'file'.\n"
                       "Note that to enable expanding/collapsing of directories, the control must be\n"
                       "placed inside a GuiScrollCtrl.");

private:
   void insertXmlNode(S32 parentId, const TiXmlNode* node);
   bool hasChildren(const TiXmlNode* node) const;

   bool mShowRoot;
   bool mShowXmlComments;
   bool mShowXmlDeclarations;

   std::unique_ptr<TiXmlDocument> mXmlDoc;
   Vector<const TiXmlNode*> mXmlNodes;
};

#endif // _GUI_XMLTREEVIEWCTRL_H_
