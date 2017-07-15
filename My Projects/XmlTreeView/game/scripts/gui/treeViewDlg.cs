
function TreeViewDlg::onWake( %this )
{    
    XmlTreeView.openFile("./test.xml");
}

function XmlTreeView::onSelect( %this, %itemId )
{
    %node = %this.selectedNode();
    XmlNodeName.setText( %node.name() );
    XmlNodeText.setText( %node.text() );
}