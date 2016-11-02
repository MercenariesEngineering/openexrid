m = nuke.menu( 'Nodes' ).findItem( 'Deep' )
m.addCommand( 'DeepOpenEXRId', lambda: nuke.createNode('DeepOpenEXRId') )
