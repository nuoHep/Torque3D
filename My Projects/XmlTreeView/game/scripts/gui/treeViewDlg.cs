
function TreeViewDlg::onWake( %this )
{    
    XmlTreeView.openFile("./test.xml");
}

function XmlTreeView::onSelect( %this, %itemId )
{
    %node = %this.getItemNode(%itemId);
    XmlNodeName.setText( %node.name() );
    XmlNodeText.setText( %node.text() );
}