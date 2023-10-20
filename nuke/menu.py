m = nuke.menu( 'Nodes' ).findItem( 'Deep' ) 
try:
    m.addCommand( 'DeepOpenEXRId', lambda: nuke.createNode('DeepOpenEXRId') )
except AttributeError:
    # The case with Nuke Assist.
    pass
